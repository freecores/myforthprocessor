#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psOldGen.cpp	1.23 03/01/23 12:02:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_psOldGen.cpp.incl"

PSOldGen::PSOldGen(ReservedSpace rs, size_t initial_size,
                   size_t min_size, size_t max_size,
                   const char* gen_name, int level):
  _init_gen_size(initial_size), _min_gen_size(min_size),
  _max_gen_size(max_size)

{
  //
  // Basic memory initialization
  //

  if (!_virtual_space.initialize(rs, initial_size)) {
    vm_exit_during_initialization("Could not reserve enough space for "
                                  "object heap");
  }

  _reserved = MemRegion((HeapWord*)_virtual_space.low_boundary(),
                        (HeapWord*)_virtual_space.high_boundary());

  //
  // Object start stuff
  //

  _start_array.initialize(_reserved);

  //
  // Card table stuff
  //

  MemRegion cmr((HeapWord*)_virtual_space.low(),
		(HeapWord*)_virtual_space.high());
  Universe::heap()->barrier_set()->resize_covered_region(cmr);

  CardTableModRefBS* _ct = (CardTableModRefBS*)Universe::heap()->barrier_set();
  assert (_ct->kind() == BarrierSet::CardTableModRef, "Sanity");

  // Verify that the start and end of this generation is the start of a card.
  // If this wasn't true, a single card could span more than one generation,
  // which would cause problems when we commit/uncommit memory, and when we
  // clear and dirty cards.
  guarantee(_ct->is_card_aligned(_reserved.start()), "generation must be card aligned");
  if (_reserved.end() != Universe::heap()->reserved_region().end()) {
    // Don't check at the very end of the heap as we'll assert that we're probing off
    // the end if we try.
    guarantee(_ct->is_card_aligned(_reserved.end()), "generation must be card aligned");
  }

  //
  // ObjectSpace stuff
  //

  _object_space = new MutableSpace();
  
  if (_object_space == NULL)
    vm_exit_during_initialization("Could not allocate an old gen space");

  object_space()->initialize(cmr, true);

  _object_mark_sweep = new PSMarkSweepDecorator(_object_space, start_array(), MarkSweepDeadRatio);

  if (_object_mark_sweep == NULL)
    vm_exit_during_initialization("Could not complete allocation of old generation");

  // Generation Counters, generation 'level', 1 subspace
  _gen_counters = new GenerationCounters(PERF_GC, gen_name, level, 1, &_virtual_space);
  _space_counters = new SpaceCounters(_gen_counters->name_space(), gen_name,
                                      0, _virtual_space.reserved_size(),
                                      _object_space);
}

void PSOldGen::precompact() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // Reset start array first.
  _start_array.reset();

  object_mark_sweep()->precompact();

  // Now compact the young gen
  heap->young_gen()->precompact();
}

void PSOldGen::adjust_pointers() {
  object_mark_sweep()->adjust_pointers();
}

void PSOldGen::compact() {
  object_mark_sweep()->compact(ZapUnusedHeapArea);
}

size_t PSOldGen::contiguous_available() const {
  return object_space()->free_in_bytes() + _virtual_space.uncommitted_size();
}

// Allocation. We report all successful allocations to the size policy
// Note that the perm gen does not use this method, and should not!
HeapWord* PSOldGen::allocate(size_t word_size, bool is_large_noref, bool is_tlab) {
  assert_locked_or_safepoint(Heap_lock);
  HeapWord* res = allocate_noexpand(word_size, is_large_noref, is_tlab);

  if (res == NULL) {
    res = expand_and_allocate(word_size, is_large_noref, is_tlab);
  }

  // Allocations in the old generation need to be reported
  if (res != NULL) {
    ParallelScavengeHeap::size_policy()->tenured_allocation(word_size);
  }

  return res;
}

HeapWord* PSOldGen::expand_and_allocate(size_t word_size, bool is_large_noref, bool is_tlab) {
  assert(!is_tlab, "TLAB's are not supported in PSOldGen");
  expand(word_size*HeapWordSize);
  return allocate_noexpand(word_size, is_large_noref, is_tlab);
}

HeapWord* PSOldGen::expand_and_cas_allocate(size_t word_size) {
  expand(word_size*HeapWordSize);
  return cas_allocate_noexpand(word_size);
}

void PSOldGen::expand(size_t bytes) {
  MutexLocker x(ExpandHeap_lock);
  size_t aligned_bytes  = ReservedSpace::page_align_size_up(bytes);
  size_t aligned_expand_bytes = 
                          ReservedSpace::page_align_size_up(MinHeapDeltaBytes);
  bool success = false;
  if (aligned_expand_bytes > aligned_bytes) {
    success = expand_by(aligned_expand_bytes);
  }
  if (!success) {
    success = expand_by(aligned_bytes);
  }
  if (!success) {
    expand_to_reserved();
  }
  if (GC_locker::is_active()) {
    // Tell the GC locker that we had to expand the heap
    GC_locker::heap_expanded();
  }
}

bool PSOldGen::expand_by(size_t bytes) {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);
  bool result = _virtual_space.expand_by(bytes);
  if (result) {
    post_resize();
    if (UsePerfData) {
      _space_counters->update_capacity();
      _gen_counters->update_all();
    }
  }

  if (result && Verbose && PrintGC) {
    size_t new_mem_size = _virtual_space.committed_size();
    size_t old_mem_size = new_mem_size - bytes;
    gclog_or_tty->print_cr("Expanding %s from " SIZE_FORMAT "K by " 
                                       SIZE_FORMAT "K to " 
                                       SIZE_FORMAT "K",
                    name(), old_mem_size/K, bytes/K, new_mem_size/K);
  }

  return result;
}

