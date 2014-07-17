#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_solaris_i486.inline.hpp	1.8 03/01/23 11:09:47 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

inline jint atomic::add(jint add_value, jint* dest) {
  return (*os::atomic_increment_func)(add_value, dest);
}

inline jint atomic::exchange(jint exchange_value, jint* dest) {
  return (*os::atomic_exchange_func)(exchange_value, dest);
}

inline jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  return (*os::atomic_compare_and_exchange_func)(exchange_value, dest, compare_value);
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
  return (*os::atomic_compare_and_exchange_long_func)(exchange_value, dest, compare_value);
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
  return (intptr_t)(*os::atomic_increment_func)((jint)add_value, (jint*)dest);
}

inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  return (intptr_t)(*os::atomic_exchange_func)((jint)exchange_value, (jint*)dest);
}

inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  return (*os::atomic_compare_and_exchange_func)((jint)exchange_value, (jint*)dest, (jint)compare_value);
}

inline void atomic::membar() {
  (*os::atomic_membar_func)();
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}

inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
