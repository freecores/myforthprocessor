#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)parNewGeneration.cpp	1.46 03/01/23 12:09:33 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_parNewGeneration.cpp.incl"


ParScanThreadState::ParScanThreadState(Space* to_space_,
				       Generation* old_gen_,
				       int thread_num_,
				       ObjToScanQueue *work_queue_) :
  _to_space(to_space_), _old_gen(old_gen_), _thread_num(thread_num_),
  _work_queue(work_queue_), _to_space_full(false),
  _ageTable(false), // false ==> not the global age table, no perf data.
  _to_space_alloc_buffer(ParallelGCToSpaceAllocBufferSize),
  _to_space_closure(NULL), _old_gen_closure(NULL),
  _to_space_root_closure(NULL), _old_gen_root_closure(NULL),
  _pushes(0), _pops(0), _steals(0), _steal_attempts(0), _term_attempts(0),
  _strong_roots_time(0.0), _term_time(0.0)
{
  _hash_seed = 17;  // Might want to take time-based random value.
  _start = os::elapsedTime();
}

void ParScanThreadState::trim_queues(int max_size) {
  ObjToScanQueue* queue = work_queue();
  while (queue->size() > (juint)max_size) { 
    oop obj_to_scan;
    if (queue->pop_local(obj_to_scan)) {
      note_pop();

      if ((HeapWord *)obj_to_scan < young_old_boundary()) {
	// object is in to_space
	if (obj_to_scan->is_objArray_fast() &&
	    arrayOop(obj_to_scan)->length() >=
	    ParGCMaxArrayAsNormalObject) {
	  // Use the closure that does a recursive trim.
	  obj_to_scan->oop_iterate(_to_space_root_closure);
	} else {
	  obj_to_scan->oop_iterate(_to_space_closure);
	}
      } else {
	// object is in old generation
	if (obj_to_scan->is_objArray_fast() &&
	    arrayOop(obj_to_scan)->length() >=
	    ParGCMaxArrayAsNormalObject) {
	  // Use the closure that does a recursive trim.
	  obj_to_scan->oop_iterate(_old_gen_root_closure);
	} else {
	  obj_to_scan->oop_iterate(_old_gen_closure);
	}
      }
    }
  }
}

HeapWord* ParScanThreadState::alloc_in_to_space_slow(size_t word_sz) {

  // Otherwise, if the object is small enough, try to reallocate the
  // buffer.
  HeapWord* obj = NULL;
  if (!_to_space_full) {
    if (word_sz * 100 <
	ParallelGCBufferWastePct * to_space_alloc_buffer()->word_sz()) {
      // Is small enough; abandon this buffer and start a new one.
      to_space_alloc_buffer()->retire(false, false);
      size_t buf_size = to_space_alloc_buffer()->word_sz();
      HeapWord* buf_space = to_space()->par_allocate(buf_size);
      while (buf_space == NULL) {
	size_t free_bytes = to_space()->free();
	if (free_bytes < K) break; // Leave the last 1K.
	size_t free_words = free_bytes >> LogHeapWordSize;
	to_space_alloc_buffer()->set_word_size(free_words);
	buf_space = to_space()->par_allocate(free_words);
      }
      if (buf_space != NULL) {
	to_space_alloc_buffer()->set_buf(buf_space);
	obj = to_space_alloc_buffer()->allocate(word_sz);
	assert(obj != NULL, "Buffer was definitely big enough...");

      } else {
	// We're used up.
	_to_space_full = true;
      }

    } else {
      // Too large; allocate the object individually.
      obj = to_space()->par_allocate(word_sz);
    }
  }
  return obj;
}


void ParScanThreadState::undo_alloc_in_to_space(HeapWord* obj,
						size_t word_sz) {
  // Is the alloc in the current alloc buffer?
  if (to_space_alloc_buffer()->contains(obj)) {
    assert(to_space_alloc_buffer()->contains(obj + word_sz - 1),
	   "Should contain whole object.");
    to_space_alloc_buffer()->undo_allocation(obj, word_sz);
  } else {
    SharedHeap::fill_region_with_object(MemRegion(obj, word_sz));
  }
}


