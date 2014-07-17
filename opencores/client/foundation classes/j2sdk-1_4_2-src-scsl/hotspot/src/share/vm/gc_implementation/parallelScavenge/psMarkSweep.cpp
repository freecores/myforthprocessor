#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psMarkSweep.cpp	1.31 03/03/06 15:01:15 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_psMarkSweep.cpp.incl"

elapsedTimer        PSMarkSweep::_accumulated_time;
unsigned int        PSMarkSweep::_total_invocations = 0;
jlong               PSMarkSweep::_time_of_last_gc   = 0;
CollectorCounters*  PSMarkSweep::_counters = NULL;

void PSMarkSweep::initialize() {
  _ref_processor = new ReferenceProcessor();
  ref_processor()->initialize(true,    // atomic_discovery
                              false);  // mt_discovery
  MemRegion mr = Universe::heap()->reserved_region();
  ref_processor()->set_span(mr);
  _counters = new CollectorCounters(PERF_GC, "PSMarkSweep", 1);
}

// This method contains all heap specific policy for invoking mark sweep.
// PSMarkSweep::invoke_no_policy() will only attempt to mark-sweep-compact
// the heap. It will do nothing further. If we need to bail out for policy
// reasons, scavenge before full gc, or any other specialized behavior, it
// needs to be added here.
//
// Note that this method should only be called from the vm_thread while
// at a safepoint!
void PSMarkSweep::invoke(bool& notify_ref_lock, bool maximum_heap_compaction) {
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(Thread::current() == (Thread*)VMThread::vm_thread(), "should be in vm thread");
  assert(!Universe::heap()->is_gc_active(), "not reentrant");

  AdaptiveSizePolicy* policy = ParallelScavengeHeap::size_policy();

  // Before each allocation/collection attempt, find out from the
  // policy object if GCs are, on the whole, taking too long. If so,
  // bail out without attempting a collection.
  if (!policy->gc_time_limit_exceeded()) {
    IsGCActiveMark mark;

    if (ScavengeBeforeFullGC) {
      PSScavenge::invoke_no_policy(notify_ref_lock);
    }

    int count = (maximum_heap_compaction)?1:MarkSweepAlwaysCompactCount;
    IntFlagSetting flag_setting(MarkSweepAlwaysCompactCount, count);
    PSMarkSweep::invoke_no_policy(notify_ref_lock, maximum_heap_compaction);
  }
}

