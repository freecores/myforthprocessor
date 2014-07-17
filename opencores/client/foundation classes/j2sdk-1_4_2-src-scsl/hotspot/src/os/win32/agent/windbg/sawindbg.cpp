#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)sawindbg.cpp	1.5 03/01/23 11:07:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// this is source code windbg based SA debugger agent to debug
// Dr. Watson dump files.

#include "sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal.h"
#include <limits.h>
#include <windows.h>

// the following definitions are required by dbgeng.h, but not
// found in standard headers.

typedef EXCEPTION_RECORD EXCEPTION_RECORD64;
typedef EXCEPTION_RECORD64* PEXCEPTION_RECORD64;

#ifndef STDMETHODV
#define STDMETHODV(method) virtual HRESULT STDMETHODVCALLTYPE method
#endif

#define DEBUG_NO_IMPLEMENTATION
#include <dbgeng.h>

// simple template to manage array delete across early (error) returns

template <class T> 
class AutoArrayPtr {
      T* m_ptr;
   public:
      AutoArrayPtr(T* ptr) : m_ptr(ptr) {
      }

      ~AutoArrayPtr() { 
         delete [] m_ptr;
      }

      T* asPtr() {
         return m_ptr;
      }
};

// x86 registers names array. the elements should be in the same order as
// defined in X86ThreadContext.java. FIXME: can we use javah to auto-generate
// here?

// X86ThreadContext.NPRGREG = 25

const int X86_NPRGREG = 25;

// some registers (for eg. trapno) does not work with windbg
// I've commented out and have only basic registers and
// segment registers.

const char* x86RegNames[X86_NPRGREG] = {
   "gs", 
   "fs",
   "es",
   "ds", 
   "edi", 
   "esi", 
   "ebp", 
   "esp",
   "ebx", 
   "edx", 
   "ecx", 
   "eax", 
   NULL /* trapno */, 
   NULL /* err    */, 
   "eip",
   "cs",
   NULL /* efl    */,
   NULL /* uesp   */, 
   "ss",
   NULL /* dr0    */,
   NULL /* dr1    */,
   NULL /* dr2    */,
   NULL /* dr3    */,
   NULL /* dr6    */,
   NULL /* dr7    */,
};

// field and method IDs we want here
 
static jfieldID imagePath_ID                    = 0;
static jfieldID ptrIDebugClient_ID              = 0;
static jfieldID ptrIDebugControl_ID             = 0;
static jfieldID ptrIDebugDataSpaces_ID          = 0;
static jfieldID ptrIDebugOutputCallbacks_ID     = 0;
static jfieldID ptrIDebugRegisters_ID           = 0;
static jfieldID ptrIDebugSymbols_ID             = 0;
static jfieldID ptrIDebugSystemObjects_ID       = 0;

static jmethodID addLoadObject_ID               = 0;
static jmethodID addThread_ID                   = 0;
static jmethodID setThreadIntegerRegisterSet_ID = 0;

#define CHECK_EXCEPTION_(value) if(env->ExceptionOccurred()) { return value; }
#define CHECK_EXCEPTION if(env->ExceptionOccurred()) { return;}

#define THROW_NEW_DEBUGGER_EXCEPTION_(str, value) { \
                          throwNewDebuggerException(env, str); return value; }

#define THROW_NEW_DEBUGGER_EXCEPTION(str) { throwNewDebuggerException(env, str); \
 return;}

static void throwNewDebuggerException(JNIEnv* env, const char* errMsg) {
  env->ThrowNew(env->FindClass("sun/jvm/hotspot/debugger/DebuggerException"), errMsg);
}