ParScanClosure::ParScanClosure(ParNewGeneration* g,
			       ParScanThreadState* par_scan_state) :
  OopsInGenClosure(g), _par_scan_state(par_scan_state), _g(g)
{
  assert(_g->level() == 0, "Optimized for youngest generation");
  _boundary = _g->reserved().end();
}


class ParRootScanWithBarrierTwoGensClosure: public ParScanClosure {
public:
  ParRootScanWithBarrierTwoGensClosure(ParNewGeneration* g,
				       ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p) { do_oop_work(p, true, true, true, false); }
};
class ParRootScanWithBarrierNGensClosure: public ParScanClosure {
public:
  ParRootScanWithBarrierNGensClosure(ParNewGeneration* g,
				     ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p) { do_oop_work(p, true, false, true, false); }
};
class ParRootScanWithoutBarrierClosure: public ParScanClosure {
public:
  ParRootScanWithoutBarrierClosure(ParNewGeneration* g,
				   ParScanThreadState* par_scan_state) :
    ParScanClosure(g, par_scan_state) {}
  void do_oop(oop* p) { do_oop_work(p, false, true, true, false); }
};

#ifdef WIN32
#pragma warning(disable: 4786) /* identifier was truncated to '255' characters in the browser information */
#endif

class ParEvacuateFollowersClosure: public VoidClosure {
  ParScanThreadState* _par_scan_state;
  ParScanThreadState* par_scan_state() { return _par_scan_state; }

  // We want to preserve the specific types here (rather than "OopClosure") 
  // for later de-virtualization of do_oop calls.
  ParScanWithoutBarrierClosure* _to_space_closure;
  ParScanWithoutBarrierClosure* to_space_closure() {
    return _to_space_closure;
  }
  ParRootScanWithoutBarrierClosure* _to_space_root_closure;
  ParRootScanWithoutBarrierClosure* to_space_root_closure() {
    return _to_space_root_closure;
  }

  ParScanWithBarrierClosure* _old_gen_closure;
  ParScanWithBarrierClosure* old_gen_closure () {
    return _old_gen_closure;
  }
  ParRootScanWithBarrierTwoGensClosure* _old_gen_root_closure;
  ParRootScanWithBarrierTwoGensClosure* old_gen_root_closure () {
    return _old_gen_root_closure;
  }

  ParNewGeneration* _par_gen;
  ParNewGeneration* par_gen() { return _par_gen; }
  
  ObjToScanQueueSet*  _task_queues;
  ObjToScanQueueSet*  task_queues() { return _task_queues; }

  ParallelTaskTerminator* _terminator;
  ParallelTaskTerminator* terminator() { return _terminator; }

public:
  ParEvacuateFollowersClosure(
    ParScanThreadState* par_scan_state_,
    ParScanWithoutBarrierClosure* to_space_closure_,
    ParScanWithBarrierClosure* old_gen_closure_,
    ParRootScanWithoutBarrierClosure* to_space_root_closure_,
    ParNewGeneration* par_gen_,
    ParRootScanWithBarrierTwoGensClosure* old_gen_root_closure_,
    ObjToScanQueueSet* task_queues_,
    ParallelTaskTerminator* terminator_) :

    _par_scan_state(par_scan_state_),
    _to_space_closure(to_space_closure_),
    _old_gen_closure(old_gen_closure_),
    _to_space_root_closure(to_space_root_closure_),
    _old_gen_root_closure(old_gen_root_closure_),
    _par_gen(par_gen_),
    _task_queues(task_queues_),
    _terminator(terminator_)
    {}

  void do_void() {
    ObjToScanQueue* work_q = par_scan_state()->work_queue();

    while (true) {

      // Scan to-space and old-gen objs until we run out of both.
      oop obj_to_scan;
      par_scan_state()->trim_queues(0);

      // We have no local work, attempt to steal from other threads.

      // attempt to steal work from promoted.
      par_scan_state()->note_steal_attempt();
      if (task_queues()->steal(par_scan_state()->thread_num(),
			       par_scan_state()->hash_seed(),
			       obj_to_scan)) {
	par_scan_state()->note_steal();
	bool res = work_q->push(obj_to_scan);
	assert(res, "Empty queue should have room for a push.");

	par_scan_state()->note_push();
	//   if successful, goto Start.
	continue;

	// try global overflow list.
      } else if (par_gen()->take_from_overflow_list(par_scan_state())) {
	continue;

      }

      // Otherwise, offer termination.
      par_scan_state()->start_term_time();
      if (terminator()->offer_termination()) break;
      par_scan_state()->end_term_time();
    }
    // Finish the last termination pause.
    par_scan_state()->end_term_time();
    // Retire the last allocation buffers.
    par_scan_state()->to_space_alloc_buffer()->retire(true, false);
    
  }
};

