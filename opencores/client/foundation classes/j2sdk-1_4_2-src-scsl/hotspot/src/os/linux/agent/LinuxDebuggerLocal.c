#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)LinuxDebuggerLocal.c	1.2 03/01/23 11:02:46 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <linux/user.h>

#include <jni.h>
#include "LinuxDebuggerLocal.h"

// FIXME: move debuggee into LinuxDebuggerLocal
process_info debuggee;

#define CHECK_EXCEPTION_(value) if ((*env)->ExceptionOccurred(env)) { return value; }
#define CHECK_EXCEPTION if ((*env)->ExceptionOccurred(env)) { return;}

/*
 * Class:     sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal
 * Method:    init0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_init0
  (JNIEnv *env, jclass cls) {

  utils_init();

  debuggee.attached = false;
  debuggee.pid = 0;
  debuggee.num_thread = 0;
  debuggee.libs = NULL;
}

/*
 * Class:     sun_jvm_hotspot_debugger_proc_LinuxDebuggerLocal
 * Method:    attach0
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_attach0
  (JNIEnv *env, jobject this_obj, jint jpid) {

  pid_t pid = (pid_t)jpid;
  if (debuggee.attached) {
    return;
  } else if (!ptrace_attach(pid)) {
    return;
  } else {
    char link[BUF_SIZE];

    debuggee.pid      = pid;

    // read library info and symbol tables, must do this before attach_threads,
    // as the symbols in the pthread library will be used to figure out
    // the list of threads within the same process.
    read_lib_info(pid);

    attach_threads(pid);

    debuggee.attached = true;

    sprintf(link, "/proc/%d/exe", pid);
    realpath(link, debuggee.name);
  }
}

/*
 * Class:     sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal
 * Method:    detach0
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_detach0
  (JNIEnv *env, jobject this_obj) {

  if (!debuggee.attached) {
    return;
  } else if (debuggee.num_thread > 0) {
    int i;
    for (i = 0; i < debuggee.num_thread; i++) {
      if (!ptrace_detach(debuggee.threads[i].pid)) {
         // failed to detach
         printf("Cannot detach %d\n", debuggee.threads[i].pid);
         // FIXME: shall we abort immediately or try to detach other threads?
      }
    }
  } else {
    // single threaded
    if (!ptrace_detach(debuggee.pid)) {
      printf("Cannot detach %d\n", debuggee.pid);
      return;
    }
  }

  destroy_lib_info();

  debuggee.attached = false;
  debuggee.pid = 0;
  debuggee.num_thread = 0;
  debuggee.libs = NULL;

  return;
}

/*
 * Class:     sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal
 * Method:    lookup0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_lookup0
  (JNIEnv *env, jobject this_obj, jstring objectName, jstring symbolName) {

  const char *objectName_cstr, *symbolName_cstr;
  address addr;
  jboolean isCopy;

  objectName_cstr = (*env)->GetStringUTFChars(env, objectName, &isCopy);
  CHECK_EXCEPTION_(0);
  symbolName_cstr = (*env)->GetStringUTFChars(env, symbolName, &isCopy);
  CHECK_EXCEPTION_(0);

  addr = lookup_symbol(debuggee.pid, objectName_cstr, symbolName_cstr, NULL);

  (*env)->ReleaseStringUTFChars(env, objectName, objectName_cstr);
  CHECK_EXCEPTION_(0);
  (*env)->ReleaseStringUTFChars(env, symbolName, symbolName_cstr);
  CHECK_EXCEPTION_(0);
  return (jlong)(intptr_t)addr;
}

/*
 * Class:     sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal
 * Method:    readBytesFromProcess0
 * Signature: (JJ)Lsun/jvm/hotspot/debugger/ReadResult;
 */
JNIEXPORT jbyteArray JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_readBytesFromProcess0
  (JNIEnv *env, jobject this_obj, jlong addr, jlong numBytes) {

  jboolean isCopy;
  jbyteArray array;
  jbyte *bufPtr;

  array = (*env)->NewByteArray(env, numBytes);
  CHECK_EXCEPTION_(0);
  bufPtr = (*env)->GetByteArrayElements(env, array, &isCopy);
  CHECK_EXCEPTION_(0);

  // FIXME: check return value and throw exception if failed
  ptrace_read_data(debuggee.pid, (address)(intptr_t)addr, bufPtr, numBytes);

  (*env)->ReleaseByteArrayElements(env, array, bufPtr, JNI_COMMIT);
  return array;
}

JNIEXPORT jlongArray JNICALL Java_sun_jvm_hotspot_debugger_linux_LinuxDebuggerLocal_getThreadIntegerRegisterSet0
  (JNIEnv *env, jobject this_obj, jint pid) {

  struct user_regs_struct gregs;
  jboolean isCopy;
  jlongArray array;
  jlong *regs;

  ptrace_getregs(pid, (void *)&gregs);

  array = (*env)->NewLongArray(env, 25);
  CHECK_EXCEPTION_(0);
  regs = (*env)->GetLongArrayElements(env, array, &isCopy);

  #define SA_GS       0
  #define SA_FS       1
  #define SA_ES       2
  #define SA_DS       3
  #define SA_EDI      4
  #define SA_ESI      5
  #define SA_EBP      6
  #define SA_ESP      7
  #define SA_EBX      8
  #define SA_EDX      9
  #define SA_ECX      10
  #define SA_EAX      11
  #define SA_EIP      14
  #define SA_CS       15
  #define SA_UESP     17
  #define SA_SS       18

  regs[SA_GS]  = gregs.gs;
  regs[SA_FS]  = gregs.fs;
  regs[SA_ES]  = gregs.es;
  regs[SA_DS]  = gregs.ds;
  regs[SA_EDI] = gregs.edi;
  regs[SA_ESI] = gregs.esi;
  regs[SA_EBP] = gregs.ebp;
  regs[SA_ESP] = gregs.esp;
  regs[SA_EBX] = gregs.ebx;
  regs[SA_EDX] = gregs.edx;
  regs[SA_ECX] = gregs.ecx;
  regs[SA_EAX] = gregs.eax;
  regs[SA_EIP] = gregs.eip;
  regs[SA_CS]  = gregs.cs;
  regs[SA_UESP] = gregs.esp;
  regs[SA_SS]  = gregs.ss;

  (*env)->ReleaseLongArrayElements(env, array, regs, JNI_COMMIT);
  return array;
}

