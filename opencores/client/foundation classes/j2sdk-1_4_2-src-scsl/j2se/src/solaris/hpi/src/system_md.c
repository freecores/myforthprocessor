/*
 * @(#)system_md.c	1.80 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "hpi_impl.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h> /* timeval */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <netdb.h>
#include <limits.h>
#include <errno.h>

#include <dlfcn.h>

#include "jni_md.h"
#include "mutex_md.h"

#include "hpi_init.h"

#include "interrupt.h"
#include "threads_md.h"
#include "monitor_md.h"
#include "largefile.h"


#define O_DELETE 0x10000

int sysThreadBootstrap(sys_thread_t **tidP, sys_mon_t **lockP, int nb)
{
    threadBootstrapMD(tidP, lockP, nb);

    intrInit();

#ifndef NATIVE
    /* Initialize the special case for sbrk on Solaris (see synch.c) */
    InitializeSbrk();
    /* Initialize the async io */
    InitializeAsyncIO();
    InitializeMem();
    /* Initialize Clock and Idle threads */
    InitializeHelperThreads();
#else /* if NATIVE */
    initializeContentionCountMutex();
    InitializeMem();
#endif /* NATIVE */

    return SYS_OK;
}

int sysShutdown()
{
    return SYS_OK;
}

long
sysGetMilliTicks()
{
    struct timeval tv;

    (void) gettimeofday(&tv, (void *) 0);
    return((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}

jlong
sysTimeMillis()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return ((jlong)t.tv_sec) * 1000 + (jlong)(t.tv_usec/1000);
}

int
sysGetLastErrorString(char *buf, int len)
{
    if (errno == 0) {
	return 0;
    } else {
	const char *s = strerror(errno);
	int n = strlen(s);
	if (n >= len) n = len - 1;
	strncpy(buf, s, n);
	buf[n] = '\0';
	return n;
    }
}

/*
 * File system
 *
 * These are all the sys API which implement the straight POSIX
 * API. Those that do not are defined by thread-specific files
 * (i.e. io_md.c)
 */

/*
 * Open a file. Unlink the file immediately after open returns 
 * if the specified oflag has the O_DELETE flag set.
 */
int sysOpen(const char *path, int oflag, int mode)
{
    int fd;
    int delete = (oflag & O_DELETE);
    oflag = oflag & ~O_DELETE;
    fd = open64_w(path, oflag, mode);
    if (delete != 0) {
        unlink(path);
    }
    return fd;
}

char *sysNativePath(char *path)
{
    return path;
}

/*
 * Support for 64 bit File I/O.
 */

/* 64 bit versions of low-level File I/O routines */

typedef int (*FSTAT64)(int fd, struct stat64 *);
typedef jlong (*LSEEK64)(int, jlong, int);
typedef int (*FTRUNCATE64)(int, jlong);
#ifdef __linux__
typedef int (*OPEN64)(const char *path, int oflag, ...);
#else
typedef int (*OPEN64)(const char *path, int oflag, mode_t mode);
#endif

#if defined(__linux__) && defined(_LARGEFILE64_SOURCE)
static FSTAT64 fstat64_ptr = &fstat64;
static LSEEK64 lseek64_ptr = &lseek64;
static FTRUNCATE64 ftruncate64_ptr = &ftruncate64;
static OPEN64 open64_ptr = &open64;
#else
static FSTAT64 fstat64_ptr = NULL;
static LSEEK64 lseek64_ptr = NULL;
static FTRUNCATE64 ftruncate64_ptr = NULL;
static OPEN64 open64_ptr = NULL;
#endif

/*
 * 64-bit IO initialization routine.  The #pragma makes this routine get called
 * when this code (shared library) is loaded.  Find the pointers to these libc
 * routines if they exist.
 */
static void init64IO(void) {
    void *handle = dlopen(0, RTLD_LAZY);
    fstat64_ptr = (FSTAT64) dlsym(handle, "_fstat64");
    /* Look for others only if the first one succeeded */
    if (fstat64_ptr != NULL) {
	lseek64_ptr = (LSEEK64) dlsym(handle, "_lseek64");
	ftruncate64_ptr = (FTRUNCATE64) dlsym(handle, "_ftruncate64");
	open64_ptr = (OPEN64) dlsym(handle, "_open64");
    }
    dlclose(handle);
}
#ifdef __GNUC__
static void init64IO(void) __attribute__((constructor));
#else
#pragma init(init64IO)
#endif

int
sysFileSizeFD(int fd, jlong *size)
{
    int ret;

    if (fstat64_ptr != 0) {
        struct stat64 buf64;
	ret = (*fstat64_ptr)(fd, &buf64);
	*size = buf64.st_size;
    } else {
        struct stat buf;
	ret = fstat(fd, &buf);
	*size = buf.st_size;
    }
    return ret;
}

int
sysFfileMode(int fd, int *mode)
{
    int ret;
    
    if (fstat64_ptr != 0) {
        struct stat64 buf64;
	ret = (*fstat64_ptr)(fd, &buf64);
	(*mode) = buf64.st_mode;
    } else {
        struct stat buf;
	ret = fstat(fd, &buf);
	(*mode) = buf.st_mode;
    }
    return ret;
}

int
sysFileType(const char *path)
{
    int ret;
    struct stat buf;

    if ((ret = stat(path, &buf)) == 0) {
      mode_t mode = buf.st_mode & S_IFMT;
      if (mode == S_IFREG) return SYS_FILETYPE_REGULAR;
      if (mode == S_IFDIR) return SYS_FILETYPE_DIRECTORY;
      return SYS_FILETYPE_OTHER;
    }
    return ret;
}

/* 
 * Wrapper functions for low-level I/O routines - use the 64 bit
 * version if available, else revert to the 32 bit versions.
 */

off64_t 
lseek64_w(int fd, off64_t offset, int whence)
{
    if (lseek64_ptr != 0) {
        return (*lseek64_ptr)(fd, offset, whence);
    } else {
        if ((off_t) offset != offset) {
	    errno = EOVERFLOW;
	    return -1;
	}
	return lseek(fd, offset, whence);
    }
}

int 
ftruncate64_w(int fd, off64_t length) 
{
    if (ftruncate64_ptr != 0) {
        return (*ftruncate64_ptr)(fd, length);
    }
    if ((off_t)length != length) {
	errno = EOVERFLOW;
        return -1;
    }
    return ftruncate(fd, length);
}

int 
open64_w(const char *path, int oflag, int mode)
{
    int result;

    if (open64_ptr != 0) {
        /* 
	 * Using O_LARGEFILE with _open is the same as _open64.
	 * We use this because on green threads we have our own version
	 * of open, but no _open64.  So, rather than implement _open64
	 * we use O_LARGEFILE.
	 */
        result = open(path, oflag | O_LARGEFILE, mode);
    } else {
	result = open(path, oflag, mode);
    }
    if (result != -1) {
	/* If the open succeeded, the file might still be a directory */
	int st_mode;
	if (sysFfileMode(result, &st_mode) != -1) {
	    if ((st_mode & S_IFMT) == S_IFDIR) {
		errno = EISDIR;
		close(result);
		return -1;
	    }
	} else {
	    close(result);
	    return -1;
	}
    } 
    return result;
}  

void setFPMode(void)
{
#ifdef i386
    asm("	pushl $575");
    asm("	fldcw (%esp)");
    asm("	popl %eax");
#endif
#if defined(__linux__) && defined(__mc68000__)
    asm("     fmovel #0x80,%fpcr");
#endif
}
