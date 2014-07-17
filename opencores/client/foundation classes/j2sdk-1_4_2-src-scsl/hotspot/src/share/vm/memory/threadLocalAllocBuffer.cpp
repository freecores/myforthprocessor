#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLocalAllocBuffer.cpp	1.31 03/01/23 12:10:35 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Thread-Local Edens support

# include "incls/_precompiled.incl"
# include "incls/_threadLocalAllocBuffer.cpp.incl"

// static member initialization
size_t ThreadLocalAllocBuffer::_log2_fragmentation_ratio = 0;
size_t ThreadLocalAllocBuffer::_max_size = 0;


// Fills the current tlab with a dummy filler array to create
// an illusion of a contiguous Eden and clears the tlab info
void ThreadLocalAllocBuffer::reset() {
  if (end() != NULL) {
    assert(top() <= end(), "pointers out of order");
    const size_t aligned_header_size =
      align_object_size(typeArrayOopDesc::header_size(T_INT));
    // TLAB's never allocate the last aligned_header_size 
    // so they can always fill with an array.
    HeapWord* tlab_end = end() + aligned_header_size;
    typeArrayOop filler_oop = (typeArrayOop) top();
    filler_oop->set_mark();
    filler_oop->set_klass(Universe::intArrayKlassObj());
    const size_t array_length =
      pointer_delta(tlab_end, top()) - typeArrayOopDesc::header_size(T_INT);
    filler_oop->set_length((int)(array_length * (HeapWordSize/sizeof(jint))));

#ifdef ASSERT
    HeapWord* elt_words =
      (((HeapWord*)filler_oop) + typeArrayOopDesc::header_size(T_INT));
    Memory::set_words(elt_words, array_length, 0xDEAFBABE);
#endif
       
    if(PrintTLAB) {
      static size_t total_used;
      static size_t total_unused;
      const size_t used = pointer_delta(top(), start());
      const size_t unused = aligned_header_size + array_length;
      // This fragmentation figure gets messed up 
      // by explicit System.gc() calls,
      // which can find a lot of TLABs only partially used,
      // through no fault of theirs.
      // The total_* accumulators are also liable to overflow.
      total_used += used;
      total_unused += unused;
      const double fragmentation =
        (double) total_unused / (double) (total_used + total_unused);
      tty->print_cr("reset TLAB:  thread: " INTPTR_FORMAT
                    "  size: " SIZE_FORMAT "KB"
                    "  unused: " SIZE_FORMAT "B"
                    "  Total fragmentation %f", 
                    thread(),
                    size() / K * HeapWordSize, 
                    unused * HeapWordSize,
                    fragmentation);
    }
    
    set_thread(NULL);
    set_start(NULL);
    set_end(NULL);
    set_top(NULL);
    reset_original_size();
    // _has_terminated is irrelevant and cleared on re-initialization
       
  }
  assert(start() == NULL && end() == NULL && top() == NULL,
         "TLAB must be reset");
}

// Adjusts the size of TLABs as a function of the number of threads, etc.
// If the TLAB max_size gets bigger than can be allocated, that's fixed in 
// ThreadLocalAllocBuffer::compute_new_size, and if that fails, you just 
// can't get a TLAB, so you will collect.
void ThreadLocalAllocBuffer::adjust_max_size() {
  assert(ResizeTLAB, "Must not call if ResizeTLAB=false");
  const size_t heap_word_tlab_capacity =
    Universe::heap()->tlab_capacity() / HeapWordSize;
  const size_t tlab_words_per_ratio =
    align_object_size(heap_word_tlab_capacity / MaxTLABRatio);
  // For highly threaded applications, allow at least a certain fraction
  // of threads to share the eden.
  // The "+1" prevents a divide by zero if there aren't many threads.
  const size_t satisfiable_threads =
    (Threads::number_of_non_daemon_threads() / TLABThreadRatio) + 1;
  const size_t tlab_words_per_thread =
    align_object_size(heap_word_tlab_capacity / satisfiable_threads);
  // Don't allow TLABs larger than the smaller of those constraints. 
  const size_t min_tlab_words = MIN2(tlab_words_per_ratio,
                                     tlab_words_per_thread);
  // Don't make it smaller than the user-requested TLAB size.
  const size_t max_tlab_words = MAX2(TLABSize / HeapWordSize, min_tlab_words);
  // TLABs can't be bigger than we can fill with a int[Integer.MAX_VALUE].
  // We expect to remove this restriction by enabling filling with multiple arrays.
  // If we compute that the reasonable way as
  //    header_size + ((sizeof(jint) * max_jint) / HeapWordSize)
  // we'll overflow on the multiply, so we do the divide first.
  // We actually lose a little by dividing first,
  // but that just makes the TLAB  somewhat smaller than the biggest array,
  // which is fine, since we'll be able to fill that.
  const size_t tlab_words_per_fill =
    align_object_size(typeArrayOopDesc::header_size(T_INT) +
                      sizeof(jint) *
                      ((juint) max_jint / (size_t) HeapWordSize));
  const size_t new_tlab_words =  MIN2(max_tlab_words, tlab_words_per_fill);
  if(PrintTLAB) {
    size_t old_max_tlab_words = max_size();
    tty->print_cr("[TLAB max size "
                  SIZE_FORMAT "KB -> "
                  "max(TLABSize: " SIZE_FORMAT "KB, "
                  "min("
                  "ratio: " SIZE_FORMAT "KB, "
                  "thread: " SIZE_FORMAT "KB, "
                  "fill: " SIZE_FORMAT "KB) "
                  "-> " SIZE_FORMAT "KB]",
		  old_max_tlab_words / K * HeapWordSize,
                  TLABSize / K,
                  tlab_words_per_ratio / K * HeapWordSize,
                  tlab_words_per_thread / K * HeapWordSize,
                  tlab_words_per_fill / K * HeapWordSize,
		  new_tlab_words / K * HeapWordSize);
  }
  set_max_size(new_tlab_words);
}

void ThreadLocalAllocBuffer::print() {
  tty->print("TLAB:  thread: " INTPTR_FORMAT
             "  size: " SIZE_FORMAT "KB"
             "  used: " SIZE_FORMAT "B",
             thread(), 
             size() / K * HeapWordSize,
             pointer_delta(top(), start()));
  tty->print_cr("  [" INTPTR_FORMAT ", " INTPTR_FORMAT ", " INTPTR_FORMAT ")",
                start(), top(), end());
}

void ThreadLocalAllocBuffer::verify() {
  HeapWord* p = start();
  HeapWord* t = top();
  HeapWord* prev_p = NULL;
  while (p < t) {
    oop(p)->verify();
    prev_p = p;
    p += oop(p)->size();
  }
  guarantee(p == top(), "end of last object must match end of space");
}

