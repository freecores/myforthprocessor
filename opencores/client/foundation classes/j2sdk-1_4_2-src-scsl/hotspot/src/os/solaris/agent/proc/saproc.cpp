#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)saproc.cpp	1.9 03/01/23 11:04:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "libproc.h"
#include "sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal.h"
#include <thread_db.h>
#include <strings.h>
#include <limits.h>
/*
* Portions of user thread level detail gathering code is from pstack source
* code. See pstack.c in Solaris 2.8 user commands source code.
*/

#define CHECK_EXCEPTION_(value) if(env->ExceptionOccurred()) { return value; }
#define CHECK_EXCEPTION if(env->ExceptionOccurred()) { return;}
#define THROW_NEW_DEBUGGER_EXCEPTION_(str, value) { throwNewDebuggerException(env, str); return value; }
#define THROW_NEW_DEBUGGER_EXCEPTION(str) { throwNewDebuggerException(env, str); return;}

struct Debugger {
    JNIEnv* env;
    jobject obj;
};    

static jfieldID ps_prochandle_ptr_ID = 0;

// libthread.so dlopen handle and function pointers
static jfieldID tdb_handle_ID        = 0;
static jfieldID p_td_init_ID         = 0;
static jfieldID p_td_ta_new_ID       = 0;
static jfieldID p_td_ta_delete_ID    = 0;
static jfieldID p_td_ta_thr_iter_ID  = 0;
static jfieldID p_td_thr_get_info_ID = 0;
static jfieldID p_td_thr_getgregs_ID = 0;

/*
 * Functions we need from libthread_db
 */
typedef td_err_e
        (*p_td_init_t)(void);
typedef td_err_e
        (*p_td_ta_new_t)(void *, td_thragent_t **);
typedef td_err_e
        (*p_td_ta_delete_t)(td_thragent_t *);
typedef td_err_e
        (*p_td_ta_thr_iter_t)(const td_thragent_t *, td_thr_iter_f *, void *,
                td_thr_state_e, int, sigset_t *, unsigned);
typedef td_err_e
        (*p_td_thr_get_info_t)(const td_thrhandle_t *, td_thrinfo_t *);
typedef td_err_e
        (*p_td_thr_getgregs_t)(const td_thrhandle_t *, prgregset_t);

static jmethodID setThreadIntegerRegisterSet_ID = 0;

static void throwNewDebuggerException(JNIEnv* env, const char* errMsg) {
  env->ThrowNew(env->FindClass("sun/jvm/hotspot/debugger/DebuggerException"), errMsg);
}

/*
 * Part of workaround for 4705086.
 *
 * The following fields are added to the ProcDebuggerLocal class just to work
 * around libproc bug # 4705086.
 *
 * libproc fails on ps_pread on some valid virtual addresses. These are
 * mostly read only data addresses in text pages.
 *
 * If ps_pread fails then we attempt to read the 'memory' directly from
 * libjvm[_g].so.
*/

static jfieldID libjvm_fd_ID       = 0;
static jfieldID libjvm_text_start_ID = 0;
static jfieldID libjvm_text_size_ID     = 0;

extern "C"  {

int iterate_map(void *cd, const prmap_t* pmp, const char* obj_name) {

    /*
     * we are iterating over load maps of the process/core.
     * look for first mapping of libjvm[_g].so. If we get this mapping
     * store the initial virtual address of the libjvm[_g].so and libjvm[_g].so
     * size in the debugger object.
     */

    if (obj_name != 0) {
       if (strstr(obj_name, "libjvm.so") != 0 || strstr(obj_name, "libjvm_g.so") != 0) {

           Debugger* dbg = (Debugger*) cd;
           JNIEnv*   env = dbg->env;

           /*
            * get libjvm[_g].so's starting text virtual address. We just take the
            * starting virtual address of libjvm[_g].so's mapping.
            */
 
           dbg->env->SetLongField(dbg->obj, libjvm_text_start_ID, pmp->pr_vaddr);

           // open the libjvm.so
           int fd = open(obj_name, O_RDONLY);
           if (fd < 0) {
               char errMsg[PATH_MAX + 128];
               sprintf(errMsg, "Can't open %s!", obj_name);
               THROW_NEW_DEBUGGER_EXCEPTION_(errMsg, 1);
           }

           dbg->env->SetIntField(dbg->obj, libjvm_fd_ID, fd);
           return 1;
       }
    }

    return 0;
}

} // extern "C"


