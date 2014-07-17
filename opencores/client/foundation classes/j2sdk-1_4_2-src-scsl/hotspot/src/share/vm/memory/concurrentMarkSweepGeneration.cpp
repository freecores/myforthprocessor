#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)concurrentMarkSweepGeneration.cpp	1.139 03/05/26 09:52:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_concurrentMarkSweepGeneration.cpp.incl"

// statics
CMSCollector* ConcurrentMarkSweepGeneration::_collector = NULL;

//////////////////////////////////////////////////////////////////
// In support of CMS/VM thread synchronization
//////////////////////////////////////////////////////////////////
// We split use of the CMS_lock into 2 "levels".
// The low-level locking is of the usual CMS_lock monitor. We introduce
// a higher level "token" (hereafter "CMS token") built on top of the
// low level monitor (hereafter "CMS lock").
// The token-passing protocol gives priority to the VM thread. The
// CMS-lock doesn't provide any fairness guarantees, but clients
// should ensure that it is only held for very short, bounded
// durations.
// 
// When either of the CMS thread or the VM thread is involved in
// collection operations during which it does not want the other
// thread to interfere, it obtains the CMS token.
// 
// If either thread tries to get the token while the other has
// it, that thread waits. However, if the VM thread and CMS thread
// both want the token, then the VM thread gets priority while the
// CMS thread waits. This ensures, for instance, that the "concurrent"
// phases of the CMS thread's work do not block out the VM thread
// for long periods of time as the CMS thread continues to hog
// the token. (See bug 4616232).
// 
// The baton-passing functions are, however, controlled by the
// flags _foregroundGCShouldWait and _foregroundGCIsActive,
// and here the low-level CMS lock, not the high level token,
// ensures mutual exclusion.
// 
// Two important conditions that we have to satisfy:
// 1. if a thread does a low-level wait on the CMS lock, then it
//    relinquishes the CMS token if it were holding that token
//    when it acquired the low-level CMS lock.
// 2. any low-level notifications on the low-level lock
//    should only be sent when a thread has relinquished the token.
// 
// In the absence of either property, we'd have potential deadlock.
// 
// We protect each of the CMS (concurrent and sequential) phases
// with the CMS _token_, not the CMS _lock_.
// 
// The only code protected by CMS lock is the token acquisition code
// itself, see ConcurrentMarkSweepThread::[de]synchronize(), and the
// baton-passing code.
// 
// Unfortunately, i couldn't come up with a good abstraction to factor and
// hide the naked CMS_lock manipulation in the baton-passing code
// further below. That's something we should try to do. Also, the proof
// of correctness of this 2-level locking scheme is far from obvious,
// and potentially quite slippery. We have an uneasy supsicion, for instance,
// that there may be a theoretical possibility of delay/starvation in the
// low-level lock/wait/notify scheme used for the baton-passing because of
// potential intereference with the priority scheme embodied in the
// CMS-token-passing protocol. See related comments at a CMS_lock->wait()
// invocation further below and marked with "XXX 20011219YSR".
// Indeed, as we note elsewhere, this may become yet more slippery
// in the presence of multiple CMS and/or multiple VM threads. XXX

class CMSTokenSync: public StackObj {
 private:
  bool _is_cms_thread;
 public:
  CMSTokenSync(bool is_cms_thread):
    _is_cms_thread(is_cms_thread) {
    assert(is_cms_thread == Thread::current()->is_ConcurrentMarkSweep_thread(),
           "Incorrect argument to constructor");
    ConcurrentMarkSweepThread::synchronize(_is_cms_thread);
  }

  ~CMSTokenSync() {
    assert(_is_cms_thread ?
             ConcurrentMarkSweepThread::cms_thread_has_cms_token() :
             ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
          "Incorrect state");
    ConcurrentMarkSweepThread::desynchronize(_is_cms_thread);
  }
};

// Convenience class that does a CMSTokenSync, and then acquires
// upto three locks.
class CMSTokenSyncWithLocks: public CMSTokenSync {
 private:
  // Note: locks are acquired in textual declaration order
  // and released in the opposite order
  MutexLockerEx _locker1, _locker2, _locker3;
 public:
  CMSTokenSyncWithLocks(bool is_cms_thread, Mutex* mutex1,
                        Mutex* mutex2 = NULL, Mutex* mutex3 = NULL):
    CMSTokenSync(is_cms_thread),
    _locker1(mutex1, Mutex::_no_safepoint_check_flag),
    _locker2(mutex2, Mutex::_no_safepoint_check_flag),
    _locker3(mutex3, Mutex::_no_safepoint_check_flag)
  { }
};

// Convenience class that locks free list locks for given collector
class FreelistLocker: public StackObj {
 private:
  CMSCollector* _collector;
 public:
  FreelistLocker(CMSCollector* collector):
    _collector(collector) {
    _collector->getFreelistLocks();
  }

  ~FreelistLocker() {
    _collector->releaseFreelistLocks();
  }
};

// Wrapper class to temporarily disable icms during a foreground cms collection.
class ICMSDisabler: public StackObj {
 public:
  // The ctor disables icms and wakes up the thread so it notices the change;
  // the dtor re-enables icms.  Note that the CMSCollector methods will check
  // CMSIncrementalMode.
  ICMSDisabler()  { CMSCollector::disable_icms(); CMSCollector::start_icms(); }
  ~ICMSDisabler() { CMSCollector::enable_icms(); }
};

//////////////////////////////////////////////////////////////////
//  Concurrent Mark-Sweep Generation /////////////////////////////
//////////////////////////////////////////////////////////////////

NOT_PRODUCT(CompactibleFreeListSpace* debug_cms_space;)

// This struct contains per-thread things necessary to support parallel
// young-gen collection.
class CMSParGCThreadState: public CHeapObj {
 public:
  CFLS_LAB lab;
  PromotionInfo promo;

  // Constructor.
  CMSParGCThreadState(CompactibleFreeListSpace* cfls) : lab(cfls) {
    promo.setSpace(cfls);
  }
};

ConcurrentMarkSweepGeneration::ConcurrentMarkSweepGeneration(
     ReservedSpace rs, size_t initial_byte_size, int level,
     CardTableRS* ct, bool use_adaptive_freelists,
     FreeBlockDictionary::DictionaryChoice dictionaryChoice) :
  CardGeneration(rs, initial_byte_size, level, ct)
{
  HeapWord* bottom = (HeapWord*) _virtual_space.low();
  HeapWord* end    = (HeapWord*) _virtual_space.high();

  _direct_allocated_words = 0;
  NOT_PRODUCT(
    _numObjectsPromoted = 0;
    _numWordsPromoted = 0;
    _numObjectsAllocated = 0;
    _numWordsAllocated = 0;
  )

  /*
   * if (jvmpi::is_event_enabled(JVMPI_EVENT_ARENA_NEW)) {
   *   jvmpi::post_arena_new_event(Universe::heap()->addr_to_arena_id(bottom),
   *                               name());
   * }
   */

  _cmsSpace = new CompactibleFreeListSpace(_bts, MemRegion(bottom, end),
                                           use_adaptive_freelists,
					   dictionaryChoice);
  NOT_PRODUCT(debug_cms_space = _cmsSpace;)
  if (_cmsSpace == NULL) {
    vm_exit_during_initialization(
      "CompactibleFreeListSpace allocation failure");
  }

  // Verify the assumption that FreeChunk::_prev and OopDesc::_klass
  // offsets match. The ability to tell free chunks from objects
  // depends on this property.
  debug_only(
    FreeChunk* junk = NULL;
    assert(junk->prev_addr() == (void*)(oop(junk)->klass_addr()),
           "Offset of FreeChunk::_prev within FreeChunk must match"
           "  that of OopDesc::_klass within OopDesc");
  )
  if (ParallelGCThreads > 0) {
    typedef CMSParGCThreadState* CMSParGCThreadStatePtr;
    _par_gc_thread_states =
      NEW_C_HEAP_ARRAY(CMSParGCThreadStatePtr, ParallelGCThreads);
    if (_par_gc_thread_states == NULL) {
      vm_exit_during_initialization("Could not allocate par gc structs");
    }
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _par_gc_thread_states[i] = new CMSParGCThreadState(cmsSpace());
      if (_par_gc_thread_states[i] == NULL) { 
        vm_exit_during_initialization("Could not allocate par gc structs");
      }
    }
  } else {
    _par_gc_thread_states = NULL;
  }
}

void ConcurrentMarkSweepGeneration::initialize_performance_counters() {

  const char* gen_name = "old";

  // Generation Counters - generation 1, 1 subspace
  _gen_counters = new GenerationCounters(PERF_GC, gen_name, 1, 1,
                                         &_virtual_space);

  _space_counters = new GSpaceCounters(_gen_counters->name_space(), gen_name,
                                       0, _virtual_space.reserved_size(),
                                       this);
}

CMSStats::CMSStats(ConcurrentMarkSweepGeneration* cms_gen, unsigned int alpha):
  _cms_gen(cms_gen)
{
  assert(alpha <= 100, "bad value");
  _saved_alpha = alpha;

  // Initialize the alphas to the bootstrap value of 100.
  _gc0_alpha = _cms_alpha = 100;

  _cms_begin_time.update();
  _cms_end_time.update();

  _gc0_duration = 0.0;
  _gc0_period = 0.0;
  _gc0_promoted = 0;

  _cms_duration = 0.0;
  _cms_period = 0.0;
  _cms_allocated = 0;

  _cms_used_at_gc0_begin = 0;
  _cms_used_at_gc0_end = 0;
  _allow_duty_cycle_reduction = false;
  _valid_bits = 0;
  _icms_duty_cycle = CMSIncrementalDutyCycle;
}

double CMSStats::time_until_cms_gen_full() const {
  size_t cms_free = _cms_gen->cmsSpace()->free();
  size_t gen0_capacity = GenCollectedHeap::heap()->get_gen(0)->capacity();
  if (cms_free > gen0_capacity) {
    // Take the worst case requirement of a full promotion into account by
    // subtracting the young gen capacity from the free space.
    cms_free -= gen0_capacity;

    // Adjust by the safety factor.
    double cms_free_dbl = (double)cms_free;
    cms_free_dbl = cms_free_dbl * (100.0 - CMSIncrementalSafetyFactor) / 100.0;

    // Add 1 in case the consumption rate goes to zero.
    return cms_free_dbl / (cms_consumption_rate() + 1.0);
  }
  return 0.0;
}

// Return a duty cycle based on old_duty_cycle and new_duty_cycle, limiting the
// amount of change to prevent wild oscillation.
unsigned int CMSStats::icms_damped_duty_cycle(unsigned int old_duty_cycle,
					      unsigned int new_duty_cycle) {
  assert(old_duty_cycle <= 100, "bad input value");
  assert(new_duty_cycle <= 100, "bad input value");

  // Note:  use subtraction with caution since it may underflow (values are
  // unsigned).  Addition is safe since we're in the range 0-100.
  unsigned int damped_duty_cycle = new_duty_cycle;
  if (new_duty_cycle < old_duty_cycle) {
    const unsigned int largest_delta = MAX2(old_duty_cycle / 4, 5U);
    if (new_duty_cycle + largest_delta < old_duty_cycle) {
      damped_duty_cycle = old_duty_cycle - largest_delta;
    }
  } else if (new_duty_cycle > old_duty_cycle) {
    const unsigned int largest_delta = MAX2(old_duty_cycle / 4, 15U);
    if (new_duty_cycle > old_duty_cycle + largest_delta) {
      damped_duty_cycle = MIN2(old_duty_cycle + largest_delta, 100U);
    }
  }
  assert(damped_duty_cycle <= 100, "invalid duty cycle computed");

  if (CMSTraceIncrementalPacing) {
    gclog_or_tty->print(" [icms_damped_duty_cycle(%d,%d) = %d] ",
			   old_duty_cycle, new_duty_cycle, damped_duty_cycle);
  }
  return damped_duty_cycle;
}

unsigned int CMSStats::icms_update_duty_cycle_impl() {
  assert(CMSIncrementalPacing && valid(),
	 "should be handled in icms_update_duty_cycle()");

  double cms_time_so_far = cms_timer().seconds();
  double scaled_duration = cms_duration_per_mb() * _cms_used_at_gc0_end / M;
  double scaled_duration_remaining = scaled_duration - cms_time_so_far;
  // fabs() not always available.
  if (scaled_duration_remaining < 0.0) {
    scaled_duration_remaining = -scaled_duration_remaining;
  }

  // Avoid division by 0.
  double time_until_full = MAX2(time_until_cms_gen_full(), 0.01);
  double duty_cycle_dbl = 100.0 * scaled_duration_remaining / time_until_full;

  unsigned int new_duty_cycle = MIN2((unsigned int)duty_cycle_dbl, 100U);
  if (new_duty_cycle > _icms_duty_cycle) {
    // Avoid very small duty cycles (1 or 2); 0 is allowed.
    if (new_duty_cycle > 2) {
      _icms_duty_cycle = icms_damped_duty_cycle(_icms_duty_cycle,
						new_duty_cycle);
    }
  } else if (_allow_duty_cycle_reduction) {
    // The duty cycle is reduced only once per cms cycle (see record_cms_end()).
    new_duty_cycle = icms_damped_duty_cycle(_icms_duty_cycle, new_duty_cycle);
    // Respect the minimum duty cycle.
    unsigned int min_duty_cycle = (unsigned int)CMSIncrementalDutyCycleMin;
    _icms_duty_cycle = MAX2(new_duty_cycle, min_duty_cycle);
  }

  if (PrintGCDetails || CMSTraceIncrementalPacing) {
    gclog_or_tty->print(" icms_dc=%d ", _icms_duty_cycle);
  }

  _allow_duty_cycle_reduction = false;
  return _icms_duty_cycle;
}

#ifndef PRODUCT
void CMSStats::print_on(outputStream *st) const {
  st->print(" gc0_alpha=%d,cms_alpha=%d", _gc0_alpha, _cms_alpha);
  st->print(",gc0_dur=%g,gc0_per=%g,gc0_promo=" SIZE_FORMAT,
	       gc0_duration(), gc0_period(), gc0_promoted());
  st->print(",cms_dur=%g,cms_dur_per_mb=%g,cms_per=%g,cms_alloc=" SIZE_FORMAT,
	    cms_duration(), cms_duration_per_mb(),
	    cms_period(), cms_allocated());
  st->print(",cms_since_beg=%g,cms_since_end=%g",
	    cms_time_since_begin(), cms_time_since_end());
  st->print(",cms_used_beg=" SIZE_FORMAT ",cms_used_end=" SIZE_FORMAT,
	    _cms_used_at_gc0_begin, _cms_used_at_gc0_end);
  if (CMSIncrementalMode) {
    st->print(",dc=%d", icms_duty_cycle());
  }

  if (valid()) {
    st->print(",promo_rate=%g,cms_alloc_rate=%g",
	      promotion_rate(), cms_allocation_rate());
    st->print(",cms_consumption_rate=%g,time_until_full=%g",
	      cms_consumption_rate(), time_until_cms_gen_full());
  }
  st->print(" ");
}
#endif // #ifndef PRODUCT

CMSCollector::CMSCollector(ConcurrentMarkSweepGeneration* cmsGen,
                           ConcurrentMarkSweepGeneration* permGen,
                           CardTableRS*                   ct):
  _cmsGen(cmsGen),
  _permGen(permGen),
  _ct(ct),
  _ref_processor(cmsGen->ref_processor()),
  _between_prologue_and_epilogue(false),
  _markBitMap(0, Mutex::leaf + 1, "CMS_markBitMap_lock"),
  _modUnionTable((CardTableModRefBS::card_shift - LogHeapWordSize),
                 Mutex::leaf, "CMS_modUnionTable_lock"),
  _collectorState(Idling), _foregroundGCIsActive(false),
  _foregroundGCShouldWait(false),
  _modUnionClosure(&_modUnionTable),
  _modUnionClosurePar(&_modUnionTable),
  _stats(cmsGen)
{
  // Adjust any global flags for consistency:
  // Perm Gen shouldn't be swept if class unloading is disabled
  CMSPermGenSweepingEnabled = CMSClassUnloadingEnabled &&
                              CMSPermGenSweepingEnabled;

  // Now expand the span and allocate the collection support structures
  // (MUT, marking bit map etc.) to cover both generations subject to
  // collection.

  // First check that _permGen is adjacent to _cmsGen and above it.
  assert(   _cmsGen->reserved().word_size()  > 0
         && _permGen->reserved().word_size() > 0,
         "generations should not be of zero size");
  assert(_cmsGen->reserved().intersection(_permGen->reserved()).is_empty(),
         "_cmsGen and _permGen should not overlap");
  assert(_cmsGen->reserved().end() == _permGen->reserved().start(),
         "_cmsGen->end() different from _permGen->start()");

  // For use by dirty card to oop closures.
  _cmsGen->cmsSpace()->set_collector(this);
  _permGen->cmsSpace()->set_collector(this);

  // Adjust my (and ref_proc's) span to cover old gen and cms gen
  _span = _cmsGen->reserved()._union(_permGen->reserved());
  _ref_processor->set_span(_span);

  // Allocate MUT and marking bit map
  _markBitMap.allocate(_span);
  assert(_markBitMap.covers(_span), "_markBitMap inconsistency?");
  _modUnionTable.allocate(_span);
  assert(_modUnionTable.covers(_span), "_modUnionTable inconsistency?");

  // Should make stack grow dynamically as needed rather than a fixed max
  // as i am doing here. In fact, probably best to use GrowableArray<>
  // as done in other collectors (eg mark compact) -- YSR. XXX
  _markStack.allocate(CMSMarkStackSize);
  _revisitStack.allocate(CMSRevisitStackSize);

  // Support for parallel remark
  if (CMSParallelRemarkEnabled && ParallelGCThreads > 0) {
    _task_queues = new OopTaskQueueSet(ParallelGCThreads);
    if (_task_queues == NULL) {
      vm_exit_during_initialization("task_queues allocation failure.");
    }
    _hash_seed = NEW_C_HEAP_ARRAY(int, ParallelGCThreads);
    if (_hash_seed == NULL) {
      vm_exit_during_initialization("_hash_seed array allocation failure");
    }

    // XXX use a global constant instead of 64!
    typedef struct OopTaskQueuePadded {
      OopTaskQueue work_queue;
      char pad[64 - sizeof(OopTaskQueue)];  // prevent false sharing
    } OopTaskQueuePadded;

    uint i;
    for (i = 0; i < ParallelGCThreads; i++) {
      OopTaskQueuePadded *q_padded = new OopTaskQueuePadded();
      if (q_padded == NULL) {
        vm_exit_during_initialization("work_queue allocation failure.");
      }
      _task_queues->register_queue(i, &q_padded->work_queue);
    }
    for (i = 0; i < ParallelGCThreads; i++) {
      _task_queues->queue(i)->initialize();
      _hash_seed[i] = 17;  // copied from ParNew
    }
  }

  // "initiatingOccupancy" is the occupancy ratio at which we trigger
  // a new collection cycle.  Unless explicitly specified via
  // CMSTriggerRatio, it is calculated by:
  //   Let "f" be MinHeapFreeRatio in
  //
  //    intiatingOccupancy = 100-f +
  //                         f * (CMSTriggerRatio/100)
  // That is, if we assume the heap is at its desired maximum occupancy at the
  // end of a collection, we let CMSTriggerRatio of the (purported) free
  // space be allocated before initiating a new collection cycle.
  if (CMSInitiatingOccupancyFraction > 0) {
    _initiatingOccupancy = (double)CMSInitiatingOccupancyFraction / 100.0;
  } else {
    _initiatingOccupancy = ((100 - MinHeapFreeRatio) +
                           (double)(CMSTriggerRatio *
                                    MinHeapFreeRatio) / 100.0)
			   / 100.0;
  }

  _full_gcs_since_conc_gc = 0;

  // Now tell CMS generations the identity of their collector
  ConcurrentMarkSweepGeneration::set_collector(this);

  // Create & start a CMS thread for this CMS collector
  ConcurrentMarkSweepThread::start(this);
  assert(cmsThread() != NULL, "CMS Thread should have been created");
  assert(cmsThread()->collector() == this,
         "CMS Thread should refer to this gen");
  assert(CMS_lock != NULL, "Where's the CMS_lock?");

  _gc_counters = new CollectorCounters(PERF_GC, "CMS", 1);
}

const char* ConcurrentMarkSweepGeneration::name() const {
  return "concurrent mark-sweep generation";
}

void ConcurrentMarkSweepGeneration::update_counters() {
  if (UsePerfData) {
    _space_counters->update_all();
    _gen_counters->update_all();
  }
}

// this is an optimized version of update_counters(). it takes the
// used value as a parameter rather than computing it. 
//
void ConcurrentMarkSweepGeneration::update_counters(size_t used) {
  if (UsePerfData) {
    _space_counters->update_used(used);
    _space_counters->update_capacity();
    _gen_counters->update_all();
  }
}

void ConcurrentMarkSweepGeneration::print() const {
  Generation::print();
  cmsSpace()->print();
}

#ifndef PRODUCT
void ConcurrentMarkSweepGeneration::print_statistics() {
  cmsSpace()->printFLCensus(0);
}
#endif

void ConcurrentMarkSweepGeneration::printOccupancy(const char *s) {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  if (PrintGCDetails) {
    gclog_or_tty->print(" [%d %s: %dK(%dK)]", level(), s, used() / K, capacity() / K);
  }
  gclog_or_tty->print(" %dK(%dK)",
              gch->used() / K, gch->capacity() / K);
}

size_t
ConcurrentMarkSweepGeneration::contiguous_available() const {
  // dld proposes an improvement in precision here. If the committed
  // part of the space ends in a free block we should add that to
  // uncommitted size in the calculation below. Will make this
  // change later, staying with the approximation below for the
  // time being. -- ysr.
  return MAX2(_virtual_space.uncommitted_size(), unsafe_max_alloc_nogc());
}

size_t
ConcurrentMarkSweepGeneration::unsafe_max_alloc_nogc() const {
  return _cmsSpace->max_alloc_in_words() * HeapWordSize;
}

CompactibleSpace*
ConcurrentMarkSweepGeneration::first_compaction_space() const {
  return _cmsSpace;
}

void ConcurrentMarkSweepGeneration::reset_after_compaction() {
  // Clear the promotion information.  These pointers can be adjusted
  // along with all the other pointers into the heap but
  // compaction is expected to be a rare event with 
  // a heap using cms so don't do it without seeing the need.
  if (ParallelGCThreads > 0) {
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _par_gc_thread_states[i]->promo.reset();
    }
  }
}

void ConcurrentMarkSweepGeneration::space_iterate(SpaceClosure* blk, bool usedOnly) {
  blk->do_space(_cmsSpace);
}

// The desired expansion delta is computed so that:
// . desired free percentage or greater is used
void ConcurrentMarkSweepGeneration::compute_new_size() {
  size_t expand_bytes = 0;
  double free_percentage = ((double) free()) / capacity();
  double desired_free_percentage = (double) MinHeapFreeRatio / 100;
  double maximum_free_percentage = (double) MaxHeapFreeRatio / 100;

  // compute expansion delta needed for reaching desired free percentage
  if (free_percentage < desired_free_percentage) {
    size_t desired_capacity = used() / ((double) 1 - desired_free_percentage);
    assert(desired_capacity >= capacity(), "invalid expansion size");
    expand_bytes = MAX2(desired_capacity - capacity(), MinHeapDeltaBytes);
  }
  if (expand_bytes > 0) {
    if (PrintGCDetails && Verbose) {
      size_t desired_capacity = used() / ((double) 1 - desired_free_percentage);
      gclog_or_tty->print_cr("\nFrom compute_new_size: ");
      gclog_or_tty->print_cr("  Free fraction %f", free_percentage);
      gclog_or_tty->print_cr("  Desired free fraction %f", 
        desired_free_percentage);
      gclog_or_tty->print_cr("  Maximum free fraction %f", 
        maximum_free_percentage);
      gclog_or_tty->print_cr("  Capactiy %d", capacity()/1000);
      gclog_or_tty->print_cr("  Desired capacity %d", 
        desired_capacity/1000);
      int prev_level = level() - 1;
      if (prev_level >= 0) {
        size_t prev_size = 0;
        GenCollectedHeap* gch = GenCollectedHeap::heap();
        Generation* prev_gen = gch->_gens[prev_level];
        prev_size = prev_gen->capacity();
          gclog_or_tty->print_cr("  Younger gen size %d", prev_size/1000);
      }
      gclog_or_tty->print_cr("  unsafe_max_alloc_nogc %d", 
	unsafe_max_alloc_nogc()/1000);
      gclog_or_tty->print_cr("  contiguous available %d", 
	contiguous_available()/1000);
      gclog_or_tty->print_cr("  Expand by %d (bytes)", expand_bytes);
    }
    // safe if expansion fails
    expand(expand_bytes, 0, CMSExpansionCause::_satisfy_free_ratio); 
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print_cr("  Expanded free fraction %f", 
	((double) free()) / capacity());
    }
  }
}

