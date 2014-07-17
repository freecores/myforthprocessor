#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic.hpp	1.11 03/01/23 12:21:25 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class atomic : AllStatic {
 public:
  // Support for atomic 32-bit word operations

  // Atomically add to a 32-bit location, return updated value
  static jint add(jint add_value, jint* dest);
  // Atomically increment a 32-bit location
  static void increment(jint* dest);
  // Atomically decrement a 32-bit location
  static void decrement(jint* dest);
  // Performs atomic exchange of *dest with exchange_value. Returns old prior value of *dest.
  static jint exchange(jint exchange_value, jint* dest);
  // Atomically store to a 32-bit location
  static void store(jint store_value, volatile jint* dest);
  // Performs atomic compare of *dest and compare_value, and exchanges *dest with exchange_value
  // if the comparison succeded. Returns prior value of *dest.
  static jint compare_and_exchange(jint exchange_value, jint* dest, jint compare_value);
  // Similar to above, but works on bytes.
  static jbyte compare_and_exchange_byte(jbyte exchange_value,
					 jbyte* dest,
                                         jbyte compare_value);

  // Performs atomic compare of *dest and compare_value, and exchanges *dest with exchange_value
  // if the comparison succeded. Returns prior value of *dest.
  static jlong compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value);

  // Support for atomic machine word operations

  // Atomically add to a machine location, return updated value
  static intptr_t add_ptr(intptr_t add_value, intptr_t* dest);
  // Atomically increment a machine location
  static void increment_ptr(intptr_t* dest);
  // Atomically decrement a machine location
  static void decrement_ptr(intptr_t* dest);
  // Performs atomic exchange of *dest with exchange_value. Returns prior value of *dest.
  static intptr_t exchange_ptr(intptr_t exchange_value, intptr_t* dest);
  // Atomically store to a machine location
  static void store_ptr(intptr_t store_value, volatile intptr_t* dest);
  // Performs atomic compare of *dest and compare_value, and exchanges *dest with exchange_value
  // if the comparison succeded. Returns prior value of *dest.
  static intptr_t compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value);

  // Performs both read barrier and write barrier
  static void membar();

  // Technically not atomic, but inlined with the atomics
  static uintptr_t raw_thread_id();

  // Prefetch

  enum prefetch_style {
    prefetch_do_none,  // Do no prefetching
    prefetch_do_read,  // Do read prefetching
    prefetch_do_write  // Do write prefetching
  };

  // Prefetch anticipating read; must not fault, semantically a no-op
  static void prefetch_read(void* loc, intx interval);

  // Prefetch anticipating write; must not fault, sematically a no-op
  static void prefetch_write(void* loc, intx interval);
};

class AtomicCounter VALUE_OBJ_CLASS_SPEC {
public:
  AtomicCounter(jint* dest) : _dest(dest) {
    atomic::increment(_dest);
  }
  ~AtomicCounter() {
    atomic::decrement(_dest);
  }
private:
  jint* _dest;
};

class AtomicPtrCounter VALUE_OBJ_CLASS_SPEC {
public:
  AtomicPtrCounter(intptr_t* dest) : _dest(dest) {
    atomic::increment_ptr(_dest);
  }
  ~AtomicPtrCounter() {
    atomic::decrement_ptr(_dest);
  }
private:
  intptr_t* _dest;
};

