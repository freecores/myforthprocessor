#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_win32_ia64.inline.hpp	1.4 03/01/23 11:11:19 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 


inline void atomic::increment(jint* dest) {
  (void)add(1, dest);
}

inline void atomic::decrement(jint* dest) {
  (void)add(-1, dest);
}

inline void atomic::increment_ptr(intptr_t* dest) {
  (void)add_ptr(1, dest);
}

inline void atomic::decrement_ptr(intptr_t* dest) {
  (void)add_ptr(-1, dest);
}

inline void atomic::store(jint store_value, volatile jint* dest) {
  *dest = store_value;
}

inline void atomic::store_ptr(intptr_t store_value, volatile intptr_t* dest) {
  *dest = store_value;
}

inline void atomic::prefetch_read(void *loc, intx interval) {
  // no-op
}

inline void atomic::prefetch_write(void *loc, intx interval) {
  // no-op
}
