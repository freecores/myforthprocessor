/*
 * @(#)hprof_md.c	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <unistd.h>
#include <errno.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/time.h>

#include "jni.h"
#include "jlong.h"
#include "hprof.h"

int hprof_send(int s, const char *msg, int len, int flags)
{
    int res;
    do {
        res = send(s, msg, (SIZE_T)len, flags);
    } while ((res < 0) && (errno == EINTR));
    
    return res;
}

int hprof_write(int filedes, const void *buf, SIZE_T nbyte)
{
    int res;
    do {
        res = write(filedes, buf, nbyte);
    } while ((res < 0) && (errno == EINTR));

    return res;
}


jint hprof_get_milliticks()
{
    struct timeval tv;

    (void) gettimeofday(&tv, (void *) 0);
    return((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

jlong hprof_get_timemillis()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return jlong_add(jlong_mul(jint_to_jlong(t.tv_sec), jint_to_jlong(1000)), 
		     jint_to_jlong(t.tv_usec/1000));
}

void hprof_get_prelude_path(char *path)
{
    char home_dir[MAXPATHLEN];
    Dl_info dlinfo;

    dladdr((void *)hprof_get_prelude_path, &dlinfo);
    strcpy(home_dir, (char *)dlinfo.dli_fname);
    *(strrchr(home_dir, '/')) = '\0';
    *(strrchr(home_dir, '/')) = '\0';
    *(strrchr(home_dir, '/')) = '\0';

    sprintf(path, "%s/lib/jvm.hprof.txt", home_dir);
}
