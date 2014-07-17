#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psScavenge.cpp	1.46 03/03/06 15:00:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


# include "incls/_precompiled.incl"
# include "incls/_psScavenge.cpp.incl"

ReferenceProcessor*        PSScavenge::_ref_processor = NULL;
CardTableExtension*        PSScavenge::_card_table = NULL;
bool                       PSScavenge::_survivor_overflow = false;
int                        PSScavenge::_tenuring_threshold = 0;
HeapWord*                  PSScavenge::_eden_boundary = NULL;
elapsedTimer               PSScavenge::_accumulated_time;
GCTaskManager*             PSScavenge::_gc_task_manager = NULL;
GrowableArray<markOop>*    PSScavenge::_preserved_mark_stack = NULL;
GrowableArray<oop>*        PSScavenge::_preserved_oop_stack = NULL;
CollectorCounters*         PSScavenge::_counters = NULL;

// Define before use
class PSIsAliveClosure: public BoolObjectClosure {
public:
  void do_object(oop p) {
    assert(false, "Do not call.");
  }
  bool do_object_b(oop p) {
    return (HeapWord*)p >= PSScavenge::_eden_boundary || p->is_forwarded();
  }
};

class PSKeepAliveClosure: public OopClosure {
protected:
  MutableSpace* _to_space;
  PSPromotionManager* _promotion_manager;

public:
  PSKeepAliveClosure(PSPromotionManager* pm) : _promotion_manager(pm) {
    ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
    assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
    _to_space = heap->young_gen()->to_space();
    
    assert(_promotion_manager != NULL, "Sanity");
  }

  void do_oop(oop* p) {
    assert (*p != NULL, "expected non-null ref");
    assert ((*p)->is_oop(), "expected an oop while scanning weak refs");
  
    oop obj = oop(*p);
    // Weak refs may be visited more than once.
    if (PSScavenge::should_scavenge(obj) && !_to_space->contains(obj)) {
      PSScavenge::copy_and_push_safe_barrier(_promotion_manager, p);
    }
  }
};

class PSEvacuateFollowersClosure: public VoidClosure {
 private:
  PSPromotionManager* _promotion_manager;
 public:
  PSEvacuateFollowersClosure(PSPromotionManager* pm) : _promotion_manager(pm) {}

  void do_void() {
    assert(_promotion_manager != NULL, "Sanity");
    _promotion_manager->drain_stacks();
  }
};

class PSPromotionFailedClosure : public ObjectClosure {
  virtual void do_object(oop obj) {
    if (obj->is_forwarded()) {
      obj->init_mark();
    }
  }
};

// This method contains all heap specific policy for invoking scavenge.
// PSScavenge::invoke_no_policy() will do nothing but attempt to
// scavenge. It will not clean up after failed promotions, bail out if
// we've exceeded policy time limits, or any other special behavior.
// All such policy should be placed here.
//
// Note that this method should only be called from the vm_thread while
// at a safepoint!
void PSScavenge::invoke(bool& notify_ref_lock)
{
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == (Thread*)VMThread::vm_thread(), "should be in vm thread");
  assert(!Universe::heap()->is_gc_active(), "not reentrant");

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  AdaptiveSizePolicy* policy = ParallelScavengeHeap::size_policy();

  // Before each allocation/collection attempt, find out from the
  // policy object if GCs are, on the whole, taking too long. If so,
  // bail out without attempting a collection.
  if (!policy->gc_time_limit_exceeded()) {
    IsGCActiveMark mark;

    PSScavenge::invoke_no_policy(notify_ref_lock);

    if (policy->should_full_GC(heap->old_gen()->used_in_bytes())) { 
      PSMarkSweep::invoke_no_policy(notify_ref_lock, false);
    }
  }
}