Mutex* ConcurrentMarkSweepGeneration::freelistLock() const {
  return cmsSpace()->freelistLock();
}

HeapWord* ConcurrentMarkSweepGeneration::allocate(size_t size,
                                                  bool   large_noref,
                                                  bool   tlab) {
  CMSSynchronousYieldRequest yr;
  MutexLockerEx x(freelistLock(),
                  Mutex::_no_safepoint_check_flag);
  return have_lock_and_allocate(size, large_noref, tlab);
}

HeapWord* ConcurrentMarkSweepGeneration::have_lock_and_allocate(size_t size,
                                                  bool   large_noref,
                                                  bool   tlab) {
  assert_lock_strong(freelistLock());
  size_t adjustedSize = CompactibleFreeListSpace::adjustObjectSize(size);
  HeapWord* res = cmsSpace()->allocate(adjustedSize, large_noref);
  // Allocate the object live (grey) if the background collector has
  // started marking. This is necessary because the marker may
  // have passed this address and consequently this object will
  // not otherwise be greyed and would be incorrectly swept up.
  // Note that if this object contains references, the writing
  // of those references will dirty the card containing this object
  // allowing the object to be blackened (and its references scanned)
  // either during a preclean phase or at the final checkpoint.
  if (res != NULL) {
    collector()->direct_allocated(res, adjustedSize);
    _direct_allocated_words += adjustedSize;
    // allocation counters
    NOT_PRODUCT(
      _numObjectsAllocated++;
      _numWordsAllocated += (int)adjustedSize;
    )
  }
  return res;
}

// In the case of direct allocation by mutators in a generation that
// is being concurrently collected, the object must be allocated
// live (grey) if the background collector has started marking.
// This is necessary because the marker may
// have passed this address and consequently this object will
// not otherwise be greyed and would be incorrectly swept up.
// Note that if this object contains references, the writing
// of those references will dirty the card containing this object
// allowing the object to be blackened (and its references scanned)
// either during a preclean phase or at the final checkpoint.
void CMSCollector::direct_allocated(HeapWord* start, size_t size) {
  assert(_markBitMap.covers(start, size), "Out of bounds");
  if (_collectorState >= Marking) {
    MutexLockerEx y(_markBitMap.lock(),
                    Mutex::_no_safepoint_check_flag);
    // [see comments preceding SweepClosure::do_blk() below for details]
    // 1. need to mark the object as live so it isn't collected
    // 2. need to mark the end of the object so sweeper can skip over it
    //    if it's uninitialized when the sweeper reaches it.
    _markBitMap.mark(start);          // object is live
    _markBitMap.mark(start + 1);      // object is potentially uninitialized?
    _markBitMap.mark(start + size - 1);
                                      // mark end of object
  }
  // check that oop looks uninitialized
  assert(oop(start)->klass() == NULL, "_klass should be NULL");
}

void CMSCollector::promoted(bool par, HeapWord* start) {
  assert(_markBitMap.covers(start), "Out of bounds");
  // See comment in direct_allocated() about when objects should
  // be allocated live.
  if (_collectorState >= Marking) {
    // we already hold the marking bit map lock, taken in
    // the prologue
    if (par) {
      _markBitMap.par_mark(start);
    } else {
      _markBitMap.mark(start);
    }
    // We don't need to mark the object as uninitialized (as
    // in direct_allocated above) because this is being done with the
    // world stopped and the object will be initialized by the
    // time the sweeper gets to look at it.
    assert(SafepointSynchronize::is_at_safepoint(),
           "expect promotion only at safepoints");

    if (_collectorState < Sweeping) {
      // Mark the appropriate card in the modUnionTable, so that
      // this object gets scanned before the sweep. If this is
      // not done, CMS generation references in the object might
      // not get marked.
      if (par) {
	_modUnionTable.par_mark(start);
      } else {
	_modUnionTable.mark(start);
      }
    }
  }
}

static inline size_t percent_of_space(Space* space, HeapWord* addr)
{
  size_t delta = pointer_delta(addr, space->bottom());
  return (size_t)(delta * 100.0 / (space->capacity() / HeapWordSize));
}

void CMSCollector::icms_update_allocation_limits()
{
  Generation* gen0 = GenCollectedHeap::heap()->get_gen(0);
  assert(gen0->kind() == Generation::DefNew || 
	 gen0->kind() == Generation::ParNew,
	 "gen0 must be DefNew, or a subclass");
  EdenSpace* eden = ((DefNewGeneration*)gen0)->eden();

  const unsigned int duty_cycle = stats().icms_update_duty_cycle();
  if (CMSTraceIncrementalPacing) {
    stats().print();
  }

  assert(duty_cycle <= 100, "invalid duty cycle");
  if (duty_cycle != 0) {
    // The duty_cycle is a percentage between 0 and 100; convert to words and
    // then compute the offset from the endpoints of the space.
    size_t capacity_words = eden->capacity() / HeapWordSize;
    double capacity_words_dbl = (double)capacity_words;
    size_t duty_cycle_words = (size_t)(capacity_words_dbl * duty_cycle / 100.0);
    size_t offset_words = (capacity_words - duty_cycle_words) / 2;

    _icms_start_limit = eden->bottom() + offset_words;
    _icms_stop_limit = eden->end() - offset_words;

    // The limits may be adjusted (shifted to the right) by
    // CMSIncrementalOffset, to allow the application more mutator time after a
    // young gen gc (when all mutators were stopped) and before CMS starts and
    // takes away one or more cpus.
    if (CMSIncrementalOffset != 0) {
      double adjustment_dbl = capacity_words_dbl * CMSIncrementalOffset / 100.0;
      size_t adjustment = (size_t)adjustment_dbl;
      HeapWord* tmp_stop = _icms_stop_limit + adjustment;
      if (tmp_stop > _icms_stop_limit && tmp_stop < eden->end()) {
	_icms_start_limit += adjustment;
	_icms_stop_limit = tmp_stop;
      }
    }
  } else {
    _icms_start_limit = _icms_stop_limit = eden->end();
  }

  // Install the new start limit.
  eden->set_soft_end(_icms_start_limit);

  if (CMSTraceIncrementalMode) {
    gclog_or_tty->print(" icms alloc limits:  "
			   PTR_FORMAT "," PTR_FORMAT
			   " (" SIZE_FORMAT "%%," SIZE_FORMAT "%%) ",
			   _icms_start_limit, _icms_stop_limit,
			   percent_of_space(eden, _icms_start_limit),
			   percent_of_space(eden, _icms_stop_limit));
    if (Verbose) {
      gclog_or_tty->print("eden:  ");
      eden->print_on(gclog_or_tty);
    }
  }
}

HeapWord* 
CMSCollector::allocation_limit_reached(Space* space, HeapWord* top,
				       size_t word_size)
{
  // A start_limit equal to end() means the duty cycle is 0, so treat that as a
  // nop.
  if (CMSIncrementalMode && _icms_start_limit != space->end()) {
    if (top <= _icms_start_limit) {
      if (CMSTraceIncrementalMode) {
	space->print_on(gclog_or_tty);
	gclog_or_tty->stamp();
	gclog_or_tty->print_cr(" start limit top=" PTR_FORMAT
			       ", new limit=" PTR_FORMAT
			       " (" SIZE_FORMAT "%%)",
			       top, _icms_stop_limit,
			       percent_of_space(space, _icms_stop_limit));
      }
      ConcurrentMarkSweepThread::start_icms();
      if (top + word_size < _icms_stop_limit) {
	return _icms_stop_limit;
      }

      // The allocation will cross both the _start and _stop limits, so do the
      // stop notification also and return end().
      if (CMSTraceIncrementalMode) {
	space->print_on(gclog_or_tty);
	gclog_or_tty->stamp();
	gclog_or_tty->print_cr(" +stop limit top=" PTR_FORMAT
			       ", new limit=" PTR_FORMAT,
			       " (" SIZE_FORMAT "%%)",
			       top, space->end(),
			       percent_of_space(space, space->end()));
      }
      ConcurrentMarkSweepThread::stop_icms();
      return space->end();
    }

    if (top <= _icms_stop_limit) {
      if (CMSTraceIncrementalMode) {
	space->print_on(gclog_or_tty);
	gclog_or_tty->stamp();
	gclog_or_tty->print_cr(" stop limit top=" PTR_FORMAT
			       ", new limit=" PTR_FORMAT,
			       " (" SIZE_FORMAT "%%)",
			       top, space->end(),
			       percent_of_space(space, space->end()));
      }
      ConcurrentMarkSweepThread::stop_icms();
      return space->end();
    }

    if (CMSTraceIncrementalMode) {
      space->print_on(gclog_or_tty);
      gclog_or_tty->stamp();
      gclog_or_tty->print_cr(" end limit top=" PTR_FORMAT
			     ", new limit=" PTR_FORMAT,
			     top, NULL);
    }
  }

  return NULL;
}

oop ConcurrentMarkSweepGeneration::promote(oop obj, size_t obj_size, oop* ref) {
  assert(obj_size == (size_t)obj->size(), "bad obj_size passed in");
  // allocate, copy and if necessary update promoinfo --
  // delegate to underlying space.
  assert_lock_strong(freelistLock());
  oop res = _cmsSpace->promote(obj, obj_size, ref);
  if (res == NULL) {
    // expand and retry
    size_t s = _cmsSpace->expansionSpaceRequired(obj_size);  // HeapWords
    expand(s*HeapWordSize, MinHeapDeltaBytes, 
      CMSExpansionCause::_satisfy_promotion);
    // Since there's currently no next generation, we don't try to promote
    // into a more senior generation.
    assert(next_gen() == NULL, "assumption, based upon which no attempt "
                               "is made to pass on a possibly failing "
                               "promotion to next generation");
    res = _cmsSpace->promote(obj, obj_size, ref);
  }
  if (res != NULL) {
    // See comment in allocate() about when objects should
    // be allocated live.
    collector()->promoted(false, (HeapWord*)res); // Not parallel.
    // promotion counters
    NOT_PRODUCT(
      _numObjectsPromoted++;
      _numWordsPromoted +=
        (int)(CompactibleFreeListSpace::adjustObjectSize(obj->size()));
    )
  }
  return res;
}


HeapWord*
ConcurrentMarkSweepGeneration::allocation_limit_reached(Space* space,
					     HeapWord* top,
					     size_t word_sz)
{
  return collector()->allocation_limit_reached(space, top, word_sz);
}

// Things to support parallel young-gen collection.
oop
ConcurrentMarkSweepGeneration::par_promote(int thread_num,
					   oop old, markOop m,
					   size_t word_sz) {
						
  CMSParGCThreadState* ps = _par_gc_thread_states[thread_num];
  PromotionInfo* promoInfo = &ps->promo;
 // if we are tracking promotions, then first ensure space for
  // promotion (including spooling space for saving header if necessary).
  // then allocate and copy, then track promoted info if needed.
  // When tracking (see PromotionInfo::track()), the mark word may
  // be displaced and in this case restoration of the mark word
  // occurs in the (oop_since_save_marks_)iterate phase.
  if (promoInfo->tracking() && !promoInfo->ensureSpoolingSpace()) {
    return NULL;
  }
  HeapWord* obj_ptr = ps->lab.alloc(word_sz);
  if (obj_ptr == NULL) {
     obj_ptr = expand_and_par_lab_allocate(ps, word_sz);
     if (obj_ptr == NULL) return NULL;
  }
  oop obj = oop(obj_ptr);
  assert(obj->klass() == NULL, "Object should be uninitialized here.");
  // Otherwise, copy the object.  Here we must be careful to insert the
  // klass pointer last, since this marks the block as an allocated object.
  HeapWord* old_ptr = (HeapWord*)old;
  if (word_sz > (size_t)oopDesc::header_size()) {
    Memory::copy_words_aligned(old_ptr + oopDesc::header_size(),
			       obj_ptr + oopDesc::header_size(),
			       word_sz - oopDesc::header_size());
  }
  // Restore the mark word copied above.
  obj->set_mark(m);
  // Now we can track the promoted object, if necessary.  We take care 
  // To delay the transition from uninitialized to full object
  // (i.e., insertion of klass pointer) until after, so that it
  // atomically becomes a promoted object.
  if (promoInfo->tracking()) {
    promoInfo->track((PromotedObject*)obj);
  }
  // Finally, install the klass pointer.
  obj->set_klass(old->klass());

  collector()->promoted(true, obj_ptr); // parallel
  
  NOT_PRODUCT(
    atomic::increment((jint*)&_numObjectsPromoted);
    atomic::add((int)CompactibleFreeListSpace::adjustObjectSize(obj->size()),
                (jint*)&_numWordsPromoted);
  )

  return obj; 
}

void
ConcurrentMarkSweepGeneration::
par_promote_alloc_undo(int thread_num,
		       HeapWord* obj, size_t word_sz) {
  // CMS does not support promotion undo.
  ShouldNotReachHere();
}

void
ConcurrentMarkSweepGeneration::
par_promote_alloc_done(int thread_num) {
  CMSParGCThreadState* ps = _par_gc_thread_states[thread_num];
  ps->lab.retire();
#if CFLS_LAB_REFILL_STATS
  if (thread_num == 0) {
    _cmsSpace->print_par_alloc_stats();
  }
#endif
}

void
ConcurrentMarkSweepGeneration::
par_oop_since_save_marks_iterate_done(int thread_num) {
  CMSParGCThreadState* ps = _par_gc_thread_states[thread_num];
  ps->promo.promoted_oops_iterate(NULL);
}

// XXXPERM
bool ConcurrentMarkSweepGeneration::should_collect(bool   full,
                                                   size_t size,
                                                   bool   large_noref,
                                                   bool   tlab)
{
  // Either a full collection was requested or
  // we are out of space for this allocation.
  return full || should_allocate(size, large_noref, tlab);
}

bool CMSCollector::shouldConcurrentCollect() {
  // We start a collection cycle if either the perm gen or
  // old gen want a collection cycle started. Each may use
  // an appropriate criterion for making this decision.
  // XXX We need to make sure that the gen expansion
  // criterion dovetails well with this.
  FreelistLocker x(this);
  return _cmsGen->shouldConcurrentCollect(initiatingOccupancy())
         || (CMSClassUnloadingEnabled && CMSPermGenSweepingEnabled
             && _permGen->shouldConcurrentCollect(initiatingOccupancy()));
}

bool ConcurrentMarkSweepGeneration::shouldConcurrentCollect(
  double initiatingOccupancy) {
  // We should be conservative in starting a collection cycle.  To
  // start too eagerly runs the risk of collecting too often in the
  // extreme.  To collect too rarely falls back on full collections,
  // which works, even if not optimum in terms of concurrent work.
  // As a work around for too eagerly collecting, use the flag
  // UseCMSInitiatingOccupancyOnly.  This also has the advantage of
  // giving the user an easily understandable way of controlling the
  // collections.
  // We want to start a new collection cycle if any of the following
  // conditions hold:
  // . our current occupancy exceeds the initiating occupancy, or
  // . we recently needed to expand and have not since that expansion,
  //   collected, or
  // . we are not using adaptive free lists and linear allocation is
  //   going to fail, or
  // . (for old gen) incremental collection has already failed or
  //   may soon fail in the near future as we may not be able to absorb
  //   promotions.
  assert_lock_strong(freelistLock());

  // ------------------------------------------------------------------
  // Print out lots of information which affects the initiation of
  // a collection.
  if (PrintCMSInitiationStatistics && collector()->stats().valid()) {
    gclog_or_tty->print("%s shouldConcurrentCollect: ", short_name()); 
    gclog_or_tty->stamp(); 
    gclog_or_tty->print_cr("");
    CMSStats stats = collector()->stats();
    stats.print_on(gclog_or_tty);
    gclog_or_tty->print_cr("time_until_cms_gen_full %3.7f", 
      stats.time_until_cms_gen_full());
    gclog_or_tty->print_cr("free=%d", free());
    gclog_or_tty->print_cr("contiguous_available=%d", contiguous_available());
    gclog_or_tty->print_cr("promotion_rate=%d", stats.promotion_rate());
    gclog_or_tty->print_cr("cms_allocation_rate=%d", stats.cms_allocation_rate());
    gclog_or_tty->print_cr("occupancy=%d", occupancy());
    gclog_or_tty->print_cr("initiatingOccupancy=%d", initiatingOccupancy);
  }
  // ------------------------------------------------------------------

  if (occupancy() > initiatingOccupancy) {
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print(" %s: collect because of occupancy %f / %f  ",
	short_name(), occupancy(), initiatingOccupancy);
    }
    return true;
  }
  if (UseCMSInitiatingOccupancyOnly) {
    return false;
  }
  if (expansion_cause() == CMSExpansionCause::_satisfy_allocation) {
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print(" %s: collect because expanded for allocation ",
	short_name());
    }
    return true;
  }
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  assert(gch->collector_policy()->is_two_generation_policy(),
         "You may want to check the correctness of the following");
  if (gch->incremental_collection_will_fail()) {
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print(" %s: collect because incremental collection will fail ",
	short_name());
    }
    return true;
  }
  if (!_cmsSpace->adaptive_freelists() && 
      _cmsSpace->linearAllocationWouldFail()) {
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print(" %s: collect because of linAB ",
	short_name());
    }
    return true;
  }

  // if the time to complete a cms collectionm (cms_duration()) is
  // less than the time remaining unti the cms generation is full,
  // start a collection.
  if (collector()->stats().valid() &&
     (collector()->stats().cms_duration() > 
      collector()->stats().time_until_cms_gen_full())) {
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print(" %s: collect because of anticipated promotion "
	" %3.7f > %3.7f", short_name(), collector()->stats().cms_duration(),
	collector()->stats().time_until_cms_gen_full());
    }
    return true;
  }

  return false;
}

void ConcurrentMarkSweepGeneration::collect(bool   full,
                                            bool   clear_all_soft_refs,
                                            size_t size,
                                            bool   large_noref,
                                            bool   tlab)
{
  collector()->collect(full, clear_all_soft_refs, size, large_noref, tlab);
}

void CMSCollector::collect(bool   full,
                           bool   clear_all_soft_refs,
                           size_t size,
                           bool   large_noref,
                           bool   tlab)
{
  if (!UseCMSCollectionPassing && _collectorState > Idling) {
    // For debugging purposes skip the collection if the state
    // is not currently idle
    if (TraceCMSState) {
      gclog_or_tty->print_cr("Thread " INTPTR_FORMAT " skipped full:%d CMS state %d", 
	Thread::current(), full, _collectorState);
    }
    return;
  }
  acquire_control_and_collect(full, clear_all_soft_refs);
  _full_gcs_since_conc_gc++;
}

// The foreground and background collectors need to coordinate in order
// to make sure that they do not mutually interfere with CMS collections.
// When a background collection is active,
// the foreground collector may need to take over (preempt) and
// synchronously complete an ongoing collection. Depending on the 
// frequency of the background collections and the heap usage
// of the application, this preemption can be seldom or frequent.
// There are only certain
// points in the background collection that the "collection-baton"
// can be passed to the foreground collector.
//
// The foreground collector will wait for the baton before
// starting any part of the collection.  The foreground collector
// will only wait at one location.
//
// The background collector will yield the baton before starting a new
// phase of the collection (e.g., before initial marking, marking from roots,
// precleaning, final re-mark, sweep etc.)  This is normally done at the head
// of the loop which switches the phases. The background collector does some
// of the phases (initial mark, final re-mark) with the world stopped.
// Because of locking involved in stopping the world,
// the foreground collector should not block waiting for the background
// collector when it is doing a stop-the-world phase.  The background
// collector will yield the baton at an additional point just before
// it enters a stop-the-world phase.  Once the world is stopped, the
// background collector checks the phase of the collection.  If the
// phase has not changed, it proceeds with the collection.  If the
// phase has changed, it skips that phase of the collection.  See
// the comments on the use of the Heap_lock in collect_in_background().
//
// Variable used in baton passing.
//   _foregroundGCIsActive - Set to true by the foreground collector when
//	it wants the baton.  The foreground clears it when it has finished
//	the collection.
//   _foregroundGCShouldWait - Set to true by the background collector
//        when it is running.  The foreground collector waits while
//	_foregroundGCShouldWait is true.
//  CMS_lock - monitor used to protect access to the above variables
//	and to notify the foreground and background collectors.
//  _collectorState - current state of the CMS collection.
// 
// The foreground collector 
//   acquires the CMS_lock
//   sets _foregroundGCIsActive
//   waits on the CMS_lock for _foregroundGCShouldWait to be false
//     various locks acquired in preparation for the collection
//     are released so as not to block the background collector
//     that is in the midst of a collection
//   proceeds with the collection
//   clears _foregroundGCIsActive
//   returns
// The background collector in a loop iterating on the phases of the
//	collection
//   acquires the CMS_lock
//   sets _foregroundGCShouldWait
//   if _foregroundGCIsActive is set
//     waits for _foregroundGCIsActive to become false and 
//     exits the loop.
//   otherwise
//     proceed with that phase of the collection
//     if the phase is a stop-the-world phase, possibly
//	 yield the baton once more just before stopping
//	 the world.
//   returns after all phases of the collection are done
//   
//
// Debugging code
//  In order to try and force the baton passing in different phases
// of the collection, an attempt is made to make the background collector
// wait at each of the phases of a collection using the _debug_wait_lock1
// and _debug_wait_state variables.  The _debug_wait_state is initialized
// to the first non-idle state of a collection.  When the background
// is running and the _collectorState is equal to the _debug_wait_state,
// the background collector waits on the monitor _debug_wait_lock1.
// The foreground collector does a notify on _debug_wait_lock1 when it is
// done with a collection.  The background collector increments
// the _debug_wait_state so that it will wait at the next state the
// next time it is run.  It is expected that the background collector
// will find that the collection is done and exit the collection.

