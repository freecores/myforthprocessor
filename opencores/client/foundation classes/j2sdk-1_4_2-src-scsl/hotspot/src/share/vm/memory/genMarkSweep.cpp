#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)genMarkSweep.cpp	1.7 03/01/23 12:08:23 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_genMarkSweep.cpp.incl"

void GenMarkSweep::invoke_at_safepoint(int level, ReferenceProcessor* rp,
  bool clear_all_softrefs) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at a safepoint");

  // hook up weak ref data so it can be used during Mark-Sweep
  assert(ref_processor() == NULL, "no stomping");
  _ref_processor = rp;
  assert(rp != NULL, "should be non-NULL");

  TraceTime t1("Full GC", PrintGC && !PrintGCDetails, true, gclog_or_tty);

  // When collecting the permanent generation methodOops may be moving,
  // so we either have to flush all bcp data or convert it into bci.
  NOT_CORE(CodeCache::gc_prologue());
  Threads::gc_prologue();

  // Increment the invocation count for the permanent generation, since it is 
  // implicitly collected whenever we do a full mark sweep collection. 
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  gch->perm_gen()->stat_record()->invocations++;

  // Capture heap size before collection for printing.
  size_t gch_prev_used = gch->used();

  bool marked_for_unloading = false;

  allocate_stacks();

  mark_sweep_phase1(level, marked_for_unloading, clear_all_softrefs);

  if (jvmpi::is_event_enabled(JVMPI_EVENT_OBJECT_FREE)) {
    JVMPI_Object_Free clo;
    gch->object_iterate(&clo);
  }

  mark_sweep_phase2();

  // Don't add any more derived pointers during phase3
  COMPILER2_ONLY(assert(DerivedPointerTable::is_active(), "Sanity"));
  COMPILER2_ONLY(DerivedPointerTable::set_active(false));
    
  mark_sweep_phase3(level);

  VALIDATE_MARK_SWEEP_ONLY(if (ValidateMarkSweep) guarantee(_root_refs_stack->length() == 0, "should be empty by now"));

  mark_sweep_phase4();

  VALIDATE_MARK_SWEEP_ONLY(if (ValidateMarkSweep) guarantee(_live_oops->length() == _live_oops_moved_to->length(), "should be the same size"));

  restore_marks();

  // Set saved marks for allocation profiler (and other things? -- dld)
  // (Should this be in general part?)
  gch->save_marks();

  deallocate_stacks();

  // "free at last gc" is calculated from these.
  Universe::set_heap_capacity_at_last_gc(Universe::heap()->capacity());
  Universe::set_heap_used_at_last_gc(Universe::heap()->used());

  // If compaction completely evacuated all generations younger than this
  // one, then we can clear the card table.  Otherwise, we must invalidate
  // it (consider all cards dirty).  In the future, we might consider doing 
  // compaction within generations only, and doing card-table sliding.
  bool all_empty = true;
  for (int i = 0; all_empty && i < level; i++) {
    Generation* g = gch->get_gen(i);
    all_empty = all_empty && gch->get_gen(i)->used() == 0;
  }
  GenRemSet* rs = gch->rem_set();
  if (all_empty) {
    // We've evacuated all generations below us.
    Generation* g = gch->get_gen(level-1);
    rs->clear_into_gen_and_younger(g);
  } else {
    rs->invalidate(gch->reserved_region());
  }

  Threads::gc_epilogue();
  NOT_CORE(CodeCache::gc_epilogue());

  if (jvmdi::enabled()) {
    jvmdi::gc_epilogue();
  }

  if (PrintGC && !PrintGCDetails) {
    gch->print_heap_change(gch_prev_used);
  }

  // refs processing: clean slate
  _ref_processor = NULL;

  // Update time of last gc for all generations we collected
  // (which curently is all the generations in the heap).
  gch->update_time_of_last_gc(os::javaTimeMillis());
}

void GenMarkSweep::allocate_stacks() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  // Scratch request on behalf of oldest generation; will do no
  // allocation.
  ScratchBlock* scratch = gch->gather_scratch(gch->_gens[gch->_n_gens-1], 0);

  // $$$ To cut a corner, we'll only use the first scratch block, and then
  // revert to malloc.
  _preserved_count_max = scratch->num_words * sizeof(juint) / sizeof(PreservedMark);
  _preserved_marks = (PreservedMark*)scratch;
  _preserved_count = 0;
  _preserved_mark_stack = NULL;
  _preserved_oop_stack = NULL;

  _marking_stack       = new GrowableArray<oop>(4000, true);

  int size = SystemDictionary::number_of_classes() * 2;
  _revisit_klass_stack = new GrowableArray<Klass*>(size, true);

#ifdef VALIDATE_MARK_SWEEP
  if (ValidateMarkSweep) {
    _root_refs_stack    = new GrowableArray<oop*>(100, true);
    _other_refs_stack   = new GrowableArray<oop*>(100, true);
    _adjusted_pointers  = new GrowableArray<oop*>(100, true);
    _live_oops          = new GrowableArray<oop>(100, true);
    _live_oops_moved_to = new GrowableArray<oop>(100, true);
    _live_oops_size     = new GrowableArray<size_t>(100, true);
  }
#endif
}


void GenMarkSweep::deallocate_stacks() {
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

#ifdef VALIDATE_MARK_SWEEP
  if (ValidateMarkSweep) {
    _root_refs_stack->clear_and_deallocate();
    _other_refs_stack->clear_and_deallocate();
    _adjusted_pointers->clear_and_deallocate();
    _live_oops->clear_and_deallocate();
    _live_oops_size->clear_and_deallocate();
    _live_oops_moved_to->clear_and_deallocate();
    _live_oops_index = 0;
    _live_oops_index_at_perm = 0;
  }
#endif
}