// This method contains no policy. You should probably
// be calling invoke() instead. 
void PSScavenge::invoke_no_policy(bool& notify_ref_lock) {
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == (Thread*)VMThread::vm_thread(), "should be in vm thread");

  TimeStamp scavenge_entry;
  TimeStamp scavenge_midpoint;
  TimeStamp scavenge_exit;

  scavenge_entry.update();

  if (GC_locker::is_active()) return;

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // Check for potential problems.
  if (!should_attempt_scavenge()) {
    return;
  }

  PSYoungGen* young_gen = heap->young_gen();
  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();
  AdaptiveSizePolicy* size_policy = heap->size_policy();

  heap->increment_total_collections();

  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" {Heap before GC invocations=%d:", heap->total_collections());
    Universe::print();
  }

  assert(!NeverTenure || _tenuring_threshold == markOopDesc::max_age + 1, "Sanity");
  assert(!AlwaysTenure || _tenuring_threshold == 0, "Sanity");

  size_t prev_used = heap->used();
  assert(promotion_failed() == false, "Sanity");

  // Fill in TLABs
  heap->ensure_parseability();

  if (VerifyBeforeGC && heap->total_collections() >= VerifyGCStartAt) {
    HandleMark hm;  // Discard invalid handles created during verification
    tty->print(" VerifyBeforeGC:");
    Universe::verify(true);
  }

  {
    ResourceMark rm;
    HandleMark hm;

    TraceTime t1("GC", PrintGC, true, gclog_or_tty);
    TraceCollectorStats tcs(counters());
    if (TraceGen0Time) accumulated_time()->start();

    // Let the size policy know we're starting
    size_policy->minor_collection_begin();
    
    // Verify no unmarked old->young roots
    if (VerifyRememberedSets) {
      old_gen->verify_object_start_array();
      perm_gen->verify_object_start_array();
      CardTableExtension::verify_all_young_refs_imprecise();
    }
    
    assert(young_gen->to_space()->is_empty(), "Attempt to scavenge with live objects in to_space");
    young_gen->to_space()->clear();

    NOT_PRODUCT(reference_processor()->verify_no_references_recorded());
    COMPILER2_ONLY(DerivedPointerTable::clear(););

    reference_processor()->enable_discovery();
    
    // We track how much was promoted to the next generation for
    // the AdaptiveSizePolicy.
    size_t old_gen_used_before = old_gen->object_space()->used_in_bytes();

    // Reset our survivor overflow.
    set_survivor_overflow(false);
    
    // We need to save the old/perm top values before
    // creating the promotion_manager. We pass the top
    // values to the card_table, to prevent it from
    // straying into the promotion labs.
    HeapWord* old_top = old_gen->object_space()->top();
    HeapWord* perm_top = perm_gen->object_space()->top();

    // Release all previously held resources
    gc_task_manager()->release_all_resources();

    PSPromotionManager::pre_scavenge();

    // We'll use the promotion manager again later.
    PSPromotionManager* promotion_manager = PSPromotionManager::vm_thread_promotion_manager();
    {
      // TraceTime("Roots");
      
      GCTaskQueue* q = GCTaskQueue::create();
      
      for(uint i=0; i<ParallelGCThreads; i++) {
        q->enqueue(new OldToYoungRootsTask(old_gen, old_top, i));
      }

      q->enqueue(new SerialOldToYoungRootsTask(perm_gen, perm_top));

      q->enqueue(new ScavengeRootsTask(ScavengeRootsTask::universe));
      q->enqueue(new ScavengeRootsTask(ScavengeRootsTask::jni_handles));
      // We scan the thread roots in parallel
      Threads::create_thread_roots_tasks(q);
      q->enqueue(new ScavengeRootsTask(ScavengeRootsTask::object_synchronizer));
      q->enqueue(new ScavengeRootsTask(ScavengeRootsTask::flat_profiler));
      q->enqueue(new ScavengeRootsTask(ScavengeRootsTask::system_dictionary));

      if (ParallelGCThreads>1) {
        for (uint j=0; j<ParallelGCThreads-1; j++) {
          q->enqueue(new StealTask(false));
        }
        q->enqueue(new StealTask(true));
      }

      WaitForBarrierGCTask* fin = WaitForBarrierGCTask::create();
      q->enqueue(fin);

      gc_task_manager()->add_list(q);
      
      fin->wait_for();

      // We have to release the barrier tasks!
      WaitForBarrierGCTask::destroy(fin);
    }

    scavenge_midpoint.update();

    NOT_COMPILER2(ReferencePolicy *soft_ref_policy = new LRUCurrentHeapPolicy());
    COMPILER2_ONLY(ReferencePolicy *soft_ref_policy = new LRUMaxHeapPolicy());
    
    PSIsAliveClosure is_alive;
    PSKeepAliveClosure keep_alive(promotion_manager);
    PSEvacuateFollowersClosure evac_followers(promotion_manager);
    
    // Process reference objects discovered during scavenge
    reference_processor()->process_discovered_references(soft_ref_policy, &is_alive,
                                                         &keep_alive, &evac_followers);
    
    // Enqueue reference objects discovered during scavenge.
    notify_ref_lock = reference_processor()->enqueue_discovered_references();
    
    // Finally, flush the promotion_manager's labs, and deallocate its stacks.
    assert(promotion_manager->claimed_stack()->size() == 0, "Sanity");
    PSPromotionManager::post_scavenge();

    bool scavenge_promotion_failure = promotion_failed();
    if (scavenge_promotion_failure) {
      clean_up_failed_promotion();
      if (PrintGC) {
        gclog_or_tty->print("--");
      }
    }

    // Let the size policy know we're done. Note that we count promotion
    // failure cleanup time as part of the collection (otherwise, we're implicitly
    // saying it's mutator time).
    size_policy->minor_collection_end();

    if (!scavenge_promotion_failure) {
      // Swap the survivor spaces.
      young_gen->eden_space()->clear();
      young_gen->from_space()->clear();
      young_gen->swap_spaces();

      if (UseAdaptiveSizePolicy) {
        // Calculate the new survivor size and tenuring threshold
        size_t survived = young_gen->from_space()->used_in_bytes();
        size_t promoted = old_gen->used_in_bytes() - old_gen_used_before;

        if (PrintAdaptiveSizePolicy) {
          tty->print_cr("AdaptiveSizeStart: collection: %d ",
                         heap->total_collections());
        }

        size_t survivor_limit = 
	  size_policy->max_survivor_size(young_gen->max_size());
        _tenuring_threshold = 
           size_policy->compute_survivor_space_size_and_threshold(survived, 
                                                           promoted,
                                                           _survivor_overflow, 
                                                           _tenuring_threshold,
                                                           survivor_limit);

        // Calculate optimial free space amounts
        size_policy->compute_generation_free_space(young_gen->used_in_bytes(),
                                                   old_gen->used_in_bytes(),
                                                   perm_gen->used_in_bytes(),
                                                   false  /* full gc*/);
        
        // Resize the old and young generations
        old_gen->resize(size_policy->calculated_old_free_size_in_bytes());
        
        young_gen->resize(size_policy->calculated_eden_size_in_bytes(),
                          size_policy->calculated_survivor_size_in_bytes());

        if (PrintAdaptiveSizePolicy) {
          tty->print_cr("AdaptiveSizeStop: collection: %d ",
                         heap->total_collections());
        }


      }

      assert(young_gen->to_space()->is_empty(), "to space should be empty now");
    }

    COMPILER2_ONLY(DerivedPointerTable::update_pointers());
    
    NOT_PRODUCT(reference_processor()->verify_no_references_recorded());
        
    // Verify all old -> young cards are now precise
    if (VerifyRememberedSets) {
      // Precise verification will give false positives. Until this is fixed,
      // use imprecise verification.
      // CardTableExtension::verify_all_young_refs_precise();
      CardTableExtension::verify_all_young_refs_imprecise();
    }

    if (TraceGen0Time) accumulated_time()->stop();
    
    if (PrintGC) {
      heap->print_heap_change(prev_used);
    }

    heap->update_counters();
  }

  if (VerifyAfterGC && heap->total_collections() >= VerifyGCStartAt) {
    HandleMark hm;  // Discard invalid handles created during verification
    tty->print(" VerifyAfterGC:");
    Universe::verify(false);
  }

  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" Heap after GC invocations=%d:", heap->total_collections());
    Universe::print();
    gclog_or_tty->print("} ");
  }

  scavenge_exit.update();

  if (PrintGCTaskTimeStamps) {
    tty->print_cr("VM-Thread " INT64_FORMAT " " INT64_FORMAT " " INT64_FORMAT,
                  scavenge_entry.ticks(), scavenge_midpoint.ticks(), scavenge_exit.ticks());
  
    gc_task_manager()->print_task_time_stamps();
  }
}