class ParNewGenTask: public AbstractGangTask {
  ParNewGeneration* _gen;
  WorkGang* _workers;
  Generation* _next_gen;
  HeapWord* _young_old_boundary;

  // The per-thread work queues, available here for stealing.
  ObjToScanQueueSet* _task_queues;
  ParallelTaskTerminator _term;

  Mutex _stats_lock;  
  int _pushes;
  int _pops;
  int _steals;

public:
  ParNewGenTask(ParNewGeneration* gen, Generation* next_gen,
		WorkGang* workers, HeapWord* young_old_boundary,
		ObjToScanQueueSet *task_queues_) :
    AbstractGangTask("ParNewGeneration collection"),
    _gen(gen), _next_gen(next_gen), _workers(workers),
    _task_queues(task_queues_),
    _term(workers->total_workers(), _task_queues),
    _stats_lock(Mutex::leaf, "ParNewGen stats lock", true),
    _pushes(0), _pops(0), _steals(0),
    _young_old_boundary(young_old_boundary)
  {}

  ObjToScanQueueSet* task_queues() { return _task_queues; }

  ObjToScanQueue *work_queue(int i) {
        return task_queues()->queue(i);
  }

  HeapWord* young_old_boundary() { return _young_old_boundary; }

  void work(int i) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    // Since this is being done in a separate thread, need new resource
    // and handle marks.
    ResourceMark rm;
    HandleMark hm;
    // We would need multiple old-gen queues otherwise.
    guarantee(gch->n_gens() == 2,
	      "Par young collection currently only works with one older gen.");
    guarantee(!Universe::jvmpi_slow_allocation(),
	      "To support jvmpi_slow_allocation, must add new "
	      "ParScanClosure types.");

    Generation* old_gen = gch->next_gen(_gen);

    ParScanThreadState par_scan_state(_gen->to(), old_gen, i, work_queue(i));

    par_scan_state.set_young_old_boundary(_young_old_boundary);

    ParScanWithoutBarrierClosure scan_without_gc_barrier(_gen,
							 &par_scan_state);
    ParScanWithBarrierClosure    scan_with_gc_barrier(_gen,
						      &par_scan_state);
    scan_with_gc_barrier.set_generation(old_gen);
    
    ParRootScanWithoutBarrierClosure
      scan_root_without_gc_barrier(_gen, &par_scan_state);
    // One of these two will be passed to process_strong_roots, which will
    // set its generation.  The first is for two-gen configs where the
    // old-gen collects the perm gen; the second is for arbitrary configs.
    ParRootScanWithBarrierTwoGensClosure
      scan_gen_2_root_with_gc_barrier(_gen, &par_scan_state);
    ParRootScanWithBarrierNGensClosure
      scan_gen_n_root_with_gc_barrier(_gen, &par_scan_state);

    // This closure will always be bound to the old gen; it will be used
    // in evacuate_followers.
    ParRootScanWithBarrierTwoGensClosure
      scan_old_root_with_gc_barrier(_gen, &par_scan_state);
    scan_old_root_with_gc_barrier.set_generation(old_gen);

    ParEvacuateFollowersClosure
      evacuate_followers(&par_scan_state,
			 &scan_without_gc_barrier,
			 &scan_with_gc_barrier,
			 &scan_root_without_gc_barrier,
			 _gen,
			 &scan_old_root_with_gc_barrier,
			 task_queues(),
			 &_term);

    par_scan_state.set_to_space_closure(&scan_without_gc_barrier);
    par_scan_state.set_old_gen_closure(&scan_with_gc_barrier);
    par_scan_state.set_to_space_root_closure(&scan_root_without_gc_barrier);
    par_scan_state.set_old_gen_root_closure(&scan_old_root_with_gc_barrier);

    // Not very pretty.
    CollectorPolicy* cp = gch->collector_policy();
    
