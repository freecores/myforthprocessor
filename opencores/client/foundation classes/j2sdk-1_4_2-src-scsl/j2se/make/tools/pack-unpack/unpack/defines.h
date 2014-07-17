/*
 * @(#)defines.h	1.8 03/04/02
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
// random definitions

#ifdef _MSC_VER 
#include<windows.h>
#endif

#ifndef FULL
#define FULL 1 /* Adds <500 bytes to the zipped final product. */
#endif

#if FULL  // define this if you want debugging and/or compile-time attributes
#define IF_FULL(x) x
#else
#define IF_FULL(x) /*x*/
#endif

#ifdef PRODUCT
#define IF_PRODUCT(xxx) xxx
#define NOT_PRODUCT(xxx)
#define assert(p) (1)
#define printcr false &&
#else
#define IF_PRODUCT(xxx)
#define NOT_PRODUCT(xxx) xxx
#define assert(p) ((p) || (assert_failed(#p), 1))
extern "C" void breakpoint();
extern void assert_failed(const char*);
#define BREAK (breakpoint())
#endif

#define lengthof(array) (sizeof(array)/sizeof(array[0]))

#define NEW(T, n) (T*) must_malloc(sizeof(T)*(n));


// bytes and byte arrays

typedef unsigned int uint;


// uLong needs to 32bit for historical zip reasons.
#ifdef _LP64
#ifdef _sparc
typedef uint uLong;
#else
typedef unsigned long uLong;
#endif
#else
typedef unsigned long uLong;
#endif

#ifdef _MSC_VER 
typedef LONGLONG 	jlong;
typedef DWORDLONG 	julong;
#define MKDIR(dir) 	mkdir(dir)
#define getpid() 	_getpid()
#define PATH_MAX 	MAX_PATH
#define dup2(a,b)	_dup2(a,b)
#define strcasecmp(s1, s2) _stricmp(s1,s2)
#define tempname	_tempname
#else
typedef signed char byte;
typedef long long jlong;
typedef long long unsigned julong;
#define MKDIR(dir) mkdir(dir, 0777);
#endif

#ifdef OLDCC
typedef int bool;
enum { false, true };
#endif

#define null (0)


