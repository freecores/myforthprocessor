/*
 * @(#)Win32Process_md.c	1.15 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "java_lang_Win32Process.h"

#include "jni.h"
#include "jvm.h"
#include "jni_util.h"
#include "typedefs.h"

#define PIPE_SIZE 512

extern jfieldID IO_fd_fdID;

char *
extractExecutablePath(JNIEnv *env, char *source)
{
    char *p, *r;

    /* If no spaces, then use entire thing */
    if ((p = strchr(source, ' ')) == NULL)
        return source;

    /* If no quotes, or quotes after space, return up to space */
    if (((r = strchr(source, '"')) == NULL) || (r > p)) {
        *p = 0;
        return source;
    }

    /* Quotes before space, return up to space after next quotes */
    p = strchr(r, '"');
    if ((p = strchr(p, ' ')) == NULL)
        return source;
    *p = 0;
    return source;
}

DWORD
selectProcessFlag(JNIEnv *env, jstring cmd0)
{
    char buf[MAX_PATH];
    DWORD newFlag = 0;
    DWORD *type;
    char *exe, *p, *name;
    unsigned char buffer[2];
    long headerLoc = 0;
    int fd = 0;

    exe = (char *)JNU_GetStringPlatformChars(env, cmd0, 0);
    exe = extractExecutablePath(env, exe);

    if (exe != NULL) {
        if ((p = strchr(exe, '\\')) == NULL) {
            SearchPath(NULL, exe, ".exe", MAX_PATH, buf, &name);
        } else {
            p = strrchr(exe, '\\');
            *p = 0;
            p++;
            SearchPath(exe, p, ".exe", MAX_PATH, buf, &name);
        }
    }

    fd = _open(buf, _O_RDONLY);
    if (fd > 0) {
        _read(fd, buffer, 2);
        if (buffer[0] == 'M' && buffer[1] == 'Z') {
            _lseek(fd, 60L, SEEK_SET);
            _read(fd, buffer, 2);
            headerLoc = (long)buffer[1] << 8 | (long)buffer[0];
            _lseek(fd, headerLoc, SEEK_SET);
            _read(fd, buffer, 2);
            if (buffer[0] == 'P' && buffer[1] == 'E') {
                newFlag = DETACHED_PROCESS;
            }
        }
        _close(fd);
    }
    JNU_ReleaseStringPlatformChars(env, cmd0, exe);
    return newFlag;
}