    ParScanClosure* older_gen_closure;
    if (!cp->is_train_policy()) {
      older_gen_closure = &scan_gen_2_root_with_gc_barrier;
    } else {
      older_gen_closure = &scan_gen_n_root_with_gc_barrier;
    }

    par_scan_state.start_strong_roots();
    gch->process_strong_roots(_gen->level(),
                              true, // Process younger gens, if any,
                                    // as strong roots.
                              false,// not collecting perm generation.
                              GenCollectedHeap::CSO_AllClasses,
                              older_gen_closure, // See above.
                              &scan_root_without_gc_barrier);
    par_scan_state.end_strong_roots();

    // "evacuate followers".
    evacuate_followers.do_void();

    // Every thread has its own age table.  We need to merge them all into
    // one.
    {
      MutexLocker atable(ParGCRareEvent_lock);
      ageTable *local_table = par_scan_state.age_table();
      _gen->age_table()->merge(local_table);
    }

    // Inform old gen that we're done.
    _next_gen->par_promote_alloc_done(i);
    _next_gen->par_oop_since_save_marks_iterate_done(i);

    // This could conceivably become a bottleneck; if so, we'll put the
    // stat's gathering under the flag.
    if (PAR_STATS_ENABLED) {
      MutexLocker x(&_stats_lock);
      _pushes += par_scan_state.pushes();
      _pops   += par_scan_state.pops();
      _steals += par_scan_state.steals();
      if (ParallelGCVerbose) {
	tty->print("Thread %d complete:\n"
		   "  Pushes: %7d    Pops: %7d    Steals %7d (in %d attempts)\n",
		   i,
		   par_scan_state.pushes(),
		   par_scan_state.pops(),
		   par_scan_state.steals(),
		   par_scan_state.steal_attempts());
	if (par_scan_state.overflow_pushes() > 0 ||
	    par_scan_state.overflow_refills() > 0) {
	  tty->print("  Overflow pushes: %7d    "
		     "Overflow refills: %7d for %d objs.\n",
		     par_scan_state.overflow_pushes(),
		     par_scan_state.overflow_refills(),
		     par_scan_state.overflow_refill_objs());
	}

	double elapsed = par_scan_state.elapsed();
	double strong_roots = par_scan_state.strong_roots_time();
	double term = par_scan_state.term_time();
	tty->print("  Elapsed: %7.2f ms.\n"
		   "    Strong roots: %7.2f ms (%6.2f%%)\n"
		   "    Termination:  %7.2f ms (%6.2f%%) (in %d entries)\n",
		   elapsed * 1000.0,
		   strong_roots * 1000.0, (strong_roots*100.0/elapsed),
		   term * 1000.0, (term*100.0/elapsed),
		   par_scan_state.term_attempts());
      }
    }
    _pushes += par_scan_state.pushes();
    _pops   += par_scan_state.pops();
    _steals += par_scan_state.steals();

  }

  int pushes() { return _pushes; }
  int pops()   { return _pops; }
  int steals() { return _steals; }
};

ParNewGeneration::
ParNewGeneration(ReservedSpace rs, size_t initial_byte_size, int level)
  : DefNewGeneration(rs, initial_byte_size, level, "PCopy"),
  _overflow_list(NULL)
{
  _task_queues = new ObjToScanQueueSet(ParallelGCThreads);
  guarantee(_task_queues != NULL, "task_queues allocation failure.");

  for (uint i1 = 0; i1 < ParallelGCThreads; i1++) {
    ObjToScanQueuePadded *q_padded = new ObjToScanQueuePadded();
    guarantee(q_padded != NULL, "work_queue Allocation failure.");

    _task_queues->register_queue(i1, &q_padded->work_queue);
  }

  for (uint i2 = 0; i2 < ParallelGCThreads; i2++)
    _task_queues->queue(i2)->initialize();

  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    const char* cname;
    cname = PerfDataManager::counter_name(_gen_counters->name_space(), "threads");
    PerfDataManager::create_constant(cname, PerfData::U_None,
                                     ParallelGCThreads, CHECK);
  }
}

ParNewGeneration::
ParKeepAliveClosure::ParKeepAliveClosure(ScanWeakRefClosure* cl) :
  DefNewGeneration::KeepAliveClosure(cl) {}