void CMSCollector::acquire_control_and_collect(bool full,
	bool clear_all_soft_refs) {
  assert(SafepointSynchronize::is_at_safepoint(), "should be at safepoint");
  assert(!Thread::current()->is_ConcurrentMarkSweep_thread(),
         "shouldn't try to acquire control from self!");

  // Start the protocol for acquiring control of the
  // collection from the background collector (aka CMS thread).
  assert(ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
         "VM thread should have CMS token");
  // Remember the possibly interrupted state of an ongoing
  // concurrent collection
  CollectorState first_state = _collectorState;

  // Signal to a possibly ongoing concurrent collection that
  // we want to do a foreground collection.
  _foregroundGCIsActive = true;

  // Disable incremental mode during a foreground collection.
  ICMSDisabler icms_disabler;

  // release locks and wait for a notify from the background collector
  // releasing the locks in only necessary for phases which
  // do yields to improve the granularity of the collection.
  assert_lock_strong(_modUnionTable.lock());
  assert_lock_strong(bitMapLock());
  // We need to lock the Free list lock for the space that we are
  // currently collecting.
  assert(haveFreelistLocks(), "Must be holding free list locks");
  _modUnionTable.lock()->unlock();
  bitMapLock()->unlock();
  releaseFreelistLocks();
  {
    MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
    if (_foregroundGCShouldWait) {
      // We are going to be waiting for action for the CMS thread;
      // it had better not be gone (for instance at shutdown)!
      assert(ConcurrentMarkSweepThread::first_thread() != NULL,
             "CMS thread must be running");
      // Wait here until the background collector gives us the go-ahead
      ConcurrentMarkSweepThread::clear_CMS_flag(
        ConcurrentMarkSweepThread::CMS_vm_has_token);  // release token
      // Get a possibly blocked CMS thread going:
      //   Note that we set _foregroundGCIsActive true above,
      //   without protection of the CMS_lock.
      CMS_lock->notify();
      assert(!ConcurrentMarkSweepThread::vm_thread_wants_cms_token(),
             "Possible deadlock");
      while (_foregroundGCShouldWait) {
        // wait for notification
        CMS_lock->wait(Mutex::_no_safepoint_check_flag);
        // Possibility of delay/starvation here, since CMS token does
        // not know to give priority to VM thread? Actually, i think
        // there wouldn't be any delay/starvation, but the proof of
        // that "fact" (?) appears non-trivial. XXX 20011219YSR
      }
      ConcurrentMarkSweepThread::set_CMS_flag(
        ConcurrentMarkSweepThread::CMS_vm_has_token);
    }
  }
  // The CMS_token is already held.  Get back the other locks.
  assert(ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
         "VM thread should have CMS token");
  getFreelistLocks();
  bitMapLock()->lock_without_safepoint_check();
  _modUnionTable.lock()->lock_without_safepoint_check();
  if (TraceCMSState) {
    gclog_or_tty->print_cr("CMS foreground collector has asked for control "
      INTPTR_FORMAT " with first state %d", Thread::current(), first_state);
    gclog_or_tty->print_cr("	gets control with state %d", _collectorState);
  }

  // Check if we need to do a compaction, or if not, whether
  // we need to start the mark-sweep from scratch.
  bool should_compact    = false;
  bool should_start_over = false;
  decide_foreground_collection_type(clear_all_soft_refs,
    &should_compact, &should_start_over);

  if (should_compact) {
    do_compaction_work(clear_all_soft_refs);
  } else {
    do_mark_sweep_work(clear_all_soft_refs, first_state,
      should_start_over);
  }
  _foregroundGCIsActive = false;
  return;
}

// A work method used by foreground collection to determine
// what type of collection (compacting or not, continuing or fresh)
// it should do.
// NOTE: the intent is to make UseCMSCompactAtFullCollection
// and CMSCompactWhenClearAllSoftRefs the default in the future
// and do away with the flags after a suitable period.
void CMSCollector::decide_foreground_collection_type(
  bool clear_all_soft_refs, bool* should_compact,
  bool* should_start_over) {
  // Normally, we'll compact only if the UseCMSCompactAtFullCollection
  // flag is set, and we have either requested a System.gc() or
  // the number of full gc's since the last concurrent cycle
  // has exceeded the threshold set by CMSFullGCsBeforeCompaction,
  // or if an incremental collection has failed
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  assert(gch->collector_policy()->is_two_generation_policy(),
         "You may want to check the correctness of the following");
  *should_compact =
    UseCMSCompactAtFullCollection &&
    ((_full_gcs_since_conc_gc >= CMSFullGCsBeforeCompaction) ||
     (gch->gc_cause() == GCCause::_java_lang_system_gc) ||
     gch->incremental_collection_will_fail());
  *should_start_over = false;
  if (clear_all_soft_refs && !*should_compact) {
    // We are about to do a last ditch collection attempt
    // so it would normally make sense to do a compaction
    // to reclaim as much space as possible.
    if (CMSCompactWhenClearAllSoftRefs) {
      // Default: The rationale is that in this case either
      // we are past the final marking phase, in which case
      // we'd have to start over, or so little has been done
      // that there's little point in saving that work. Compaction
      // appears to be the sensible choice in either case.
      *should_compact = true;
    } else {
      // We have been asked to clear all soft refs, but not to
      // compact. Make sure that we aren't past the final checkpoint
      // phase, for that is where we process soft refs. If we are already
      // past that phase, we'll need to redo the refs discovery phase and
      // if necessary clear soft refs that weren't previously
      // cleared. We do so by remembering the phase in which
      // we came in, and if we are past the refs processing
      // phase, we'll choose to just redo the mark-sweep
      // collection from scratch.
      if (_collectorState > FinalMarking) {
        // We are past the refs processing phase;
        // start over and do a fresh synchronous CMS cycle
        _collectorState = Resetting; // skip to reset to start new cycle
        reset(false /* == !asynch */);
        *should_start_over = true;
      } // else we can continue a possibly ongoing current cycle
    }
  }
}

// A work method used by the foreground collector to do
// a mark-sweep-compact.
void CMSCollector::do_compaction_work(bool clear_all_soft_refs) {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  if (PrintGC && Verbose &&
    !(gch->gc_cause() == GCCause::_java_lang_system_gc)) {
    gclog_or_tty->print_cr("Compact ConcurrentMarkSweepGeneration after %d "
      "collections passed to foreground collector", _full_gcs_since_conc_gc);
  }
  // Widen the span of the weak reference processing to the entire
  // heap.
  MemRegion new_span(GenCollectedHeap::heap()->reserved_region());
  ReferenceProcessorSpanMutator x(ref_processor(), new_span);
  ref_processor()->set_enqueuing_is_done(false);
  ref_processor()->enable_discovery();
  // If an asynchronous collection finishes, the _modUnionTable is
  // all clear.  If we are assuming the collection from an asynchronous
  // collection, clear the _modUnionTable.
  assert(_collectorState != Idling || _modUnionTable.isAllClear(),
    "_modUnionTable should be clear if the baton was not passed");
  _modUnionTable.clearAll();
  GenMarkSweep::invoke_at_safepoint(_cmsGen->level(),
    ref_processor(), clear_all_soft_refs);
  #ifdef ASSERT
    CompactibleFreeListSpace* cms_space = _cmsGen->cmsSpace();
    assert(cms_space->free() ==
           pointer_delta(cms_space->end(), cms_space->compaction_top())
           * HeapWordSize,
      "All the free space should be compacted into one chunk at top");
    assert(cms_space->dictionary()->totalChunkSize(
                                      debug_only(cms_space->freelistLock())) == 0 ||
           cms_space->totalSizeInIndexedFreeLists() == 0,
      "All the free space should be in a single chunk");
    assert((cms_space->free() == 0 && cms_space->totalCount() == 0) ||
           (cms_space->free() > 0  && cms_space->totalCount() == 1),
         "There should be at most 1 free chunk after compaction");
  #endif // ASSERT
  _collectorState = Resetting;
  reset(false /* == !asynch */);
  _cmsGen->reset_after_compaction();
}

// A work method used by the foreground collector to do
// a mark-sweep, after taking over from a possibly on-going
// concurrent mark-sweep collection.
void CMSCollector::do_mark_sweep_work(bool clear_all_soft_refs,
  CollectorState first_state, bool should_start_over) {
  if (PrintGC && Verbose) {
    gclog_or_tty->print_cr("Pass concurrent collection to foreground "
      "collector with count %d",
      _full_gcs_since_conc_gc);
  }
  switch (_collectorState) {
    case Idling:
      if (first_state == Idling || should_start_over) {
        // The background GC was not active, or should
        // restarted from scratch;  start the cycle.
        _collectorState = InitialMarking;
      }
      // If first_state was not Idling, then a background GC
      // was in progress and has now finished.  No need to do it
      // again.  Leave the state as Idling.
      break;
    case Precleaning:
      // In the foreground case don't do the precleaning since
      // it is not done concurrently and there is extra work
      // required.
      _collectorState = FinalMarking;
  }
  collect_in_foreground(clear_all_soft_refs);
}


void CMSCollector::getFreelistLocks() {
  // Get locks for all free lists in all generations that this
  // collector is responsible for
  _cmsGen->freelistLock()->lock_without_safepoint_check();
  _permGen->freelistLock()->lock_without_safepoint_check();
}

void CMSCollector::releaseFreelistLocks() {
  // Release locks for all free lists in all generations that this
  // collector is responsible for
  _cmsGen->freelistLock()->unlock();
  _permGen->freelistLock()->unlock();
}

bool CMSCollector::haveFreelistLocks() {  // FIX ME!!!
  // Check locks for all free lists in all generations that this
  // collector is responsible for
  assert_lock_strong(_cmsGen->freelistLock());
  assert_lock_strong(_permGen->freelistLock());
  return true;
}

// Used for debugging with CMSTestStateTransition1
static Monitor* _debug_wait_lock1 = NULL;
static int _debug_wait_state = 1;

// There are separate collect_in_background and collect_in_foreground because of
// the different locking requirements of the background collector and the
// foreground collector.  There was originally an attempt to share
// one "collect" method between the background collector and the foreground
// collector but the if-then-else required made it cleaner to have
// separate methods.
void CMSCollector::collect_in_background(bool clear_all_soft_refs) {
  assert(Thread::current()->is_ConcurrentMarkSweep_thread(),
    "A CMS asynchronous collection is only allowed on a CMS thread.");
  debug_only(
    if (_debug_wait_lock1 == NULL) {
      _debug_wait_lock1 = new Monitor(Mutex::nonleaf, "_debug_wait_lock1", true);
    }
  )
    
  {
    bool safepoint_check = Mutex::_no_safepoint_check_flag;
    MutexLockerEx x(CMS_lock, safepoint_check);
    if (_foregroundGCIsActive || !UseAsyncConcMarkSweepGC) {
      // The foreground collector is active or we're
      // not using asynchronous collections.  Skip this
      // background collection.
      assert(!_foregroundGCShouldWait, "Should be clear");
      return;
    } else {
      assert(_collectorState == Idling, "Should be idling before start.");
        if (GC_locker::is_active()) {
          // skip this bkgrd collection, instead
          // expanding the heap if necessary
	  MutexUnlockerEx unx(CMS_lock, safepoint_check);
	  // for the call to free() in compute_new_size()
	  FreelistLocker z(this);
          _permGen->compute_new_size();
          _cmsGen->compute_new_size();
          return;
        }
      _collectorState = InitialMarking;
    }
  }

  // Used for PrintGC
  size_t prev_used;
  if (PrintGC && Verbose) {
    prev_used = _cmsGen->used(); // XXXPERM
  }

  // The change of the collection state is normally done at this level.  The
  // the exceptions are states that are executed while the world is
  // stopped.  For those states the change of state is done while the
  // world is stopped.  For baton passing purposes this allows the 
  // background collector to finish the phase and change state atomically.
  // The foreground collector cannot wait on a phase that is done
  // while the world is stopped because the foreground collector already
  // has the world stopped and would deadlock.
  while (_collectorState != Idling) {
    if (TraceCMSState) {
      gclog_or_tty->print_cr("Thread " INTPTR_FORMAT " in CMS state %d", 
	Thread::current(), _collectorState);
    }
    // ------------- debugging code -------------
    // The background collector should wait here if the collector state
    // is at the desired state "_debug_wait_state1".  It should be notified
    // by the foreground collector once it starts a collection and hopefully
    // acquires the collection at this state.  This is not always effective
    // since the background collector may not get to "_debug_wait_state1".
    debug_only(
      if (_collectorState == _debug_wait_state && CMSTestStateTransition1) {
        MutexLockerEx(_debug_wait_lock1, Mutex::_no_safepoint_check_flag);
        if (TraceCMSState) {
          gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT 
            " waiting at CMS state %d", Thread::current(), _collectorState);
          }
        _debug_wait_lock1->wait(true);
        _debug_wait_state++;
        _debug_wait_state = _debug_wait_state % LastCollectorState;
        if (_debug_wait_state == Idling) {
          _debug_wait_state++;
        }
      }
    )

    // The foreground collector 
    //   holds the Heap_lock throughout its collection.
    //	 holds the CMS token (but not the lock)
    //     except while it is waiting for the background collector to yield.
    // The foreground collector should be blocked (not for long)
    //   if the background collector is about to start a phase
    //   executed during a stop-the-world.  If the background
    //   collector has already started such a phase, the
    //   foreground collector is blocked waiting for the
    //   Heap_lock.  These are currently phases InitialMarking 
    //   and FinalMarking.
    // The locking order is
    //   Heap_lock  (locked in stopWorldAndDo())
    //   CMS token  (claimed in
    //                stopWorldAndDo() -->
    //                  safepoint_synchronize() -->
    //                    CMSThread::synchronize())

    {
      // Check if the FG collector wants us to yield.
      CMSTokenSync x(true); // is cms thread
      if (waitForForegroundGC()) {
        // We yielded to a foreground GC, nothing more to be
        // done this round.
        assert(_foregroundGCShouldWait == false, "We set it to false in "
               "waitForForegroundGC()");
        if (TraceCMSState) {
          gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT 
            " exiting collection CMS state %d", 
            Thread::current(), _collectorState);
        }
        return;
      } else {
        // The background collector can run but check to see if the
        // foreground collector has done a collection while the
        // background collector was waiting to get the CMS_lock
        // above.  If yes, break so that _foregroundGCShouldWait
        // is cleared before returning.
        if (_collectorState == Idling) {
          break;
        }
      }
    }

    assert(_foregroundGCShouldWait, "Foreground collector, if active, "
      "should be waiting");
    switch (_collectorState) {
      case InitialMarking:
	if (CMSIncrementalMode && CMSIncrementalPacing) {
	  stats().record_cms_begin();
	}
	stopWorldAndDo(CMS_op_checkpointRootsInitial);
	// The collector state may be any legal state at this point
	// since the background collector may have yielded to the
	// foreground collector.
	break;
      case Marking:
	// initial marking in checkpointRootsInitialWork has been completed
        markFromRoots(true);
	assert(_collectorState == Precleaning, "Collector state should "
	  "have changed");
	break;
      case Precleaning:
	// marking from roots in markFromRoots has been completed
	preclean(true);
	assert(_collectorState == FinalMarking, "Collector state should "
	  "have changed");
	break;
      case FinalMarking:
	{
          // If a foreground collection is in progress, it already has
	  // the pending list lock.  This is similar to the situation
	  // with the Heap_lock.  See comments in stopWorldAndDo()
 	  // about racing for the Heap_lock.
          { 
            MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
            // allow a potentially blocked foreground collector to proceed
            _foregroundGCShouldWait = false;
            if (_foregroundGCIsActive) {
              CMS_lock->notify();
            }
            assert(!ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
                   "Possible deadlock");
          }

	  ConcurrentMarkSweepThread::manipulatePLL(
            SurrogateLockerThread::acquirePLL);
          bool didSomeWork = false;
          {
            MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
            // For regularity, set _foregroundGCShouldWait
	    // The background collector is grabbing the locks
	    // it needs to do the collection but on releasing the
	    // locks still make the foreground collector wait for
	    // the _foregroundGCShouldWait flag.
            _foregroundGCShouldWait = true;
          }
          if (_collectorState == FinalMarking) {
            // we didn't lose a race to FG thread
	    stopWorldAndDo(CMS_op_checkpointRootsFinal);
            didSomeWork = true;
          } else {
            // else we did lose a race to FG thread
	    assert(_collectorState == Idling, "The foreground collector"
	           " should have finished the collection");
	  }
          // Check if we need to post a notification on PLL;
          if (didSomeWork &&
              _ref_processor->read_and_reset_notify_ref_lock()) {
            ConcurrentMarkSweepThread::manipulatePLL(
              SurrogateLockerThread::releaseAndNotifyPLL);
          } else {
            ConcurrentMarkSweepThread::manipulatePLL(
              SurrogateLockerThread::releasePLL);
          }
	}
	break;
      case Sweeping:
	// final marking in checkpointRootsFinal has been completed
        sweep(true);
	assert(_collectorState == Resetting, "Collector state change "
	  "to Resetting must be done under the free_list_lock");
        _full_gcs_since_conc_gc = 0;
	break;
      case Resetting:
	// sweep in sweep() has been completed
        reset(true);
	assert(_collectorState == Idling, "Collector state should "
	  "have changed");
	if (CMSIncrementalMode && CMSIncrementalPacing) {
	  stats().record_cms_end();
	}
	break;
      case Idling:
      default:
	ShouldNotReachHere();
	break;
    }
    if (TraceCMSState) {
      gclog_or_tty->print_cr("	Thread " INTPTR_FORMAT " done - next CMS state %d", 
	Thread::current(), _collectorState);
    }
    assert(_foregroundGCShouldWait, "Foreground collector should be waiting");
    if (CMSTestStateTransition1) {
      debug_only(
        MutexLockerEx(_debug_wait_lock1, Mutex::_no_safepoint_check_flag);
        _debug_wait_lock1->notify_all();
      )
    }
  }

  {  
    // Clear _foregroundGCShouldWait before returning.
    // In the event that the  foregrould collector is
    // waiting, this notifies it.
    MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
    _foregroundGCShouldWait = false;
    if (_foregroundGCIsActive) {
      CMS_lock->notify();
    }
    assert(!ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
           "Possible deadlock");
  }
  if (TraceCMSState) {
    gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT 
      " exiting collection CMS state %d", 
      Thread::current(), _collectorState);
  }
  if (PrintGC && Verbose) {
    _cmsGen->print_heap_change(prev_used);
  }
}

void CMSCollector::collect_in_foreground(bool clear_all_soft_refs) {
  assert(_foregroundGCIsActive && !_foregroundGCShouldWait,
         "Foreground collector should be waiting, not executing");
  assert(Thread::current()->is_VM_thread(), "A foreground collection" 
    "may only be done by the VM Thread with the world stopped");
  assert(ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
         "VM thread should have CMS token");

  COMPILER2_ONLY(DerivedPointerTableDeactivate dpt_deact;)

  if (VerifyBeforeGC &&
      GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
    Universe::verify(true);
  }
  debug_only(
    if (_debug_wait_lock1 == NULL) {
      _debug_wait_lock1 = new Monitor(Mutex::nonleaf, "_debug_wait_lock1", true);
    }
  )


  bool init_mark_was_synchronous = false; // until proven otherwise
  while (_collectorState != Idling) {
    if (TraceCMSState) {
      gclog_or_tty->print_cr("Thread " INTPTR_FORMAT " in CMS state %d", 
	Thread::current(), _collectorState);
    }

    switch (_collectorState) {
      case InitialMarking:
        init_mark_was_synchronous = true;  // fact to be exploited in re-mark
        checkpointRootsInitial(false);
	assert(_collectorState == Marking, "Collector state should have changed"
	  " within checkpointRootsInitial()");
	break;
      case Marking:
	// initial marking in checkpointRootsInitialWork has been completed
        if (VerifyDuringGC &&
            GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
          Universe::verify(true);
        }
        markFromRoots(false);
	assert(_collectorState == FinalMarking, "Collector state should "
	  "have changed");
	break;
      case FinalMarking:
        if (VerifyDuringGC &&
            GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
          Universe::verify(true);
        }
        checkpointRootsFinal(false, clear_all_soft_refs,
                             init_mark_was_synchronous);
	assert(_collectorState == Sweeping, "Collector state should not "
	  "have changed within checkpointRootsFinal()");
	break;
      case Sweeping:
	// final marking in checkpointRootsFinal has been completed
        if (VerifyDuringGC &&
            GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
          Universe::verify(true);
        }
        sweep(false);
	assert(_collectorState == Resetting, "Collector state change "
	  "to Resetting must be done under the free_list_lock");
	break;
      case Resetting:
	// sweep in sweep() has been completed
        if (VerifyDuringGC &&
            GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
          Universe::verify(true);
        }
        reset(false);
	assert(_collectorState == Idling, "Collector state should "
	  "have changed");
	break;
      case Precleaning:
      default:
	ShouldNotReachHere();
    }
    if (TraceCMSState) {
      gclog_or_tty->print_cr("	Thread " INTPTR_FORMAT " done - next CMS state %d", 
	Thread::current(), _collectorState);
    }
    if (CMSTestStateTransition1) {
      debug_only(
        MutexLockerEx(_debug_wait_lock1, Mutex::_no_safepoint_check_flag);
        _debug_wait_lock1->notify_all();
      )
    }
  }

  if (VerifyAfterGC &&
      GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
    Universe::verify(true);
  }
  if (TraceCMSState) {
    gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT 
      " exiting collection CMS state %d", 
      Thread::current(), _collectorState);
  }
}

bool CMSCollector::waitForForegroundGC() {
  bool res = false;
  assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
         "CMS thread should have CMS token");
  // Block the foreground collector until the
  // background collectors decides whether to
  // yield.
  MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
  _foregroundGCShouldWait = true;
  if (_foregroundGCIsActive) {
    // The background collector yields to the
    // foreground collector and returns a value
    // indicating that it has yielded.  The foreground
    // collector can proceed.
    res = true;
    _foregroundGCShouldWait = false;
    ConcurrentMarkSweepThread::clear_CMS_flag(
      ConcurrentMarkSweepThread::CMS_cms_has_token);
    ConcurrentMarkSweepThread::set_CMS_flag(
      ConcurrentMarkSweepThread::CMS_cms_wants_token);
    // Get a possibly blocked foreground thread going
    CMS_lock->notify();
    if (TraceCMSState) {
      gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT " waiting at CMS state %d",
        Thread::current(), _collectorState);
    }
    while (_foregroundGCIsActive) {
      CMS_lock->wait(Mutex::_no_safepoint_check_flag);
    }
    ConcurrentMarkSweepThread::set_CMS_flag(
      ConcurrentMarkSweepThread::CMS_cms_has_token);
    ConcurrentMarkSweepThread::clear_CMS_flag(
      ConcurrentMarkSweepThread::CMS_cms_wants_token);
  }
  if (TraceCMSState) {
    gclog_or_tty->print_cr("CMS Thread " INTPTR_FORMAT " continuing at CMS state %d",
      Thread::current(), _collectorState);
  }
  return res;
}

// Because of the need to lock the free lists and other structures in
// the collector, common to all the generations that the collector is
// collecting, we need the gc_prologues of individual CMS generations
// delegate to their collector. It may have been simpler had the
// current infrastructure allowed one to call a prologue on a
// collector. In the absence of that we have the generation's
// prologue delegate to the collector, which delegates back
// some "local" work to a worker method in the individual generations
// that it's responsible for collecting, while itself doing any
// work common to all generations it's responsible for. A similar
// comment applies to the  gc_epilogue()'s.
// The role of the varaible _between_prologue_and_epilogue is to
// enforce the invocation protocol.
void CMSCollector::gc_prologue(bool full) {
  // Call gc_prologue_work() for each CMSGen and PermGen that
  // we are responsible for.

  // The following locking discipline assumes that we are only called
  // when the world is stopped.
  assert(SafepointSynchronize::is_at_safepoint(), "world is stopped assumption");

  // The CMSCollector prologue must call the gc_prologues for the
  // "generations" (including PermGen if any) that it's responsible
  // for.

  assert(ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
         "VM thread should have CMS token");

  if (_between_prologue_and_epilogue) {
    // We have already been invoked; this is a gc_prologue delegation
    // from yet another CMS generation that we are responsible for, just
    // ignore it since all relevant work has already been done.
    return;
  }
  // set a bit saying prologue has been called; cleared in epilogue
  _between_prologue_and_epilogue = true;

  // Claim locks for common data structures, then call gc_prologue_work()
  // for each CMSGen and PermGen that we are responsible for.

  getFreelistLocks();   // gets free list locks on constituent spaces
  bitMapLock()->lock_without_safepoint_check();
  modUnionTableLock()->lock_without_safepoint_check();

  // Should call gc_prologue_work() for all cms gens we are responsible for
  bool registerClosure =    _collectorState >= Marking
                         && _collectorState < Sweeping;
  ModUnionClosure* muc = ParallelGCThreads > 0 ? &_modUnionClosurePar
                                               : &_modUnionClosure;
  _cmsGen->gc_prologue_work(full, registerClosure, muc);
  _permGen->gc_prologue_work(full, registerClosure, muc);

  if (!full) {
    if (CMSIncrementalMode && CMSIncrementalPacing) {
      stats().record_gc0_begin();
    }
  }
}

