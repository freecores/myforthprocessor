#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memory_sparc.hpp	1.12 03/01/23 11:01:48 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// Inline functions for memory copy and fill:

static void pd_copy_words(HeapWord* from, HeapWord* to, size_t count) {
  switch (count) {
  case 8:       to[7] = from[7];                // and fall through
  case 7:       to[6] = from[6];                // and fall through
  case 6:       to[5] = from[5];                // and fall through
  case 5:       to[4] = from[4];                // and fall through
  case 4:       to[3] = from[3];                // and fall through
  case 3:       to[2] = from[2];                // and fall through
  case 2:       to[1] = from[1];                // and fall through
  case 1:       to[0] = from[0];                // and fall through
  case 0:       break;
  default:      memcpy(to, from, count * HeapWordSize);
                break;
  }
}

static void pd_copy_words_aligned(HeapWord* from, HeapWord* to, size_t count) {
  pd_copy_words(from, to, count);
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

static void pd_copy_words_aligned_overlapping(HeapWord* from,
					      HeapWord* to, size_t count) {
  memmove(to, from, count * HeapWordSize);
}

// Fills a region starting at "tohw" and extending for "count" words
// with copies of "value".
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