// This method contains no policy. You should probably
// be calling invoke() instead. 
void PSMarkSweep::invoke_no_policy(bool& notify_ref_lock, bool clear_all_softrefs) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");
  assert(ref_processor() != NULL, "Sanity");

  if (GC_locker::is_active()) return;

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSYoungGen* young_gen = heap->young_gen();
  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  // Increment the invocation count
  heap->increment_total_collections();

  // We need to track unique mark sweep invocations as well.
  _total_invocations++;

  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" {Heap before GC invocations=%d:", heap->total_collections());
    Universe::print();
  }

  // Fill in TLABs
  heap->ensure_parseability();

  if (VerifyBeforeGC && heap->total_collections() >= VerifyGCStartAt) {
    HandleMark hm;  // Discard invalid handles created during verification
    tty->print(" VerifyBeforeGC:");
    Universe::verify(true);
  }
  
  {
    HandleMark hm;
    TraceTime t1("Full GC", PrintGC, true, gclog_or_tty);
    TraceCollectorStats tcs(counters());
    if (TraceGen1Time) accumulated_time()->start();
  
    // Let the size policy know we're starting
    AdaptiveSizePolicy* size_policy = heap->size_policy();
    size_policy->major_collection_begin();

    // When collecting the permanent generation methodOops may be moving,
    // so we either have to flush all bcp data or convert it into bci.
    NOT_CORE(CodeCache::gc_prologue());
    Threads::gc_prologue();
    
    // Capture heap size before collection for printing.
    size_t prev_used = heap->used();

    // Capture perm gen size before collection for sizing.
    size_t perm_gen_prev_used = perm_gen->used_in_bytes();
    
    bool marked_for_unloading = false;
    
    allocate_stacks();
    
    NOT_PRODUCT(ref_processor()->verify_no_references_recorded());
    COMPILER2_ONLY(DerivedPointerTable::clear());
  
    ref_processor()->enable_discovery();

    mark_sweep_phase1(marked_for_unloading, clear_all_softrefs);

    mark_sweep_phase2();
    
    // Don't add any more derived pointers during phase3
    COMPILER2_ONLY(assert(DerivedPointerTable::is_active(), "Sanity"));
    COMPILER2_ONLY(DerivedPointerTable::set_active(false));
    
    mark_sweep_phase3();
    
    mark_sweep_phase4();
    
    restore_marks();
    
    deallocate_stacks();
    
    // "free at last gc" is calculated from these.
    Universe::set_heap_capacity_at_last_gc(Universe::heap()->capacity());
    Universe::set_heap_used_at_last_gc(Universe::heap()->used());
    
    bool all_empty = young_gen->eden_space()->is_empty() &&
      young_gen->from_space()->is_empty() &&
      young_gen->to_space()->is_empty();
    
    BarrierSet* bs = heap->barrier_set();
    if (bs->is_a(BarrierSet::ModRef)) {
      ModRefBarrierSet* modBS = (ModRefBarrierSet*)bs;
      MemRegion old_mr = heap->old_gen()->reserved();
      MemRegion perm_mr = heap->perm_gen()->reserved();
      assert(old_mr.end() <= perm_mr.start(), "Generations out of order");
      
      if (all_empty) {
        modBS->clear(MemRegion(old_mr.start(), perm_mr.end()));
      } else {
        modBS->invalidate(MemRegion(old_mr.start(), perm_mr.end()));
      }
    }
    
    Threads::gc_epilogue();
    NOT_CORE(CodeCache::gc_epilogue());
    
    COMPILER2_ONLY(DerivedPointerTable::update_pointers());
  
    notify_ref_lock |= ref_processor()->enqueue_discovered_references();

    // Update time of last GC
    reset_millis_since_last_gc();

    // Let the size policy know we're done
    size_policy->major_collection_end(old_gen->used_in_bytes());

    if (UseAdaptiveSizePolicy) {

      if (PrintAdaptiveSizePolicy) {
        tty->print_cr("AdaptiveSizeStart: collection: %d ",
                       heap->total_collections());
      }

      // Calculate optimial free space amounts
      size_policy->compute_generation_free_space(young_gen->used_in_bytes(),
                                                 old_gen->used_in_bytes(),
                                                 perm_gen->used_in_bytes(),
                                                 true /* full gc*/);

      // Resize old and young generations
      old_gen->resize(size_policy->calculated_old_free_size_in_bytes());

      young_gen->resize(size_policy->calculated_eden_size_in_bytes(),
                        size_policy->calculated_survivor_size_in_bytes());

      if (PrintAdaptiveSizePolicy) {
        tty->print_cr("AdaptiveSizeStop: collection: %d ",
                       heap->total_collections());
      }
    }

    // We collected the perm gen, so we'll resize it here.
    perm_gen->compute_new_size(perm_gen_prev_used);
    
    if (TraceGen1Time) accumulated_time()->stop();

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

  NOT_PRODUCT(ref_processor()->verify_no_references_recorded());

  if (PrintHeapAtGC){
    gclog_or_tty->print_cr(" Heap after GC invocations=%d:", heap->total_collections());
    Universe::print();
    gclog_or_tty->print("} ");
  }
}


