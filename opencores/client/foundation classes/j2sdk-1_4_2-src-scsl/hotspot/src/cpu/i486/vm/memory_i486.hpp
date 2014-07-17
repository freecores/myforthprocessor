#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memory_i486.hpp	1.12 03/01/23 10:55:12 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

  // Inline functions for memory copy and fill:

  static void pd_copy_words(HeapWord* from, HeapWord* to, size_t count) {
    memcpy(to, from, count * HeapWordSize);
  }

  static void pd_copy_words_aligned(HeapWord* from, HeapWord* to, size_t count) {
    memcpy(to, from, count * HeapWordSize);
  }

  static void pd_copy_words_overlapping(HeapWord* from, HeapWord* to, size_t count) {
    memmove(to, from, count * HeapWordSize);
  }

  static void pd_copy_oops_overlapping_atomic(oop* from, oop* to, size_t count) {
    // Do better than this: inline memmove body  NEEDS CLEANUP
    if (from > to) {
      while (count-- > 0) {
        *to++ = *from++; // copy forwards
      }
    } else {
      from = &from[count];
      to = &to[count];
      while (count-- > 0) {
        *--to = *--from; // copy backwards
      }
    }
  }

  static void pd_copy_words_aligned_overlapping(HeapWord* from, HeapWord* to, size_t count) {
    memmove(to, from, count * HeapWordSize);
  }

  static void pd_set_words(HeapWord* tohw, size_t count, juint value) {
    juint* to = (juint*)tohw;
    // This should almost always be optimized away.
    count *= (HeapWordSize / sizeof(juint));
    while (count-- > 0) {
      *to++ = value;
    }
  }

  static void pd_copy_bytes_overlapping(void* from, void* to, size_t count) {
    memmove(to, from, count);
  }

  static void pd_set_words(jint* to, size_t count, jint value) {
    while (count-- > 0) {
      *to++ = value;
    }
  }