/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_initIDs
  (JNIEnv *env, jclass clazz) {
  imagePath_ID = env->GetStaticFieldID(clazz, "imagePath", "Ljava/lang/String;");
  CHECK_EXCEPTION;

  ptrIDebugClient_ID = env->GetFieldID(clazz, "ptrIDebugClient", "J");
  CHECK_EXCEPTION;

  ptrIDebugControl_ID = env->GetFieldID(clazz, "ptrIDebugControl", "J");
  CHECK_EXCEPTION;

  ptrIDebugDataSpaces_ID = env->GetFieldID(clazz, "ptrIDebugDataSpaces", "J");
  CHECK_EXCEPTION;

  ptrIDebugOutputCallbacks_ID = env->GetFieldID(clazz, 
                                            "ptrIDebugOutputCallbacks", "J");
  CHECK_EXCEPTION;

  ptrIDebugRegisters_ID = env->GetFieldID(clazz, "ptrIDebugRegisters", "J");
  CHECK_EXCEPTION;

  ptrIDebugSymbols_ID = env->GetFieldID(clazz, 
                                         "ptrIDebugSymbols", "J");
  CHECK_EXCEPTION;

  ptrIDebugSystemObjects_ID = env->GetFieldID(clazz, 
                                         "ptrIDebugSystemObjects", "J");
  CHECK_EXCEPTION;

  addLoadObject_ID = env->GetMethodID(clazz, "addLoadObject", 
                                         "(Ljava/lang/String;JJ)V");
  CHECK_EXCEPTION;

  addThread_ID = env->GetMethodID(clazz, "addThread", "(J)V");
  CHECK_EXCEPTION;

  setThreadIntegerRegisterSet_ID = env->GetMethodID(clazz, 
                                         "setThreadIntegerRegisterSet", "(J[J)V");
  CHECK_EXCEPTION;

}

// class for IDebugOutputCallbacks

class SAOutputCallbacks : public IDebugOutputCallbacks {
   LONG  m_refCount;
   char* m_msgBuffer;

   public:
      SAOutputCallbacks() : m_refCount(0), m_msgBuffer(0) {
      }

      ~SAOutputCallbacks() {
         clearBuffer();
      }

      const char* getBuffer() const {
         return m_msgBuffer;
      }

      void clearBuffer() {
         if (m_msgBuffer) {
            free(m_msgBuffer);
            m_msgBuffer = 0;
         }
      }

      STDMETHOD_(ULONG, AddRef)(THIS);
      STDMETHOD_(ULONG, Release)(THIS);
      STDMETHOD(QueryInterface)(THIS_
                                IN REFIID interfaceId,
                                OUT PVOID* ppInterface);
      STDMETHOD(Output)(THIS_
                        IN ULONG mask,
                        IN PCSTR msg);
};

STDMETHODIMP_(ULONG) SAOutputCallbacks::AddRef(THIS) {
   InterlockedIncrement(&m_refCount);
   return m_refCount;
}

STDMETHODIMP_(ULONG) SAOutputCallbacks::Release(THIS) {
   LONG retVal;
   InterlockedDecrement(&m_refCount);
   retVal = m_refCount;
   if (retVal == 0) {
      delete this;
   }
   return retVal;
}

STDMETHODIMP SAOutputCallbacks::QueryInterface(THIS_
                                          IN REFIID interfaceId,
                                          OUT PVOID* ppInterface) {
   *ppInterface = 0;
   HRESULT res = E_NOINTERFACE;
   if (TRUE == IsEqualIID(interfaceId, __uuidof(IUnknown)) ||
       TRUE == IsEqualIID(interfaceId, __uuidof(IDebugOutputCallbacks))) {
      *ppInterface = (IDebugOutputCallbacks*) this;
      AddRef();
      res = S_OK;
   }
   return res;
}

STDMETHODIMP SAOutputCallbacks::Output(THIS_
                                       IN ULONG mask,
                                       IN PCSTR msg) {
   int len = strlen(msg) + 1;
   if (m_msgBuffer == 0) {
      m_msgBuffer = (char*) malloc(len);
      if (m_msgBuffer == 0) {
         fprintf(stderr, "out of memory debugger output!\n");
         return S_FALSE;
      }
      strcpy(m_msgBuffer, msg);
   } else {
      m_msgBuffer = (char*) realloc(m_msgBuffer, len + strlen(m_msgBuffer));
      if (m_msgBuffer == 0) {
         fprintf(stderr, "out of memory debugger output!\n");
         return S_FALSE;
      }
      strcat(m_msgBuffer, msg);
   }
   return S_OK;
}

