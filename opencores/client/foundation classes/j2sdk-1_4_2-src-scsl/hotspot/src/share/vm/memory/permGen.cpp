#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)permGen.cpp	1.31 03/01/23 12:09:39 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_permGen.cpp.incl"

CompactingPermGen::CompactingPermGen(ReservedSpace rs,
				     size_t initial_byte_size,
				     GenRemSet* remset)
{
  CompactingPermGenGen* g =
    new CompactingPermGenGen(rs, initial_byte_size, -1, remset, NULL);
  if (g == NULL)
    vm_exit_during_initialization("Could not allocate a CompactingPermGen");
  
  HeapWord* bottom = (HeapWord*) g->_virtual_space.low();
  HeapWord* end    = (HeapWord*) g->_virtual_space.high();
  g->_the_space    = new ContigPermSpace(g->_bts, MemRegion(bottom, end));
  if (g->_the_space == NULL)
    vm_exit_during_initialization("Could not allocate a CompactingPermGen Space");
  _gen = g;

  g->initialize_performance_counters();

  _capacity_expansion_limit = g->capacity() + MaxPermHeapExpansion;
}

HeapWord* CompactingPermGen::mem_allocate(size_t size) {
  MutexLocker ml(Heap_lock);
  HeapWord* obj = _gen->allocate(size, false, false);
  bool tried_collection = false;
  bool tried_expansion = false;
  while (obj == NULL) {
    if (_gen->capacity() >= _capacity_expansion_limit || tried_expansion) {
      // Expansion limit reached, try collection before expanding further
      // For now we force a full collection, this could be changed
      SharedHeap::heap()->collect_locked(GCCause::_permanent_generation_full);
      obj = _gen->allocate(size, false, false);
      tried_collection = true;
    }
    if (obj == NULL && !tried_expansion) {
      obj = _gen->expand_and_allocate(size, false, false);
      tried_expansion = true;
    }
    if (obj == NULL && tried_collection && tried_expansion) {
      break; // Return null.
    }
  }
  return obj;
}

void CompactingPermGen::compute_new_size() {
  size_t desired_capacity = align_size_up(_gen->used(), MinPermHeapExpansion);
  if (desired_capacity < PermSize) {
    desired_capacity = PermSize;
  }
  if (_gen->capacity() > desired_capacity) {
    _gen->shrink(_gen->capacity() - desired_capacity);
  }
  _capacity_expansion_limit = _gen->capacity() + MaxPermHeapExpansion;
}

CMSPermGen::CMSPermGen(ReservedSpace rs, size_t initial_byte_size,
             CardTableRS* ct,
             FreeBlockDictionary::DictionaryChoice dictionaryChoice) {
  CMSPermGenGen* g =
    new CMSPermGenGen(rs, initial_byte_size, -1, ct);
  if (g == NULL) {
    vm_exit_during_initialization("Could not allocate a CompactingPermGen");
  }

  g->initialize_performance_counters();

  _gen = g;
  _capacity_expansion_limit = g->capacity() + MaxPermHeapExpansion;
}

HeapWord* CMSPermGen::mem_allocate(size_t size) {
  // This case is slightly complicated by the need for taking care
  // of two cases: we may be calling here sometimes while holding the
  // underlying cms space's free list lock and sometimes without.
  // The solution is an efficient recursive lock for the  free list,
  // but here we use a naive and inefficient solution, that also,
  // unfortunately, exposes the implementation details. FIX ME!!!
  Mutex* lock = _gen->freelistLock();
  HeapWord* obj = NULL;
  bool tried_collection = false;
  bool tried_expansion = false;
  bool lock_owned = lock->owned_by_self();
  if (lock_owned) { // already own it, allocate unlocked
    obj = _gen->have_lock_and_allocate(size, false, false);
  } else {
    obj = _gen->allocate(size, false, false);
  }
  while (obj == NULL) {
    if (_gen->capacity() >= _capacity_expansion_limit || tried_expansion) {
      // Expansion limit reached, try collection before expanding further
      // For now we force a full collection, this could be changed
      GenCollectedHeap::heap()->collect(GCCause::_permanent_generation_full);
      if (lock_owned) { // already own it, allocate unlocked
        obj = _gen->have_lock_and_allocate(size, false, false);
      } else {
        obj = _gen->allocate(size, false, false);
      }
      tried_collection = true;
    }
    if (obj == NULL && !tried_expansion) {
      obj = _gen->expand_and_allocate(size, false, false);
      tried_expansion = true;
    }
    if (obj == NULL && tried_collection && tried_expansion) {
      break; // Return null.
    }
  }
  return obj;
}

void CMSPermGen::compute_new_size() {
  _gen->compute_new_size();
}

void CMSPermGenGen::initialize_performance_counters() {

  const char* gen_name = "perm";

  // Generation Counters - generation 2, 1 subspace
  _gen_counters = new GenerationCounters(PERF_GC, gen_name, 2, 1,
                                         &_virtual_space);

  _gc_counters = NULL;

  const char* ns = _gen_counters->name_space();
  _space_counters = new GSpaceCounters(_gen_counters->name_space(), gen_name,
                                       0, _virtual_space.reserved_size(),
                                      this);
}

void CompactingPermGenGen::initialize_performance_counters() {

  const char* gen_name = "perm";

  // Generation Counters - generation 2, 1 subspace
  _gen_counters = new GenerationCounters(PERF_GC, gen_name, 2, 1,
                                         &_virtual_space);

  const char* ns = _gen_counters->name_space();
  _space_counters = new CSpaceCounters(_gen_counters->name_space(), gen_name,
                                       0, _virtual_space.reserved_size(),
                                      _the_space);
}

void CompactingPermGenGen::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}