void PSMarkSweep::allocate_stacks() {
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSYoungGen* young_gen = heap->young_gen();

  MutableSpace* to_space = young_gen->to_space();
  _preserved_marks = (PreservedMark*)to_space->top();
  _preserved_count = 0;

  // We want to calculate the size in bytes first.
  _preserved_count_max  = pointer_delta(to_space->end(), to_space->top(), sizeof(jbyte));
  // Now divide by the size of a PreservedMark
  _preserved_count_max /= sizeof(PreservedMark);

  _preserved_mark_stack = NULL;
  _preserved_oop_stack = NULL;

  _marking_stack = new GrowableArray<oop>(4000, true);

  int size = SystemDictionary::number_of_classes() * 2;
  _revisit_klass_stack = new GrowableArray<Klass*>(size, true);
}


void PSMarkSweep::deallocate_stacks() {
  if (_preserved_oop_stack) {
    _preserved_mark_stack->clear_and_deallocate();
    delete _preserved_mark_stack;
    _preserved_mark_stack = NULL;
    _preserved_oop_stack->clear_and_deallocate();
    delete _preserved_oop_stack;
    _preserved_oop_stack = NULL;
  }

  _marking_stack->clear_and_deallocate();
  _revisit_klass_stack->clear_and_deallocate();
}

void PSMarkSweep::mark_sweep_phase1( bool& marked_for_unloading, bool clear_all_softrefs) {
  // Recursively traverse all live objects and mark them
  EventMark m("1 mark object");
  TraceTime tm("phase 1", PrintGC && Verbose, true, gclog_or_tty);
  trace(" 1");

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  // General strong roots.
  Universe::oops_do(mark_and_push_closure());
  JNIHandles::oops_do(mark_and_push_closure());   // Global (strong) JNI handles
  Threads::oops_do(mark_and_push_closure());
  ObjectSynchronizer::oops_do(mark_and_push_closure());
  FlatProfiler::oops_do(mark_and_push_closure());
  SystemDictionary::always_strong_oops_do(mark_and_push_closure());

  guarantee(!jvmdi::enabled(), "Should not be used with jvmdi");
  vmSymbols::oops_do(mark_and_push_closure());

  // Flush marking stack.
  follow_stack();

  // Process reference objects found during marking
  ReferencePolicy *soft_ref_policy;
  if (clear_all_softrefs) {
    soft_ref_policy = new AlwaysClearPolicy();
  } else {
    NOT_COMPILER2(soft_ref_policy = new LRUCurrentHeapPolicy();)
    COMPILER2_ONLY(soft_ref_policy = new LRUMaxHeapPolicy();)
  }
  assert(soft_ref_policy != NULL,"No soft reference policy");
  ref_processor()->process_discovered_references(soft_ref_policy,
                                                 is_alive_closure(),
                                                 mark_and_push_closure(),
                                                 follow_stack_closure());

  // Follow system dictionary roots and unload classes
  bool purged_class = SystemDictionary::do_unloading();
  assert(_marking_stack->is_empty(), "stack should be empty by now");

  // Follow code cache roots (has to be done after system dictionary, assumes all live klasses are marked)
  NOT_CORE(CodeCache::do_unloading(purged_class, marked_for_unloading);)   // did we mark any nmethods for unloading?

  // Update subklass/sibling/implementor links of live klasses
  follow_weak_klass_links();

  // Visit symbol and interned string tables and delete unmarked oops
  SymbolTable::unlink();
  StringTable::unlink();

  assert(_marking_stack->is_empty(), "stack should be empty by now");
}


void PSMarkSweep::mark_sweep_phase2() {
  EventMark m("2 compute new addresses");
  TraceTime tm("phase 2", PrintGC && Verbose, true, gclog_or_tty);
  trace("2");

  // Now all live objects are marked, compute the new object addresses.

  // It is imperative that we traverse perm_gen LAST. If dead space is
  // allowed a range of dead object may get overwritten by a dead int
  // array. If perm_gen is not traversed last a klassOop may get
  // overwritten. This is fine since it is dead, but if the class has dead
  // instances we have to skip them, and in order to find their size we
  // need the klassOop! 
  //
  // It is not required that we traverse spaces in the same order in
  // phase2, phase3 and phase4, but the ValidateMarkSweep live oops
  // tracking expects us to do so. See comment under phase4.

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  // Begin compacting into the old gen
  PSMarkSweepDecorator::set_destination_decorator_tenured();

  // This will also compact the young gen spaces.
  old_gen->precompact();

  // Compact the perm gen into the perm gen
  PSMarkSweepDecorator::set_destination_decorator_perm_gen();

  perm_gen->precompact();
}

