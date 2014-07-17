#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parallelScavengeHeap.cpp	1.33 03/01/23 12:02:12 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_parallelScavengeHeap.cpp.incl"

PSYoungGen*  ParallelScavengeHeap::_young_gen = NULL;
PSOldGen*    ParallelScavengeHeap::_old_gen = NULL;
PSPermGen*   ParallelScavengeHeap::_perm_gen = NULL;
AdaptiveSizePolicy* ParallelScavengeHeap::_size_policy = NULL;
GCPolicyCounters* ParallelScavengeHeap::_gc_policy_counters = NULL;

void ParallelScavengeHeap::initialize() {
  // Cannot be initialized until after the flags are parsed
  GenerationSizer flag_parser;

  const size_t alignment = min_alignment();

  // Check alignments
// NEEDS_CLEANUP   The default TwoGenerationCollectorPolicy uses
//   NewRatio;  it should check UseAdaptiveSizePolicy. Changes from
//   generationSizer could move to the common code.
  size_t young_size = align_size_up(flag_parser.young_gen_size(), alignment);
  size_t max_young_size = align_size_up(flag_parser.max_young_gen_size(), 
                                                                  alignment);

  size_t old_size = align_size_up(flag_parser.old_gen_size(), alignment);
  size_t max_old_size = align_size_up(flag_parser.max_old_gen_size(), 
                                                                  alignment);

  size_t perm_size = align_size_up(flag_parser.perm_gen_size(), alignment);
  size_t max_perm_size = align_size_up(flag_parser.max_perm_gen_size(), 
                                                                  alignment);

  // Calculate the total size.
  size_t total_reserved = max_young_size + max_old_size + max_perm_size;

  if (UseISM || UsePermISM) {
    total_reserved = round_to(total_reserved, LargePageSizeInBytes);
  }

  ReservedSpace heap_rs(total_reserved, alignment, UseISM || UsePermISM);
  if (!heap_rs.is_reserved()) {
    vm_exit_during_initialization("Could not reserve enough space for "
                                  "object heap");
  }

  _reserved = MemRegion((HeapWord*)heap_rs.base(),
			(HeapWord*)(heap_rs.base() + heap_rs.size()));

  HeapWord* boundary = (HeapWord*)(heap_rs.base() + max_young_size);
  CardTableExtension* card_table_barrier_set = new CardTableExtension(_reserved, 3);
  _barrier_set = card_table_barrier_set;

  oopDesc::set_bs(_barrier_set);
  if (_barrier_set == NULL) {
    vm_exit_during_initialization("Could not reserve enough space for "
                                  "barrier set"); 
  }

  // Initial young gen size is 4 Mb
  size_t init_young_size = align_size_up(4 * M, alignment);
  init_young_size = MAX2(MIN2(init_young_size, max_young_size), young_size);

  ReservedSpace generation_rs = heap_rs.first_part(max_young_size);
  _young_gen = new PSYoungGen(generation_rs, 
                              init_young_size, 
                              young_size, 
                              max_young_size);
  heap_rs = heap_rs.last_part(max_young_size);

  generation_rs = heap_rs.first_part(max_old_size);
  _old_gen = new PSOldGen(generation_rs,
                          old_size,
                          old_size,
                          max_old_size,
                          "old", 1);

  heap_rs = heap_rs.last_part(max_old_size);

  _perm_gen = new PSPermGen(heap_rs,
                            perm_size,
                            perm_size,
                            max_perm_size,
                            "perm", 2);

  _size_policy = new AdaptiveSizePolicy(young_gen()->eden_space()->capacity_in_bytes(),
                                        old_gen()->capacity_in_bytes(),
                                        young_gen()->to_space()->capacity_in_bytes(),
                                        max_young_size,
                                        max_old_size,
                                        alignment);

  // initialize the policy counters - 2 collectors, 3 generations
  _gc_policy_counters = new GCPolicyCounters(PERF_GC, "ParScav:MSC", 2, 3);
}

void ParallelScavengeHeap::post_initialize() {
  // Need to init the tenuring threshold
  PSScavenge::initialize();
  PSMarkSweep::initialize();
  PSPromotionManager::initialize();
}

void ParallelScavengeHeap::update_counters() {
  young_gen()->update_counters();
  old_gen()->update_counters();
  perm_gen()->update_counters();
}

size_t ParallelScavengeHeap::capacity() const {
  size_t value = young_gen()->capacity_in_bytes() + old_gen()->capacity_in_bytes();
  return value;
}

