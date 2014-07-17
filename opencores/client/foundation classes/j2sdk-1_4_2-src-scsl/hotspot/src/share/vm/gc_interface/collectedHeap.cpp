#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)collectedHeap.cpp	1.8 03/01/23 12:04:17 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_collectedHeap.cpp.incl"

// Memory state functions.

#ifndef PRODUCT
void CollectedHeap::check_for_bad_heap_word_value(HeapWord* addr, size_t size) {
  if (CheckMemoryInitialization && ZapUnusedHeapArea) {
    for (size_t slot = 0; slot < size; slot += 1) {
      assert((*(intptr_t*) (addr + slot)) != ((intptr_t) badHeapWordVal),
             "Found badHeapWordValue in post-allocation check");
    }
  }
}

void CollectedHeap::check_for_non_bad_heap_word_value(HeapWord* addr, size_t size)
 {
  if (CheckMemoryInitialization && ZapUnusedHeapArea) {
    for (size_t slot = 0; slot < size; slot += 1) {
      assert((*(intptr_t*) (addr + slot)) == ((intptr_t) badHeapWordVal),
             "Found non badHeapWordValue in pre-allocation check");
    }
  }
}
#endif // PRODUCT

#ifdef ASSERT
void CollectedHeap::check_for_valid_allocation_state() {
  Thread *thread = Thread::current();
  // How to choose between a pending exception and a potential
  // OutOfMemoryError?  Don't allow pending exceptions.
  // This is a VM policy failure, so how do we exhaustively test it?
  assert(!thread->has_pending_exception(),
         "shouldn't be allocating with pending exception");
  if (StrictSafepointChecks) {
    assert(thread->allow_allocation(),
           "Allocation done by thread for which allocation is blocked "
           "by No_Allocation_Verifier!");
    // Allocation of an oop can always invoke a safepoint,
    // hence, the true argument
    thread->check_for_valid_safepoint_state(true);
  }
}
#endif

HeapWord* CollectedHeap::allocate_from_tlab_slow(Thread* thread, size_t size) {
  // A large object that does not fit in the TLAB - allocated in the shared
  // space.
  if (size > ((thread->tlab().size() >>
	      ThreadLocalAllocBuffer::log2_fragmentation_ratio()))) {
    assert(thread->tlab().invariants(), "TLAB integrity violated");
    // Return NULL so that the caller allocates in the shared space.
    return NULL;
  }

  // This is a "small" object that does not fit in the TLAB: create a new TLAB
  // To minimize fragmentation, the last TLAB may be smaller than the rest

  // Make sure the TLAB is at least big enough to hold an int[0],
  // since we are about to reset the TLAB.
  size_t aligned_header_size =
    align_object_size(typeArrayOopDesc::header_size(T_INT));
  size_t new_tlab_size =
    thread->tlab().compute_new_size(size + aligned_header_size);
  thread->tlab().reset();

  HeapWord* obj = NULL;

  // create a new tle
  if (new_tlab_size == 0) {
    return NULL;
  }
  // Allocate the entire TLAB...
  obj = Universe::heap()->allocate_new_tlab(new_tlab_size);
  if (obj == NULL) {
    return NULL;
  }
  if (ZeroTLAB) {		
    // ..and clear it.
    Memory::set_words(obj, new_tlab_size);
  } else {			
    // ...and clear just the allocated object.
    Memory::set_words(obj, size);
  }
  // end is initialized in a way that allows some room for the filler array
  thread->tlab().initialize(thread,
                            obj,
                            obj + size,
                            obj + new_tlab_size - aligned_header_size,
                            new_tlab_size);
  
  assert(thread->tlab().invariants(), "TLAB integrity violated");
  return obj;
}

HeapWord* CollectedHeap::allocate_new_tlab(size_t size) {
  guarantee(false, "thread-local allocation buffers not supported");
  return NULL;
}

void CollectedHeap::fill_all_tlabs() {
  assert(UseTLAB, "should not reach here");
  // See note in ensure_parseability() below.
  assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(),
	 "should only fill tlabs at safepoint");
  for(JavaThread *thread = Threads::first(); thread; thread = thread->next()) {
     thread->tlab().reset();
  }
}

void CollectedHeap::ensure_parseability() {
  // The second disjunct in the assertion below makes a concession
  // for the start-up verification done while the VM is being
  // created. Callers be careful that you know that mutators
  // aren't going to interfere -- for instance, this is permissible
  // if we are still single-threaded and have either not yet
  // started allocating (nothing much to verify) or we have
  // started allocating but are now a full-fledged JavaThread
  // (and have thus made our TLAB's) available for filling.
  assert(SafepointSynchronize::is_at_safepoint() ||
         !is_init_completed(), 
         "Should only be called at a safepoint or at start-up"
         " otherwise concurrent mutator activity may make heap "
         " unparseable again");
  if (UseTLAB) {
    fill_all_tlabs();
  }
}

