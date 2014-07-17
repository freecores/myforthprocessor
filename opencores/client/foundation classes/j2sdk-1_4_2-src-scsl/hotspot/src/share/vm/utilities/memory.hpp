#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memory.hpp	1.15 03/01/23 12:28:33 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

class Memory: AllStatic {
 public:
  // Efficient block copy, word-aligned, non-overlapping, atomic on each word:
  static void copy_words(HeapWord* from, HeapWord* to, size_t count) {
    #ifdef ASSERT
      assert_params_ok(from, to, count);
      assert_non_overlapping(from, to, count);
    #endif
    //memcpy(to, from, count * HeapWordSize);
    pd_copy_words(from, to, count);
  }

  // Same as copy_words, but caller guarantees alignment according to ALIGN_ALL_OBJECTS:
  static void copy_words_aligned(HeapWord* from, HeapWord* to, size_t count) {
    #ifdef ASSERT
      assert_non_zero(count);
      assert_params_ok(from, to, count);
      assert_non_overlapping(from, to, count);
      assert_params_aligned (from, to, count);
    #endif
    //memcpy(to, from, count * HeapWordSize);
    pd_copy_words_aligned(from, to, count);
  }

  // Same as copy_words, but allows overlap:
  static void copy_words_overlapping(HeapWord* from, HeapWord* to, size_t count) {
    #ifdef ASSERT
      assert_params_ok(from, to, count);
    #endif
    //memmove(to, from, count * HeapWordSize);  // memmove supports overlap while memcpy does not
    pd_copy_words_overlapping(from, to, count);
  }

  // Block copy, word-aligned, overlapping, guaranteed word atomic
  static void copy_oops_overlapping_atomic(oop* from, oop* to, size_t count) {
    #ifdef ASSERT
      assert_non_zero(count);
      assert_params_ok((HeapWord*) from, (HeapWord*) to, count * HeapWordsPerOop);
    #endif
    pd_copy_oops_overlapping_atomic(from, to, count);
  }

  // Same as copy_words_aligned, but allows overlap
  static void copy_words_aligned_overlapping(HeapWord* from,
					     HeapWord* to, size_t count) {
    #ifdef ASSERT
      assert_non_zero(count);
      assert_params_ok(from, to, count);
      assert_params_aligned (from, to, count);
    #endif
    //memmove(to, from, count * HeapWordSize);  // memmove supports overlap while memcpy does not
    pd_copy_words_aligned_overlapping(from, to, count);
  }

  // Move some bytes, not guaranteed word atomic
  static void copy_bytes_overlapping(void* from, void* to, size_t count) {
    #ifdef ASSERT
      assert_non_zero(count);
    #endif
    //memmove(to, from, count);
    pd_copy_bytes_overlapping(from, to, count);
  }

  // Initialize words to the given value.
  static void set_words(HeapWord* to, size_t count, juint value = 0) {
    assert(sizeof(HeapWord) == sizeof(char*), "just checking");
    pd_set_words(to, count, value);
  }

  // Copy aligned data to lower or disjoint memory with forward traversal of source array
  inline static void copy_words_aligned_lower(intptr_t *dest, intptr_t *src, int bytes) {
    // count is in bytes to check alignment of size
    uint length = round_to(bytes,BytesPerWord)>>LogBytesPerWord;
    assert( round_to(intptr_t(dest) ,BytesPerWord) == intptr_t(dest), "Destination must be aligned");
    assert( round_to(intptr_t(src ) ,BytesPerWord) == intptr_t(src ), "Source must be aligned");
    assert( round_to(bytes          ,BytesPerWord) == bytes         , "length must be aligned");
    assert( (dest < src) || (dest + length > src), "Do not overwrite source data while copying");
    for( ; length != 0; --length ) {
      *dest++ = *src++;
    }
  }

  // Copy aligned data to higher or disjoint memory with backward traversal of source array
  inline static void copy_words_aligned_higher(intptr_t *dest, intptr_t *src, int bytes) {
    // count is in bytes to check alignment of size
    uint length = round_to(bytes,BytesPerWord)>>LogBytesPerWord;
    assert( round_to(intptr_t(dest) ,BytesPerWord) == intptr_t(dest), "Destination must be aligned");
    assert( round_to(intptr_t(src ) ,BytesPerWord) == intptr_t(src ), "Source must be aligned");
    assert( round_to(bytes          ,BytesPerWord) == bytes         , "length must be aligned");
    assert( (src < dest) || (src > dest + length), "Do not overwrite source data while copying");
    src  += length - 1;
    dest += length - 1;
    for( ; length != 0; --length ) {
      *dest-- = *src--;
    }
  }

 private:
  static bool params_non_overlapping(HeapWord* from, HeapWord* to, size_t count) {
    if (from < to) {
      return pointer_delta(to, from) >= count;
    }
    return pointer_delta(from, to) >= count;
  }

#ifdef ASSERT
  // These routines raise a fatal if they detect a problem.

  static void assert_params_ok(HeapWord* from, HeapWord* to, size_t count) {
    if (mask_bits((uintptr_t)from, right_n_bits(LogBytesPerWord)) != 0)
      basic_fatal("not word aligned");
    if (mask_bits((uintptr_t)to  , right_n_bits(LogBytesPerWord)) != 0)
      basic_fatal("not word aligned");
  }

  static void assert_params_aligned(HeapWord* from, HeapWord* to, size_t count) {
    if (!ALIGN_ALL_OBJECTS)
      return;
    if (mask_bits((uintptr_t)from, right_n_bits(LogBytesPerLong)) != 0)
      basic_fatal("not doubleword aligned");
    if (mask_bits((uintptr_t)to, right_n_bits(LogBytesPerLong)) != 0)
      basic_fatal("not doubleword aligned");
    // Note: count might be odd, but it is OK to copy the trailing extra word.
  }

  static void assert_non_overlapping(HeapWord* from, HeapWord* to, size_t count) {
    if (!params_non_overlapping(from, to, count))
      basic_fatal("source and dest overlap");
  }

  static void assert_non_zero(size_t count) {
    if (count == 0) {
      basic_fatal("count must be non-zero");
    }
  }

#endif

  // The following header contains implementations of the above functions:
  #include "incls/_memory_pd.hpp.incl"
};