void ConcurrentMarkSweepGeneration::gc_prologue(bool full) {
  // Delegate to CMScollector which knows how to coordinate between
  // this and any other CMS generations that it is responsible for
  // collecting.
  collector()->gc_prologue(full);
}

// This is a "private" interface for use by this generation's CMSCollector.
// Not to be called directly by any other entity (for instance,
// GenCollectedHeap, which calls the "public" gc_prologue method above).
void ConcurrentMarkSweepGeneration::gc_prologue_work(bool full,
  bool registerClosure, ModUnionClosure* modUnionClosure) {
  assert(cmsSpace()->preconsumptionDirtyCardClosure() == NULL,
    "Should be NULL");
  if (registerClosure) {
    cmsSpace()->setPreconsumptionDirtyCardClosure(modUnionClosure);
  }
  cmsSpace()->gc_prologue();
  // Clear stat counters
  NOT_PRODUCT(
    assert(_numObjectsPromoted == 0, "check");
    assert(_numWordsPromoted   == 0, "check");
    if (Verbose && PrintGC) {
      gclog_or_tty->print("Allocated %d objects, %d bytes concurrently",
      _numObjectsAllocated, _numWordsAllocated*sizeof(HeapWord));
    }
    _numObjectsAllocated = 0;
    _numWordsAllocated   = 0;
  )
}

void CMSCollector::gc_epilogue(bool full) {
  // The following locking discipline assumes that we are only called
  // when the world is stopped.
  assert(SafepointSynchronize::is_at_safepoint(),
         "world is stopped assumption");

  // Currently the CMS epilogue (see CompactibleFreeListSpace) merely checks
  // if linear allocation blocks need to be appropriately marked to allow the
  // the blocks to be parsable. We also check here whether we need to nudge the
  // CMS collector thread to start a new cycle (if it's not already active).
  assert(Thread::current()->is_VM_thread()
         && ConcurrentMarkSweepThread::vm_thread_has_cms_token(),
         "VM thread should have CMS token");
  
  if (!_between_prologue_and_epilogue) {
    // We have already been invoked; this is a gc_epilogue delegation
    // from yet another CMS generation that we are responsible for, just
    // ignore it since all relevant work has already been done.
    return;
  }

  assert(haveFreelistLocks(), "must have freelist locks");
  assert_lock_strong(bitMapLock());
  assert_lock_strong(modUnionTableLock());

  _cmsGen->gc_epilogue_work(full);
  _permGen->gc_epilogue_work(full);

  size_t cms_used   = _cmsGen->cmsSpace()->used();
  size_t perm_used  = _permGen->cmsSpace()->used();

  // update performance counters - this uses a special version of
  // update_counters() that allows the utilization to be passed as a
  // parameter, avoiding multiple calls to used().
  //
  _cmsGen->update_counters(cms_used);
  _permGen->update_counters(perm_used);

  if (!full) {
    if (CMSIncrementalMode && CMSIncrementalPacing) {
      stats().record_gc0_end(cms_used);
    }
  }

  if (CMSIncrementalMode) {
    icms_update_allocation_limits();
  }

  modUnionTableLock()->unlock();
  bitMapLock()->unlock();
  releaseFreelistLocks();

  _between_prologue_and_epilogue = false;  // ready for next cycle
}

void ConcurrentMarkSweepGeneration::gc_epilogue(bool full) {
  collector()->gc_epilogue(full);
  set_expansion_cause(CMSExpansionCause::_no_expansion);

  // Also reset promotion tracking in par gc thread states.
  if (ParallelGCThreads > 0) {
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _par_gc_thread_states[i]->promo.stopTrackingPromotions();
    }
  }
}

void ConcurrentMarkSweepGeneration::gc_epilogue_work(bool full) {
  cmsSpace()->setPreconsumptionDirtyCardClosure(NULL);
  cmsSpace()->gc_epilogue();
    // Print stat counters
  NOT_PRODUCT(
    assert(_numObjectsAllocated == 0, "check");
    assert(_numWordsAllocated == 0, "check");
    if (Verbose && PrintGC) {
      gclog_or_tty->print("Promoted %d objects, %d bytes",
                 _numObjectsPromoted, _numWordsPromoted*sizeof(HeapWord));
    }
    _numObjectsPromoted = 0;
    _numWordsPromoted   = 0;
  )

  if (PrintGC && Verbose) {
    // Call down the chain in contiguous_available needs the freelistLock
    // so print this out before releasing the freeListLock.
    gclog_or_tty->print(" Contiguous available %d bytes ", contiguous_available());
  }
}

// Check reachability of the given heap address in CMS generation,
// treating all other generations as roots.
bool CMSCollector::is_cms_reachable(HeapWord* addr) {
  // We could "guarantee" below, rather than assert, but i'll
  // leave these as "asserts" so that an adventurous debugger
  // could try this in the product build provided some subset of
  // the consitions were met, provided they were intersted in the
  // results and knew that the computation below wouldn't interfere
  // with other concurrent computations mutating the structures
  // being read or written.
  assert(SafepointSynchronize::is_at_safepoint(),
         "Else mutations in object graph will make answer suspect");
  assert(   (   Thread::current()->is_VM_thread()
             && ConcurrentMarkSweepThread::vm_thread_has_cms_token())
         || (   Thread::current()->is_ConcurrentMarkSweep_thread()
             && ConcurrentMarkSweepThread::cms_thread_has_cms_token()),
         "Else there may be mutual interference in use of CMS data structures");
  assert(_collectorState < Marking || _collectorState > Sweeping,
         "Else method will change CMS data structure state");
  assert(haveFreelistLocks(), "must hold free list locks");
  assert_lock_strong(bitMapLock());

  // Clear the marking bit map array before starting, but, just
  // for kicks, first report if the given address is already marked
  gclog_or_tty->print_cr("Start: Address 0x%x is%s marked", addr,
                _markBitMap.isMarked(addr) ? "" : " not");
  _markBitMap.clearAll();
  
  ResourceMark rm;
  HandleMark  hm;
  // Mark from roots one level into CMS
  {
    MarkRefsIntoClosure notOlder(_span, &_markBitMap, true  /* nmethods */);
    GenCollectedHeap* gch = GenCollectedHeap::heap();
  
    assert(_markStack.isEmpty(), "markStack should be empty");
    gch->ensure_parseability();
    // Update the saved marks which may affect the root scans.
    gch->save_marks();
  
    COMPILER2_ONLY(DerivedPointerTableDeactivate dpt_deact;)

    warning("is_cms_reachable: PERM needs change");
    gch->rem_set()->prepare_for_younger_refs_iterate(false); // Not parallel.
    gch->process_strong_roots(_cmsGen->level(),
                              true,   // younger gens are roots
                              true,   // collecting perm gen
                              CMSClassUnloadingEnabled ?
                                GenCollectedHeap::CSO_SystemClasses :
                                GenCollectedHeap::CSO_AllClasses,
                              NULL, &notOlder);
  }

  // Just for kicks, report if the said address is directly
  // reachable from roots into CMS, if so we are done, but
  // we'll do the full marking anyway
  gclog_or_tty->print_cr("RootMark: Address 0x%x is%s marked", addr,
                _markBitMap.isMarked(addr) ? "" : " not");

  
  // Now mark from the roots
  {
    MarkFromRootsClosure markFromRootsClosure(this, _span,
      &_markBitMap, &_markStack, &_revisitStack, false);  // don't yield
    _markBitMap.iterate(&markFromRootsClosure);
  }
  gclog_or_tty->print_cr("TransitiveMark: Address 0x%x is%s marked", addr,
                _markBitMap.isMarked(addr) ? "" : " not");

  gclog_or_tty->print_cr("Now manually run _markBitMap.clearAll() on CMSGen, "
                " otherwise subsequent execution will potentially "
                " lead to assertion violations or crashes.");
  return _markBitMap.isMarked(addr);
}

void ConcurrentMarkSweepGeneration::save_marks() {
  // delegate to CMS space
  cmsSpace()->save_marks();
  if (ParallelGCThreads > 0) {
    for (uint i = 0; i < ParallelGCThreads; i++) {
      _par_gc_thread_states[i]->promo.startTrackingPromotions();
    }
  }
}

bool ConcurrentMarkSweepGeneration::no_allocs_since_save_marks() {
  return cmsSpace()->no_allocs_since_save_marks();
}

#define CMS_SINCE_SAVE_MARKS_DEFN(OopClosureType, nv_suffix)    \
                                                                \
void ConcurrentMarkSweepGeneration::                            \
oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl) {   \
  cl->set_generation(this);                                     \
  cmsSpace()->oop_since_save_marks_iterate##nv_suffix(cl);      \
  cl->reset_generation();                                       \
  save_marks();                                                 \
}

ALL_SINCE_SAVE_MARKS_CLOSURES(CMS_SINCE_SAVE_MARKS_DEFN)

void
ConcurrentMarkSweepGeneration::object_iterate_since_last_GC(ObjectClosure* blk)
{
  // Not currently implemented; need to do the following. -- ysr.
  // dld -- I think that is used for some sort of allocation profiler.  So it
  // really means the objects allocated by the mutator since the last
  // GC.  We could potentially implement this cheaply by recording only
  // the direct allocations in a side data structure.
  //
  // I think we probably ought not to be required to support these
  // iterations at any arbitrary point; I think there ought to be some
  // call to enable/disable allocation profiling in a generation/space,
  // and the iterator ought to return the objects allocated in the
  // gen/space since the enable call, or the last iterator call (which
  // will probably be at a GC.)  That way, for gens like CM&S that would
  // require some extra data structure to support this, we only pay the
  // cost when it's in use...
  cmsSpace()->object_iterate_since_last_GC(blk);
}

void
ConcurrentMarkSweepGeneration::younger_refs_iterate(OopsInGenClosure* cl) {
  cl->set_generation(this);
  younger_refs_in_space_iterate(_cmsSpace, cl);
  cl->reset_generation();
}

void
ConcurrentMarkSweepGeneration::get_locks_and_oop_iterate(OopClosure* cl) {
  if (freelistLock()->owned_by_self()) {
    oop_iterate(cl);
  } else {
    MutexLockerEx x(freelistLock(), Mutex::_no_safepoint_check_flag);
    oop_iterate(cl);
  }
}

void
ConcurrentMarkSweepGeneration::get_locks_and_object_iterate(ObjectClosure* cl) {
  if (freelistLock()->owned_by_self()) {
    object_iterate(cl);
  } else {
    MutexLockerEx x(freelistLock(), Mutex::_no_safepoint_check_flag);
    object_iterate(cl);
  }
}

void
ConcurrentMarkSweepGeneration::prepare_for_verify() {
  // Fix the linear allocation blocks to look like free blocks.

  // Locks are normally acquired/released in gc_prologue/gc_epilogue, but those
  // are not called when the heap is verified during universe initialization and
  // at vm shutdown.
  if (freelistLock()->owned_by_self()) {
    cmsSpace()->prepare_for_verify();
  } else {
    MutexLockerEx fll(freelistLock(), Mutex::_no_safepoint_check_flag);
    cmsSpace()->prepare_for_verify();
  }
}

void
ConcurrentMarkSweepGeneration::verify(bool allow_dirty /* ignored */) {
  // Locks are normally acquired/released in gc_prologue/gc_epilogue, but those
  // are not called when the heap is verified during universe initialization and
  // at vm shutdown.
  if (freelistLock()->owned_by_self()) {
    cmsSpace()->verify(false /* ignored */);
  } else {
    MutexLockerEx fll(freelistLock(), Mutex::_no_safepoint_check_flag);
    cmsSpace()->verify(false /* ignored */);
  }
}

#ifndef PRODUCT
void CMSCollector::verify(bool allow_dirty /* ignored */) {
  _cmsGen->verify(allow_dirty);
  _permGen->verify(allow_dirty);
}

HeapWord* CMSCollector::block_start(const void* p) const {
  const HeapWord* addr = (HeapWord*)p;
  if (_span.contains(p)) {
    if (_cmsGen->cmsSpace()->contains(addr)) {
      return _cmsGen->cmsSpace()->block_start(p);
    } else {
      assert(_permGen->cmsSpace()->contains(addr), "Inconsistent _span?");
      return _permGen->cmsSpace()->block_start(p);
    }
  }
  return NULL;
}
#endif

HeapWord*
ConcurrentMarkSweepGeneration::expand_and_allocate(size_t word_size,
                                                   bool   large_noref,
                                                   bool   tlab,
						   bool   parallel) {
  MutexLockerEx x(freelistLock(), Mutex::_no_safepoint_check_flag);
  expand(word_size*HeapWordSize, MinHeapDeltaBytes,
    CMSExpansionCause::_satisfy_allocation);
  size_t adj_word_sz = CompactibleFreeListSpace::adjustObjectSize(word_size);
  if (parallel) {
    return cmsSpace()->par_allocate(adj_word_sz);
  } else {
    return cmsSpace()->allocate(adj_word_sz, large_noref);
  }
}

// YSR: All of this generation expansion/shrinking stuff is an exact copy of
// OneContigSpaceCardGeneration, which makes me wonder if we should move this
// to CardGeneration and share it...
void ConcurrentMarkSweepGeneration::expand(size_t bytes, size_t expand_bytes,
  CMSExpansionCause::Cause cause)
{
  GCMutexLocker x(ExpandHeap_lock);
  size_t aligned_bytes  = ReservedSpace::page_align_size_up(bytes);
  size_t aligned_expand_bytes = ReservedSpace::page_align_size_up(expand_bytes);
  bool success = false;
  if (aligned_expand_bytes > aligned_bytes) {
    success = grow_by(aligned_expand_bytes);
  }
  if (!success) {
    success = grow_by(aligned_bytes);
  }
  if (!success) {
    size_t remaining_bytes = _virtual_space.uncommitted_size();
    if (remaining_bytes > 0) {
      success = grow_by(remaining_bytes);
    }
  }
  if (GC_locker::is_active()) {
    // Tell the GC locker that we had to expand the heap
    GC_locker::heap_expanded();
    if (PrintGC && Verbose) {
      gclog_or_tty->print_cr("Garbage collection disabled, expanded heap instead");
    }
  }
  // remember why we expanded; this information is used
  // by shouldConcurrentCollect() when making decisions on whether to start
  // a new CMS cycle.
  if (success) {
    set_expansion_cause(cause);
    if (PrintGCDetails && Verbose) {
      gclog_or_tty->print_cr("Expanded CMS gen for %s", 
	CMSExpansionCause::to_string(cause));
    }
  }
}

HeapWord* ConcurrentMarkSweepGeneration::expand_and_par_lab_allocate(CMSParGCThreadState* ps, size_t word_sz) {
  HeapWord* res = NULL;
  MutexLocker x(ParGCRareEvent_lock);
  while (true) {
    // Expansion by some other thread might make alloc OK now:
    res = ps->lab.alloc(word_sz);
    if (res != NULL) return res;
    // If there's not enough expansion space available, give up.
    if (_virtual_space.uncommitted_size() < (word_sz * HeapWordSize)) {
      return NULL;
    }
    // Otherwise, we try expansion.
    expand(word_sz*HeapWordSize, MinHeapDeltaBytes,
      CMSExpansionCause::_allocate_par_lab);
    // Now go around the loop and try alloc again;
    // A competing par_promote might beat us to the expansion space,
    // so we may go around the loop again if promotion fails agaion.
  }
}


void ConcurrentMarkSweepGeneration::shrink(size_t bytes) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  size_t size = ReservedSpace::page_align_size_down(bytes);
  if (size > 0) {
    shrink_by(size);
  }
}

bool ConcurrentMarkSweepGeneration::grow_by(size_t bytes) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  bool result = _virtual_space.expand_by(bytes);
  if (result) {
    HeapWord* old_end = _cmsSpace->end();
    size_t new_word_size = 
      heap_word_size(_virtual_space.committed_size());
    MemRegion mr(_cmsSpace->bottom(), new_word_size);
    _bts->resize(new_word_size);  // resize the block offset shared array
    Universe::heap()->barrier_set()->resize_covered_region(mr);
    // Hmmmm... why doesn't CFLS::set_end verify locking?
    // This is quite ugly; FIX ME XXX
    _cmsSpace->assert_locked();
    _cmsSpace->set_end((HeapWord*)_virtual_space.high());
    // call after resetting end so that a list exists to
    // hold census for this expansion.
    _cmsSpace->splitBirth(pointer_delta(_cmsSpace->end(), old_end));

    // update the space and generation capacity counters
    if (UsePerfData) {
      _space_counters->update_capacity();
      _gen_counters->update_all();
    }

    if (Verbose && PrintGC) {
      size_t new_mem_size = _virtual_space.committed_size();
      size_t old_mem_size = new_mem_size - bytes;
      gclog_or_tty->print_cr("Expanding %s from %ldK by %ldK to %ldK",
                    name(), old_mem_size/K, bytes/K, new_mem_size/K);
    }
  }
  return result;
}

void ConcurrentMarkSweepGeneration::grow_to_reserved() {
  assert_locked_or_safepoint(ExpandHeap_lock);
  size_t remaining_bytes = _virtual_space.uncommitted_size();
  if (remaining_bytes > 0) {
    bool success = grow_by(remaining_bytes);
    assert(success, "grow to reserved failed");
  }
}


void ConcurrentMarkSweepGeneration::shrink_by(size_t bytes) {
  assert_locked_or_safepoint(ExpandHeap_lock);
  assert_lock_strong(freelistLock());
  // XXX Fix when compaction is implemented.
  warning("Shrinking of CMS not yet implemented");
  return;
}

// Simple ctor/dtor wrapper for accounting & timer chores around concurrent
// phases.
class CMSPhaseAccounting: public StackObj {
 public:
  CMSPhaseAccounting(CMSCollector *collector, const char *phase);
  ~CMSPhaseAccounting();

 private:
  CMSCollector *_collector;
  const char *_phase;
  elapsedTimer _wallclock;
};

CMSPhaseAccounting::CMSPhaseAccounting(CMSCollector *collector,
				       const char *phase) {
  _collector = collector;
  _phase = phase;

  if (PrintCMSStatistics != 0) {
    _collector->resetYields();
  }
  if (PrintGCDetails && PrintGCTimeStamps) {
    gclog_or_tty->stamp();
    gclog_or_tty->print_cr(": [CMS-concurrent-%s-start]", _phase);
  }
  _collector->resetTimer();
  _wallclock.start();
  _collector->startTimer();
}

CMSPhaseAccounting::~CMSPhaseAccounting() {
  _collector->stopTimer();
  _wallclock.stop();
  if (PrintGCDetails) {
    if (PrintGCTimeStamps) {
      gclog_or_tty->stamp();
      gclog_or_tty->print(": ");
    }
    gclog_or_tty->print_cr("[CMS-concurrent-%s: %3.3f/%3.3f secs]", 
		  _phase, _collector->timerValue(), _wallclock.seconds());
    if (PrintCMSStatistics != 0) {
      gclog_or_tty->print_cr(" (CMS-concurrent-%s yielded %d times)", _phase,
		    _collector->yields());
    }
  }
}

// CMS work

// Checkpoint the roots into this generation from outside
// this generation. [Note this initial checkpoint need only
// be approximate -- we'll do a catch up phase subsequently.]
void CMSCollector::checkpointRootsInitial(bool asynch) {
  assert(_collectorState == InitialMarking, "Wrong collector state");
  checkCorrectThreadExecuting();
  ReferenceProcessor* rp = ref_processor();
  SpecializationStats::clear();
  if (asynch) {
    // acquire locks for subsequent manipulations
    MutexLockerEx x(bitMapLock(),
                    Mutex::_no_safepoint_check_flag);
    MutexLockerEx y(modUnionTableLock(),
                    Mutex::_no_safepoint_check_flag);
    checkpointRootsInitialWork(asynch);
    rp->verify_no_references_recorded();
    rp->enable_discovery(); // enable ("weak") refs discovery
    _collectorState = Marking;
  } else {
    // (Weak) Refs discovery: this is controlled from genCollectedHeap::do_collection
    // which recognizes if we are a CMS generation, and doesn't try to turn on
    // discovery; verify that they aren't meddling.
    assert(!rp->discovery_is_atomic(),
           "incorrect setting of discovery predicate");
    assert(!rp->discovery_enabled(), "genCollectedHeap shouldn't control "
           "ref discovery for this generation kind");
    // already have locks
    checkpointRootsInitialWork(asynch);
    rp->enable_discovery(); // now enable ("weak") refs discovery
    _collectorState = Marking;
  }
  SpecializationStats::print();
}

void CMSCollector::checkpointRootsInitialWork(bool asynch) {
  assert(SafepointSynchronize::is_at_safepoint(), "world should be stopped");
  assert(_collectorState == InitialMarking, "just checking");

  // If there has not been a GC[n-1] since last GC[n] cycle completed,
  // precede our marking with a collection of all
  // younger generations to keep floating garbage to a minimum.
  // XXX: we won't do this for now -- it's an optimization to be done later.

  // already have locks
  assert_lock_strong(bitMapLock());
  assert(_markBitMap.isAllClear(), "was reset at end of previous cycle");

  ResourceMark rm;
  HandleMark  hm;

  FalseClosure falseClosure;
  // In the case of a synchronous collection, we will elide the
  // remark step, so it's important to catch all the nmethod oops
  // in this step; hence the last argument to the constrcutor below.
  MarkRefsIntoClosure notOlder(_span, &_markBitMap, !asynch /* nmethods */);
  GenCollectedHeap* gch = GenCollectedHeap::heap();

  assert(_markStack.isEmpty(), "markStack should be empty");
  gch->ensure_parseability();
  // Update the saved marks which may affect the root scans.
  gch->save_marks();

  // weak reference processing has not started yet.
  ref_processor()->set_enqueuing_is_done(false);

  {
    COMPILER2_ONLY(DerivedPointerTableDeactivate dpt_deact;)
    gch->rem_set()->prepare_for_younger_refs_iterate(false); // Not parallel.
    gch->process_strong_roots(_cmsGen->level(),
                              true,   // younger gens are roots
                              true,   // collecting perm gen
                              CMSClassUnloadingEnabled ?
                                GenCollectedHeap::CSO_SystemClasses :
                                GenCollectedHeap::CSO_AllClasses,
                              NULL, &notOlder);
  }

  // Clear mod-union table; it will be dirtied in the prologue of
  // CMS generation per each younger generation collection.

  // already have the lock
  assert_lock_strong(modUnionTableLock());
  assert(_modUnionTable.isAllClear(),
       "Was cleared in most recent final checkpoint phase"
       " or no bits are set in the gc_prologue before the start of the next "
       "subsequent marking phase.");

  // Temporarily disabled, since pre/post-consumption closures don't
  // care about precleaned cards
  #if 0
  {
    MemRegion mr = MemRegion((HeapWord*)_virtual_space.low(),
			     (HeapWord*)_virtual_space.high());
    _ct->ct_bs()->preclean_dirty_cards(mr);
  }
  #endif

  // Save the end of the used_region of the constituent generations
  // to be used to limit the extent of sweep in each generation.
  save_sweep_limits();
}