// This should be moved to the shared markSweep code!
class PSAlwaysTrueClosure: public BoolObjectClosure {
public:
  void do_object(oop p) { ShouldNotReachHere(); }
  bool do_object_b(oop p) { return true; }
};
static PSAlwaysTrueClosure always_true;

void PSMarkSweep::mark_sweep_phase3() {
  // Adjust the pointers to reflect the new locations
  EventMark m("3 adjust pointers");
  TraceTime tm("phase 3", PrintGC && Verbose, true, gclog_or_tty);
  trace("3");

  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSYoungGen* young_gen = heap->young_gen();
  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  // General strong roots.
  Universe::oops_do(adjust_root_pointer_closure());
  JNIHandles::oops_do(adjust_root_pointer_closure());   // Global (strong) JNI handles
  Threads::oops_do(adjust_root_pointer_closure());
  ObjectSynchronizer::oops_do(adjust_root_pointer_closure());
  FlatProfiler::oops_do(adjust_root_pointer_closure());
  // CSO_AllClasses
  SystemDictionary::oops_do(adjust_root_pointer_closure());

  guarantee(!jvmdi::enabled(), "Should not be used with jvmdi");
  vmSymbols::oops_do(adjust_root_pointer_closure());

  // Now adjust pointers in remaining weak roots.  (All of which should
  // have been cleared if they pointed to non-surviving objects.)
  // Global (weak) JNI handles
  JNIHandles::weak_oops_do(&always_true, adjust_root_pointer_closure());

  NOT_CORE(CodeCache::oops_do(adjust_pointer_closure()));
  SymbolTable::oops_do(adjust_root_pointer_closure());
  StringTable::oops_do(adjust_root_pointer_closure());
  ReferenceProcessor::oops_do_statics(adjust_root_pointer_closure());
  ref_processor()->oops_do(adjust_root_pointer_closure());
  PSScavenge::reference_processor()->oops_do(adjust_root_pointer_closure());

  adjust_marks();

  young_gen->adjust_pointers();
  old_gen->adjust_pointers();
  perm_gen->adjust_pointers();
}

void PSMarkSweep::mark_sweep_phase4() {
  EventMark m("4 compact heap");
  TraceTime tm("phase 4", PrintGC && Verbose, true, gclog_or_tty);
  trace("4");

  // All pointers are now adjusted, move objects accordingly

  // It is imperative that we traverse perm_gen first in phase4. All
  // classes must be allocated earlier than their instances, and traversing
  // perm_gen first makes sure that all klassOops have moved to their new
  // location before any instance does a dispatch through it's klass!
  ParallelScavengeHeap* heap = (ParallelScavengeHeap*)Universe::heap();
  assert(heap->kind() == CollectedHeap::ParallelScavengeHeap, "Sanity");

  PSYoungGen* young_gen = heap->young_gen();
  PSOldGen* old_gen = heap->old_gen();
  PSPermGen* perm_gen = heap->perm_gen();

  perm_gen->compact();
  old_gen->compact();
  young_gen->compact();
}

jlong PSMarkSweep::millis_since_last_gc() { 
  jlong ret_val = os::javaTimeMillis() - _time_of_last_gc; 
  // XXX See note in genCollectedHeap::millis_since_last_gc().
  if (ret_val < 0) {
    NOT_PRODUCT(warning("time warp: %d", ret_val);)
    return 0;
  }
  return ret_val;
}

void PSMarkSweep::reset_millis_since_last_gc() { 
  _time_of_last_gc = os::javaTimeMillis(); 
}