void
ParNewGeneration::
ParKeepAliveClosure::do_oop(oop* p) {
  // We never expect to see a null reference being processed
  // as a weak reference.
  assert (*p != NULL, "expected non-null ref");
  assert ((*p)->is_oop(), "expected an oop while scanning weak refs");

  _cl->do_oop_nv(p);

  if (Universe::heap()->is_in_reserved(p)) {
    _rs->write_ref_field_gc_par(p, *p);
  }
}

// Closure for scanning ParNewGeneration.
// Same as ScanClosure, except does parallel GC barrier.
class ScanClosureWithParBarrier: public ScanClosure {
public:
  ScanClosureWithParBarrier(ParNewGeneration* g, bool gc_barrier) :
    ScanClosure(g, gc_barrier) {}
  void do_oop(oop* p);
};

void ScanClosureWithParBarrier::do_oop(oop* p) {
  oop obj = *p;
  // Should we copy the obj?
  if (obj != NULL) {
    if ((HeapWord*)obj < _boundary) {
      assert(!_g->to()->contains(obj), "Scanning field twice?");
      if (obj->is_forwarded()) {
        *p = obj->forwardee();
      } else {        
        *p = _g->DefNewGeneration::copy_to_survivor_space(obj, p);
      }
    }
    if (_gc_barrier) {
      // If p points to a younger generation, mark the card.
      if ((HeapWord*)obj < _gen_boundary) {
	_rs->write_ref_field_gc_par(p, obj);
      }
    }
  }
}

class EvacuateFollowersClosureGeneral: public VoidClosure {
    GenCollectedHeap* _gch;
    int _level;
    OopsInGenClosure* _scan_cur_or_nonheap;
    OopsInGenClosure* _scan_older;
  public:
    EvacuateFollowersClosureGeneral(GenCollectedHeap* gch, int level,
				    OopsInGenClosure* cur,
				    OopsInGenClosure* older);
    void do_void();
  };

EvacuateFollowersClosureGeneral::
EvacuateFollowersClosureGeneral(GenCollectedHeap* gch, int level,
				OopsInGenClosure* cur,
				OopsInGenClosure* older) :
  _gch(gch), _level(level),
  _scan_cur_or_nonheap(cur), _scan_older(older)
{}

void EvacuateFollowersClosureGeneral::do_void() {
  do {
    // Beware: this call will lead to closure applications via virtual
    // calls.
    _gch->oop_since_save_marks_iterate(_level,
				       _scan_cur_or_nonheap,
				       _scan_older);
  } while (!_gch->no_allocs_since_save_marks(_level));
}


bool ParNewGeneration::_avoid_promotion_undo = false;


