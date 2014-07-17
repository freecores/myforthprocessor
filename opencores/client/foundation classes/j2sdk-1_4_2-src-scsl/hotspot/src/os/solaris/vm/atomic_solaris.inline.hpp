#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)atomic_solaris.inline.hpp	1.11 03/01/23 11:04:54 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Implementation of class atomic

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
