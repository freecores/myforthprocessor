#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_linux_i486.inline.hpp	1.12 03/03/06 09:50:17 JVM"
#endif
//
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

inline jint atomic::add(jint add_value, jint* dest) {
  jint addend = add_value;
  __asm__ volatile (  "lock;xaddl %0,(%2)"
		    : "=r" (addend)
		    : "0" (addend), "r" (dest)
		    : "memory");
  return addend + add_value;
}

inline void atomic::increment(jint* dest) {
  __asm__ volatile ("lock;incl (%0)" : : "r" (dest) : "memory");
}

inline void atomic::decrement(jint* dest) {
  __asm__ volatile ("lock;decl (%0)" : : "r" (dest) : "memory");
}

inline jint atomic::exchange(jint exchange_value, jint* dest) {
  __asm__ volatile (  "xchgl (%2),%0"
		    : "=r" (exchange_value)
		    : "0" (exchange_value), "r" (dest)
		    : "memory");
  return exchange_value;
}

inline void atomic::store(jint store_value, volatile jint* dest) {
  *dest = store_value;
}

inline jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  __asm__ volatile (  "lock;cmpxchgl %1,(%3)"
                    : "=a" (exchange_value)
                    : "r" (exchange_value), "a" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
  jlong old_value;
  __asm__ volatile (  "pushl %%ebx;mov 4+%1,%%ecx;mov %1,%%ebx;lock;cmpxchg8b (%3);popl %%ebx"
                    : "=A" (old_value)
                    : "o" (exchange_value), "A" (compare_value), "r" (dest)
                    : "%ebx", "%ecx", "memory");
  return old_value;
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
  return (intptr_t)atomic::add((jint)add_value, (jint*)dest);
}

inline void atomic::increment_ptr(intptr_t* dest) {
  atomic::increment((jint*)dest);
}

inline void atomic::decrement_ptr(intptr_t* dest) {
  atomic::decrement((jint*)dest);
}

inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  return (intptr_t)exchange((jint)exchange_value, (jint*)dest);
}

inline void atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) {
  *dest = store_value;
}

inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  return (intptr_t)atomic::compare_and_exchange((jint)exchange_value, (jint*)dest, (jint)compare_value);
}

inline void atomic::membar() {
  __asm__ volatile ("lock; addl $0, 0(%%esp)": : :"memory");
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}

inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