void CMSCollector::markFromRoots(bool asynch) {
  // we might be tempted to assert that:
  // assert(asynch == !SafepointSynchronize::is_at_safepoint(),
  //        "inconsistent argument?");
  // However that wouldn't be right, because it's possible that
  // a safepoint is indeed in progress as a younger generation
  // stop-the-world GC happens even as we mark in this generation.

  assert(_collectorState == Marking, "inconsistent state?");
  checkCorrectThreadExecuting();
  if (asynch) {
    // Weak ref discovery note: We may be discovering weak
    // refs in this generation concurrent (but interleaved) with
    // weak ref discovery by a younger generation collector.
    CMSTokenSyncWithLocks ts(true, bitMapLock());
    CMSPhaseAccounting pa(this, "mark");
    markFromRootsWork(asynch);
    _collectorState = Precleaning;
  } else {
    assert(SafepointSynchronize::is_at_safepoint(),
           "inconsistent with asynch == false");
    // already have locks
    markFromRootsWork(asynch);
    _collectorState = FinalMarking;
  }
}

void CMSCollector::markFromRootsWork(bool asynch) {
  // iterate over marked bits in bit map, doing a full scan and mark
  // from these roots using the following algorithm:
  // . if oop is to the right of the current scan pointer,
  //   mark corresponding bit (we'll process it later)
  // . else (oop is to left of current scan pointer)
  //   push oop on marking stack
  // . drain the marking stack

  // Note that when we do a marking step we need to hold the
  // bit map lock -- recall that direct allocation (by mutators)
  // and promotion (by younger generation collectors) is also
  // marking the bit map. [the so-called allocate live policy.]
  // Because the implementation of bit map marking is not
  // robust wrt simultaneous marking of bits in the same word,
  // we need to make sure that there is no such interference
  // between concurrent such updates.

  // already have locks
  assert_lock_strong(bitMapLock());

  ResourceMark rm;
  HandleMark   hm;

  // Clear the revisit stack, just in case there are any
  // obsolete contents from a short-circuited previous CMS cycle.
  _revisitStack.reset();
  assert(_revisitStack.isEmpty(), "tabula rasa");
  assert(_markStack.isEmpty(),    "tabula rasa");
  MarkFromRootsClosure markFromRootsClosure(this, _span, &_markBitMap,
    &_markStack, &_revisitStack, CMSYield && asynch);
  // the last argument to iterate indicates whether the iteration
  // should be incremental with periodic yields.
  _markBitMap.iterate(&markFromRootsClosure);
}

void CMSCollector::preclean(bool asynch) {
  checkCorrectThreadExecuting();
  if (asynch) {
    if (CMSPrecleaningEnabled) {
      CMSPhaseAccounting(this, "preclean");
      precleanWork();
    }
    CMSTokenSync x(true); // is cms thread
    _collectorState = FinalMarking;
  } else {
    if (CMSPrecleaningEnabled) {
      precleanWork();
    }
    _collectorState = FinalMarking;
  }
}
    

void CMSCollector::precleanWork() {

  assert(_collectorState == Precleaning, "incorrect state");

  MarkRefsIntoAndScanClosure
    mrias_cl(_span, ref_processor(), &_markBitMap, &_markStack,
             &_revisitStack, this, CMSYield);
  // CAUTION: This closure has persistent state that may need to
  // be reset upon a decrease in the sequence of addresses it
  // processes.
  ScanMarkedObjectsAgainCarefullyClosure
    smoac_cl(this, _span,
      &_markBitMap, &_markStack, &_revisitStack, &mrias_cl, CMSYield);

  // Preclean dirty cards in ModUnionTable and CardTable using
  // appropriate convergence criterion;
  // repeat CMSPrecleanIter times unless we find that
  // we are losing.
  assert(CMSPrecleanIter < 10, "CMSPrecleanIter is too large");
  assert(CMSPrecleanNumerator < CMSPrecleanDenominator,
         "Bad convergence multiplier");
  assert(CMSPrecleanThreshold >= 100,
         "Unreasonably low CMSPrecleanThreshold");

  size_t numIter, cumNumCards, lastNumCards, curNumCards;
  for (numIter = 0, cumNumCards = lastNumCards = curNumCards = 0;
       numIter < CMSPrecleanIter;
       numIter++, lastNumCards = curNumCards, cumNumCards += curNumCards) {
    smoac_cl.forget(); // clear persistent memory
    curNumCards  = precleanModUnionTable(&smoac_cl);
    if (Verbose && PrintGCDetails) {
      gclog_or_tty->print(" (modUnionTable: %d cards)", curNumCards);
    }
    // Either there are very few dirty cards, so re-mark
    // pause will be small anyway, or our pre-cleaning isn't
    // that much faster than the rate at which cards are being
    // dirtied, so we might as well stop and re-mark since
    // precleaning won't improve our re-mark time by much.
    if (curNumCards <= CMSPrecleanThreshold ||
        (numIter > 0 &&
         (curNumCards * CMSPrecleanDenominator >
         lastNumCards * CMSPrecleanNumerator))) {
      numIter++;
      cumNumCards += curNumCards;
      break;
    }
  }
  smoac_cl.forget(); // clear persistent memory
  curNumCards = precleanCardTable(_cmsGen, &smoac_cl);
  if (CMSPermGenPrecleaningEnabled) {
    smoac_cl.forget(); // clear persistent memory
    curNumCards += precleanCardTable(_permGen, &smoac_cl);
  }
  cumNumCards += curNumCards;
  if (PrintGCDetails && PrintCMSStatistics != 0) {
    gclog_or_tty->print_cr(" (cardTable: %d cards, re-scanned %d cards, %d iterations)",
		  curNumCards, cumNumCards, numIter);
  }
}

// PRECLEANING NOTES:
// Precleaning involves:
// . reading the bits of the modUnionTable and clearing the set bits.
//   This requires the modUnionTable lock.
// . For the cards corresponding to the set bits, we scan the
//   objects on those cards. This means we need the free_list_lock
//   so that we can safely iterate over the CMS space when scanning
//   for oops.
// . When we scan the objects, we'll be both reading and setting
//   marks in the marking bit map, so we'll need the marking bit map.
// . For protecting _collector_state transitions, we take the CMS_lock.
//   Note that any races in the reading of of card table entries by the
//   CMS thread on the one hand and the clearing of those entries by the
//   VM thread or the setting of those entries by the mutator threads on the
//   other are quite benign. However, for efficiency it makes sense to keep
//   the VM thread from racing with the CMS thread while the latter is
//   dirty card info to the modUnionTable. We therefore also use the
//   CMS_lock to protect the reading of the card table by the CM thread.
// . We run concurrently with mutator updates, so scanning
//   needs to be done carefully  -- we should not try to scan
//   potentially uninitialized objects.
//
// Locking strategy: We lock the modUnionTable, scan
// over and reset a maximal dirty range, release the lock,
// then lock free_list_lock and bitmap lock to do a full
// marking; and repeat. This allows for a certain amount of
// fairness in the sharing of these locks between the CMS
// collector on the one hand, and the VM thread and the
// mutators on the other. In the case of cardTable precleaning
// further below, we protect (as described above), the reading
// of the cardTable by means of the CMS_lock.

size_t CMSCollector::precleanModUnionTable(
  ScanMarkedObjectsAgainCarefullyClosure* cl) {
  // strategy: starting with the first card, accumulate contiguous
  // ranges of dirty cards; clear these cards, then scan the region
  // covered by these cards.

  // Since we commit the entire modUnionTable up front the size
  // won't change on the fly, so just read it here.
  HeapWord* endAddr = _modUnionTable.endWord();

  size_t numDirtyCards, cumNumDirtyCards;
  HeapWord *nextAddr, *lastAddr;
  for (cumNumDirtyCards = numDirtyCards = 0,
       nextAddr = lastAddr = _modUnionTable.startWord();
       nextAddr < endAddr;
       nextAddr = lastAddr, cumNumDirtyCards += numDirtyCards) {

    ResourceMark rm;
    HandleMark   hm;

    MemRegion dirtyRegion;
    {
      CMSTokenSyncWithLocks ts(true, modUnionTableLock());

      if (PrintGCDetails) {
        startTimer();
      }

      // Get dirty region starting at nextOffset (inclusive),
      // simultaneously clearing it.
      dirtyRegion = 
        _modUnionTable.getAndClearMarkedRegion(nextAddr);
      assert(dirtyRegion.start() >= nextAddr,
             "returned region inconsistent?");
      // Remember where the next search should begin.
      // The returned region (if non-empty) is a right open interval,
      // so lastOffset is obtained from the right end of that
      // interval.
      lastAddr = dirtyRegion.end();
      // Should do something more transparent and less hacky XXX
      numDirtyCards =
        _modUnionTable.heapWordDiffToOffsetDiff(dirtyRegion.word_size());
      if (PrintGCDetails) {
        stopTimer();
      }
    }

    // We can now release the _modUnionTable lock while
    // we scan the cards in the dirty region (with periodic
    // yields). Note that the release of the modUnionTable lock
    // ensures a form of fairness, because it keeps us from
    // monopolizing these three locks that are needed for a
    // foreground GC.

    if (!dirtyRegion.is_empty()) {
      assert(numDirtyCards > 0, "consistency check");
      if (!dirtyRegion.intersection(_cmsGen->reserved()).is_empty()) {
        CMSTokenSyncWithLocks ts(true, _cmsGen->freelistLock(),
                                 bitMapLock());
        assert(_markStack.isEmpty(), "should be empty");
        if (PrintGCDetails) {
          startTimer();
        }
        cl->setFreelistLock(_cmsGen->freelistLock());
        HeapWord* stop_point =
          _cmsGen->cmsSpace()->object_iterate_mem_careful(dirtyRegion, cl);
        assert(stop_point == NULL, "unparsable objects are in perm gen.");
        if (PrintGCDetails) {
          stopTimer();
        }
      }
      if (CMSPermGenPrecleaningEnabled &&
          !dirtyRegion.intersection(_permGen->reserved()).is_empty()) {
        CMSTokenSyncWithLocks ts(true, _permGen->freelistLock(),
                                 bitMapLock());
        assert(_markStack.isEmpty(), "should be empty");
        if (PrintGCDetails) {
          startTimer();
        }
        cl->setFreelistLock(_permGen->freelistLock());
        HeapWord* stop_point =
          _permGen->cmsSpace()->object_iterate_mem_careful(dirtyRegion, cl);
        if (stop_point != NULL) {
          // The careful iteration stopped early because it found an
          // uninitialized object.  Redirty the bits corresponding to the
          // partially-scanned or unscanned cards, and start again at the
          // next block boundary.
          assert(CMSPermGenPrecleaningEnabled, "unparsable objects are in perm gen.");
          MutexLockerEx mutl(modUnionTableLock(), Mutex::_no_safepoint_check_flag);
          _modUnionTable.markRange(MemRegion(stop_point, dirtyRegion.end()));
          lastAddr = next_card_start_after_block(stop_point);
        }
        if (PrintGCDetails) {
          stopTimer();
        }
      }
    } else {
      assert(lastAddr == endAddr, "consistency check");
      assert(numDirtyCards == 0, "consistency check");
      break;
    }
  }
  assert(_markStack.isEmpty(), "should be empty");
  return cumNumDirtyCards;
}

size_t CMSCollector::precleanCardTable(ConcurrentMarkSweepGeneration* gen,
  ScanMarkedObjectsAgainCarefullyClosure* cl) {
  // strategy: it's similar to precleamModUnionTable above, in that
  // we accumulate contiguous ranges of dirty cards, mark these cards
  // precleaned, then scan the region covered by these cards.
  HeapWord* endAddr   = (HeapWord*)(gen->_virtual_space.high());
  HeapWord* startAddr = (HeapWord*)(gen->_virtual_space.low());
  cl->setFreelistLock(gen->freelistLock());
  size_t numDirtyCards, cumNumDirtyCards;
  HeapWord *lastAddr, *nextAddr;

  for (cumNumDirtyCards = numDirtyCards = 0,
       nextAddr = lastAddr = startAddr;
       nextAddr < endAddr;
       nextAddr = lastAddr, cumNumDirtyCards += numDirtyCards) {

    ResourceMark rm;
    HandleMark   hm;

    MemRegion dirtyRegion;
    {
      // See comments in "Precleaning notes" above on why we
      // do this locking. XXX Could the locking overheads be
      // too high when dirty cards are sparse? [I don't think so.]
      CMSTokenSync x(true); // is cms thread

      if (PrintGCDetails) {
        startTimer();
      }

      // Get and clear dirty region from card table
      dirtyRegion = _ct->ct_bs()->dirty_card_range_after_preclean(
                                    MemRegion(nextAddr, endAddr));
      assert(dirtyRegion.start() >= nextAddr,
             "returned region inconsistent?");
    }
    lastAddr = dirtyRegion.end();
    numDirtyCards =
      dirtyRegion.word_size()/CardTableModRefBS::card_size_in_words;

    if (!dirtyRegion.is_empty()) {
      if (PrintGCDetails) {
        stopTimer();
      }
      CMSTokenSyncWithLocks ts(true, gen->freelistLock(), bitMapLock());
      assert(_markStack.isEmpty(), "should be empty");
      if (PrintGCDetails) {
        startTimer();
      }
      HeapWord* stop_point =
        gen->cmsSpace()->object_iterate_mem_careful(dirtyRegion, cl);
      if (stop_point != NULL) {
        // The careful iteration stopped early because it found an
        // uninitialized object.  Redirty the bits corresponding to the
        // partially-scanned or unscanned cards, and start again at the
        // next block boundary.
        assert(CMSPermGenPrecleaningEnabled, "unparsable objects are in perm gen.");
        _ct->ct_bs()->invalidate(MemRegion(stop_point, dirtyRegion.end()));
        lastAddr = next_card_start_after_block(stop_point);
      }
    } else {
      break;
    }
  }
  if (PrintGCDetails) {
    stopTimer();
  }
  assert(_markStack.isEmpty(), "should be empty");
  return cumNumDirtyCards;
}

void CMSCollector::checkpointRootsFinal(bool asynch,
  bool clear_all_soft_refs, bool init_mark_was_synchronous) {
  assert(_collectorState == FinalMarking, "incorrect state transition?");
  checkCorrectThreadExecuting();
  // world is stopped at this checkpoint
  assert(SafepointSynchronize::is_at_safepoint(),
         "world should be stopped");

  SpecializationStats::clear();
  if (asynch) {
    FreelistLocker x(this);
    MutexLockerEx y(bitMapLock(),
                    Mutex::_no_safepoint_check_flag);
    MutexLockerEx z(modUnionTableLock(),
                    Mutex::_no_safepoint_check_flag);
    assert(!init_mark_was_synchronous, "but that's impossible!");
    checkpointRootsFinalWork(asynch, clear_all_soft_refs, false);
  } else {
    // already have all the locks
    checkpointRootsFinalWork(asynch, clear_all_soft_refs,
                             init_mark_was_synchronous);
  }
  SpecializationStats::print();
}

void CMSCollector::checkpointRootsFinalWork(bool asynch,
  bool clear_all_soft_refs, bool init_mark_was_synchronous) {
  assert(haveFreelistLocks(), "must have free list locks");
  assert_lock_strong(bitMapLock());
  assert_lock_strong(modUnionTableLock());

  ResourceMark rm;
  HandleMark   hm;

  NOT_CORE(
    if (CMSClassUnloadingEnabled) {
      CodeCache::gc_prologue();
    }
  )
  assert(haveFreelistLocks(), "must have free list locks");
  assert_lock_strong(bitMapLock());
  assert_lock_strong(modUnionTableLock());

  if (!init_mark_was_synchronous) {
    GenCollectedHeap* gch = GenCollectedHeap::heap();
    gch->ensure_parseability();
    // Update the saved marks which may affect the root scans.
    gch->save_marks();
  
    {
      COMPILER2_ONLY(DerivedPointerTableDeactivate dpt_deact;)
  
      // Note on the role of the mod union table:
      // Since the marker in "markFromRoots" marks concurrently with
      // mutators, it is possible for some reachable objects not to have been
      // scanned. For instance, an only reference to an object A was
      // placed in object B after the marker scanned B. Unless B is rescanned,
      // A would be collected. Such updates to references in marked objects
      // are detected via the mod union table which is the set of all cards
      // dirtied since the first checkpoint in this GC cycle and prior to
      // the most recent young generation GC, minus those cleaned up by the
      // concurrent precleaning.
      if (CMSParallelRemarkEnabled && ParallelGCThreads > 0) {
        TraceTime t("Rescan (parallel) ", PrintGCDetails, false);
        do_remark_parallel();
      } else {
        TraceTime t("Rescan (non-parallel) ", PrintGCDetails, false);
        do_remark_non_parallel();
      }
    }
  } else {
    assert(!asynch, "Can't have init_mark_was_synchronous in asynch mode");
    // The initial mark was stop-world, so there's no rescanning to
    // do; go straight on to the next step below.
  }

  refProcessingWork(asynch, clear_all_soft_refs);
  NOT_CORE(
    if (CMSClassUnloadingEnabled) {
      CodeCache::gc_epilogue();
    }
  )
  // Change under the freelistLocks.
  _collectorState = Sweeping;
  // Call isAllClear() under bitMapLock
  assert(_modUnionTable.isAllClear(), "Should be clear by end of the"
    " final marking");
}

// Parallel remark task
class CMSParRemarkTask: public AbstractGangTask {
  CMSCollector* _collector;
  WorkGang*     _workers;
  int           _n_workers;
  CompactibleFreeListSpace* _cms_space;
  CompactibleFreeListSpace* _perm_space;

  // The per-thread work queues, available here for stealing.
  OopTaskQueueSet*       _task_queues;
  ParallelTaskTerminator _term;

 public:
  CMSParRemarkTask(CMSCollector* collector,
                   CompactibleFreeListSpace* cms_space,
                   CompactibleFreeListSpace* perm_space,
                   int n_workers, WorkGang* workers,
                   OopTaskQueueSet* task_queues):
    AbstractGangTask("Rescan roots and grey objects in parallel"),
    _collector(collector),
    _cms_space(cms_space), _perm_space(perm_space),
    _n_workers(n_workers),
    _workers(workers),
    _task_queues(task_queues),
    _term(workers->total_workers(), task_queues) { }

  OopTaskQueueSet* task_queues() { return _task_queues; }

  OopTaskQueue* work_queue(int i) { return task_queues()->queue(i); }

  ParallelTaskTerminator* terminator() { return &_term; }

  void work(int i);

 private:
  // A private work method in support of parallel rescan
  void do_rescan_tasks(CompactibleFreeListSpace* sp, int i,
                       Par_MarkRefsIntoAndScanClosure* cl);
  void do_work_steal(int i, Par_MarkRefsIntoAndScanClosure* cl, int* seed);
};

void CMSParRemarkTask::work(int i) {
  elapsedTimer _timer;
  ResourceMark rm;
  HandleMark   hm;

  // ---------- rescan from roots --------------
  _timer.start();
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  Par_MarkRefsIntoAndScanClosure par_mrias_cl(_collector->_span,
    _collector->ref_processor(), &(_collector->_markBitMap),
    work_queue(i), &(_collector->_revisitStack));
  gch->process_strong_roots(_collector->_cmsGen->level(),
                            true,  // younger gens as roots
                            true,  // collecting perm gen
                            CMSClassUnloadingEnabled?
                              GenCollectedHeap::CSO_SystemClasses:
                              GenCollectedHeap::CSO_AllClasses,
                            NULL, &par_mrias_cl);
  _timer.stop();
  if (PrintGC && Verbose) {
    gclog_or_tty->print_cr(
      "Finished root rescan work in %dth thread: %3.3f sec",
      i, _timer.seconds());
  }

  // ---------- rescan dirty cards ------------
  _timer.reset();
  _timer.start();

  // Do the rescan tasks for each of the two spaces
  // (cms_space and perm_space) in turn.
  do_rescan_tasks(_cms_space, i, &par_mrias_cl);
  do_rescan_tasks(_perm_space, i, &par_mrias_cl);
  _timer.stop();
  if (PrintGC && Verbose) {
    gclog_or_tty->print_cr(
      "Finished dirty card rescan work in %dth thread: %3.3f sec",
      i, _timer.seconds());
  }

  // ---------- steal work from other threads -------
  _timer.reset();
  _timer.start();
  do_work_steal(i, &par_mrias_cl, _collector->hash_seed(i));
  _timer.stop();
  if (PrintGC && Verbose) {
    gclog_or_tty->print_cr(
      "Finished work stealing in %dth thread: %3.3f sec",
      i, _timer.seconds());
  }
}

void
CMSParRemarkTask::do_rescan_tasks(CompactibleFreeListSpace* sp, int i,
                                  Par_MarkRefsIntoAndScanClosure* cl) {
  // Until all tasks completed:
  // . claim an unclaimed task
  // . compute region boundaries corresponding to task claimed
  // . transfer dirty bits ct->mut for that region
  // . apply rescanclosure to dirty mut bits for that region

  ResourceMark rm;
  HandleMark   hm;

  OopTaskQueue* work_q = work_queue(i);
  ModUnionClosure modUnionClosure(&(_collector->_modUnionTable));
  // CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION! CAUTION!
  // CAUTION: This closure has state that persists across calls to
  // the work method dirty_range_iterate_clear() in that it has
  // imbedded in it a (subtype of) UpwardsObjectClosure. The
  // use of that state in the imbedded UpwardsObjectClosure instance
  // assumes that the cards are always iterated (even if in parallel
  // by several threads) in monotonically increasing order per each
  // thread. This is true of the implementation below which picks
  // card ranges (chunks) in monotonically increasing order globally
  // and, a-fortiori, in monotonically increasing order per thread
  // (the latter order being a subsequence of the former).
  // If the work code below is ever reorganized into a more chaotic
  // work-partitioning form than the current "sequential tasks"
  // paradigm, the use of that persistent state will have to be
  // revisited and modified appropriately. See also related
  // bug 4756801 work on which should examine this code to make
  // sure that the changes there do not run counter to the
  // assumptions made here and necessary for correctness and
  // efficiency. Note also that this code might yield inefficient
  // behaviour in the case of very large objects that span one or
  // more work chunks. Such objects would potentially be scanned 
  // several times redundantly. Work on 4756801 should try and
  // address that performance anomaly if at all possible. XXX
  MemRegion  full_span  = _collector->_span;
  CMSBitMap* bm    = &(_collector->_markBitMap);     // shared
  CMSMarkStack* rs = &(_collector->_revisitStack);   // shared
  MarkFromDirtyCardsClosure
    greyRescanClosure(_collector, full_span, // entire span of interest
                      sp, bm, work_q, rs, cl);

  SequentialSubTasksDone* pst = sp->par_seq_tasks();
  int nth_task = 0;
  const int alignment = CardTableModRefBS::card_size * BitsPerWord;
  MemRegion span = sp->used_region();
  HeapWord* start_addr = span.start();
  HeapWord* end_addr = (HeapWord*)align_size_up((intptr_t)span.end(),
                                                alignment);
  const size_t chunk_size = sp->rescan_task_size(); // in HeapWord units
  assert((HeapWord*)align_size_up((intptr_t)start_addr, alignment) ==
         start_addr, "Check alignment");
  assert((size_t)align_size_up((intptr_t)chunk_size, alignment) ==
         chunk_size, "Check alignment");

  while (!pst->is_task_claimed(/* reference */ nth_task)) {
    // Having claimed the nth_task, compute corresponding mem-region,
    // which is a-fortiori aligned correctly (i.e. at a MUT bopundary).
    // The alignment restriction ensures that we do not need any
    // synchronization with other gang-workers while setting or
    // clearing bits in thus chunk of the MUT.
    MemRegion this_span = MemRegion(start_addr + nth_task*chunk_size,
                                    start_addr + (nth_task+1)*chunk_size);
    // The last chunk's end might be way beyond end of the
    // used region. In that case pull back appropriately.
    if (this_span.end() > end_addr) {
      this_span.set_end(end_addr);
      assert(!this_span.is_empty(), "Program logic (calculation of n_tasks)");
    }
    // Iterate over the dirty cards covering this chunk, marking them
    // precleaned, and setting the corresponding bits in the mod union
    // table. Since we have been careful to partition at Card and MUT-word
    // boundaries no synchronization is needed between parallel threads.
    _collector->_ct->ct_bs()->dirty_card_iterate(this_span,
                                                 &modUnionClosure);

    // Having carried these marks into the modUnionTable,
    // rescan the marked objects on the dirty cards in the modUnionTable.
    // Even if this is at a synchronous collection, the initial marking
    // may have been done during an asynchronous collection so there
    // may be dirty bits in the mod-union table.
    _collector->_modUnionTable.dirty_range_iterate_clear(
                  this_span, &greyRescanClosure);
    _collector->_modUnionTable.verifyNoOneBitsInRange(
                                 this_span.start(),
                                 this_span.end());
  }
  pst->all_tasks_completed();  // declare that i am done
}