// A Generation that does parallel young-gen collection.
void ParNewGeneration::collect(bool   full,
                               bool   clear_all_soft_refs,
			       size_t size,
                               bool   is_large_noref,
                               bool   is_tlab) {
  assert(full || size > 0, "otherwise we don't want to collect");
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  WorkGang* workers = gch->workers();
  _next_gen = gch->next_gen(this);
  assert(_next_gen != NULL, 
    "This must be the youngest gen, and not the only gen");
  assert(gch->n_gens() == 2,
	 "Par collection currently only works with single older gen.");
  // Do we have to avoid promotion_undo?
  if (gch->collector_policy()->is_concurrent_mark_sweep_policy()) {
    _avoid_promotion_undo = true;
  }
  // If the next generation is too full to accomodate worst-case promotion
  // from this generation, pass on collection; let the next generation
  // do it.
  if (!full_promotion_would_succeed()) {
    gch->set_incremental_collection_will_fail();
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("ParNewGeneration::collect"
                    " contiguous_available: %d < used: %d",
                    _next_gen->max_contiguous_available(), used());
    }
    return;
  }

  TraceTime t1("ParNew", PrintGC && !PrintGCDetails, true, gclog_or_tty);
  // Capture heap used before collection (for printing).
  size_t gch_prev_used = gch->used();

  SpecializationStats::clear();

  age_table()->clear();
  to()->clear();

  gch->save_marks();
  assert(workers != NULL, "Need parallel worker threads.");

  ParNewGenTask tsk(this, _next_gen, workers, reserved().end(), task_queues());
  int n_workers = workers->total_workers();
  gch->set_par_threads(n_workers);
  gch->change_strong_roots_parity();
  gch->rem_set()->prepare_for_younger_refs_iterate(true);
  // It turns out that even when we're using 1 thread, doing the work in a
  // separate thread causes wide variance in run times.  We can't help this 
  // in the multi-threaded case, but we special-case n=1 here to get
  // repeatable measurements of the 1-thread overhead of the parallel code.
  if (n_workers > 1) {
    workers->run_task(&tsk);
  } else {
    tsk.work(0);
  }
  gch->set_par_threads(0);  // 0 ==> non-parallel.

  if (ParallelGCVerbose) {
    tty->print("Thread totals:\n"
	       "  Pushes: %7d    Pops: %7d    Steals %7d (sum = %7d).\n",
	       tsk.pushes(), tsk.pops(), tsk.steals(),
	       tsk.pops()+tsk.steals());
  }
  assert(tsk.pushes() == tsk.pops() + tsk.steals(),
	 "Or else the queues are leaky.");

  // For now, process discovered weak refs sequentially.
  NOT_COMPILER2(ReferencePolicy *soft_ref_policy = new LRUCurrentHeapPolicy();)
  COMPILER2_ONLY(ReferencePolicy *soft_ref_policy = new LRUMaxHeapPolicy();)
 
  IsAliveClosure is_alive(this);

  ScanWeakRefClosure scan_weak_ref(this);
  ParKeepAliveClosure keep_alive(&scan_weak_ref);

  ScanClosure               scan_without_gc_barrier(this, false);
  ScanClosureWithParBarrier scan_with_gc_barrier(this, true);
  EvacuateFollowersClosureGeneral
    evacuate_followers(gch, _level,
		       &scan_without_gc_barrier, 
		       &scan_with_gc_barrier);

  gch->save_marks();
  ref_processor()->process_discovered_references(soft_ref_policy,
						 &is_alive,
						 &keep_alive,
						 &evacuate_followers);

  // Swap the survivor spaces.
  eden()->clear();
  from()->clear();
  swap_spaces();

  assert(to()->is_empty(), "to space should be empty now");

  // Set the desired survivor size to half the real survivor space
  _tenuring_threshold =
    age_table()->compute_tenuring_threshold(to()->capacity()/HeapWordSize);

  if (PrintGC && !PrintGCDetails) {
    gch->print_heap_change(gch_prev_used);
  }
  update_time_of_last_gc(os::javaTimeMillis());
  SpecializationStats::print();
}


static const oop ClaimedForwardPtr = oop(0x4);

static int sum;
void ParNewGeneration::waste_some_time() {
  for (int i = 0; i < 100; i++) {
    sum += i;
  }
}

oop ParNewGeneration::real_forwardee_slow(oop obj) {
  // Spin-read if it is claimed but not yet written by another thread.
  oop forward_ptr = obj->forwardee();
  while (forward_ptr == ClaimedForwardPtr) {
    waste_some_time();
    assert(obj->is_forwarded(), "precondition");
    forward_ptr = obj->forwardee();
  }
  return forward_ptr;
}

#ifdef ASSERT
bool ParNewGeneration::is_legal_forward_ptr(oop p) {
  return
    (_avoid_promotion_undo && p == ClaimedForwardPtr)
    || Universe::heap()->is_in_reserved(p);
}
#endif


