/*
 * @(#)hprof_md.c	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <mmsystem.h>

#include "jni.h"
#include "jlong.h"
#include "hprof.h"

int hprof_send(int s, const char *msg, int len, int flags)
{
    return send(s, msg, len, flags);
}

int hprof_write(int filedes, const void *buf, SIZE_T nbyte)
{
    return write(filedes, buf, nbyte);
}

jint hprof_get_milliticks()
{
    return(timeGetTime());
}

#define FT2JLONG(ft) \
	((jlong)(ft).dwHighDateTime << 32 | (jlong)(ft).dwLowDateTime)

jlong hprof_get_timemillis()
{
    static jlong fileTime_1_1_70 = 0;
    SYSTEMTIME st0;
    FILETIME   ft0;

    if (fileTime_1_1_70 == 0) {
        /* Initialize fileTime_1_1_70 -- the Win32 file time of midnight
	 * 1/1/70.
         */ 

        memset(&st0, 0, sizeof(st0));
        st0.wYear  = 1970;
        st0.wMonth = 1;
        st0.wDay   = 1;
        SystemTimeToFileTime(&st0, &ft0);
	fileTime_1_1_70 = FT2JLONG(ft0);
    } 

    GetSystemTime(&st0);
    SystemTimeToFileTime(&st0, &ft0);

    return (FT2JLONG(ft0) - fileTime_1_1_70) / 10000;
}

HINSTANCE hJavaInst;
static int nError = 0;

BOOL WINAPI
DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
    WSADATA wsaData;
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            hJavaInst = hinst;
            nError = WSAStartup(MAKEWORD(2,0), &wsaData);
            break;
        case DLL_PROCESS_DETACH:
            WSACleanup();
            hJavaInst = NULL;
        default:
            break;
    }
    return TRUE;
}

void hprof_get_prelude_path(char *path)
{
    char home_dir[MAX_PATH];

    GetModuleFileName(hJavaInst, home_dir, MAX_PATH);
    *(strrchr(home_dir, '\\')) = '\0';
    *(strrchr(home_dir, '\\')) = '\0';

    sprintf(path, "%s\\lib\\jvm.hprof.txt", home_dir);
}