// . see if we can share work_queues with youn gen stuff ?
void
CMSParRemarkTask::do_work_steal(int i, Par_MarkRefsIntoAndScanClosure* cl,
                                int* seed) {
  OopTaskQueue* work_q = work_queue(i);
  NOT_PRODUCT(int num_steals = 0;)
  oop obj_to_scan;
  CMSBitMap* bm = &(_collector->_markBitMap);

  while (true) {
    // Completely finish any left over work from (an) earlier round(s)
    cl->trim_queue(0);
    // Verify that we have no work before we resort to stealing
    assert(work_q->size() == 0, "Have work, shouldn't steal");
    // Try to steal from other queues that have work
    if (task_queues()->steal(i, seed, /* reference */ obj_to_scan)) {
      NOT_PRODUCT(num_steals++;)
      assert(obj_to_scan->is_oop(), "Oops, not an oop!");
      assert(bm->isMarked((HeapWord*)obj_to_scan), "Stole an unmarked oop?");
      // Do scanning work
      obj_to_scan->oop_iterate(cl);
      // Loop around, finish this work, and try to steal some more
    } else if (terminator()->offer_termination()) {
        break;  // nirvana from the infinite cycle
    }
  }
  NOT_PRODUCT(
    if (PrintCMSStatistics != 0) {
      gclog_or_tty->print("\n\t(%d: stole %d oops)", i, num_steals);
    }
  )
}

// Parallel version of remark
void CMSCollector::do_remark_parallel() {
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  WorkGang* workers = gch->workers();
  assert(workers != NULL, "Need parallel worker threads.");
  int n_workers = workers->total_workers();
  CompactibleFreeListSpace* cms_space  = _cmsGen->cmsSpace();
  CompactibleFreeListSpace* perm_space = _permGen->cmsSpace();

  CMSParRemarkTask tsk(this,
    cms_space, perm_space,
    n_workers, workers, task_queues());

  gch->set_par_threads(n_workers);
  gch->change_strong_roots_parity();
  // We won't be iterating over the cards in the card table updating
  // the younger_gen cards, so we shouldn't call the following else
  // the verification code as well as subsequent younger_refs_iterate
  // code would get confused. XXX
  // gch->rem_set()->prepare_for_younger_refs_iterate(true); // parallel

  // The dirty card rescan work is broken up into a "sequence"
  // of parallel tasks (per constituent space) that are dynamically
  // claimed by the parallel threads.
  cms_space->initialize_sequential_subtasks_for_rescan(n_workers);
  perm_space->initialize_sequential_subtasks_for_rescan(n_workers);
  
  // It turns out that even when we're using 1 thread, doing the work in a
  // separate thread causes wide variance in run times.  We can't help this
  // in the multi-threaded case, but we special-case n=1 here to get
  // repeatable measurements of the 1-thread overhead of the parallel code.
  if (n_workers > 1) {
    // Make refs discovery MT-safe
    ReferenceProcessorMTMutator mt(ref_processor(), true);
    workers->run_task(&tsk);
  } else {
    tsk.work(0);
  }
  gch->set_par_threads(0);  // 0 ==> non-parallel.
}

// Non-parallel version of remark
void CMSCollector::do_remark_non_parallel() {
  ResourceMark rm;
  HandleMark   hm;
  GenCollectedHeap* gch = GenCollectedHeap::heap();
  MarkRefsIntoAndScanClosure
    mrias_cl(_span, ref_processor(), &_markBitMap,
             &_markStack, &_revisitStack, this, false /* should_yield */);
  MarkFromDirtyCardsClosure
    markFromDirtyCardsClosure(this, _span,
                              NULL,  // space is set further below
                              &_markBitMap, &_markStack, &_revisitStack,
                              &mrias_cl);
  {
    TraceTime t("grey object rescan", PrintGCDetails, false, gclog_or_tty);
    // Iterate over the dirty cards, marking them precleaned, and
    // setting the corresponding bits in the mod union table.
    // Need modUnionTable lock for this iteration.
    {
      ModUnionClosure modUnionClosure(&_modUnionTable);
      _ct->ct_bs()->dirty_card_iterate(
                      _cmsGen->used_region(),
                      &modUnionClosure);
      _ct->ct_bs()->dirty_card_iterate(
                      _permGen->used_region(),
                      &modUnionClosure);
    }
    // Having carried these marks into the modUnionTable, we just need
    // to rescan the marked objects on the dirty cards in the modUnionTable.
    // The initial marking may have been done during an asynchronous
    // collection so there may be dirty bits in the mod-union table.
    const int alignment =
      CardTableModRefBS::card_size * BitsPerWord;
    { 
      // ... First handle dirty cards in CMS gen
      markFromDirtyCardsClosure.set_space(_cmsGen->cmsSpace());
      MemRegion ur = _cmsGen->used_region();
      HeapWord* lb = ur.start();
      HeapWord* ub = (HeapWord*)align_size_up((intptr_t)ur.end(),
                                              alignment);
      MemRegion cms_span(lb, ub);
      _modUnionTable.dirty_range_iterate_clear(cms_span,
                                               &markFromDirtyCardsClosure);
      assert(_markStack.isEmpty(), "mark stack should be empty");
      if (PrintCMSStatistics != 0) {
        gclog_or_tty->print(" (re-scanned %d dirty cards in cms gen) ",
          markFromDirtyCardsClosure.num_dirty_cards());
      }
    } 
    {
      // .. and then repeat for dirty cards in perm gen
      markFromDirtyCardsClosure.set_space(_permGen->cmsSpace());
      MemRegion ur = _permGen->used_region();
      HeapWord* lb = ur.start();
      HeapWord* ub = (HeapWord*)align_size_up((intptr_t)ur.end(),
                                              alignment);
      MemRegion perm_span(lb, ub);
      _modUnionTable.dirty_range_iterate_clear(perm_span,
                                               &markFromDirtyCardsClosure);
      assert(_markStack.isEmpty(), "mark stack should be empty");
      if (PrintCMSStatistics != 0) {
        gclog_or_tty->print(" (re-scanned %d dirty cards in perm gen) ",
          markFromDirtyCardsClosure.num_dirty_cards());
      }
    }
  }
  if (VerifyDuringGC &&
      GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
    Universe::verify(true);
  }
  {
    TraceTime t("root rescan", PrintGCDetails, false, gclog_or_tty);

    assert(_markStack.isEmpty(), "should be empty");
  
    gch->rem_set()->prepare_for_younger_refs_iterate(false); // Not parallel.
    gch->process_strong_roots(_cmsGen->level(),
                              true,  // younger gens as roots
                              true,  // collecting perm gen
                              CMSClassUnloadingEnabled ?
                                GenCollectedHeap::CSO_SystemClasses :
                                GenCollectedHeap::CSO_AllClasses,
                              NULL, &mrias_cl);
  }
  assert(_markStack.isEmpty(), "should be empty");
}

void CMSCollector::refProcessingWork(bool asynch,
  bool clear_all_soft_refs) {

  ResourceMark rm;
  HandleMark   hm;
  ReferencePolicy* soft_ref_policy;

  assert(!ref_processor()->enqueuing_is_done(), "Enqueuing should not be complete");
  // Process weak references.
  if (clear_all_soft_refs) {
    soft_ref_policy = new AlwaysClearPolicy();
  } else {
    NOT_COMPILER2(soft_ref_policy = new LRUCurrentHeapPolicy();)
    COMPILER2_ONLY(soft_ref_policy = new LRUMaxHeapPolicy();)
  }
  assert(_markStack.isEmpty(), "mark stack should be empty");
  CMSIsAliveClosure cmsIsAliveClosure(_span, &_markBitMap);
  CMSKeepAliveClosure cmsKeepAliveClosure(_span, &_markBitMap, &_markStack);
  CMSDrainMarkingStackClosure cmsDrainMarkingStackClosure(_span,
                                &_markBitMap, &_markStack,
                                &cmsKeepAliveClosure);
  
  ReferenceProcessor* rp = ref_processor();
  assert(rp->span().equals(_span), "Spans should be equal");
  {
    TraceTime t("weak refs processing", PrintGCDetails, false, gclog_or_tty);
    rp->process_discovered_references(soft_ref_policy,
                                      &cmsIsAliveClosure,
                                      &cmsKeepAliveClosure,
                                      &cmsDrainMarkingStackClosure);
    assert(_markStack.isEmpty(), "mark stack should be empty");
  }

  if (CMSClassUnloadingEnabled) {
    TraceTime t("class unloading", PrintGCDetails, false, gclog_or_tty);
    // Now follow SystemDictionary roots and unload classes
    bool purged_class = SystemDictionary::
                          do_unloading_for_CMS(&cmsIsAliveClosure,
                                               &cmsKeepAliveClosure);
    assert(_markStack.isEmpty(), "mark stack should be empty");

    NOT_CORE(
      bool junk = false;
      // Follow CodeCache roots and unload any methods marked for unloading
      CodeCache::do_unloading_for_CMS(&cmsIsAliveClosure,
                                             &cmsKeepAliveClosure,
                                             purged_class, junk);
    )

    cmsDrainMarkingStackClosure.do_void();
    assert(_markStack.isEmpty(), "mark stack should be empty");
  
    // Update subklass/sibling/implementor links in KlassKlass descendants
    assert(!_revisitStack.isEmpty(), "revisit stack should not be empty");
    oop k;
    while ((k = _revisitStack.pop()) != NULL) {
      ((Klass*)k)->follow_weak_klass_links_for_CMS(
                     &cmsIsAliveClosure, &cmsKeepAliveClosure);
    }
    assert(_revisitStack.isEmpty(), "revisit stack should have been drained");
    if (!_markStack.isEmpty()) {
      cmsDrainMarkingStackClosure.do_void();
    }
  }
  assert(_revisitStack.isEmpty(), "revisit stack should have been drained");
  rp->set_enqueuing_is_done(true);
  assert(_markStack.isEmpty(), "mark stack should be empty");

  if (CMSClassUnloadingEnabled) {
    TraceTime t("scrub symbol & string tables", PrintGCDetails, false, gclog_or_tty);
    // Now clean up stale oops in SymbolTable and StringTable
    SymbolTable::unlink_for_cms(&cmsIsAliveClosure);
    StringTable::unlink_for_cms(&cmsIsAliveClosure);
  }

  assert(_markStack.isEmpty(), "mark stack should be empty");

  assert(rp->notify_ref_lock() == false,
         "Should have been reset in previous cycle");
  rp->set_notify_ref_lock(rp->enqueue_discovered_references());
  rp->verify_no_references_recorded();
  assert(!rp->discovery_enabled(), "should have been disabled");
}

#ifndef PRODUCT
void CMSCollector::checkCorrectThreadExecuting() {
  // Only the VM thread or the CMS thread should be here.
  assert(Thread::current()->is_ConcurrentMarkSweep_thread() ||
    Thread::current()->is_VM_thread(), "Wrong thread is executing");
  // If this is the vm thread, the foreground process 
  // should not be waiting.  Note that _foregroundGCIsActive is 
  // true while the foreground collector is waiting.
  assert((!Thread::current()->is_VM_thread() && _foregroundGCShouldWait) || 
    (Thread::current()->is_VM_thread() && !_foregroundGCShouldWait), 
    "Wrong thread is executing");
}
#endif

void CMSCollector::sweep(bool asynch) {
  assert(_collectorState == Sweeping, "just checking");
  checkCorrectThreadExecuting();
  incrementSweepCount();
  if (asynch) {
    CMSPhaseAccounting pa(this, "sweep");
    // First sweep the old gen then the perm gen
    {
      CMSTokenSyncWithLocks ts(true, _cmsGen->freelistLock(),
                               bitMapLock());
      sweepWork(_cmsGen, asynch);
    }
    // Now repeat for perm gen
    if (CMSClassUnloadingEnabled && CMSPermGenSweepingEnabled) {
      CMSTokenSyncWithLocks ts(true, _permGen->freelistLock(),
                             bitMapLock());
      sweepWork(_permGen, asynch);
    }
    { // We need all the free list locks to make the abstract state
      // transition from Sweeping to Resetting. See detailed note
      // further below.
      FreelistLocker x(this);
      _collectorState = Resetting;
    }
  } else {
    // already have needed locks
    sweepWork(_cmsGen,  asynch);
    if (CMSClassUnloadingEnabled && CMSPermGenSweepingEnabled) {
      sweepWork(_permGen, asynch);
    }
    _collectorState = Resetting;
  }

  // NOTE on abstract state transitions:
  // Mutators allocate-live and/or mark the mod-union table dirty
  // based on the state of the collection.  The former is done in
  // the interval [Marking, Sweeping] and the latter in the interval
  // [Marking, Sweeping).  Thus the transition into the Marking state
  // and out of the Sweeping state must be synchronously visible 
  // globally to the mutators.
  // The transition into the Marking state happens with the world
  // stopped so the mutators will globally see it.  Sweeping is
  // done asynchronously by the background collector so the transition
  // from the Sweeping state to the Resetting state must be done
  // under the freelistLock (as is the check for whether to 
  // allocate-live and whether to dirty the mod-union table).
  assert(_collectorState == Resetting, "Change of collector state to"
    " Resetting must be done under the freelistLocks (plural)");

  // Now that sweeping has been completed, if the GCH's
  // incremental_collection_will_fail flag is set, clear it,
  // thus inviting a younger gen collection to promote into
  // this generation. If such a promotion may still fail,
  // the flag will be set again when a young collection is
  // attempted.
  // I think the incremental_collection_will_fail flag's use
  // is specific to a 2 generation collection policy, so i'll
  // assert that that's the configuration we are operating within.
  // The use of the flag can and should be generalized appropriately
  // in the future to deal with a general n-generation system.

  GenCollectedHeap* gch = GenCollectedHeap::heap();
  assert(gch->collector_policy()->is_two_generation_policy(),
         "Resetting of incremental_collection_will_fail flag"
         " may be incorrect otherwise");
  gch->clear_incremental_collection_will_fail();
}

// FIX ME!!! Looks like this belongs in CFLSpace, with
// CMSGen merely delegating to it.
void ConcurrentMarkSweepGeneration::setNearLargestChunk() {
  double nearLargestPercent = 0.999;
  HeapWord*  minAddr        = _cmsSpace->bottom();
  HeapWord*  largestAddr    = 
    (HeapWord*) _cmsSpace->dictionary()->findLargestDict();
  if (largestAddr == 0) {
    // The dictionary appears to be empty.  In this case 
    // try to coalesce at the end of the heap.
    largestAddr = _cmsSpace->end();
  }
  size_t largestOffset     = pointer_delta(largestAddr, minAddr);
  size_t nearLargestOffset =
    (size_t)((double)largestOffset * nearLargestPercent) - MinChunkSize;
  _cmsSpace->set_nearLargestChunk(minAddr + nearLargestOffset);
}

bool ConcurrentMarkSweepGeneration::isNearLargestChunk(HeapWord* addr) {
  return addr >= _cmsSpace->nearLargestChunk();
}

void CMSCollector::sweepWork(ConcurrentMarkSweepGeneration* gen,
  bool asynch) {
  // We iterate over the space(s) underlying this generation,
  // checking the mark bit map to see if the bits corresponding
  // to specific blocks are marked or not. Blocks that are
  // marked are live and are not swept up. All remaining blocks
  // are swept up, with coalescing on-the-fly as we sweep up
  // contiguous free and/or garbage blocks:
  // We need to ensure that the sweeper synchronizes with allocators
  // and stop-the-world collectors. In particular, the following
  // locks are used:
  // . CMS token: if this is held, a stop the world collection cannot occur
  // . freelistLock: if this is held no allocation can occur from this
  //                 generation by another thread
  // . bitMapLock: if this is held, no other thread can access or update
  //
    
  // Note that we need to hold the freelistLock if we use
  // block iterate below; else the iterator might go awry if
  // a mutator (or promotion) causes block contents to change
  // (for instance if the allocator divvies up a block).
  // If we hold the free list lock, for all practical purposes
  // young generation GC's can't occur (they'll usually need to
  // promote), so we might as well prevent all young generation
  // GC's while we do a sweeping step. For the same reason, we might
  // as well take the bit map lock for the entire duration
  
  // check that we hold the requisite locks
  assert(   (asynch && ConcurrentMarkSweepThread::cms_thread_has_cms_token())
         || (!asynch && ConcurrentMarkSweepThread::vm_thread_has_cms_token()),
        "Should possess CMS token to sweep");
  assert_lock_strong(gen->freelistLock());
  assert_lock_strong(bitMapLock());

  gen->cmsSpace()->beginSweepFLCensus();
  incrementSweepCount();
  gen->setNearLargestChunk();

  {
    SweepClosure sweepClosure(this, gen, &_markBitMap,
                            CMSYield && asynch);
    gen->cmsSpace()->blk_iterate_careful(&sweepClosure);
    // We need to free-up/coalesce garbage/blocks from a
    // co-terminal free run. This is done in the SweepClosure
    // destructor; so, do not remove this scope, else the
    // end-of-sweep-census below will be off by a little bit.
  }
  gen->cmsSpace()->endSweepFLCensus(sweepCount());

  // Consider changing the size of the generation after the 
  // sweep so that the amount of free space is accurate.
  gen->compute_new_size();

  update_time_of_last_gc(os::javaTimeMillis());
}

// Reset CMS data structures (for now just the marking bit map)
// preparatory for the next cycle.
void CMSCollector::reset(bool asynch) {
  if (asynch) {
    CMSTokenSyncWithLocks ts(true, bitMapLock());

    // If the state is not "Resetting", the foreground  thread
    // has done a collection and the resetting.
    if (_collectorState != Resetting) {
      assert(_collectorState == Idling, "The state should only change"
	" because the foreground collector has finished the collection");
      return;
    }

    // Clear the mark bitmap (no grey objects to start with)
    // for the next cycle.
    CMSPhaseAccounting cmspa(this, "reset");

    HeapWord* curAddr = _markBitMap.startWord();
    while (curAddr < _markBitMap.endWord()) {
      size_t remaining  = pointer_delta(_markBitMap.endWord(), curAddr); 
      MemRegion chunk(curAddr, MIN2(CMSBitMapYieldQuantum, remaining));
      _markBitMap.clearRange(chunk);
      if (ConcurrentMarkSweepThread::should_yield() &&
          !foregroundGCIsActive() &&
          CMSYield) {
        assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
               "CMS thread should hold CMS token");
        assert_lock_strong(bitMapLock());
        bitMapLock()->unlock();
        ConcurrentMarkSweepThread::desynchronize(true);
        ConcurrentMarkSweepThread::acknowledge_yield_request();
        stopTimer();
        if (PrintCMSStatistics != 0) {
          incrementYields();
        }
        icms_wait();
        ConcurrentMarkSweepThread::synchronize(true);
        bitMapLock()->lock_without_safepoint_check();
        startTimer();
      }
      curAddr = chunk.end();
    }
    _collectorState = Idling;
  } else {
    // already have the lock
    assert(_collectorState == Resetting, "just checking");
    assert_lock_strong(bitMapLock());
    _markBitMap.clearAll();
    _collectorState = Idling;
  }

  // Stop incremental mode after a cycle completes, so that any future cycles
  // are triggered by allocation.
  stop_icms();
}

void CMSCollector::doCMSOperation(CMS_op_type op) {
  TraceTime t("GC", PrintGC, true, gclog_or_tty);
  TraceCollectorStats tcs(counters());

  switch (op) {
    case CMS_op_checkpointRootsInitial: {
      checkpointRootsInitial(true);       // asynch
      if (PrintGC) {
        _cmsGen->printOccupancy("CMS-initial-mark");
      }
      break;
    }
    case CMS_op_checkpointRootsFinal: {
      checkpointRootsFinal(true,   // asynch
                          false,   // !clear_all_soft_refs
                          false);  // !init_mark_was_synchronous
      if (PrintGC) {
        _cmsGen->printOccupancy("CMS-remark");
      }
      break;
    }
    default:
      guarantee(false, "No such CMS_op");
  }
}


bool CMSCollector::stopWorldAndDo(CMS_op_type op) {
  assert(Thread::current()->is_ConcurrentMarkSweep_thread(), "Only the CMS "
    "thread should be executing here");
  // The foreground collector should not be blocked on phases
  // of the background collection that are done with 
  // the world stopped because they will block on the Heap_lock 
  // (which the foreground collector already holds).
  // Execution can reach this pointer with
  // the foreground collector waiting. Release the
  // foreground collector and use the Heap_lock to
  // synchronized collections with the world stopped.

  // If the foreground collector has acquired the Heap_lock
  // and is waiting in acquire_control_and_collect(), this
  // will release the foreground collector.  The background
  // collector will block on the Heap_lock.
  // If the foreground collector has not acquired the Heap_lock,
  // the foreground collector and background collector will race
  // for the Heap_lock and the winner will proceed with the
  // collection.  
  //   If the background collector wins, on exit from this method
  //   the background collector will release the Heap_lock and
  //   the foreground collector and the background collector will
  //   race for control of the remainder of the collection.  Set
  //   _foregroundGCShouldWait after getting the Heap_lock
  //   and rely on the code in collect_in_background.
  //   If the foreground collector wins, the background collector
  //   will wait on the Heap_lock until the foreground collector
  //   is done. The background collector should skip this phase
  //   and return.  If another foreground collection starts 
  //   and is waiting at the Heap_lock, it will be stopped by
  //   _foregroundGCShouldWait until the background collector
  //   yields control (the code in collect_in_background). 

  {
    MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
    // allow a potentially blocked foreground collector to proceed
    _foregroundGCShouldWait = false;
    if (_foregroundGCIsActive) {
      CMS_lock->notify();
    }
    assert(!ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
           "Possible deadlock");
  }
  // We are potentially in a race with a foregorund collector for the Heap_lock
  MutexLockerEx z(Heap_lock,
	  Mutex::_no_safepoint_check_flag);
  // If we lost the race against the foreground collector, our _collectorState
  // would have been reset.
  {
     MutexLockerEx x(CMS_lock, Mutex::_no_safepoint_check_flag);
     // For regularity, return from this function with _foregroundGCShouldWait
     // set.  The foreground collector may attempt a new collection by
     // the time this function returns and it is better to have the
     // foreground collector wait until the background collector is 
     // exitting before the foreground collector is allowed to proceed.
     _foregroundGCShouldWait = true;
     if (_collectorState == Idling) {
       // we were beaten by the foreground collector; we can return
       assert(!ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
              "Possible deadlock");
       return false;  // false return indicates nothing was done
     }
     // else we didn't lose a race to FG thread
  }

  if (PrintGCApplicationConcurrentTime) {
     double app_time =  VMThread::stop_and_get_application_timer();
     gclog_or_tty->print_cr("Application time: %3.7f seconds", app_time);
  }
  if (PrintGCApplicationStoppedTime) {
     VMThread::restart_world_stopped_timer();
  }

  // warning("CMS: about to try stopping world");
  SafepointSynchronize::begin();
  // warning("CMS: successfully stopped world");
  if (VerifyBeforeGC &&
      GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
    FreelistLocker x(this);
    MutexLockerEx  y(bitMapLock(), Mutex::_no_safepoint_check_flag);
    Universe::heap()->prepare_for_verify();
    Universe::verify(true);
  }
  doCMSOperation(op);
  if (VerifyAfterGC &&
      GenCollectedHeap::heap()->total_collections() >= VerifyGCStartAt) {
    FreelistLocker x(this);
    MutexLockerEx  y(bitMapLock(), Mutex::_no_safepoint_check_flag);
    Universe::verify(true);
  }
  SafepointSynchronize::end();
  // warning("CMS: successfully restarted world");

  if (PrintGCApplicationStoppedTime) {
    double world_stopped_time =  VMThread::stop_and_get_world_stopped_timer();
    gclog_or_tty->print_cr("Total time for which application threads "
      "were stopped: %3.7f seconds", world_stopped_time);
  }
  if (PrintGCApplicationConcurrentTime) {
    VMThread::restart_application_timer();
  }
  return true;
}