static bool getWindbgInterfaces(JNIEnv* env, jobject obj) {
  // get windbg interfaces ..

  IDebugClient* ptrIDebugClient = 0;
  if (DebugCreate(__uuidof(IDebugClient), (PVOID*) &ptrIDebugClient) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to create IDebugClient object!", false);
  }
  env->SetLongField(obj, ptrIDebugClient_ID, (jlong) ptrIDebugClient);

  IDebugControl* ptrIDebugControl = 0;
  if (ptrIDebugClient->QueryInterface(__uuidof(IDebugControl), (PVOID*) &ptrIDebugControl)
     != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to get IDebugControl", false);
  }
  env->SetLongField(obj, ptrIDebugControl_ID, (jlong) ptrIDebugControl);

  IDebugDataSpaces* ptrIDebugDataSpaces = 0;
  if (ptrIDebugClient->QueryInterface(__uuidof(IDebugDataSpaces), (PVOID*) &ptrIDebugDataSpaces)
     != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to get IDebugDataSpaces object!", false);
  }
  env->SetLongField(obj, ptrIDebugDataSpaces_ID, (jlong) ptrIDebugDataSpaces);

  SAOutputCallbacks* ptrIDebugOutputCallbacks = new SAOutputCallbacks();
  ptrIDebugOutputCallbacks->AddRef();
  env->SetLongField(obj, ptrIDebugOutputCallbacks_ID, (jlong) ptrIDebugOutputCallbacks);
  CHECK_EXCEPTION_(false);

  IDebugRegisters* ptrIDebugRegisters = 0;
  if (ptrIDebugClient->QueryInterface(__uuidof(IDebugRegisters), (PVOID*) &ptrIDebugRegisters)
     != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to get IDebugRegisters object!", false);
  } 
  env->SetLongField(obj, ptrIDebugRegisters_ID, (jlong) ptrIDebugRegisters);

  IDebugSymbols* ptrIDebugSymbols = 0;
  if (ptrIDebugClient->QueryInterface(__uuidof(IDebugSymbols), (PVOID*) &ptrIDebugSymbols)
     != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to get IDebugSymbols object!", false);
  } 
  env->SetLongField(obj, ptrIDebugSymbols_ID, (jlong) ptrIDebugSymbols);

  IDebugSystemObjects* ptrIDebugSystemObjects = 0;
  if (ptrIDebugClient->QueryInterface(__uuidof(IDebugSystemObjects), (PVOID*) &ptrIDebugSystemObjects)
     != S_OK) { 
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: not able to get IDebugSystemObjects object!", false);
  }
  env->SetLongField(obj, ptrIDebugSystemObjects_ID, (jlong) ptrIDebugSystemObjects);

  return true;
}

static bool openDumpFile(JNIEnv* env, jobject obj, jstring coreFileName) {
  // open the dump file
  jboolean isCopy;
  const char* dumpFile_cstr = env->GetStringUTFChars(coreFileName, &isCopy);
  CHECK_EXCEPTION_(false);

  jclass clazz = env->GetObjectClass(obj);
  jstring path = (jstring) env->GetStaticObjectField(clazz, imagePath_ID);
  const char* imagePath_cstr = env->GetStringUTFChars(path, &isCopy);
  CHECK_EXCEPTION_(false);

  IDebugSymbols* ptrIDebugSymbols = (IDebugSymbols*) env->GetLongField(obj, 
                                                      ptrIDebugSymbols_ID);
  CHECK_EXCEPTION_(false);

  ptrIDebugSymbols->SetImagePath(imagePath_cstr);
  ptrIDebugSymbols->SetSymbolPath(imagePath_cstr);

  env->ReleaseStringUTFChars(path, imagePath_cstr);
  CHECK_EXCEPTION_(false);

  IDebugClient* ptrIDebugClient = (IDebugClient*) env->GetLongField(obj, 
                                                      ptrIDebugClient_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugClient->OpenDumpFile(dumpFile_cstr) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: OpenDumpFile failed!", false);
  }

  IDebugControl* ptrIDebugControl = (IDebugControl*) env->GetLongField(obj, 
                                                     ptrIDebugControl_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: WaitForEvent failed!", false);
  }

  env->ReleaseStringUTFChars(coreFileName, dumpFile_cstr);
  CHECK_EXCEPTION_(false);

  return true;
}