/*
 * Thread iteration call-back function.
 * Called once for each user-level thread.
 * Used to build the list of all threads.
 */

extern "C" { 
static int thr_stack(const td_thrhandle_t *Thp, void *cd) {
   Debugger* pDebugger = (Debugger*) cd;
   JNIEnv* env = pDebugger->env;
   jobject this_obj = pDebugger->obj;

   td_thrinfo_t thrinfo;

   p_td_thr_get_info_t p_td_thr_get_info = (p_td_thr_get_info_t)
                                    env->GetLongField(this_obj, p_td_thr_get_info_ID);

   if (p_td_thr_get_info(Thp, &thrinfo) != TD_OK)
         return (0);

   prgregset_t regs;   
   (void) memset(regs, 0, sizeof (prgregset_t));

   p_td_thr_getgregs_t p_td_thr_getgregs = (p_td_thr_getgregs_t)
                                    env->GetLongField(this_obj, p_td_thr_getgregs_ID);

   (void) p_td_thr_getgregs(Thp, regs);

   jlongArray regSetArray = env->NewLongArray(NPRGREG);
   CHECK_EXCEPTION_(0);
   jboolean isCopy;
   jlong* ptrRes = env->GetLongArrayElements(regSetArray, &isCopy); 
   CHECK_EXCEPTION_(0);
       
   // copy the reg set
   for(int c = 0; c < NPRGREG; c++)
       ptrRes[c] = (jlong) (uintptr_t) regs[c];
  
   env->ReleaseLongArrayElements(regSetArray, ptrRes, JNI_COMMIT);
   env->CallVoidMethod(pDebugger->obj, 
                       setThreadIntegerRegisterSet_ID,
                       thrinfo.ti_tid,
                       regSetArray);
   CHECK_EXCEPTION_(0);
   return (0);
}

static int
object_iter(void *cd, const prmap_t *pmp, const char *object_name) {
  Debugger* dbg = (Debugger*) cd;
  JNIEnv* env = dbg->env;
  jobject this_obj = dbg->obj;

  ps_prochandle* Pr = 0;
  Pr = (ps_prochandle*) env->GetLongField(this_obj, ps_prochandle_ptr_ID);

  char *s1 = 0, *s2 = 0;
  char libthread_db[PATH_MAX];

  if (strstr(object_name, "/libthread.so.") == NULL)
     return (0);

  /*
   * We found a libthread.
   * dlopen() the matching libthread_db and get the thread agent handle.
   */
  if (Pstatus(Pr)->pr_dmodel == PR_MODEL_NATIVE) {
     (void) strcpy(libthread_db, object_name);
     s1 = (char*) strstr(object_name, ".so.");
     s2 = (char*) strstr(libthread_db, ".so.");
     (void) strcpy(s2, "_db");
     s2 += 3;
     (void) strcpy(s2, s1);
  } else {
#ifdef _SYSCALL32
     /*
      * The victim process is 32-bit, we are 64-bit.
      * We have to find the 64-bit version of libthread_db
      * that matches the victim's 32-bit version of libthread.
      */
     (void) strcpy(libthread_db, object_name);
     s1 = (char*) strstr(object_name, "/libthread.so.");
     s2 = (char*) strstr(libthread_db, "/libthread.so.");
     (void) strcpy(s2, "/64");
     s2 += 3;
     (void) strcpy(s2, s1);
     s1 = (char*) strstr(s1, ".so.");
     s2 = (char*) strstr(s2, ".so.");
     (void) strcpy(s2, "_db");
     s2 += 3;
     (void) strcpy(s2, s1);
#else
     return (0);
#endif  /* _SYSCALL32 */
  }

  void* tdb_handle = 0;
  if ((tdb_handle = dlopen(libthread_db, RTLD_LAZY|RTLD_LOCAL)) == NULL) {
     char errMsg[PATH_MAX + 256];
     sprintf(errMsg, "Can't load %s!", libthread_db);
     THROW_NEW_DEBUGGER_EXCEPTION_(errMsg, 0);
  }
  env->SetLongField(this_obj, tdb_handle_ID, (jlong) tdb_handle);

  void* tmpPtr = 0;
  tmpPtr = dlsym(tdb_handle, "td_init");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_init!", 0);
  env->SetLongField(this_obj, p_td_init_ID, (jlong) tmpPtr);

  tmpPtr =dlsym(tdb_handle, "td_ta_new");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_ta_new!", 0);
  env->SetLongField(this_obj, p_td_ta_new_ID, (jlong) tmpPtr);

  tmpPtr = dlsym(tdb_handle, "td_ta_delete");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_ta_delete!", 0);
  env->SetLongField(this_obj, p_td_ta_delete_ID, (jlong) tmpPtr);

  tmpPtr = dlsym(tdb_handle, "td_ta_thr_iter");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_ta_thr_iter!", 0);
  env->SetLongField(this_obj, p_td_ta_thr_iter_ID, (jlong) tmpPtr);

  tmpPtr = dlsym(tdb_handle, "td_thr_get_info");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_thr_get_info!", 0);
  env->SetLongField(this_obj, p_td_thr_get_info_ID, (jlong) tmpPtr);

  tmpPtr = dlsym(tdb_handle, "td_thr_getgregs");
  if (tmpPtr == 0)
     THROW_NEW_DEBUGGER_EXCEPTION_("dlsym failed on td_thr_getgregs!", 0);
  env->SetLongField(this_obj, p_td_thr_getgregs_ID, (jlong) tmpPtr);

  return 1;
}

} // extern "C"
/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    attach0
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_attach0
  (JNIEnv *env, jobject this_obj, jstring cmdLine) {
  jboolean isCopy;
  int gcode;
  const char* cmdLine_cstr = env->GetStringUTFChars(cmdLine, &isCopy);
  CHECK_EXCEPTION;

  struct ps_prochandle* Pr = proc_arg_grab(cmdLine_cstr, PR_ARG_ANY, PGRAB_FORCE, &gcode);
  env->ReleaseStringUTFChars(cmdLine, cmdLine_cstr);
  if(! Pr) 
    THROW_NEW_DEBUGGER_EXCEPTION("Not able to attach to process/core file!");

  env->SetLongField(this_obj, ps_prochandle_ptr_ID, (jlong) Pr);

  // initialize libthread_db pointers
  
  /*
   * Iterate over the process mappings looking
   * for libthread and then dlopen the appropriate
   * libthread_db and get pointers to functions.
   */

  Debugger dbg;
  dbg.env = env;
  dbg.obj = this_obj;
  (void) Pobject_iter(Pr, object_iter, &dbg);
  CHECK_EXCEPTION;
  
  // get the user level threads info
  td_thragent_t *Tap = 0;
  p_td_init_t p_td_init = (p_td_init_t) env->GetLongField(this_obj, p_td_init_ID);

  if (p_td_init == 0)
     return;

  p_td_ta_new_t p_td_ta_new = (p_td_ta_new_t) env->GetLongField(this_obj, p_td_ta_new_ID);

  if (p_td_init() != TD_OK) 
     THROW_NEW_DEBUGGER_EXCEPTION("Can't initialize thread_db!");

  if (p_td_ta_new(Pr, &Tap) != TD_OK)
     THROW_NEW_DEBUGGER_EXCEPTION("Can't create thread_db agent!");
 
  /*
   * Iterate over all threads, calling:
   *   thr_stack(td_thrhandle_t *Thp, NULL);
   * for each one to generate the list of threads.
   */

  p_td_ta_thr_iter_t p_td_ta_thr_iter = (p_td_ta_thr_iter_t)
                                     env->GetLongField(this_obj, p_td_ta_thr_iter_ID);

  (void) p_td_ta_thr_iter(Tap, thr_stack, &dbg,
                          TD_THR_ANY_STATE, TD_THR_LOWEST_PRIORITY,
                           TD_SIGNO_MASK, TD_THR_ANY_USER_FLAGS);
  CHECK_EXCEPTION;

  p_td_ta_delete_t p_td_ta_delete = (p_td_ta_delete_t)
                                     env->GetLongField(this_obj, p_td_ta_delete_ID);
  (void) p_td_ta_delete(Tap);
  
  // Part of workaround for 4705086.

  /* 
   * iterate over maps of the process/core to get first
   * libjvm[_g].so mapping.
   */

  Pmapping_iter(Pr, iterate_map, &dbg);
  CHECK_EXCEPTION;

  /*
   * Get libjvm[_g].so text size. First location after the end of text segment
   * is marked by the global reserved symbol '_etext' in any ELF file.
   * Please refer to page 53 of "Linkers and Libraries Guide - 816-0559".
   */

  psaddr_t etext_addr;
  if (ps_pglobal_lookup(Pr, "libjvm.so", "_etext", &etext_addr) != PS_OK) {
      // try the debug version
      if (ps_pglobal_lookup(Pr, "libjvm_g.so", "_etext", &etext_addr) != PS_OK) 
         THROW_NEW_DEBUGGER_EXCEPTION("Can't get end of text address of libjvm!");
  }

  // now calculate and set libjvm text size.
  jlong libjvm_text_start = env->GetLongField(this_obj, libjvm_text_start_ID);
  env->SetLongField(this_obj, libjvm_text_size_ID, (jlong) (etext_addr - libjvm_text_start));
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    detach0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_detach0
  (JNIEnv *env, jobject this_obj) {

  jlong ps_prochandle_ptr;
  ps_prochandle_ptr = env->GetLongField(this_obj, ps_prochandle_ptr_ID);
  if(ps_prochandle_ptr != 0L)
    Prelease((struct ps_prochandle*) ps_prochandle_ptr, PRELEASE_CLEAR);

  // release libthread.so
  void* tdb_handle = (void*) env->GetLongField(this_obj, tdb_handle_ID);
  if (tdb_handle != 0)
    dlclose(tdb_handle);

  // Part of workaround for 4705086.

  // get libjvm[_g].so file descriptor and close it.
  int libjvm_fd = env->GetIntField(this_obj, libjvm_fd_ID);
  if (libjvm_fd != -1)
     close(libjvm_fd);
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    getPageSize0
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_getPageSize0
  (JNIEnv *env, jobject this_obj) {

/* 
  We are not yet attached to a java process or core file. getPageSize is called from
  the constructor of ProcDebuggerLocal. The following won't work!

    jlong ps_prochandle_ptr;
    ps_prochandle_ptr = env->GetLongField(this_obj, ps_prochandle_ptr_ID);
    CHECK_EXCEPTION_(-1);
    struct ps_prochandle* prochandle = (struct ps_prochandle*) ps_prochandle_ptr;
    return (Pstate(prochandle) == PS_DEAD) ? Pgetauxval(prochandle, AT_PAGESZ) 
                                           : getpagesize();

  So even though core may have been generated with a different page size settings, for now
  call getpagesize.
*/

  return getpagesize();
}


/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    initIDs
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_initIDs
  (JNIEnv *env, jclass clazz) { 

  void* libproc_handle = dlopen("libproc.so", RTLD_LAZY | RTLD_GLOBAL);
  if (libproc_handle == 0) 
     THROW_NEW_DEBUGGER_EXCEPTION("Can't load libproc.so, if you are using Solaris 5.7 or below, copy libproc.so from 5.8!");

  ps_prochandle_ptr_ID = env->GetFieldID(clazz, "ps_prochandle_ptr", "J");
  CHECK_EXCEPTION;

  tdb_handle_ID = env->GetFieldID(clazz, "tdb_handle", "J");
  CHECK_EXCEPTION;

  p_td_init_ID = env->GetFieldID(clazz, "p_td_init", "J");
  CHECK_EXCEPTION;

  p_td_ta_new_ID = env->GetFieldID(clazz, "p_td_ta_new", "J");
  CHECK_EXCEPTION;

  p_td_ta_delete_ID = env->GetFieldID(clazz, "p_td_ta_delete", "J");
  CHECK_EXCEPTION;

  p_td_ta_thr_iter_ID = env->GetFieldID(clazz, "p_td_ta_thr_iter", "J");
  CHECK_EXCEPTION;

  p_td_thr_get_info_ID = env->GetFieldID(clazz, "p_td_thr_get_info", "J");
  CHECK_EXCEPTION;

  p_td_thr_getgregs_ID = env->GetFieldID(clazz, "p_td_thr_getgregs", "J");
  CHECK_EXCEPTION;

  setThreadIntegerRegisterSet_ID = env->GetMethodID(clazz, 
                                      "setThreadIntegerRegisterSet", "(I[J)V");
  CHECK_EXCEPTION;

  // Part of workaround for 4705086.

  // libjvm file descriptor
  libjvm_fd_ID = env->GetFieldID(clazz, "libjvm_fd", "I");
  CHECK_EXCEPTION;

  // libjvm start virtual address
  libjvm_text_start_ID = env->GetFieldID(clazz, "libjvm_text_start", "J");
  CHECK_EXCEPTION;

  // size of text in libjvm[_g].so.
  libjvm_text_size_ID = env->GetFieldID(clazz, "libjvm_text_size", "J");
  CHECK_EXCEPTION;
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    lookup0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_lookup0
  (JNIEnv *env, jobject this_obj, jstring objectName, jstring symbolName) {
  jlong ps_prochandle_ptr;
  ps_prochandle_ptr = env->GetLongField(this_obj, ps_prochandle_ptr_ID);

  jboolean isCopy;
  const char* objectName_cstr = env->GetStringUTFChars(objectName, &isCopy);
  CHECK_EXCEPTION_(0);
  const char* symbolName_cstr = env->GetStringUTFChars(symbolName, &isCopy);
  CHECK_EXCEPTION_(0);

  psaddr_t symbol_addr = (psaddr_t) 0;
  ps_pglobal_lookup((struct ps_prochandle*) ps_prochandle_ptr, objectName_cstr,
                   symbolName_cstr, &symbol_addr); 

  env->ReleaseStringUTFChars(objectName, objectName_cstr);
  env->ReleaseStringUTFChars(symbolName, symbolName_cstr);
  return (jlong) symbol_addr;
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    readBytesFromProcess0
 * Signature: (JJ)Lsun/jvm/hotspot/debugger/ReadResult;
 */
JNIEXPORT jbyteArray JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_readBytesFromProcess0
  (JNIEnv *env, jobject this_obj, jlong address, jlong numBytes) {

  jbyteArray array = env->NewByteArray(numBytes);
  CHECK_EXCEPTION_(0);
  jboolean isCopy;
  jbyte* bufPtr = env->GetByteArrayElements(array, &isCopy);
  CHECK_EXCEPTION_(0);
 
  jlong ps_prochandle_ptr = env->GetLongField(this_obj, ps_prochandle_ptr_ID);
  ps_err_e ret = ps_pread((struct ps_prochandle*) ps_prochandle_ptr, 
                       (psaddr_t)address, bufPtr, (size_t)numBytes); 

  if(ret != PS_OK) {

     // Part of workaround for 4705086.

     jint  libjvm_fd = env->GetIntField(this_obj, libjvm_fd_ID);
     jlong libjvm_text_start = env->GetLongField(this_obj, libjvm_text_start_ID);
     jlong libjvm_text_size = env->GetLongField(this_obj, libjvm_text_size_ID);

     // get the file descriptor for libjvm.so
     jlong offset = address - libjvm_text_start;

     // do bounds check to verify that the given address is in
     // libjvm text
     if (offset >= libjvm_text_size || offset < 0) {
        env->ReleaseByteArrayElements(array, bufPtr, JNI_COMMIT);
        // the address given is not in libjvm[_g].so text
        return jbyteArray(0);
     }

     ssize_t bytes_read = pread(libjvm_fd, bufPtr, numBytes, (off_t)offset); 
     if (bytes_read != (ssize_t) numBytes) {
         env->ReleaseByteArrayElements(array, bufPtr, JNI_COMMIT);
         return jbyteArray(0);
     }
  }

  env->ReleaseByteArrayElements(array, bufPtr, JNI_COMMIT);
  return array;
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal
 * Method:    getRemoteProcessAddressSize0
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_sun_jvm_hotspot_debugger_proc_ProcDebuggerLocal_getRemoteProcessAddressSize0
  (JNIEnv *env, jobject this_obj) {
  jlong ps_prochandle_ptr;
  ps_prochandle_ptr = env->GetLongField(this_obj, ps_prochandle_ptr_ID);
  int data_model = PR_MODEL_ILP32;
  ps_pdmodel((struct ps_prochandle*) ps_prochandle_ptr, &data_model);
  return (jint) data_model == PR_MODEL_ILP32? 32 : 64;
}