size_t ParallelScavengeHeap::used() const {
  size_t value = young_gen()->used_in_bytes() + old_gen()->used_in_bytes();
  return value;
}

size_t ParallelScavengeHeap::permanent_capacity() const {
  return perm_gen()->capacity_in_bytes();
}

size_t ParallelScavengeHeap::permanent_used() const {
  return perm_gen()->used_in_bytes();
}

size_t ParallelScavengeHeap::max_capacity() const {
  size_t estimated = reserved_region().byte_size();
  estimated -= perm_gen()->reserved().byte_size();
  if (UseAdaptiveSizePolicy) {
    estimated -= _size_policy->max_survivor_size(young_gen()->max_size());
  } else {
    estimated -= young_gen()->to_space()->capacity_in_bytes();
  }
  return MAX2(estimated, capacity());
}

bool ParallelScavengeHeap::is_in(const void* p) const {
  if (young_gen()->is_in(p)) {
    return true;
  }

  if (old_gen()->is_in(p)) {
    return true;
  }

  if (perm_gen()->is_in(p)) {
    return true;
  }

  return false;
}

// Static method
bool ParallelScavengeHeap::is_in_young(oop* p) {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, 
                                            "Must be ParallelScavengeHeap");

  PSYoungGen* young_gen = heap->young_gen();

  if (young_gen->is_in(p)) {
    return true;
  }

  return false;
}

// Static method
bool ParallelScavengeHeap::is_in_old_or_perm(oop* p) {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, 
                                            "Must be ParallelScavengeHeap");

  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  if (old_gen->is_in(p)) {
    return true;
  }

  if (perm_gen->is_in(p)) {
    return true;
  }

  return false;
}

// There are two levels of allocation policy here.
//
// When an allocation request fails, the requesting thread must invoke a VM
// operation, transfer control to the VM thread, and await the results of a
// garbage collection. That is quite expensive, and we should avoid doing it
// multiple times if possible.
//
// To accomplish this, we have a basic allocation policy, and also a
// failed allocation policy.
//
// The basic allocation policy controls how you allocate memory without
// attempting garbage collection. It is okay to grab locks and
// expand the heap, if that can be done without coming to a safepoint.
// It is likely that the basic allocation policy will not be very
// aggressive.
//
// The failed allocation policy is invoked from the VM thread after
// the basic allocation policy is unable to satisfy a mem_allocate
// request. This policy needs to cover the entire range of collection,
// heap expansion, and out-of-memory conditions. It should make every
// attempt to allocate the requested memory.

// Basic allocation policy. Should never be called at a safepoint, or
// from the VM thread.
//
// This method must handle cases where many mem_allocate requests fail
// simultaneously. When that happens, only one VM operation will succeed,
// and the rest will not be executed. For that reason, this method loops
// during failed allocation attempts. If the java heap becomes exhausted,
// we rely on the size_policy object to force a bail out.
HeapWord* ParallelScavengeHeap::mem_allocate(size_t size, bool is_noref, bool is_tlab) {
  assert(!SafepointSynchronize::is_at_safepoint(), "should not be at safepoint");
  assert(Thread::current() != (Thread*)VMThread::vm_thread(), "should not be in vm thread");
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");

  HeapWord* result;

  uint loop_count = 0;

  do {
    result = young_gen()->allocate(size, is_noref, is_tlab);

    // In some cases, the requested object will be too large to easily
    // fit in the young_gen. Rather than force a safepoint and collection
    // for each one, try allocation in old_gen for objects likely to fail
    // allocation in eden.
    if (result == NULL && size >= (young_gen()->eden_space()->capacity_in_words() / 2) && !is_tlab) {
      MutexLocker ml(Heap_lock);
      result = old_gen()->allocate(size, is_noref, is_tlab);
    }

    if (result == NULL) {
      // Generate a VM operation
      VM_ParallelGCFailedAllocation op(size, is_noref, is_tlab);
      VMThread::execute(&op);
      
      // Did the VM operation execute? If so, return the result directly.
      // This prevents us from looping until time out on requests that can
      // not be satisfied.
      if (op.prologue_succeeded()) {
        assert(Universe::heap()->is_in_or_null(op.result()), "result not in heap");
        return op.result();
      }
    }

    // The policy object will prevent us from looping forever. If the
    // time spent in gc crosses a threshold, we will bail out.
    loop_count++;
    if ((QueuedAllocationWarningCount > 0) && (loop_count % QueuedAllocationWarningCount == 0)) {
      warning("ParallelScavengeHeap::mem_allocate retries %d times \n\t"
              " size=%d %s", loop_count, size, is_tlab ? "(TLAB)" : "");
    }
  } while (result == NULL && !size_policy()->gc_time_limit_exceeded());

  return result;
}