#ifndef PRODUCT
size_t const CMSCollector::skip_header_HeapWords() {
  return FreeChunk::header_size();
}

// Try and collect here conditions that should hold when
// CMS thread is exiting. The idea is that the foreground GC
// thread should not be blocked if it wants to terminate
// the CMS thread and yet continue to run the VM for a while
// after that.
void CMSCollector::verify_ok_to_terminate() const {
  assert(Thread::current()->is_ConcurrentMarkSweep_thread(), 
         "should be called by CMS thread");
  assert(!_foregroundGCShouldWait, "should be false");
  // We could check here that all the various low-level locks
  // are not held by the CMS thread, but that is overkill; see
  // also CMSThread::verify_ok_to_terminate() where the CMS_lock
  // is checked.
}
#endif

size_t CMSCollector::block_size_using_printezis_bits(HeapWord* addr) const {
  assert(_markBitMap.isMarked(addr) && _markBitMap.isMarked(addr + 1),
         "missing Printezis mark?");
  HeapWord* nextOneAddr = _markBitMap.getNextMarkedWordAddress(addr + 2);
  size_t size = pointer_delta(nextOneAddr + 1, addr);
  assert(size == CompactibleFreeListSpace::adjustObjectSize(size),
         "alignment problem");
  assert(size >= 3, "Necessary for Printezis marks to work");
  return size;
}

HeapWord* CMSCollector::next_card_start_after_block(HeapWord* addr) const {
  size_t sz = 0;
  oop p = (oop)addr;
  if (p->klass() != NULL && p->is_parsable()) {
    sz = CompactibleFreeListSpace::adjustObjectSize(p->size());
  } else {
    sz = block_size_using_printezis_bits(addr);
  }
  assert(sz > 0, "size must be nonzero");
  HeapWord* next_block = addr + sz;
  HeapWord* next_card  = (HeapWord*)round_to((uintptr_t)next_block,
                                             CardTableModRefBS::card_size);
  assert(round_down((uintptr_t)addr,      CardTableModRefBS::card_size) <
         round_down((uintptr_t)next_card, CardTableModRefBS::card_size),
         "must be different cards");
  return next_card;
}


// CMS Bit Map Wrapper /////////////////////////////////////////

// Construct a CMS bit map infrastructure, but don't create the 
// bit vector itself. That is done by a separate call CMSBitMap::allocate()
// further below.
CMSBitMap::CMSBitMap(int shifter, int mutexRank, const char* mutexName):
  _bm(NULL,0),
  _shifter(shifter),
  _lock(mutexRank, mutexName, true) {
  _bmStartWord = 0;
  _bmWordSize  = 0;
}

void CMSBitMap::allocate(MemRegion mr) {
  _bmStartWord = mr.start();
  _bmWordSize  = mr.word_size();
  ReservedSpace brs(ReservedSpace::page_align_size_up(
                     (_bmWordSize >> (_shifter + LogBitsPerByte)) + 1));
  if (!brs.is_reserved()) {
    vm_exit_during_initialization("CMS bit map allocation failure");
  }
  // For now we'll just commit all of the bit map up fromt.
  // Later on we'll try to be more parsimonious with swap.
  if (!_virtual_space.initialize(brs, brs.size())) {
    vm_exit_during_initialization("CMS bit map backing store failure");
  }
  assert(_virtual_space.committed_size() == brs.size(),
         "didn't reserve backing store for all of CMS bit map?");
  _bm.set_map((uintptr_t*)_virtual_space.low());
  assert(_virtual_space.committed_size() << (_shifter + LogBitsPerByte) >=
         _bmWordSize, "inconsistency in bit map sizing");
  _bm.set_size(_bmWordSize >> _shifter);

  // bm.clear(); // can we rely on getting zero'd memory? verify below
  {
    MutexLockerEx x(lock(), Mutex::_no_safepoint_check_flag);
    assert(isAllClear(),
         "Expected zero'd memory from ReservedSpace constructor");
  }
  assert(_bm.size() == heapWordDiffToOffsetDiff(sizeInWords()),
         "consistency check");
}

void CMSBitMap::dirty_range_iterate_clear(MemRegion mr, MemRegionClosure* cl) {
  HeapWord *next_addr, *end_addr, *last_addr;
  assert_locked();
  assert(covers(mr), "out-of-range error");
  // XXX assert that start and end are appropriately aligned
  for (next_addr = mr.start(), end_addr = mr.end();
       next_addr < end_addr; next_addr = last_addr) {
    MemRegion dirty_region = getAndClearMarkedRegion(next_addr, end_addr);
    last_addr = dirty_region.end();
    if (!dirty_region.is_empty()) {
      cl->do_MemRegion(dirty_region);
    } else {
      assert(last_addr == end_addr, "program logic");
      return;
    }
  }
}

#ifndef PRODUCT
void CMSBitMap::assert_locked() const {
  CMSLockVerifier::assert_locked(lock());
}

bool CMSBitMap::covers(MemRegion mr) const {
  // assert(_bm.map() == _virtual_space.low(), "map inconsistency");
  assert((size_t)_bm.size() == (_bmWordSize >> _shifter),
         "size inconsistency");
  return (mr.start() >= _bmStartWord) &&
         (mr.end()   <= endWord());
}

bool CMSBitMap::covers(HeapWord* start, size_t size) const {
    return (start >= _bmStartWord && (start + size) <= endWord());
}

void CMSBitMap::verifyNoOneBitsInRange(HeapWord* left, HeapWord* right) {
  // verify that there are no 1 bits in the interval [left, right)
  FalseBitMapClosure falseBitMapClosure;
  iterate(&falseBitMapClosure, left, right);
}
#endif

void CMSMarkStack::allocate(size_t size) {
  // allocate a stack of the requisite depth
  ReservedSpace rs(ReservedSpace::page_align_size_up(
                   size * sizeof(oop)));
  if (!rs.is_reserved()) {
    vm_exit_during_initialization("CMSMarkStack allocation failure");
  }
  if (!_virtual_space.initialize(rs, rs.size())) {
    vm_exit_during_initialization("CMSMarkStack backing store failure");
  }
  assert(_virtual_space.committed_size() == rs.size(),
         "didn't reserve backing store for all of CMS stack?");
  _base = (oop*)(_virtual_space.low());
  index = 0;
  _capacity = size;
  NOT_PRODUCT(_max_depth = 0);
}


// Closures
// XXX: there seems to be a lot of code  duplication here;
// should refactor and consolidate common code.

// This closure is used to mark refs into the CMS generation in
// the CMS bit map. Called at the first checkpoint. This closure
// assumes that we do not need to re-mark dirty cards; if the CMS
// generation on which this is used is not an oldest (modulo perm gen)
// generation then this will lose younger_gen cards!

MarkRefsIntoClosure::MarkRefsIntoClosure(
    MemRegion span, CMSBitMap* bitMap, bool should_do_nmethods):
    _span(span),
    _bitMap(bitMap),
    _should_do_nmethods(should_do_nmethods) {
    assert(_ref_processor == NULL, "deliberately left NULL");
    assert(_bitMap->covers(_span), "_bitMap/_span mismatch");
}

void MarkRefsIntoClosure::do_oop(oop* p) {
  // if p points into _span, then mark corresponding bit in _markBitMap
  oop thisOop = *p;
  if (thisOop != NULL) {
    assert(thisOop->is_oop() || thisOop->mark() == NULL,
           "expected an oop, possibly with mark word displaced");
    HeapWord* addr = (HeapWord*)thisOop;
    if (_span.contains(addr)) {
      // this should be made more efficient
      _bitMap->mark(addr);
    }
  }
}

// This closure is used to mark refs into the CMS generation at the
// second (final) checkpoint, and to scan and transitively follow
// the unmarked oops. It is also used during the concurrent precleaning
// phase while scanning objects on dirty cards. The marks are made in
// the marking bit map and the marking stack is used for keeping the
// (newly) grey objects during the scan phase. The parallel version
// (Par_...) appears further below.
void MarkRefsIntoAndScanClosure::do_oop(oop* p) {
  oop thisOop = *p;
  if (thisOop != NULL) {
    assert(thisOop->is_oop(),
           "expected an oop");
    HeapWord* addr = (HeapWord*)thisOop;
    assert(_mark_stack->isEmpty(), "pre-condition (eager drainage)");
    if (_span.contains(addr) &&
        !_bit_map->isMarked(addr)) {
      // mark bit map (object is now grey)
      _bit_map->mark(addr);
      // push on marking stack (stack should be empty), and drain the
      // stack by applying this closure to the oops in the oops popped
      // from the stack (i.e. blacken the grey objects)
      for (_mark_stack->push(thisOop); !_mark_stack->isEmpty();) {
        oop newOop = _mark_stack->pop();
        assert(newOop->is_oop(), "Expected an oop");
        assert(_bit_map->isMarked((HeapWord*)newOop),
               "only grey objects on this stack");
        // iterate over the oops in this oop, marking and pushing
        // the ones in CMS heap (i.e. in _span).
        newOop->oop_iterate(&_pushAndMarkClosure);
        // check if it's time to yield
        do_yield_check();
      }
    }
    assert(_mark_stack->isEmpty(), "post-condition (eager drainage)");
  }
}

void MarkRefsIntoAndScanClosure::do_yield_work() {
  assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
         "CMS thread should hold CMS token");
  assert_lock_strong(_freelistLock);
  assert_lock_strong(_bit_map->lock());
  // relinquish the free_list_lock and bitMaplock()
  _bit_map->lock()->unlock();
  _freelistLock->unlock();
  ConcurrentMarkSweepThread::desynchronize(true);
  ConcurrentMarkSweepThread::acknowledge_yield_request();
  _collector->stopTimer();
  if (PrintCMSStatistics != 0) {
    _collector->incrementYields();
  }
  _collector->icms_wait();
  // os::sleep(Thread::current(), 1, false);
  ConcurrentMarkSweepThread::synchronize(true);
  _freelistLock->lock_without_safepoint_check();
  _bit_map->lock()->lock_without_safepoint_check();
  _collector->startTimer();
}

// This closure is used to mark refs into the CMS generation at the
// second (final) checkpoint, and to scan and transitively follow
// the unmarked oops. The marks are made in the marking bit map and
// the work_queue is used for keeping the (newly) grey objects during
// the scan phase whence they are also available for stealing by parallel
// threads. Since the marking bit map is shared, updates are
// synchronized (via CAS).
void Par_MarkRefsIntoAndScanClosure::do_oop(oop* p) {
  oop thisOop = *p;
  if (thisOop != NULL) {
    assert(thisOop->is_oop(),
           "expected an oop");
    HeapWord* addr = (HeapWord*)thisOop;
    if (_span.contains(addr) &&
        !_bit_map->isMarked(addr)) {
      // mark bit map (object is now grey)
      _bit_map->par_mark(addr);
      // push on work_queue (which may not be empty), and trim the
      // queue to an appropriate length by applying this closure to
      // the oops in the oops popped from the stack (i.e. blacken the
      // grey objects)
      _work_queue->push(thisOop);
      trim_queue(CMSWorkQueueDrainThreshold * ParallelGCThreads);
    }
  }
}

// This closure is used to rescan the marked objects on the dirty cards
// in the mod union table and the card table proper.
size_t ScanMarkedObjectsAgainCarefullyClosure::do_object_careful(oop p) {
  size_t size = 0;
  HeapWord* addr = (HeapWord*)p;
  assert(_markStack->isEmpty(), "pre-condition (eager drainage)");
  assert(_span.contains(addr), "we are scanning the CMS generation");
  // check if it's time to yield
  do_yield_check();
  if (_bitMap->isMarked(addr)) {
    // it's marked; is it potentially uninitialized?
    if (p->klass() != NULL) {
      if (CMSPermGenPrecleaningEnabled && !p->is_parsable()) {
        // Signal precleaning to redirty the card since
        // the klass pointer is already installed.
        return 0;
      }
      assert(p->is_parsable(), "must be parsable.");
      // an initialized object
      assert(p->is_oop(), "should be an oop");
      size = CompactibleFreeListSpace::adjustObjectSize(p->size());
      p->oop_iterate(_scanningClosure);
      debug_only(
        assert(size >= 3, "Necessary for Printezis marks to work");
        if (!_bitMap->isMarked(addr+1)) {
          _bitMap->verifyNoOneBitsInRange(addr+2, addr+size);
        } else {
          _bitMap->verifyNoOneBitsInRange(addr+2, addr+size-1);
          assert(_bitMap->isMarked(addr+size-1),
                 "inconsistent Printezis mark");
        }
      )
    } else {
      // an unitialized object
      assert(_bitMap->isMarked(addr+1), "missing Printezis mark?");
      HeapWord* nextOneAddr = _bitMap->getNextMarkedWordAddress(addr + 2);
      size = pointer_delta(nextOneAddr + 1, addr);
      assert(size == CompactibleFreeListSpace::adjustObjectSize(size),
             "alignment problem");
      // Note that pre-cleaning needn't redirty the card. OopDesc::set_klass()
      // will dirty the card when the klass pointer is installed in the
      // object (signalling the completion of initialization).
    }
  } else {
    // an unmarked object.
    assert(p->is_oop(), "should be an oop"); 
    size = CompactibleFreeListSpace::adjustObjectSize(p->size());
  }
  assert(_markStack->isEmpty(), "post-condition (eager drainage)");
  assert(size > 0, "no unparsable object found");
  return size;
}

void ScanMarkedObjectsAgainCarefullyClosure::do_yield_work() {
  assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
         "CMS thread should hold CMS token");
  assert_lock_strong(_freelistLock);
  assert_lock_strong(_bitMap->lock());
  // relinquish the free_list_lock and bitMaplock()
  _bitMap->lock()->unlock();
  _freelistLock->unlock();
  ConcurrentMarkSweepThread::desynchronize(true);
  ConcurrentMarkSweepThread::acknowledge_yield_request();
  _collector->stopTimer();
  if (PrintCMSStatistics != 0) {
    _collector->incrementYields();
  }
  _collector->icms_wait();
  // os::sleep(Thread::current(), 1, false);
  ConcurrentMarkSweepThread::synchronize(true);
  _freelistLock->lock_without_safepoint_check();
  _bitMap->lock()->lock_without_safepoint_check();
  _collector->startTimer();
}

// This closure is used to rescan the marked objects on the dirty cards
// in the mod union table and the card table proper. In the parallel
// case, although the bitMap is shared, we do a single read so the
// isMarked() query is "safe".
void ScanMarkedObjectsAgainClosure::do_object(oop p) {
  assert(p->is_oop_or_null(), "expected an oop or null");
  HeapWord* addr = (HeapWord*)p;
  assert(!_parallel || _mark_stack->isEmpty(),
         "pre-condition (eager drainage)");
  assert(_span.contains(addr), "we are scanning the CMS generation");
  if (_bit_map->isMarked(addr)) {
    if (_parallel) {
      p->oop_iterate(_scan_closure);
    } else {
      p->oop_iterate(_par_scan_closure);
    }
  }
  assert(!_parallel || _mark_stack->isEmpty(),
         "post-condition (eager drainage)");
}

MarkFromRootsClosure::MarkFromRootsClosure(CMSCollector* collector,
                        MemRegion span,
                        CMSBitMap* bitMap, CMSMarkStack*  markStack,
                        CMSMarkStack*  revisitStack,
                        bool should_yield):
  _collector(collector),
  _span(span),
  _bitMap(bitMap),
  _markStack(markStack),
  _revisitStack(revisitStack),
  _yield(should_yield),
  _skipBits(0) {
  assert(_markStack->isEmpty(), "stack should be empty");
  _finger    = _bitMap->startWord();
}

// Should revisit to see if this should be restructured for
// greater efficiency.
void MarkFromRootsClosure::do_bit(size_t offset) {
  if (_skipBits > 0) {
    _skipBits--;
    return;
  }
  // convert offset into a HeapWord*
  HeapWord* addr = _bitMap->startWord() + offset;
  assert(_bitMap->endWord() && addr < _bitMap->endWord(),
         "address out of range");
  assert(_bitMap->isMarked(addr), "tautology");
  if (_bitMap->isMarked(addr+1)) {
    // this is an allocated but not yet initialized object
    assert(_skipBits == 0, "tautology");
    _skipBits = 2;  // skip next two marked bits ("Printezis-marks")
  } else {
    scanOopsInOop(addr);
  }
}

// We take a break if we've been at this for a while,
// so as to avoid monopolizing the locks involved.
void MarkFromRootsClosure::do_yield_work() {
  // First give up the locks, then yield, then re-lock
  // We should probably use a constructor/destructor idiom to
  // do this unlock/lock or modify the MutexUnlocker class to
  // serve our purpose. XXX
  assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
         "CMS thread should hold CMS token");
  assert_lock_strong(_bitMap->lock());
  _bitMap->lock()->unlock();
  ConcurrentMarkSweepThread::desynchronize(true);
  ConcurrentMarkSweepThread::acknowledge_yield_request();
  _collector->stopTimer();
  if (PrintCMSStatistics != 0) {
    _collector->incrementYields();
  }
  _collector->icms_wait();
  // os::sleep(Thread::current(), 1, false);
  ConcurrentMarkSweepThread::synchronize(true);
  _bitMap->lock()->lock_without_safepoint_check();
  _collector->startTimer();
}

void MarkFromRootsClosure::scanOopsInOop(HeapWord* ptr) {
  assert(_bitMap->isMarked(ptr), "expected bit to be set");
  assert(_markStack->isEmpty(),
         "should drain stack to limit stack usage");
  // convert ptr to an oop preparatory to scanning
  oop thisOop = oop(ptr);
  assert(thisOop->is_oop() || thisOop->mark() == NULL,
         "should be an oop possibly with displaced mark");
  assert(_finger <= ptr, "_finger runneth ahead");
  // advance the finger to right end of this object
  _finger = ptr + thisOop->size();
  assert(_finger > ptr, "we just incremented it above");
  // Note: the finger doesn't advance while we drain
  // the stack below.
  PushOrMarkClosure pushOrMarkClosure(_collector->ref_processor(),
                                      _span, _bitMap, _markStack,
                                      _revisitStack,
                                      _finger);
  for (_markStack->push(thisOop); !_markStack->isEmpty();) {
    oop newOop = _markStack->pop();
    assert(newOop->is_oop() || newOop->mark() == NULL,
           "Oops! expected to pop an oop, possibly with displaced mark");
    // now scan this oop's oops
    newOop->oop_iterate(&pushOrMarkClosure);
    do_yield_check();
  }
  assert(_markStack->isEmpty(), "tautology, emphasizing post-condition");
}

PushOrMarkClosure::PushOrMarkClosure(ReferenceProcessor* rp,
                     MemRegion span,
                     CMSBitMap* bitMap, CMSMarkStack*  markStack,
                     CMSMarkStack*  revisitStack,
                     HeapWord* finger) :
  OopClosure(rp),
  _span(span),
  _bitMap(bitMap),
  _markStack(markStack),
  _revisitStack(revisitStack),
  _finger(finger),
  _num_klasses(0) {
  assert(_ref_processor != NULL, "_ref_processor should not be NULL");
}

void PushOrMarkClosure::do_oop(oop* p) {
  oop    thisOop = *p;
  assert(thisOop->is_oop_or_null() || thisOop->mark() == NULL,
         "expected an oop, NULL or an oop with mark word displaced");
  HeapWord* addr = (HeapWord*)thisOop;
  if (_span.contains(addr) && !_bitMap->isMarked(addr)) {
    // Oop lies in _span and isn't yet grey or black
    _bitMap->mark(addr);            // now grey
    if (addr < _finger) {
      // the bit map iteration has already either passed, or
      // sampled, this bit in the bit map; we'll need to
      // use the marking stack to scan this oop's oops.
      _markStack->push(thisOop);
    }
    // anything including and to the right of _finger
    // will be scanned as we iterate over the remainder of the
    // bit map
  }
}

// Grey object tracking during pre-cleaning and second checkpoint phases.
// The non-parallel version. (parallel version further below.)
void PushAndMarkClosure::do_oop(oop* p) {
  oop    thisOop = *p;
  assert(thisOop->is_oop_or_null(),
         "expected an oop or NULL");
  HeapWord* addr = (HeapWord*)thisOop;
  // Check if oop points into the CMS generation
  // and is not marked
  if (_span.contains(addr) && !_bit_map->isMarked(addr)) {
    // a white object ...
    _bit_map->mark(addr);         // ... now grey
    // push on the marking stack (grey set)
    _mark_stack->push(thisOop);
  }
}

// Grey object tracking during second checkpoint phases --
// the parallel version.
void Par_PushAndMarkClosure::do_oop(oop* p) {
  oop    thisOop = *p;
  assert(thisOop->is_oop_or_null(),
         "expected an oop or NULL");
  HeapWord* addr = (HeapWord*)thisOop;
  // Check if oop points into the CMS generation 
  // and is not marked
  if (_span.contains(addr) && !_bit_map->isMarked(addr)) {
    // a white object ...
    // XXX a suitable return value from the par_mark() method
    // might allow us to avoid having the oop potentially
    // appear on multiple thread work queues; but it's not
    // clear whether it would make that much of a difference
    // in the bottom line. Certainly worth an experiment. XXX
    _bit_map->par_mark(addr);     // ... now grey
    // push on work queue (grey set)
    _work_queue->push(thisOop);
  }
}

void PushAndMarkClosure::remember_klass(Klass* k) {
  _revisit_stack->push(oop(k));
}

void Par_PushAndMarkClosure::remember_klass(Klass* k) {
  _revisit_stack->par_push(oop(k));
}

void MarkFromDirtyCardsClosure::do_MemRegion(MemRegion mr) {
  assert(((size_t)mr.start())%CardTableModRefBS::card_size_in_words == 0,
         "mr should be aligned to start at a card boundary");
  // We'd like to assert:
  // assert(mr.word_size()%CardTableModRefBS::card_size_in_words == 0,
  //        "mr should be a range of cards");
  // However, that would be too strong in one case -- the last
  // partition ends at _unallocated_block which, in general, can be
  // an arbitrary boundary, not necessarily card aligned.
  if (PrintCMSStatistics != 0) {
    _num_dirty_cards +=
         mr.word_size()/CardTableModRefBS::card_size_in_words;
  }
  _space->object_iterate_mem(mr, &_scan_cl);
}

SweepClosure::SweepClosure(CMSCollector* collector,
                           ConcurrentMarkSweepGeneration* g,
                           CMSBitMap* bitMap, bool should_yield) :
  _collector(collector),
  _g(g),
  _sp(g->cmsSpace()),
  _limit(_sp->sweep_limit()),
  _freelistLock(_sp->freelistLock()),
  _bitMap(bitMap),
  _yield(should_yield),
  _inFreeRange(false),           // No free range at beginning of sweep
  _freeRangeInFreeLists(false),  // No free range at beginning of sweep
  _lastFreeRangeCoalesced(false),
  _freeFinger(g->used_region().start()) {
  NOT_PRODUCT(
    _numObjectsFreed = 0;
    _numWordsFreed   = 0;
    _numObjectsLive = 0;
    _numWordsLive = 0;
    _numObjectsAlreadyFree = 0;
    _numWordsAlreadyFree = 0;
    _last_fc = NULL;

    _sp->initializeIndexedFreeListArrayReturnedBytes();
    _sp->dictionary()->initializeDictReturnedBytes();
  )
  assert(_limit >= _sp->bottom() && _limit <= _sp->end(),
         "sweep _limit out of bounds");
  if (CMSTraceSweeper) {
    gclog_or_tty->print("\n====================\nStarting new sweep\n");
  }
}