// This method iterates over all objects in the young generation,
// unforwarding markOops. It then restores any preserved mark oops,
// and clears the _preserved_mark_stack.
void PSScavenge::clean_up_failed_promotion() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  assert(promotion_failed(), "Sanity");

  PSYoungGen* young_gen = heap->young_gen();

  {
    ResourceMark rm;

    // Unforward all pointers in the young gen.
    PSPromotionFailedClosure unforward_closure;
    young_gen->object_iterate(&unforward_closure);

    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("Restoring %d marks", 
                              _preserved_oop_stack->length());
    }

    // Restore any saved marks.
    for (int i=0; i < _preserved_oop_stack->length(); i++) {
      oop obj       = _preserved_oop_stack->at(i);
      markOop mark  = _preserved_mark_stack->at(i);
      obj->set_mark(mark);      
    }
 
    // Deallocate the preserved mark and oop stacks.
    // The stacks were allocated as CHeap objects, so
    // we must call delete to prevent mem leaks.
    _preserved_mark_stack->clear_and_deallocate();
    delete _preserved_mark_stack;
    _preserved_mark_stack = NULL;
    _preserved_oop_stack->clear_and_deallocate();
    delete _preserved_oop_stack;
    _preserved_oop_stack = NULL;

  }
}

// This method is called whenever an attempt to promote an object
// fails. Some markOops will need preserving, some will not. Note
// that the entire eden is traversed after a failed promotion, with
// all forwarded headers replaced by the default markOop. This means
// it is not neccessary to preserve most markOops.
void PSScavenge::oop_promotion_failed(oop obj, markOop obj_mark) {
  if (_preserved_mark_stack == NULL) {
    ThreadCritical tc; // Lock and retest
    if (_preserved_mark_stack == NULL) {
      assert(_preserved_oop_stack == NULL, "Sanity");
      _preserved_mark_stack = new (ResourceObj::C_HEAP) GrowableArray<markOop>(40, true);
      _preserved_oop_stack = new (ResourceObj::C_HEAP) GrowableArray<oop>(40, true);
    }
  }

  // Because we must hold the ThreadCritical lock before using
  // the stacks, we should be safe from observing partial allocations,
  // which are also guarded by the ThreadCritical lock.
  if (obj_mark->must_be_preserved()) {
    ThreadCritical tc;
    _preserved_oop_stack->push(obj);
    _preserved_mark_stack->push(obj_mark);
  }
}