// Failed allocation policy. Must be called from the VM thread, and
// only at a safepoint! Note that this method has policy for allocation
// flow, and NOT collection policy. So we do not check for gc collection
// time over limit here, that is the responsibility of the heap specific
// collection methods. This method decides where to attempt allocations,
// and when to attempt collections, but no collection specific policy. 
HeapWord* ParallelScavengeHeap::failed_mem_allocate(bool& notify_ref_lock,
                                                    size_t size,
                                                    bool is_noref,
                                                    bool is_tlab) {
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == (Thread*)VMThread::vm_thread(), "should be in vm thread");
  assert(!Universe::heap()->is_gc_active(), "not reentrant");
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");
  // Note that this assert assumes no from-space allocation in eden
  assert(size > young_gen()->eden_space()->free_in_words(), "Allocation should fail");

  size_t mark_sweep_invocation_count = PSMarkSweep::total_invocations();

  // We assume (and assert!) that an allocation at this point will fail
  // unless we collect.

  // First level allocation failure, scavenge and allocate in young gen.
  PSScavenge::invoke(notify_ref_lock);
  HeapWord* result = young_gen()->allocate(size, is_noref, is_tlab);

  // Second level allocation failure. 
  //   Mark sweep and allocate in young generation.  
  if (result == NULL) {
    // There is some chance the scavenge method decided to invoke mark_sweep.
    // Don't mark sweep twice if so.
    if (mark_sweep_invocation_count == PSMarkSweep::total_invocations()) {
      PSMarkSweep::invoke(notify_ref_lock, false);
      result = young_gen()->allocate(size, is_noref, is_tlab);
    }
  }

  // Third level allocation failure.
  //   After mark sweep and young generation allocation failure, 
  //   allocate in old generation.
  if (result == NULL && !is_tlab) {
    result = old_gen()->allocate(size, is_noref, is_tlab);
  }

  // Fourth level allocation failure. We're running out of memory.
  //   More complete mark sweep and allocate in young generation.
  if (result == NULL) {
    PSMarkSweep::invoke(notify_ref_lock, true /* max_heap_compaction */);
    result = young_gen()->allocate(size, is_noref, is_tlab);
  }

  // Fifth level allocation failure.
  //   After more complete mark sweep, allocate in old generation.
  if (result == NULL && !is_tlab) {
    result = old_gen()->allocate(size, is_noref, is_tlab);
  }

  return result;
}

//
// This is the policy loop for allocating in the permanent generation.
// If the initial allocation fails, we create a vm operation which will
// cause a collection.
HeapWord* ParallelScavengeHeap::permanent_mem_allocate(size_t size) {
  assert(!SafepointSynchronize::is_at_safepoint(), "should not be at safepoint");
  assert(Thread::current() != (Thread*)VMThread::vm_thread(), "should not be in vm thread");
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");

  HeapWord* result;
  uint loop_count = 0;

  do {
    {
      MutexLocker ml(Heap_lock);
      result = perm_gen()->allocate_permanent(size);
    }

    if (result == NULL) {
      // Generate a VM operation
      VM_ParallelGCFailedPermanentAllocation op(size);
      VMThread::execute(&op);
      
      // Did the VM operation execute? If so, return the result directly.
      // This prevents us from looping until time out on requests that can
      // not be satisfied.
      if (op.prologue_succeeded()) {
        assert(Universe::heap()->is_in_permanent_or_null(op.result()), "result not in heap");
        return op.result();
      }
    }
    // The policy object will prevent us from looping forever. If the
    // time spent in gc crosses a threshold, we will bail out.
    loop_count++;
    if ((QueuedAllocationWarningCount > 0) && (loop_count % QueuedAllocationWarningCount == 0)) {
      warning("ParallelScavengeHeap::permanent_mem_allocate retries %d times \n\t"
              " size=%d", loop_count, size);
    }
  } while (result == NULL && !size_policy()->gc_time_limit_exceeded());

  return result;
}