oop ParNewGeneration::copy_to_survivor_space_avoiding_promotion_undo(
	ParScanThreadState* par_scan_state, oop old, size_t sz, markOop m,
	bool jvmpi_slow_alloc) {
  // In the sequential version, this assert also says that the object is
  // not forwarded.  That might not be the case here.  It is the case that
  // the caller observed it to be not forwarded at some time in the past.
  assert(is_in_reserved(old), "shouldn't be scavenging this oop");
  HeapWord* obj_ptr = NULL;

  // The sequential code read "old->age()" below.  That doesn't work here,
  // since the age is in the mark word, and that might be overwritten with
  // a forwarding pointer by a parallel thread.  So we must read the mark
  // word here, install it in a local oopDesc, and then analyze it.
  oopDesc dummyOld;
  dummyOld.set_mark(m);
  assert(!dummyOld.is_forwarded(),
	 "should not be called with forwarding pointer mark word.");
  
  // Try allocating obj in to-space (unless too old or won't fit or JVMPI
  // enabled)
  if (dummyOld.age() < tenuring_threshold() &&
      !jvmpi_slow_alloc) {
      //!Universe::jvmpi_slow_allocation()
    obj_ptr = par_scan_state->alloc_in_to_space(sz);
  }

  // Otherwise try allocating obj tenured
  oop obj;
  oop forward_ptr;
  if (obj_ptr == NULL) {
      // Now attempt to install a null forwarding pointer (atomically),
      // to claim the right to install the real forwarding pointer.
      forward_ptr = old->forward_to_atomic(ClaimedForwardPtr);
      if (forward_ptr != NULL) {
	return real_forwardee(old);
      }
      obj = _next_gen->par_promote(par_scan_state->thread_num(),
				   old, m, sz);

      if (obj == NULL) {
	// A failed promotion likely means the MaxLiveObjectEvacuationRatio flag
	// is incorrectly set. In any case, its seriously wrong to be here!
	vm_exit_out_of_memory(sz*wordSize, "promotion");
      }
      old->forward_to(obj);
      forward_ptr = NULL;
      obj_ptr = (HeapWord*)obj;
  } else {
      // Is in to-space; do copying ourselves.
      Memory::copy_words_aligned((HeapWord*)old, obj_ptr, sz);
      obj = oop(obj_ptr);
      forward_ptr = old->forward_to_atomic(obj);
      // Restore the mark word copied above.
      obj->set_mark(m);
      // Increment age if obj still in new generation
      obj->incr_age();
      par_scan_state->age_table()->add(obj, sz);
  }
  assert(obj != NULL, "just checking");

  if (forward_ptr == NULL) {
    if (Universe::jvmpi_move_event_enabled()) {
      Universe::jvmpi_object_move(old, obj);
    }

    // Push it on one of the queues of to-be-scanned objects.
    if (!par_scan_state->work_queue()->push(obj)) {
      // Add stats for overflow pushes.
      push_on_overflow_list(old);
      par_scan_state->note_overflow_push();
    }
    par_scan_state->note_push();

    return obj;

  } else {
    // Oops.  Someone beat us to it.  Undo the allocation.  Where did we
    // allocate it?
    if (is_in_reserved(obj_ptr)) {
      // Must be in to_space.
      assert(to()->contains(obj_ptr), "Checking");
      if (forward_ptr == ClaimedForwardPtr) {
	// Wait to get the real forwarding pointer value.
	forward_ptr = real_forwardee(old);
      }
      par_scan_state->undo_alloc_in_to_space(obj_ptr, sz);
    } 
    return forward_ptr;
  }
}