void GenMarkSweep::mark_sweep_phase1(int level, 
				  bool& marked_for_unloading,
				  bool clear_all_softrefs) {
  // Recursively traverse all live objects and mark them
  EventMark m("1 mark object");
  TraceTime tm("phase 1", PrintGC && Verbose, true, gclog_or_tty);
  trace(" 1");

  VALIDATE_MARK_SWEEP_ONLY(reset_live_oop_tracking(false));

  GenCollectedHeap* gch = GenCollectedHeap::heap();

  // Because follow_root_closure is created statically, cannot
  // use OopsInGenClosure constructor which takes a generation,
  // as the Universe has not been created when the static constructors
  // are run.
  follow_root_closure.set_orig_generation(gch->get_gen(level));

  gch->process_strong_roots(level,
			    false, // Younger gens are not roots.
			    true,  // Collecting permanent generation.
			    GenCollectedHeap::CSO_SystemClasses,
			    &follow_root_closure, &follow_root_closure);

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
                                                 &is_alive, &keep_alive,
                                                 &follow_stack_closure);

  // Follow system dictionary roots and unload classes
  bool purged_class = SystemDictionary::do_unloading();
  assert(_marking_stack->is_empty(), "stack should be empty by now");

  // Follow code cache roots (has to be done after system dictionary,
  // assumes all live klasses are marked)
  NOT_CORE(CodeCache::do_unloading(purged_class, marked_for_unloading);)
  // did we mark any nmethods for unloading?

  // Update subklass/sibling/implementor links of live klasses
  follow_weak_klass_links();

  // Visit symbol and interned string tables and delete unmarked oops
  SymbolTable::unlink();
  StringTable::unlink();

  assert(_marking_stack->is_empty(), "stack should be empty by now");
}


void GenMarkSweep::mark_sweep_phase2() {
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

  GenCollectedHeap* gch = GenCollectedHeap::heap();
  Generation* pg = gch->perm_gen();
  
  EventMark m("2 compute new addresses");
  TraceTime tm("phase 2", PrintGC && Verbose, true, gclog_or_tty);
  trace("2");

  VALIDATE_MARK_SWEEP_ONLY(reset_live_oop_tracking(false));

  gch->prepare_for_compaction();

  VALIDATE_MARK_SWEEP_ONLY(_live_oops_index_at_perm = _live_oops_index);
  CompactPoint perm_cp(pg, NULL, NULL);
  pg->prepare_for_compaction(&perm_cp);
}

class GenAdjustPointersClosure: public GenCollectedHeap::GenClosure {
public:
  void do_generation(Generation* gen) {
    gen->adjust_pointers();
  }
};

void GenMarkSweep::mark_sweep_phase3(int level) {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  Generation* pg = gch->perm_gen();

  // Adjust the pointers to reflect the new locations
  EventMark m("3 adjust pointers");
  TraceTime tm("phase 3", PrintGC && Verbose, true, gclog_or_tty);
  trace("3");

  VALIDATE_MARK_SWEEP_ONLY(reset_live_oop_tracking(false));

  // Because the two closures below are created statically, cannot
  // use OopsInGenClosure constructor which takes a generation,
  // as the Universe has not been created when the static constructors
  // are run.
  adjust_root_pointer_closure.set_orig_generation(gch->get_gen(level));
  adjust_pointer_closure.set_orig_generation(gch->get_gen(level));

  gch->process_strong_roots(level,
			    false, // Younger gens are not roots.
			    true,  // Collecting permanent generation.
			    GenCollectedHeap::CSO_AllClasses,
			    &adjust_root_pointer_closure,
			    &adjust_root_pointer_closure);

  // Now adjust pointers in remaining weak roots.  (All of which should
  // have been cleared if they pointed to non-surviving objects.)
  gch->process_weak_roots(&adjust_root_pointer_closure,
			  &adjust_pointer_closure);

  adjust_marks();
  GenAdjustPointersClosure blk;
  gch->generation_iterate(&blk, true);
  pg->adjust_pointers();
}

class GenCompactClosure: public GenCollectedHeap::GenClosure {
public:
  void do_generation(Generation* gen) {
    gen->compact();
  }
};

void GenMarkSweep::mark_sweep_phase4() {
  // All pointers are now adjusted, move objects accordingly

  // It is imperative that we traverse perm_gen first in phase4. All
  // classes must be allocated earlier than their instances, and traversing
  // perm_gen first makes sure that all klassOops have moved to their new
  // location before any instance does a dispatch through it's klass!

  // The ValidateMarkSweep live oops tracking expects us to traverse spaces
  // in the same order in phase2, phase3 and phase4. We don't quite do that
  // here (perm_gen first rather than last), so we tell the validate code
  // to use a higher index (saved from phase2) when verifying perm_gen.
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  Generation* pg = gch->perm_gen();

  EventMark m("4 compact heap");
  TraceTime tm("phase 4", PrintGC && Verbose, true, gclog_or_tty);
  trace("4");

  VALIDATE_MARK_SWEEP_ONLY(reset_live_oop_tracking(true));

  pg->compact();
  
  VALIDATE_MARK_SWEEP_ONLY(reset_live_oop_tracking(false));

  GenCompactClosure blk;
  gch->generation_iterate(&blk, true);
}