// We need this destructor to reclaim any space at the end
// of the space, which do_blk below may not have added back to
// the free lists. [basically dealing with the "fringe effect"]
SweepClosure::~SweepClosure() {
  assert_lock_strong(_freelistLock);
  // this should be treated as the end of a free run if any
  // The current free range should be returned to the free lists
  // as one coalesced chunk.
  if (inFreeRange()) {
    flushCurFreeChunk(freeFinger(), 
      pointer_delta(_limit, freeFinger()));
    assert(freeFinger() < _limit, "the finger pointeth off base");
    if (CMSTraceSweeper) {
      gclog_or_tty->print("destructor:");
      gclog_or_tty->print("Sweep:put_free_blk 0x%x (%d) "
                 "[coalesced:%d]\n",
                 freeFinger(), pointer_delta(_limit, freeFinger()),
                 lastFreeRangeCoalesced());
    }
  }
  NOT_PRODUCT(
    if (Verbose && PrintGC) {
      gclog_or_tty->print("Collected %d objects, %d bytes",
                 _numObjectsFreed, _numWordsFreed*sizeof(HeapWord));
      gclog_or_tty->print_cr("\nLive %d objects,  %d bytes  "
	"Already free %d objects, %d bytes",
	_numObjectsLive, _numWordsLive*sizeof(HeapWord), 
	_numObjectsAlreadyFree, _numWordsAlreadyFree*sizeof(HeapWord));
      size_t totalBytes = (_numWordsFreed + _numWordsLive + _numWordsAlreadyFree) *
	sizeof(HeapWord);
      gclog_or_tty->print_cr("Total sweep: %d bytes", totalBytes);

      if (PrintCMSStatistics && CMSVerifyReturnedBytes) {
        size_t indexListReturnedBytes = _sp->sumIndexedFreeListArrayReturnedBytes();
        size_t dictReturnedBytes = _sp->dictionary()->sumDictReturnedBytes();
        size_t returnedBytes = indexListReturnedBytes + dictReturnedBytes;
        gclog_or_tty->print("Returned %d bytes", returnedBytes);
        gclog_or_tty->print("	Indexed List Returned %d bytes", 
  	  indexListReturnedBytes);
        gclog_or_tty->print_cr("	Dictionary Returned %d bytes",
  	  dictReturnedBytes);
      }
    }
  )
  // Now, in debug mode, just null out the sweep_limit
  NOT_PRODUCT(_sp->clear_sweep_limit();)
  if (CMSTraceSweeper) {
    gclog_or_tty->print("end of sweep\n================\n");
  }
}

void SweepClosure::initialize_free_range(HeapWord* freeFinger, 
    bool freeRangeInFreeLists) {
  if (CMSTraceSweeper) {
    gclog_or_tty->print("---- Start free range 0x%x with free block [%d] (%d)\n",
               freeFinger, _sp->block_size(freeFinger),
	       freeRangeInFreeLists);
  }
  assert(!inFreeRange(), "Trampling existing free range");
  set_inFreeRange(true);
  set_lastFreeRangeCoalesced(false);

  set_freeFinger(freeFinger);
  set_freeRangeInFreeLists(freeRangeInFreeLists);
  if (CMSTestInFreeList) {
    if (freeRangeInFreeLists) { 
      FreeChunk* fc = (FreeChunk*) freeFinger;
      assert(fc->isFree(), "A chunk on the free list should be free.");
      assert(fc->size() > 0, "Free range should have a size");
      assert(_sp->verifyChunkInFreeLists(fc), "Chunk is not in free lists");
    }
  }
}

// Note that the sweeper runs concurrently with mutators. Thus,
// it is possible for direct allocation in this generation to happen
// in the middle of the sweep. Note that the sweeper also coalesces
// contiguous free blocks. Thus, unless the sweeper and the allocator
// synchronize appropriately freshly allocated blocks may get swept up.
// This is accomplished by the sweeper locking the free lists while
// it is sweeping. Thus blocks that are determined to be free are
// indeed free. There is however one additional complication:
// blocks that have been allocated since the final checkpoint and
// mark, will not have been marked and so would be treated as
// unreachable and swept up. To prevent this, the allocator marks
// the bit map when allocating during the sweep phase. This leads,
// however, to a further complication -- objects may have been allocated
// but not yet initialized -- in the sense that the header isn't yet
// installed. The sweeper can not then determine the size of the block
// in order to skip over it. To deal with this case, we use a technique
// (due to Printezis) to encode such uninitialized block sizes in the
// bit map. Since the bit map uses a bit per every HeapWord, but the
// CMS generation has a minimum object size of 3 HeapWords, it follows
// that "normal marks" won't be adjacent in the bit map (there will
// always be at least two 0 bits between successive 1 bits). We make use
// of these "unused" bits to represent uninitialized blocks -- the bit
// corresponding to the start of the uninitialized object and the next
// bit are both set. Finally, a 1 bit marks the end of the object that
// started with the two consecutive 1 bits to indicate its potentially
// uninitialized state.

size_t SweepClosure::do_blk_careful(HeapWord* addr) {
  FreeChunk* fc = (FreeChunk*)addr;
  size_t res;

  // check if we are done sweepinrg
  if (addr == _limit) { // we have swept up to the limit, do nothing more
    assert(_limit >= _sp->bottom() && _limit <= _sp->end(),
           "sweep _limit out of bounds");
    // help the closure application finish
    return pointer_delta(_sp->end(), _limit);
  }
  assert(addr <= _limit, "sweep invariant");

  // check if we should yield
  do_yield_check(addr);
  if (fc->isFree()) {
    // Chunk that is already free
    res = fc->size();
    doAlreadyFreeChunk(fc);
    debug_only(_sp->verifyFreeLists());
    assert(res == fc->size(), "Don't expect the size to change");
    NOT_PRODUCT(
      _numObjectsAlreadyFree++;
      _numWordsAlreadyFree += res;
    )
    NOT_PRODUCT(_last_fc = fc;)
  } else if (!_bitMap->isMarked(addr)) {
    // Chunk is fresh garbage
    res = doGarbageChunk(fc);
    debug_only(_sp->verifyFreeLists());
    NOT_PRODUCT(
      _numObjectsFreed++;
      _numWordsFreed += res;
    )
  } else {
    // Chunk that is alive.
    res = doLiveChunk(fc);
    debug_only(_sp->verifyFreeLists());
    NOT_PRODUCT(
	_numObjectsLive++;
	_numWordsLive += res;
    )
  }
  return res;
}

// For the smart allocation, record following
//  split deaths - a free chunk is removed from its free list because
//	it is being split into two or more chunks.
//  split birth - a free chunk is being added to its free list because
//	a larger free chunk has been split and resulted in this free chunk.
//  coal death - a free chunk is being removed from its free list because
//	it is being coalesced into a large free chunk.
//  coal birth - a free chunk is being added to its free list because
//	it was created when two or more free chunks where coalesced into
//	this free chunk.
//
// These statistics are used to determine the desired number of free
// chunks of a given size.  The desired number is chosen to be relative
// to the end of a CMS sweep.  The desired number at the end of a sweep
// is the 
// 	count-at-end-of-previous-sweep (an amount that was enough)
//		- count-at-beginning-of-current-sweep  (the excess)
//		+ split-births  (gains in this size during interval)
//		- split-deaths  (demands on this size during interval)
// where the interval is from the end of one sweep to the end of the
// next.
//
// When sweeping the sweeper maintains an accumulated chunk which is
// the chunk that is made up of chunks that have been coalesced.  That
// will be termed the left-hand chunk.  A new chunk of garbage that
// is being considered for coalescing will be referred to as the
// right-hand chunk.
//
// When making a decision on whether to coalesce a right-hand chunk with
// the current left-hand chunk, the current count vs. the desired count
// of the left-hand chunk is considered.  Also if the right-hand chunk
// is near the large chunk at the end of the heap (see 
// ConcurrentMarkSweepGeneration::isNearLargestChunk()), then the 
// left-hand chunk is coalesced.
//
// When making a decision about whether to split a chunk, the desired count
// vs. the current count of the candidate to be split is also considered.
// If the candidate is underpopulated (currently fewer chunks than desired)
// a chunk of an overpopulated (currently more chunks than desired) size may 
// be chosen.  The "hint" associated with a free list, if non-null, points
// to a free list which may be overpopulated.  
//

void SweepClosure::doAlreadyFreeChunk(FreeChunk* fc) {
  size_t size = fc->size();
  // Chunks that cannot be coalesced are not in the
  // free lists.
  if (CMSTestInFreeList && !fc->cantCoalesce()) {
    assert(_sp->verifyChunkInFreeLists(fc), 
      "free chunk should be in free lists");
  }
  // a chunk that is already free, should not have been
  // marked in the bit map
  HeapWord* addr = (HeapWord*) fc;
  assert(!_bitMap->isMarked(addr), "free chunk should be unmarked");
  // Verify that the bit map has no bits marked between
  // addr and purported end of this block.
  _bitMap->verifyNoOneBitsInRange(addr + 1, addr + size);

  // Some chunks cannot be coalesced in under any circumstances.  
  // See the definition of cantCoalesce().
  if (!fc->cantCoalesce()) {
    // This chunk can potentially be coalesced.
    if (_sp->adaptive_freelists()) {
      // All the work is done in 
      doPostIsFreeOrGarbageChunk(fc, size);
    } else {  // Not adaptive free lists
      // this is a free chunk that can potentially be coalesced by the sweeper;
      if (!inFreeRange()) {
        // if the next chunk is a free block that can't be coalesced
        // it doesn't make sense to remove this chunk from the free lists
        FreeChunk* nextChunk = (FreeChunk*)(addr + size);
        assert((HeapWord*)nextChunk <= _limit, "sweep invariant");
        if ((HeapWord*)nextChunk < _limit  &&    // there's a next chunk...
            nextChunk->isFree()    &&            // which is free...
            nextChunk->cantCoalesce()) {         // ... but cant be coalesced
          // nothing to do
        } else {
          // Potentially the start of a new free range:
	  // Don't eagerly remove it from the free lists.  
	  // No need to remove it if it will just be put
	  // back again.  (Also from a pragmatic point of view
	  // if it is a free block in a region that is beyond
	  // any allocated blocks, an assertion will fail)
          // Remember the start of a free run.
          initialize_free_range(addr, true);
          // end - can coalesce with next chunk
        }
      } else {
        // the midst of a free range, we are coalescing
        debug_only(record_free_block_coalesced(fc);)
        if (CMSTraceSweeper) { 
          gclog_or_tty->print("  -- pick up free block 0x%x (%d)\n", fc, size);
        }
        // remove it from the free lists
        _sp->removeFreeChunkFromFreeLists(fc);
        set_lastFreeRangeCoalesced(true);
        // If the chunk is being coalesced and the current free range is
        // in the free lists, remove the current free range so that it
        // will be returned to the free lists in its entirety - all
        // the coalesced pieces included.
        if (freeRangeInFreeLists()) {
	  FreeChunk* ffc = (FreeChunk*) freeFinger();
	  assert(ffc->size() == pointer_delta(addr, freeFinger()),
	    "Size of free range is inconsistent with chunk size.");
	  if (CMSTestInFreeList) {
            assert(_sp->verifyChunkInFreeLists(ffc),
	      "free range is not in free lists");
	  }
          _sp->removeFreeChunkFromFreeLists(ffc);
	  set_freeRangeInFreeLists(false);
        }
      }
    }
  } else {
    // Code path common to both original and adaptive free lists.

    // cant coalesce with previous block; this should be treated
    // as the end of a free run if any
    if (inFreeRange()) {
      // we kicked some butt; time to pick up the garbage
      assert(freeFinger() < addr, "the finger pointeth off base");
      flushCurFreeChunk(freeFinger(), pointer_delta(addr, freeFinger()));
    }
    // else, nothing to do, just continue
  }
}

size_t SweepClosure::doGarbageChunk(FreeChunk* fc) {
  // This is a chunk of garbage.  It is not in any free list.
  // Add it to a free list or let it possibly be coalesced into
  // a larger chunk.
  HeapWord* addr = (HeapWord*) fc;
  size_t size = CompactibleFreeListSpace::adjustObjectSize(oop(addr)->size());

  if (_sp->adaptive_freelists()) {
    // Verify that the bit map has no bits marked between
    // addr and purported end of just dead object.
    _bitMap->verifyNoOneBitsInRange(addr + 1, addr + size);

    doPostIsFreeOrGarbageChunk(fc, size);
  } else {
    if (!inFreeRange()) {
      // start of a new free range
      assert(size > 0, "A free range should have a size");
      initialize_free_range(addr, false);

    } else {
      // this will be swept up when we hit the end of the
      // free range
      if (CMSTraceSweeper) {
        gclog_or_tty->print("  -- pick up garbage 0x%x (%d) \n", fc, size);
      }
      // If the chunk is being coalesced and the current free range is
      // in the free lists, remove the current free range so that it
      // will be returned to the free lists in its entirety - all
      // the coalesced pieces included.
      if (freeRangeInFreeLists()) {
	FreeChunk* ffc = (FreeChunk*)freeFinger();
	assert(ffc->size() == pointer_delta(addr, freeFinger()),
	  "Size of free range is inconsistent with chunk size.");
	if (CMSTestInFreeList) {
          assert(_sp->verifyChunkInFreeLists(ffc),
	    "free range is not in free lists");
	}
        _sp->removeFreeChunkFromFreeLists(ffc);
	set_freeRangeInFreeLists(false);
      }
      set_lastFreeRangeCoalesced(true);
    }
    // this will be swept up when we hit the end of the free range

    // Verify that the bit map has no bits marked between
    // addr and purported end of just dead object.
    _bitMap->verifyNoOneBitsInRange(addr + 1, addr + size);
  }
  return size;
}

size_t SweepClosure::doLiveChunk(FreeChunk* fc) {
  HeapWord* addr = (HeapWord*) fc;
  // The sweeper has just found a live object. Return any accumulated
  // left hand chunk to the free lists.
  if (inFreeRange()) {
    if (_sp->adaptive_freelists()) {
      flushCurFreeChunk(freeFinger(),
                        pointer_delta(addr, freeFinger()));
    } else { // not adaptive freelists
      set_inFreeRange(false);
      // Add the free range back to the free list if it is not already
      // there.
      if (!freeRangeInFreeLists()) {
        assert(freeFinger() < addr, "the finger pointeth off base");
        if (CMSTraceSweeper) {
          gclog_or_tty->print("Sweep:put_free_blk 0x%x (%d) "
            "[coalesced:%d]\n",
            freeFinger(), pointer_delta(addr, freeFinger()),
            lastFreeRangeCoalesced());
        }
        _sp->addChunkAndRepairOffsetTable(freeFinger(),
          pointer_delta(addr, freeFinger()), lastFreeRangeCoalesced());
      }
    }
  }

  // Common code path for original and adaptive free lists.

  // this object is live: we'd normally expect this to be
  // an oop, and like to assert the following:
  // assert(oop(addr)->is_oop(), "live block should be an oop");
  // However, as we commented above, this may be an object whose
  // header hasn't yet been initialized.
  size_t size;
  if (oop(addr)->klass() == NULL ||
      (CMSPermGenSweepingEnabled && !oop(addr)->is_parsable())) {

    assert(_bitMap->isMarked(addr + 1), "missing Printezis mark?");
    // this is a yet uninitialized object that's alive, caclulate its end
    // from the bit map.
    HeapWord* nextOneAddr = _bitMap->getNextMarkedWordAddress(addr + 2);
    size = pointer_delta(nextOneAddr + 1, addr);
    assert(size == CompactibleFreeListSpace::adjustObjectSize(size),
           "alignment problem");
  } else {
    // an initialized object that's alive.
    assert(oop(addr)->is_oop(), "live block should be an oop");
    // Verify that the bit map has no bits marked between
    // addr and purported end of this block.
    size = CompactibleFreeListSpace::adjustObjectSize(oop(addr)->size());
    debug_only(
      assert(size >= 3, "Necessary for Printezis marks to work");
      if (!_bitMap->isMarked(addr+1)) {
        _bitMap->verifyNoOneBitsInRange(addr+2, addr+size);
      } else {
        _bitMap->verifyNoOneBitsInRange(addr+2, addr+size-1);
        assert(_bitMap->isMarked(addr+size-1),
          "inconsistent Printezis mark");
      }
    )
  }
  return size;
}

void SweepClosure::doPostIsFreeOrGarbageChunk(FreeChunk* fc, 
					    size_t chunkSize) { 
  // doPostIsFreeOrGarbageChunk() should only be called in the smart allocation
  // scheme.
  bool fcInFreeLists = fc->isFree();
  assert(_sp->adaptive_freelists(), "Should only be used in this case.");
  assert((HeapWord*)fc <= _limit, "sweep invariant");
  if (CMSTestInFreeList && fcInFreeLists) {
    assert(_sp->verifyChunkInFreeLists(fc), 
      "free chunk is not in free lists");
  }
  
 
  if (CMSTraceSweeper) {
    gclog_or_tty->print_cr("  -- pick up another chunk at 0x%x (%d)", fc, chunkSize);
  }

  HeapWord* addr = (HeapWord*) fc;
  bool coalesce         = _sp->coalOverPopulated(pointer_delta(addr, freeFinger()));
  // Should the current free range be coalesced?
  // If the chunk is in a free range and either the size of the free range
  // is not overpopulated (coalesce == true) or the chunk is near the large
  // chunk at the end of the heap (isNearLargestChunk() returns true), 
  // coalesce this chunk.
  bool doCoalesce = inFreeRange() &&
    (coalesce || _g->isNearLargestChunk((HeapWord*)fc));
  if (doCoalesce) {
    // Coalesce the current free range on the left with the new
    // chunk on the right.  If either are already in the free lists,
    // remove them.
    if (freeRangeInFreeLists()) {
      FreeChunk* ffc = (FreeChunk*)freeFinger();
      assert(ffc->size() == pointer_delta(addr, freeFinger()),
        "Size of free range is inconsistent with chunk size.");
      if (CMSTestInFreeList) {
        assert(_sp->verifyChunkInFreeLists(ffc),
	  "Chunk is not in free lists");
      }
      _sp->coalDeath(ffc->size());
      _sp->removeFreeChunkFromFreeLists(ffc);
      set_freeRangeInFreeLists(false);
    }
    if (fcInFreeLists) {
      _sp->coalDeath(chunkSize);
      assert(fc->size() == chunkSize, 
	"The chunk has the wrong size or is not in the free lists");
      _sp->removeFreeChunkFromFreeLists(fc);
    }
    set_lastFreeRangeCoalesced(true);
  } else {  // not in a free range and/or should not coalesce
    // Return the current free range and start a new one.
    if (inFreeRange()) {
      // In a free range but cannot coalesce with the right hand chunk.
      // Put the current free range into the free lists.
      flushCurFreeChunk(freeFinger(), 
	pointer_delta(addr, freeFinger()));
    }
    // Set up for new free range.  Pass along whether the right hand
    // chunk is in the free lists.
    initialize_free_range((HeapWord*)fc, fcInFreeLists);
  }
}
void SweepClosure::flushCurFreeChunk(HeapWord* chunk, size_t size) {
  assert(inFreeRange(), "Should only be called if currently in a free range.");
  assert(size > 0, 
    "A zero sized chunk cannot be added to the free lists.");
  if (!freeRangeInFreeLists()) {
    if(CMSTestInFreeList) {
      FreeChunk* fc = (FreeChunk*) chunk;
      fc->setSize(size);
      assert(!_sp->verifyChunkInFreeLists(fc),
	"chunk should not be in free lists yet");
    }
    if (CMSTraceSweeper) {
      gclog_or_tty->print_cr(" -- add free block 0x%x (%d) to free lists",
                    chunk, size);
    }
    // A new free range is going to be starting.  The current
    // free range has not been added to the free lists yet or
    // was removed so add it back.
    // If the current free range was coalesced, then the death
    // of the free range was recorded.  Record a birth now.
    if (lastFreeRangeCoalesced()) {
      _sp->coalBirth(size);
    }
    _sp->addChunkAndRepairOffsetTable(chunk, size,
	    lastFreeRangeCoalesced());
  }
  set_inFreeRange(false);
  set_freeRangeInFreeLists(false);
}

// We take a break if we've been at this for a while,
// so as to avoid monopolizing the locks involved.
void SweepClosure::do_yield_work(HeapWord* addr) {
  // Return current free chunk being used for coalescing (if any)
  // to the appropriate freelist.  After yielding, the next
  // free block encountered will start a coalescing range of
  // free blocks.  If the next free block is adjacent to the
  // chunk just flushed, they will need to wait for the next
  // sweep to be coalesced.
  if (inFreeRange()) {
    flushCurFreeChunk(freeFinger(), pointer_delta(addr, freeFinger()));
  }

  // First give up the locks, then yield, then re-lock.
  // We should probably use a constructor/destructor idiom to
  // do this unlock/lock or modify the MutexUnlocker class to
  // serve our purpose. XXX
  assert_lock_strong(_bitMap->lock());
  assert_lock_strong(_freelistLock);
  assert(ConcurrentMarkSweepThread::cms_thread_has_cms_token(),
         "CMS thread should hold CMS token");
  _bitMap->lock()->unlock();
  _freelistLock->unlock();
  ConcurrentMarkSweepThread::desynchronize(true);
  ConcurrentMarkSweepThread::acknowledge_yield_request();
  _collector->stopTimer();
  if (PrintCMSStatistics != 0) {
    _collector->incrementYields();
  }
  _collector->icms_wait();
  // os::sleep(Thread::current(), 1, false);
  ConcurrentMarkSweepThread::synchronize(true);
  _freelistLock->lock();
  _bitMap->lock()->lock_without_safepoint_check();
  _collector->startTimer();
}

#ifndef PRODUCT
// This is actually very useful in a product build if it can
// be called from the debugger.  Compile it into the product
// as needed.
bool debug_verifyChunkInFreeLists(FreeChunk* fc) {
  return debug_cms_space->verifyChunkInFreeLists(fc);
}

void SweepClosure::record_free_block_coalesced(FreeChunk* fc) const {
  if (CMSTraceSweeper) {
    gclog_or_tty->print("Sweep:coal_free_blk 0x%x (%d)\n", fc, fc->size());
  }
}
#endif

// CMSIsAliveClosure
bool CMSIsAliveClosure::do_object_b(oop obj) {
  HeapWord* addr = (HeapWord*)obj;
  return addr != NULL &&
         (!_span.contains(addr) || _bitMap->isMarked(addr));
} 

// CMSKeepAliveClosure
void CMSKeepAliveClosure::do_oop(oop* p) {
  oop thisOop = *p;
  HeapWord* addr = (HeapWord*)thisOop;
  if (_span.contains(addr) &&
      !_bitMap->isMarked(addr)) {
    _bitMap->mark(addr);
    _markStack->push(thisOop);
  }
}

//////////////////////////////////////////////////////////////////
//  CMSExpansionCause		     /////////////////////////////
//////////////////////////////////////////////////////////////////
const char* CMSExpansionCause::to_string(CMSExpansionCause::Cause cause) {
  switch (cause) {
    case _no_expansion:
      return "No expansion";
    case _satisfy_free_ratio:
      return "Free ratio";
    case _satisfy_promotion:
      return "Satisfy promotion";
    case _satisfy_allocation:
      return "allocation";
    case _allocate_par_lab:
      return "Par LAB";
    default:
      return "unknown";
  }
}