bool PSScavenge::should_attempt_scavenge() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");
  
  // Do not attempt to promote unless to_space is empty
  if (!heap->young_gen()->to_space()->is_empty()) {
    return false;
  }
  
  // Test to see if the scavenge will likely fail.
  AdaptiveSizePolicy* policy = ParallelScavengeHeap::size_policy();
  PSOldGen* old_gen = heap->old_gen();

  if (policy->average_promoted_in_bytes() > old_gen->free_in_bytes()) {
    return false;
  }

  return true;
}

void PSScavenge::initialize() {
  // Arguments must have been parsed 

  if (AlwaysTenure) {
    _tenuring_threshold = 0;
  } else if (NeverTenure) {
    _tenuring_threshold = markOopDesc::max_age + 1;
  } else {
    // We want to smooth out our startup times for the AdaptiveSizePolicy
    _tenuring_threshold = (UseAdaptiveSizePolicy) ? InitialTenuringThreshold : 
                                                    MaxTenuringThreshold;
  }
  
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSYoungGen* young_gen = heap->young_gen();
  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  // Set boundary between young_gen and old_gen
  assert(young_gen->reserved().end() <= old_gen->object_space()->bottom(), "Sanity");
  assert(young_gen->reserved().end() <= perm_gen->object_space()->bottom(), "Sanity");
  _eden_boundary = old_gen->object_space()->bottom();

  // Initialize ref handling object for scavenging.
  _ref_processor = new ReferenceProcessor();
  reference_processor()->initialize(false,  // atomic_discovery
                                    true);  // mt_discovery
  MemRegion mr = young_gen->reserved();
  reference_processor()->set_span(mr);

  // Cache the cardtable
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  _card_table = (CardTableExtension*)bs;

  // Set up the GCTaskManager
  _gc_task_manager = GCTaskManager::create(ParallelGCThreads);

  _counters = new CollectorCounters(PERF_GC, "PSScavenge", 0);
}
