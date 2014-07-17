#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_visCPP.hpp	1.45 03/01/23 12:28:18 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds compiler-dependent includes,
// globally used constants & types, class (forward)
// declarations and a few frequently used utility functions.

# include <ctype.h>
# include <string.h>
# include <stdarg.h>
# include <stdlib.h>
# include <stddef.h>// for offsetof
# include <io.h>    // for stream.cpp
# include <float.h> // for _isnan
# include <stdio.h> // for va_list
# include <time.h>
# include <fcntl.h>

// Compiler-specific primitive types
typedef	unsigned __int8  uint8_t;
typedef	unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

#ifdef _WIN64
typedef unsigned __int64 uintptr_t;
#else
typedef unsigned int uintptr_t;
#endif
typedef	signed   __int8  int8_t;
typedef signed   __int16 int16_t;
typedef signed   __int32 int32_t;
typedef signed   __int64 int64_t;
#ifdef _WIN64
typedef signed   __int64 intptr_t;
typedef signed   __int64 ssize_t;
#else
typedef signed   int intptr_t;
typedef signed   int ssize_t;
#endif

//----------------------------------------------------------------------------------------------------
// Additional Java basic types

typedef unsigned  char   jubyte;
typedef	unsigned __int16 jushort;
typedef unsigned __int32 juint;
typedef unsigned __int64 julong;

//----------------------------------------------------------------------------------------------------
// Special (possibly not-portable) casts
// Cast floats into same-size integers and vice-versa w/o changing bit-pattern

inline jint    jint_cast   (jfloat  x)           { return *(jint*   )&x; }
inline jlong   jlong_cast  (jdouble x)           { return *(jlong*  )&x; }

inline jfloat  jfloat_cast (jint    x)           { return *(jfloat* )&x; }
inline jdouble jdouble_cast(jlong   x)           { return *(jdouble*)&x; }

//----------------------------------------------------------------------------------------------------
// unaligned access
inline void store_unaligned_ushort(jushort *dst, jushort val) { *dst = val;  }
inline void store_unaligned_uint(juint *dst, juint val)       { *dst = val;  }
inline jushort load_unaligned_ushort(jushort *dst)            { return *dst; }
inline juint load_unaligned_uint(juint *dst)                  { return *dst; }


//----------------------------------------------------------------------------------------------------
// Non-standard stdlib-like stuff:
inline int strcasecmp(const char *s1, const char *s2) { return stricmp(s1,s2); }


//----------------------------------------------------------------------------------------------------
// Debugging

#if _M_IA64
extern "C" void breakpoint();
#define BREAKPOINT ::breakpoint()
#else
#define BREAKPOINT __asm { int 3 }
#endif

//----------------------------------------------------------------------------------------------------
// Checking for nanness

inline int g_isnan(jfloat  f)                    { return _isnan(f); }
inline int g_isnan(jdouble f)                    { return _isnan(f); }

//----------------------------------------------------------------------------------------------------
// Checking for finiteness

inline int g_isfinite(jfloat  f)                 { return _finite(f); }
inline int g_isfinite(jdouble f)                 { return _finite(f); }

//----------------------------------------------------------------------------------------------------
// Constant for jlong (specifying an long long constant is C++ compiler specific)

// Build a 64bit integer constant on with Visual C++
#define CONST64(x)  (x ## i64)

const jlong min_jlong = CONST64(0x8000000000000000);
const jlong max_jlong = CONST64(0x7fffffffffffffff);

//----------------------------------------------------------------------------------------------------
// Miscellaneous

inline int vsnprintf(char* buf, size_t count, const char* fmt, va_list argptr) {
  return _vsnprintf(buf, count, fmt, argptr);
}

//------------------------------------------------------------------------------------------------------
// Platform-specific memory areas
// (could potentially be to a more appropriate place - we need some platform-specific constants mechanism)
//
// Size of interpreter. Simply increase if too small (assembler will fail with assertion (in debug mode) if too small)
// On 4/13/98: The size of the Intel interpreter was: 34212 bytes (in debug mode).
// Run with +PrintInterpreter for getting the VM to print out the size.    

const int Interpreter_Code_Size = 100*1024;

#pragma warning( disable : 4100 ) // unreferenced formal parameter
#pragma warning( disable : 4127 ) // conditional expression is constant
#pragma warning( disable : 4514 ) // unreferenced inline function has been removed
#pragma warning( disable : 4244 ) // possible loss of data
#pragma warning( disable : 4512 ) // assignment operator could not be generated
#pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union (needed in windows.h)
#pragma warning( disable : 4511 ) // copy constructor could not be generated
#pragma warning( disable : 4291 ) // no matching operator delete found; memory will not be freed if initialization thows an exception

// Portability macros
#define PRAGMA_INTERFACE      
#define PRAGMA_IMPLEMENTATION
#define PRAGMA_IMPLEMENTATION_(arg)
#define VALUE_OBJ_CLASS_SPEC    : public _ValueObj

// Formatting.
#define FORMAT64_MODIFIER "I64"