static bool addLoadObjects(JNIEnv* env, jobject obj) {
  IDebugSymbols* ptrIDebugSymbols = (IDebugSymbols*) env->GetLongField(obj, 
                                                      ptrIDebugSymbols_ID);
  CHECK_EXCEPTION_(false);
  ULONG loaded = 0, unloaded = 0;
  if (ptrIDebugSymbols->GetNumberModules(&loaded, &unloaded) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetNumberModules failed!", false);
  }

  AutoArrayPtr<DEBUG_MODULE_PARAMETERS> params(new DEBUG_MODULE_PARAMETERS[loaded]);

  if (params.asPtr() == 0) {
      THROW_NEW_DEBUGGER_EXCEPTION_("out of memory to allocate debug module params!", false);
  }

  if (ptrIDebugSymbols->GetModuleParameters(loaded, 0, NULL, params.asPtr()) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetModuleParameters failed!", false);
  }

  for (int u = 0; u < (int)loaded; u++) {
     TCHAR imageName[MAX_PATH];
     if (ptrIDebugSymbols->GetModuleNames(DEBUG_ANY_ID, params.asPtr()[u].Base,
                                      imageName, MAX_PATH, NULL, NULL,
                                      0, NULL, NULL, 0, NULL) != S_OK) {
        THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetModuleNames failed!", false);
     }

     jstring strName = env->NewStringUTF(imageName);
     CHECK_EXCEPTION_(false);
     env->CallVoidMethod(obj, addLoadObject_ID, strName, (jlong) params.asPtr()[u].Size,
                               (jlong) params.asPtr()[u].Base);
     CHECK_EXCEPTION_(false);
  }

  return true;
}

static bool addThreads(JNIEnv* env, jobject obj) {
  IDebugSystemObjects* ptrIDebugSystemObjects = (IDebugSystemObjects*) env->GetLongField(obj, 
                                                      ptrIDebugSystemObjects_ID);
  CHECK_EXCEPTION_(false);

  ULONG numThreads = 0;
  if (ptrIDebugSystemObjects->GetNumberThreads(&numThreads) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetNumberThreads failed!", false);
  }

  AutoArrayPtr<ULONG> ptrSysThreadIds = new ULONG[numThreads];

  if (ptrSysThreadIds.asPtr() == 0) {
     THROW_NEW_DEBUGGER_EXCEPTION_("out of memory to allocate thread ids!", false);
  }

  AutoArrayPtr<ULONG> ptrThreadIds = new ULONG[numThreads];

  if (ptrThreadIds.asPtr() == 0) {
     THROW_NEW_DEBUGGER_EXCEPTION_("out of memory to allocate thread ids!", false);
  }

  if (ptrIDebugSystemObjects->GetThreadIdsByIndex(0, numThreads, 
                                      ptrThreadIds.asPtr(), ptrSysThreadIds.asPtr()) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetThreadIdsByIndex failed!", false);
  }


  IDebugRegisters* ptrIDebugRegisters = (IDebugRegisters*) env->GetLongField(obj, 
                                                      ptrIDebugRegisters_ID);
  CHECK_EXCEPTION_(false);

  ULONG regIndices[X86_NPRGREG];
  ULONG regNum = 0;
  for (regNum = 0; regNum < sizeof(regIndices) / sizeof(ULONG); regNum++) {
     if (x86RegNames[regNum] == NULL) {
        continue;
     }

     if (ptrIDebugRegisters->GetIndexByName(x86RegNames[regNum], &regIndices[regNum])
         != S_OK) {
       THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetIndexByName (register) failed!", false);
     }
  }

  // for each thread, get register context and save it.
  for (ULONG t = 0; t < numThreads; t++) {
     if (ptrIDebugSystemObjects->SetCurrentThreadId(ptrThreadIds.asPtr()[t]) != S_OK) {
        THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: SetCurrentThread failed!", false);
     }

     jlongArray regs = env->NewLongArray(X86_NPRGREG);
     CHECK_EXCEPTION_(false);

     jboolean isCopy = JNI_FALSE;
     jlong* ptrRegs = env->GetLongArrayElements(regs, &isCopy);
     CHECK_EXCEPTION_(false);

     // get each register value
     for (regNum = 0; regNum < sizeof(regIndices) / sizeof(ULONG); regNum++) {
        if (x86RegNames[regNum] == NULL) {
           continue;
        }

        DEBUG_VALUE debugValue;
        if (ptrIDebugRegisters->GetValue(regIndices[regNum], &debugValue) != S_OK) {
           THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetValue (register) failed!", false);
        }
        ptrRegs[regNum] = (jlong) debugValue.I32;
     }

     env->ReleaseLongArrayElements(regs, ptrRegs, JNI_COMMIT);
     CHECK_EXCEPTION_(false);

     env->CallVoidMethod(obj, setThreadIntegerRegisterSet_ID, 
                        (jlong) ptrThreadIds.asPtr()[t], regs);
     CHECK_EXCEPTION_(false);

     ULONG sysId;
     if (ptrIDebugSystemObjects->GetCurrentThreadSystemId(&sysId) != S_OK) {
        THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetCurrentThreadSystemId failed!", false);
     }

     env->CallVoidMethod(obj, addThread_ID, (jlong) sysId);
     CHECK_EXCEPTION_(false);
  }

  return true;
}