void PSOldGen::expand_to_reserved() {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);
  size_t remaining_bytes = _virtual_space.uncommitted_size();
  if (remaining_bytes > 0) {
    bool success = expand_by(remaining_bytes);
    assert(success, "grow to reserved failed");
  }
}

void PSOldGen::shrink(size_t bytes) {
  assert_lock_strong(ExpandHeap_lock);
  assert_locked_or_safepoint(Heap_lock);
  size_t size = ReservedSpace::page_align_size_down(bytes);
  if (size > 0) {
    assert_lock_strong(ExpandHeap_lock);
    _virtual_space.shrink_by(bytes);
    post_resize();

    if (Verbose && PrintGC) {
      size_t new_mem_size = _virtual_space.committed_size();
      size_t old_mem_size = new_mem_size - bytes;
      gclog_or_tty->print_cr("Shrinking %s from " SIZE_FORMAT "K by " 
                                         SIZE_FORMAT "K to " 
                                         SIZE_FORMAT "K",
                      name(), old_mem_size/K, bytes/K, new_mem_size/K);
    }
  }
}

void PSOldGen::resize(size_t desired_free_space) {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  const size_t alignment = heap->min_alignment();

  const size_t size_before = _virtual_space.committed_size();
  size_t new_size = used_in_bytes() + desired_free_space;
  new_size = align_size_up(new_size, alignment);

  assert(_max_gen_size == reserved().byte_size(), "max new size problem?");

  // Adjust according to our min and max
  new_size = MAX2(MIN2(new_size, _max_gen_size), _min_gen_size);

  const size_t current_size = capacity_in_bytes();
  if (new_size == current_size) {
    // No change requested
    return;
  }
  if (new_size > current_size) {
    size_t change_bytes = new_size - current_size;
    expand(change_bytes);
  } else {
    size_t change_bytes = current_size - new_size;
    // shrink doesn't grab this lock, expand does. Is that right?
    MutexLocker x(ExpandHeap_lock);
    shrink(change_bytes);
  }

  if (PrintAdaptiveSizePolicy) {
    gclog_or_tty->print_cr("AdaptiveSizePolicy::old generation size: "
                  "collection: %d "
                  "(" SIZE_FORMAT ") -> (" SIZE_FORMAT ") ",
                  heap->total_collections(),
                  size_before, _virtual_space.committed_size());
  }
}

// NOTE! We need to be careful about resizing. During a GC, multiple
// allocators may be active during heap expansion. If we allow the
// heap resizing to become visible before we have correctly resized
// all heap related data structures, we may cause program failures.
void PSOldGen::post_resize() {
  // First construct a memregion representing the new size
  MemRegion new_memregion((HeapWord*)_virtual_space.low(), (HeapWord*)_virtual_space.high());
  size_t new_word_size = new_memregion.word_size();

  // Block offset table resize FIX ME!!!!!
  // _bts->resize(new_word_size);
  Universe::heap()->barrier_set()->resize_covered_region(new_memregion);

  // Did we expand?
  if (object_space()->end() < (HeapWord*) _virtual_space.high()) {
    // We need to mangle the newly expanded area. The memregion spans
    // end -> new_end, we assume that top -> end is already mangled.
    // This cannot be safely tested for, as allocation may be taking
    // place.
    MemRegion mangle_region(object_space()->end(),(HeapWord*) _virtual_space.high());
    object_space()->mangle_region(mangle_region); 
  }

  // ALWAYS do this last!!
  object_space()->set_end((HeapWord*) _virtual_space.high());

  assert(new_word_size == heap_word_size(object_space()->capacity_in_bytes()), "Sanity");
}

void PSOldGen::print() const { print_on(tty);}
void PSOldGen::print_on(outputStream* st) const {
  st->print(" %-15s", name());
  st->print(" total " SIZE_FORMAT "K, used " SIZE_FORMAT "K", 
              capacity_in_bytes()/K, used_in_bytes()/K);
  st->print_cr(" [" INTPTR_FORMAT ", " INTPTR_FORMAT ", " INTPTR_FORMAT ")",
		_virtual_space.low_boundary(),
		_virtual_space.high(),
		_virtual_space.high_boundary());

  st->print("  object"); object_space()->print_on(st);
}

void PSOldGen::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}

#ifndef PRODUCT

void PSOldGen::verify(bool allow_dirty) {
  object_space()->verify(allow_dirty);
}

class VerifyObjectStartArrayClosure : public ObjectClosure {
  PSOldGen* _gen;
  ObjectStartArray* _start_array;

 public:
  VerifyObjectStartArrayClosure(PSOldGen* gen, ObjectStartArray* start_array) :
    _gen(gen), _start_array(start_array) { }

  virtual void do_object(oop obj) {
    HeapWord* test_addr = (HeapWord*)obj + 1;
    guarantee(_start_array->object_start(test_addr) == (HeapWord*)obj, "ObjectStartArray error");
  }
};

void PSOldGen::verify_object_start_array() {
  VerifyObjectStartArrayClosure check( this, &_start_array );
  object_iterate(&check);
}

#endif // !PRODUCT