oop ParNewGeneration::copy_to_survivor_space_with_undo(
	ParScanThreadState* par_scan_state, oop old, size_t sz, markOop m,
	bool jvmpi_slow_alloc) {

  // In the sequential version, this assert also says that the object is
  // not forwarded.  That might not be the case here.  It is the case that
  // the caller observed it to be not forwarded at some time in the past.
  assert(is_in_reserved(old), "shouldn't be scavenging this oop");
  HeapWord* obj_ptr = NULL;

  // The sequential code read "old->age()" below.  That doesn't work here,
  // since the age is in the mark word, and that might be overwritten with
  // a forwarding pointer by a parallel thread.  So we must read the mark
  // word here, install it in a local oopDesc, and then analyze it.
  oopDesc dummyOld;
  dummyOld.set_mark(m);
  assert(!dummyOld.is_forwarded(),
	 "should not be called with forwarding pointer mark word.");
  
  // Try allocating obj in to-space (unless too old or won't fit or JVMPI
  // enabled)
  if (dummyOld.age() < tenuring_threshold() &&
      !jvmpi_slow_alloc) {
      //!Universe::jvmpi_slow_allocation()
    obj_ptr = par_scan_state->alloc_in_to_space(sz);
  }

  // Otherwise try allocating obj tenured
  oop obj;
  oop forward_ptr;

  if (obj_ptr == NULL) {
      obj = _next_gen->par_promote(par_scan_state->thread_num(),
				   old, m, sz);

      if (obj == NULL) {
	// A failed promotion likely means the MaxLiveObjectEvacuationRatio
	// flag is incorrectly set. In any case, its seriously wrong to be
	// here!
	vm_exit_out_of_memory(sz*wordSize, "promotion");
      }
      obj_ptr = (HeapWord*)obj;
  } else {
      // Is in to-space; do copying ourselves.
      Memory::copy_words_aligned((HeapWord*)old, obj_ptr, sz);
      obj = oop(obj_ptr);
      // Restore the mark word copied above.
      obj->set_mark(m);
      // Increment age if obj still in new generation
      obj->incr_age();
      par_scan_state->age_table()->add(obj, sz);
  }
  assert(obj != NULL, "just checking");

  // Now attempt to install the forwarding pointer (atomically).
  // We have to copy the mark word before overwriting with forwarding
  // ptr, so we can restore it below in the copy.
  forward_ptr = old->forward_to_atomic(obj);

  if (forward_ptr == NULL) {
    if (Universe::jvmpi_move_event_enabled()) {
      Universe::jvmpi_object_move(old, obj);
    }

    // Push it on one of the queues of to-be-scanned objects.
    if (!par_scan_state->work_queue()->push(obj)) {
      // Add stats for overflow pushes.
      push_on_overflow_list(old);
      par_scan_state->note_overflow_push();
    }
    par_scan_state->note_push();

    return obj;

  } else {
    // Oops.  Someone beat us to it.  Undo the allocation.  Where did we
    // allocate it?
    if (is_in_reserved(obj_ptr)) {
      // Must be in to_space.
      assert(to()->contains(obj_ptr), "Checking");
      par_scan_state->undo_alloc_in_to_space(obj_ptr, sz);
    } else {
      assert(!_avoid_promotion_undo, "Should not be here if avoiding.");
      _next_gen->par_promote_alloc_undo(par_scan_state->thread_num(),
					obj_ptr, sz);
    }

    return forward_ptr;
  }
}


void ParNewGeneration::push_on_overflow_list(oop from_space_obj) {
  oop cur_overflow_list = _overflow_list;
  while (true) {
    from_space_obj->set_klass_to_list_ptr(cur_overflow_list);
    oop observed_overflow_list =
      (oop) atomic::compare_and_exchange_ptr((intptr_t)from_space_obj,
					     (intptr_t*)&_overflow_list,
					     (intptr_t)cur_overflow_list);
    if (observed_overflow_list == cur_overflow_list) break;
    // Otherwise...
    cur_overflow_list = observed_overflow_list;
  }
}

bool
ParNewGeneration::take_from_overflow_list(ParScanThreadState* par_scan_state) {
  // How many to take?
  ObjToScanQueue* work_q = par_scan_state->work_queue();
  int objsFromOverflow = MIN2(work_q->max_elems()/4,
			      (juint)ParGCDesiredObjsFromOverflowList);
  oop cur_overflow_list = _overflow_list;
  oop cur;
  while (cur_overflow_list) {

    int i = 0;
    cur = cur_overflow_list;
    while (i < objsFromOverflow && cur->klass() != NULL) {
      i++; cur = oop(cur->klass());
    }
    assert(cur != NULL, "program logic");
  
    oop cur_next = oop(cur->klass());
    oop observed_overflow_list =
      (oop) atomic::compare_and_exchange_ptr((intptr_t)cur_next,
					     (intptr_t*)&_overflow_list,
					     (intptr_t)cur_overflow_list);
    if (observed_overflow_list == cur_overflow_list) break;
    // Otherwise...
    cur_overflow_list = observed_overflow_list;
  }
  if (cur_overflow_list) {
    assert(cur != NULL, "program logic");
    // So now we own from cur_overflow_list to cur.
    cur->set_klass_to_list_ptr(NULL);
    cur = cur_overflow_list;
    int n = 0;
    while (cur != NULL) {
      work_q->push(cur->forwardee());
      oop next =  oop(cur->klass());
      cur->set_klass(cur->forwardee()->klass());
      cur = next;
      n++;
    }
    par_scan_state->note_overflow_refill(n);
    return true;
  } else {
    return false;
  }
}


const char* ParNewGeneration::name() const {
  return "par new generation";
}