/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    attach0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */

JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_attach0
  (JNIEnv *env, jobject obj, jstring execName, jstring coreFileName) {

  if (getWindbgInterfaces(env, obj) == false) {
     return;
  }

  if (openDumpFile(env, obj, coreFileName) == false) {
     return;
  }
  
  if (addLoadObjects(env, obj) == false) {
     return;
  }

  if (addThreads(env, obj) == false) {
     return;
  }
}

static bool releaseWindbgInterfaces(JNIEnv* env, jobject obj) {
  IDebugDataSpaces* ptrIDebugDataSpaces = (IDebugDataSpaces*) env->GetLongField(obj,
                                                      ptrIDebugDataSpaces_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugDataSpaces != 0) {
     ptrIDebugDataSpaces->Release();
  }

  IDebugOutputCallbacks* ptrIDebugOutputCallbacks = (IDebugOutputCallbacks*)
                          env->GetLongField(obj, ptrIDebugOutputCallbacks_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugOutputCallbacks != 0) {
     ptrIDebugOutputCallbacks->Release();
  }

  IDebugRegisters* ptrIDebugRegisters = (IDebugRegisters*) env->GetLongField(obj, 
                                                      ptrIDebugRegisters_ID);
  CHECK_EXCEPTION_(false);

  if (ptrIDebugRegisters != 0) {
     ptrIDebugRegisters->Release();
  }

  IDebugSymbols* ptrIDebugSymbols = (IDebugSymbols*) env->GetLongField(obj, 
                                                      ptrIDebugSymbols_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugSymbols != 0) {
     ptrIDebugSymbols->Release();
  }

  IDebugSystemObjects* ptrIDebugSystemObjects = (IDebugSystemObjects*) env->GetLongField(obj, 
                                                      ptrIDebugSystemObjects_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugSystemObjects != 0) {
     ptrIDebugSystemObjects->Release();
  }

  IDebugControl* ptrIDebugControl = (IDebugControl*) env->GetLongField(obj, 
                                                     ptrIDebugControl_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugControl != 0) {
     ptrIDebugControl->Release();
  }

  IDebugClient* ptrIDebugClient = (IDebugClient*) env->GetLongField(obj, 
                                                      ptrIDebugClient_ID);
  CHECK_EXCEPTION_(false);
  if (ptrIDebugClient != 0) {
     ptrIDebugClient->Release();
  }

  return true;
}

/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    detach0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_detach0
  (JNIEnv *env, jobject obj) {
  releaseWindbgInterfaces(env, obj);
}