//
// This is the policy code for permanent allocations which have failed
// and require a collection. Note that just as in failed_mem_allocate,
// we do not set collection policy, only where & when to allocate and
// collect.
HeapWord* ParallelScavengeHeap::failed_permanent_mem_allocate(bool& notify_ref_lock, size_t size) {
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == (Thread*)VMThread::vm_thread(), "should be in vm thread");
  assert(!Universe::heap()->is_gc_active(), "not reentrant");
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");
  assert(size > perm_gen()->free_in_words(), "Allocation should fail");

  // We assume (and assert!) that an allocation at this point will fail
  // unless we collect.

  // First level allocation failure. mark sweep and allocate in perm gen..  
  PSMarkSweep::invoke(notify_ref_lock, false);
  HeapWord* result = perm_gen()->allocate_permanent(size);

  // Second level allocation failure. We're running out of memory.
  if (result == NULL) {
    PSMarkSweep::invoke(notify_ref_lock, true /* max_heap_compaction */);
    result = perm_gen()->allocate_permanent(size);
  }

  return result;
}

void ParallelScavengeHeap::ensure_parseability() {
  CollectedHeap::ensure_parseability();
}

size_t ParallelScavengeHeap::unsafe_max_alloc() {
  return young_gen()->eden_space()->free_in_bytes();
}

size_t ParallelScavengeHeap::tlab_capacity() const {
  return young_gen()->eden_space()->capacity_in_bytes();
}

HeapWord* ParallelScavengeHeap::allocate_new_tlab(size_t size) {
  return young_gen()->allocate(size, false, true);
}

void ParallelScavengeHeap::fill_all_tlabs() {
  CollectedHeap::fill_all_tlabs();
}

// This method is only used by System.gc()
void ParallelScavengeHeap::collect(GCCause::Cause cause) {
  assert(!Heap_lock->owned_by_self(), "this thread should not own the Heap_lock");
  VM_ParallelGCSystemGC op;
  VMThread::execute(&op);
}

void ParallelScavengeHeap::oop_iterate(OopClosure* cl) {
  Unimplemented();
}

void ParallelScavengeHeap::object_iterate(ObjectClosure* cl) {
  Unimplemented();
}

void ParallelScavengeHeap::permanent_oop_iterate(OopClosure* cl) {
  Unimplemented();
}

void ParallelScavengeHeap::permanent_object_iterate(ObjectClosure* cl) {
  perm_gen()->object_iterate(cl);
}

HeapWord* ParallelScavengeHeap::block_start(const void* addr) const {
  if (young_gen()->is_in(addr)) {
    Unimplemented();
  } else if (old_gen()->is_in(addr)) {
    return old_gen()->start_array()->object_start((HeapWord*)addr);
  } else if (perm_gen()->is_in(addr)) {
    return perm_gen()->start_array()->object_start((HeapWord*)addr);
  }
  return 0;
}

size_t ParallelScavengeHeap::block_size(const HeapWord* addr) const {
  return oop(addr)->size();
}

bool ParallelScavengeHeap::block_is_obj(const HeapWord* addr) const {
  return block_start(addr) == addr;
}

jlong ParallelScavengeHeap::millis_since_last_gc() {
  return PSMarkSweep::millis_since_last_gc();
}

void ParallelScavengeHeap::prepare_for_verify() {
  ensure_parseability();
}

void ParallelScavengeHeap::print() const { print_on(tty); }

void ParallelScavengeHeap::print_on(outputStream* st) const {
  young_gen()->print_on(st);
  old_gen()->print_on(st);
  perm_gen()->print_on(st);
}

#ifndef PRODUCT

void ParallelScavengeHeap::verify(bool allow_dirty, bool silent) {
  // Really stupid. Can't fill the tlabs, can't verify. FIX ME!
  if (total_collections() > 0) {
    if (!silent) {
      tty->print("permanent ");
    }
    perm_gen()->verify(allow_dirty);

    if (!silent) {
      tty->print("tenured ");
    }
    old_gen()->verify(allow_dirty);

    if (!silent) {
      tty->print("eden ");
    }
    young_gen()->verify(allow_dirty);
  }
}

#endif

void ParallelScavengeHeap::print_heap_change(size_t prev_used) {
  gclog_or_tty->print(" "  SIZE_FORMAT "K"
                      "->" SIZE_FORMAT "K"
                      "("  SIZE_FORMAT "K)",
                      prev_used / K, used() / K, capacity() / K);
}

int ParallelScavengeHeap::addr_to_arena_id(void* addr) {
  fatal("JVMPI is not supported by this collector");
  return 0;
}
