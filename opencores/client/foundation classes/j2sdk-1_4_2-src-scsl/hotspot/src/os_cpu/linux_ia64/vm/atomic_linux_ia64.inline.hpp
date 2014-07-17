#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_linux_ia64.inline.hpp	1.5 03/01/23 11:09:19 JVM"
#endif
//
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

inline jint atomic::add(jint add_value, jint* dest) {
  jint sum;

  if (add_value == 1) {
    __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],1;;" : "=r" (sum) : "r" (dest) : "memory");
    ++sum;
  } else if (add_value == -1) {
    __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],-1;;" : "=r" (sum) : "r" (dest) : "memory");
    --sum;
  } else {
    jint compare_value;
    jint exchange_value;
    __asm__ volatile (  "mf;;0: ld4 %0=[%7];;add %1=%6,%3;;mov ar.ccv=%3;zxt4 %1=%4;;cmpxchg4.acq %1=[%7],%4,ar.ccv;;"
                        "cmp4.ne p15,p0=%3,%4;add %2=%6,%3;(p15) br.cond.spnt 0b;;sxt4 %2=%5;;"
                      : "=r" (compare_value), "=r" (exchange_value), "=r" (sum)
                      : "0" (compare_value), "1" (exchange_value), "2" (sum), "r" (add_value), "r" (dest)
                      : "p15", "memory");
  }
  return sum;
}

inline void atomic::increment(jint* dest) {
  jint exchange_value;
  __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void atomic::decrement(jint* dest) {
  jint exchange_value;
  __asm__ volatile ("mf;;fetchadd4.acq %0=[%1],-1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline jint atomic::exchange(jint exchange_value, jint* dest) {
  __asm__ volatile (  "mf;;xchg4 %0=[%2],%1;;sxt4 %0=%1;;"
                    : "=r" (exchange_value)
                    : "0" (exchange_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline void atomic::store(jint store_value, volatile jint* dest) {
  *dest = store_value;
}

inline jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  __asm__ volatile (  "mf;;zxt4 %1=%3;;mov ar.ccv=%3;;cmpxchg4.acq %0=[%4],%2,ar.ccv;;sxt4 %0=%2;;"
		    : "=r" (exchange_value), "=r" (compare_value)
		    : "0" (exchange_value), "1" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
  __asm__ volatile (  "mf;;mov ar.ccv=%2;;cmpxchg8.acq %0=[%3],%1,ar.ccv;;"
		    : "=r" (exchange_value)
		    : "0" (exchange_value), "r" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline intptr_t atomic::add_ptr(intptr_t add_value, intptr_t* dest) {
#if 1
  intptr_t sum;

  if (add_value == 1) {
    __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],1;;" : "=r" (sum) : "r" (dest) : "memory");
    ++sum;
  } else if (add_value == -1) {
    __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],-1;;" : "=r" (sum) : "r" (dest) : "memory");
    --sum;
  } else {
    intptr_t compare_value;
    intptr_t exchange_value;
    __asm__ volatile (  "mf;;0: ld8 %0=[%6];;add %1=%5,%3;mov ar.ccv=%3;;cmpxchg8.acq %1=[%6],%4,ar.ccv;;"
                        "cmp.ne p15,p0=%3,%4;add %2=%5,%3;(p15) br.cond.spnt 0b;;"
                      : "=r" (compare_value), "=r" (exchange_value), "=r" (sum)
                      : "0" (compare_value), "1" (exchange_value), "r" (add_value), "r" (dest)
                      : "p15", "memory");
  }
  return sum;
#else
  return (intptr_t)(*os::Linux::atomic_increment_ptr_func)(add_value, dest);
#endif
}

inline void atomic::increment_ptr(intptr_t* dest) {
  intptr_t exchange_value;
  __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline void atomic::decrement_ptr(intptr_t* dest) {
  intptr_t exchange_value;
  __asm__ volatile ("mf;;fetchadd8.acq %0=[%1],-1;;" : "=r" (exchange_value) : "r" (dest) : "memory");
}

inline intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  __asm__ volatile (  "mf;;xchg8 %0=[%2],%1;;"
                    : "=r" (exchange_value)
                    : "0" (exchange_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline void atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) {
  *dest = store_value;
}

inline intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  __asm__ volatile (  "mf;;mov ar.ccv=%2;;cmpxchg8.acq %0=[%3],%1,ar.ccv;;"
		    : "=r" (exchange_value)
		    : "0" (exchange_value), "r" (compare_value), "r" (dest)
                    : "memory");
  return exchange_value;
}

inline void atomic::membar() {
  __asm__ volatile ("mf;;");
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}

inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