/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    readBytesFromProcess0
 * Signature: (JJ)[B
 */
JNIEXPORT jbyteArray JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_readBytesFromProcess0
  (JNIEnv *env, jobject obj, jlong address, jlong numBytes) {
  jbyteArray byteArray = env->NewByteArray((long) numBytes);
  CHECK_EXCEPTION_(0);

  jboolean isCopy = JNI_FALSE;
  jbyte* bytePtr = env->GetByteArrayElements(byteArray, &isCopy);
  CHECK_EXCEPTION_(0);

  IDebugDataSpaces* ptrIDebugDataSpaces = (IDebugDataSpaces*) env->GetLongField(obj,
                                                       ptrIDebugDataSpaces_ID);
  CHECK_EXCEPTION_(0);

  ULONG bytesRead;
  if (ptrIDebugDataSpaces->ReadVirtual((ULONG64) address, (PVOID) bytePtr, 
                                  (ULONG)numBytes, &bytesRead) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: ReadVirtual failed!", 0);
  }

  if (bytesRead != numBytes) {
     return 0;
  }

  env->ReleaseByteArrayElements(byteArray, bytePtr, JNI_COMMIT);
  CHECK_EXCEPTION_(0);

  return byteArray;
}

/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    getThreadIdFromSysId0
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_getThreadIdFromSysId0
  (JNIEnv *env, jobject obj, jlong sysId) {
  IDebugSystemObjects* ptrIDebugSystemObjects = (IDebugSystemObjects*) env->GetLongField(obj,
                                                    ptrIDebugSystemObjects_ID);
  CHECK_EXCEPTION_(0);

  ULONG id = 0;
  if (ptrIDebugSystemObjects->GetThreadIdBySystemId((ULONG)sysId, &id) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: GetThreadIdBySystemId failed!", 0);
  }

  return (jlong) id;
}

// manage COM 'auto' pointers (to avoid multiple Release
// calls at every early (exception) returns). Similar to AutoArrayPtr.

template <class T>
class AutoCOMPtr {
      T* m_ptr;

   public:
      AutoCOMPtr(T* ptr) : m_ptr(ptr) {
      }

      ~AutoCOMPtr() {
         if (m_ptr) {
            m_ptr->Release();
         }
      }

      T* operator->() {
         return m_ptr;
      }
};

/*
 * Class:     sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal
 * Method:    consoleExecuteCommand0
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_sun_jvm_hotspot_debugger_windbg_WindbgDebuggerLocal_consoleExecuteCommand0
  (JNIEnv *env, jobject obj, jstring cmd) {
  jboolean isCopy = JNI_FALSE;
  const char* cmd_cstr = env->GetStringUTFChars(cmd, &isCopy);
  CHECK_EXCEPTION_(0);

  IDebugClient* ptrIDebugClient = (IDebugClient*) env->GetLongField(obj, ptrIDebugClient_ID);
  CHECK_EXCEPTION_(0);

  IDebugClient*  tmpClientPtr = 0;
  if (ptrIDebugClient->CreateClient(&tmpClientPtr) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: CreateClient failed!", 0);
  }
  AutoCOMPtr<IDebugClient> tmpClient(tmpClientPtr);

  IDebugControl* tmpControlPtr = 0;
  if (tmpClient->QueryInterface(__uuidof(IDebugControl), (PVOID*) &tmpControlPtr) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: QueryInterface (IDebugControl) failed", 0);
  }
  AutoCOMPtr<IDebugControl> tmpControl(tmpControlPtr);
 
  SAOutputCallbacks* saOutputCallbacks = (SAOutputCallbacks*) env->GetLongField(obj,
                                                                   ptrIDebugOutputCallbacks_ID);
  CHECK_EXCEPTION_(0);

  saOutputCallbacks->clearBuffer();


  if (tmpClient->SetOutputCallbacks(saOutputCallbacks) != S_OK) {
     THROW_NEW_DEBUGGER_EXCEPTION_("Windbg Error: SetOutputCallbacks failed!", 0);
  }

  tmpControl->Execute(DEBUG_OUTPUT_VERBOSE, cmd_cstr, DEBUG_EXECUTE_DEFAULT);
  
  env->ReleaseStringUTFChars(cmd, cmd_cstr);
  CHECK_EXCEPTION_(0);

  const char* output = saOutputCallbacks->getBuffer();
  if (output == 0) { 
     output = "";
  }

  jstring res = env->NewStringUTF(output);
  saOutputCallbacks->clearBuffer();
  return res;
}