JNIEXPORT jlong JNICALL
Java_java_lang_Win32Process_create(JNIEnv *env, jobject process,
				   jstring cmd0,
				   jstring env0,
                                   jstring path,
				   jobject in_fd,
				   jobject out_fd,
				   jobject err_fd)
{
    HANDLE hread[3], hwrite[3];
    SECURITY_ATTRIBUTES sa;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    char *cmd, *envcmd = 0, *cwd = 0;
    jlong ret = 0;
    OSVERSIONINFO ver;
    jboolean onNT = JNI_FALSE;
    DWORD processFlag;

    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        onNT = JNI_TRUE;
    }

    if (cmd0 == 0) {
	JNU_ThrowNullPointerException(env, 0);
	return 0;
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = 0;
    sa.bInheritHandle = TRUE;

    memset(hread, 0, sizeof(hread));
    memset(hwrite, 0, sizeof(hwrite));
    if (!(CreatePipe(&hread[0], &hwrite[0], &sa, PIPE_SIZE) &&
          CreatePipe(&hread[1], &hwrite[1], &sa, PIPE_SIZE) &&
	  CreatePipe(&hread[2], &hwrite[2], &sa, PIPE_SIZE))) {
        CloseHandle(hread[0]);
        CloseHandle(hread[1]);
        CloseHandle(hread[2]);
	CloseHandle(hwrite[0]);
	CloseHandle(hwrite[1]);
	CloseHandle(hwrite[2]);
	JNU_ThrowByName(env, "java/io/IOException", "CreatePipe");
	return 0;
    }

    cmd = (char *)JNU_GetStringPlatformChars(env, cmd0, 0);
    if (env0 != 0) {
	envcmd = (char *)JNU_GetStringPlatformChars(env, env0, 0);
    }
    if (path != 0) { 
        cwd = JVM_NativePath((char *)JNU_GetStringPlatformChars(env, path, 0));
    }

    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = hread[0];
    si.hStdOutput = hwrite[1];
    si.hStdError  = hwrite[2];

    SetHandleInformation(hwrite[0], HANDLE_FLAG_INHERIT, FALSE);
    SetHandleInformation(hread[1],  HANDLE_FLAG_INHERIT, FALSE);
    SetHandleInformation(hread[2],  HANDLE_FLAG_INHERIT, FALSE);

    processFlag = CREATE_NO_WINDOW;
    if (!onNT) {
        processFlag = selectProcessFlag(env, cmd0);
    }

    ret = CreateProcess(0,                /* executable name */
                        cmd,              /* command line */
                        0,                /* process security attribute */
                        0,                /* thread security attribute */
                        TRUE,             /* inherits system handles */
                        processFlag,      /* selected based on exe type */ 
                        envcmd,           /* environment block */
                        cwd,              /* change to the new current directory */
                        &si,              /* (in)  startup information */
                        &pi);             /* (out) process information */

    CloseHandle(hread[0]);
    CloseHandle(hwrite[1]);
    CloseHandle(hwrite[2]);

    if (!ret) {
	char msg[1024];
	CloseHandle(hwrite[0]);
	CloseHandle(hread[1]);
	CloseHandle(hread[2]);
	jio_snprintf(msg, 1024,
		     "CreateProcess: %s error=%d", cmd, GetLastError());
	JNU_ThrowByName(env, "java/io/IOException", msg);
	goto cleanup;
    }

    CloseHandle(pi.hThread);
    ret = (jlong)pi.hProcess;
    (*env)->SetIntField(env, in_fd, IO_fd_fdID,
			_open_osfhandle((intptr_t)hwrite[0], 0));
    (*env)->SetIntField(env, out_fd, IO_fd_fdID,
			_open_osfhandle((intptr_t)hread[1], 0));
    (*env)->SetIntField(env, err_fd, IO_fd_fdID,
			_open_osfhandle((intptr_t)hread[2], 0));

 cleanup:
    JNU_ReleaseStringPlatformChars(env, cmd0, cmd);
    if (env0 != 0) {
	JNU_ReleaseStringPlatformChars(env, env0, envcmd);
    }
    return ret;
}

JNIEXPORT jint JNICALL
Java_java_lang_Win32Process_exitValue(JNIEnv *env, jobject process)
{
    jint exit_code;
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return 0;
    }

    GetExitCodeProcess((void *)handle, &exit_code);
    if (exit_code == STILL_ACTIVE) {
	JNU_ThrowByName(env, "java/lang/IllegalThreadStateException",
			"process has not exited");
	return -1;
    }
    return exit_code;
}

#define PROCESS_EVENT 0
#define INTERRUPT_EVENT 1

JNIEXPORT jint JNICALL
Java_java_lang_Win32Process_waitFor(JNIEnv *env, jobject process)
{
    long exit_code;
    int which;
    HANDLE events[2];
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return 0;
    }

    events[PROCESS_EVENT] = (void *)handle;
    events[INTERRUPT_EVENT] = JVM_GetThreadInterruptEvent();

    which = WaitForMultipleObjects(2, events, FALSE, INFINITE);
    if (which == PROCESS_EVENT) {
	GetExitCodeProcess((void *)handle, &exit_code);
    } else {
	JNU_ThrowByName(env, "java/lang/InterruptedException", 0);
    }

    return exit_code;
}

#undef PROCESS_EVENT
#undef INTERRUPT_EVENT

JNIEXPORT void JNICALL
Java_java_lang_Win32Process_destroy(JNIEnv *env, jobject process)
{
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return;
    }
    TerminateProcess((void *)handle, 1);
}

JNIEXPORT void JNICALL
Java_java_lang_Win32Process_close(JNIEnv *env, jobject process)
{
    jboolean exc;
    jlong handle = JNU_GetFieldByName(env, &exc, process, "handle", "J").j;
    if (exc) {
        return;
    }
    CloseHandle((void *)handle);
}
