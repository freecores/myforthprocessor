#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)concurrentMarkSweepGeneration.hpp	1.70 03/05/26 09:52:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ConcurrentMarkSweepGeneration is in support of a concurrent
// mark-sweep old generation in the Detlefs-Printezis--Boehm-Demers-Schenker
// style. We assume, for now, that this generation is always the
// seniormost generation (modulo the PermGeneration), and for simplicity
// in the first implementation, that this generation is a single compactible
// space. Neither of these restrictions appears essential, and will be
// relaxed in the future when more time is available to implement the
// greater generality (and there's a need for it).
//
// Concurrent mode failures may be handled by
// means of a sliding mark-compact. (Not yet implemented.)

class ConcurrentMarkSweepGeneration;
class CompactibleFreeListSpace;
class FreeChunk;
class ScanMarkedObjectsAgainCarefullyClosure;

// A generic CMS bit map. It's the basis for both the CMS marking bit map
// as well as for the mod union table (in each case only a subset of the
// methods are used). This is essentially a wrapper around the BitMap class,
// with one bit per (1<<_shifter) HeapWords. (i.e. for the marking bit map,
// we have _shifter == 0. and for the mod union table we have
// shifter == CardTableModRefBS::card_shift - LogHeapWordSize.)
// XXX 64-bit issues in BitMap?
class CMSBitMap VALUE_OBJ_CLASS_SPEC {
  HeapWord* _bmStartWord;   // base address of range covered by map
  size_t    _bmWordSize;    // map size (in #HeapWords covered)
  const int _shifter;	    // shifts to convert HeapWord to bit position
  VirtualSpace _virtual_space; // underlying the bit map
  BitMap    _bm;            // the bit map itself
 public:
  mutable Mutex _lock;      // mutex protecting _bm;
                            // mutable because of ubiquity of locking
                            // even for otherwise const methods

 public:
  // constructor
  CMSBitMap(int shifter, int mutexRank, const char* mutexName);

  // allocates the actual storage for the map
  void allocate(MemRegion mr);

  // field getter
  Mutex* lock() const { return &_lock; }
  // locking verifier convenience function
  void assert_locked() const PRODUCT_RETURN;

  // inquiries
  HeapWord* startWord()   const { return _bmStartWord; }
  size_t    sizeInWords() const { return _bmWordSize;  }
  size_t    sizeInBits()  const { return _bm.size();   }
  // the following is one past the last word in space
  HeapWord* endWord()     const { return _bmStartWord + _bmWordSize; }

  // reading marks
  inline bool isMarked(HeapWord* addr) const;
  inline bool isUnmarked(HeapWord* addr) const;
  inline bool isAllClear() const;

  // writing marks
  inline void mark(HeapWord* addr);
  inline void par_mark(HeapWord* addr); // For marking by parallel GC threads.
  inline void markRange(MemRegion mr);
  inline void par_markRange(MemRegion mr);
  inline void par_clear(HeapWord* addr); // For unmarking by parallel GC threads.
  inline void clearRange(MemRegion mr);
  inline void clearAll();
  void clearAllIncrementally();  // Not yet implemented!!

  // iteration
  void iterate(BitMapClosure* cl) {
    _bm.iterate(cl);
  }
  inline void iterate(BitMapClosure* cl, HeapWord* left, HeapWord* right);
  void dirty_range_iterate_clear(MemRegionClosure* cl);
  void dirty_range_iterate_clear(MemRegion mr, MemRegionClosure* cl);
 
  // auxiliary support for iteration
  inline HeapWord* getNextMarkedWordAddress(HeapWord* addr) const;
  inline HeapWord* getNextMarkedWordAddress(HeapWord* start_addr,
                                            HeapWord* end_addr) const;
  inline HeapWord* getNextUnmarkedWordAddress(HeapWord* addr) const;
  inline HeapWord* getNextUnmarkedWordAddress(HeapWord* start_addr,
                                              HeapWord* end_addr) const;
  inline MemRegion getAndClearMarkedRegion(HeapWord* addr);
  inline MemRegion getAndClearMarkedRegion(HeapWord* start_addr, 
                                           HeapWord* end_addr);

  // conversion utilities
  inline HeapWord* offsetToHeapWord(size_t offset) const;
  inline size_t    heapWordToOffset(HeapWord* addr) const;
  inline size_t    heapWordDiffToOffsetDiff(size_t diff) const;
  
  // debugging
  // is this address range covered by the bit-map?
  NOT_PRODUCT(
    bool covers(MemRegion mr) const;
    bool covers(HeapWord* start, size_t size = 0) const;
  )
  void verifyNoOneBitsInRange(HeapWord* left, HeapWord* right) PRODUCT_RETURN;
};

// Represents a marking stack used by the CMS collector.
// Ideally this should be GrowableArray<> just like MSC's marking stack(s).
class CMSMarkStack: public CHeapObj  {
  VirtualSpace _virtual_space;  // space for the stack
  oop*   _base;      // bottom of stack
  size_t index;      // one more than last occupied index
  size_t _capacity;  // max #elements
  Mutex  _par_lock;  // an advisory lock used in case of parallel access
  NOT_PRODUCT(size_t _max_depth;)  // max depth plumbed during run

 public:
  CMSMarkStack():
    _par_lock(Mutex::event, "CMSMarkStack._par_lock", true) {}

  void allocate(size_t size);

  oop pop() {
    if (!isEmpty()) {
      return _base[--index] ;
    }
    return NULL;
  }

  void push(oop ptr) {
    // try to dynamically expand in the future (perhaps use GrowableArray<>
    // just like the mark-sweep-compact code does)
    guarantee(!isFull(), "CMSMarkStack is full");
    _base[index++] = ptr;
    NOT_PRODUCT(_max_depth = MAX2(_max_depth, index));
  }

  bool isEmpty() const { return index == 0; }
  bool isFull()  const { return index == _capacity; }

  // "Parallel versions" of some of the above
  oop par_pop() {
    // lock and pop
    MutexLockerEx x(&_par_lock, Mutex::_no_safepoint_check_flag);
    return pop();
  }

  void par_push(oop ptr) {
    // lock and push
    MutexLockerEx x(&_par_lock, Mutex::_no_safepoint_check_flag);
    push(ptr);
  }

  // Forcibly reset the stack, losing all of its contents.
  void reset() {
    index = 0;
  }
};

class CardTableRS;
class CMSParGCThreadState;

class ModUnionClosure: public MemRegionClosure {
 protected:
  CMSBitMap* _t;
 public:
  ModUnionClosure(CMSBitMap* t): _t(t) { }
  void do_MemRegion(MemRegion mr) {
    _t->markRange(mr);
  }
};

class ModUnionClosurePar: public ModUnionClosure {
 public:
  ModUnionClosurePar(CMSBitMap* t): ModUnionClosure(t) { }
  void do_MemRegion(MemRegion mr) {
    _t->par_markRange(mr);
  }
};

// 
// Timing, allocation and promotion statistics for gc scheduling and incremental
// mode pacing.  Most statistics are exponential averages.
// 
class CMSStats VALUE_OBJ_CLASS_SPEC {
 private:
  ConcurrentMarkSweepGeneration* const _cms_gen;   // The cms (old) gen.

  // The following are exponential averages with factor alpha:
  //   avg = (100 - alpha) * avg + alpha * cur_sample
  // 
  //   The durations measure:  end_time[n] - start_time[n]
  //   The periods measure:    start_time[n] - start_time[n-1]
  //
  // The cms period and duration include only concurrent collections; time spent
  // in foreground cms collections due to System.gc() or because of a failure to
  // keep up are not included.
  //
  // There are 3 alphas to "bootstrap" the statistics.  The _saved_alpha is the
  // real value, but is used only after the first period.  A value of 100 is
  // used for the first sample so it gets the entire weight.
  unsigned int _saved_alpha; // 0-100
  unsigned int _gc0_alpha;
  unsigned int _cms_alpha;

  double _gc0_duration;
  double _gc0_period;
  size_t _gc0_promoted;		// bytes promoted per gc0
  double _cms_duration;
  double _cms_duration_per_mb;
  double _cms_period;
  size_t _cms_allocated;	// bytes of direct allocation per gc0 period

  // Timers.
  elapsedTimer _cms_timer;
  TimeStamp    _gc0_begin_time;
  TimeStamp    _cms_begin_time;
  TimeStamp    _cms_end_time;

  // Snapshots of the amount used in the CMS generation.
  size_t _cms_used_at_gc0_begin;
  size_t _cms_used_at_gc0_end;
  size_t _cms_used_at_cms_begin;

  // Used to prevent the duty cycle from being reduced in the middle of a cms
  // cycle.
  bool _allow_duty_cycle_reduction;

  enum {
    _GC0_VALID = 0x1,
    _CMS_VALID = 0x2,
    _ALL_VALID = _GC0_VALID | _CMS_VALID
  };

  unsigned int _valid_bits;

  unsigned int _icms_duty_cycle;	// icms duty cycle (0-100).

 protected:
  static inline double exp_avg(double old_avg, double cur_val,
			       unsigned int alpha);
  static inline size_t exp_avg(size_t old_avg, size_t cur_val,
			       unsigned int alpha);

  // Return a duty cycle that avoids wild oscillations, by limiting the amount
  // of change between old_duty_cycle and new_duty_cycle (the latter is treated
  // as a recommended value).
  static unsigned int icms_damped_duty_cycle(unsigned int old_duty_cycle,
					     unsigned int new_duty_cycle);
  unsigned int icms_update_duty_cycle_impl();

 public:
  CMSStats(ConcurrentMarkSweepGeneration* cms_gen,
	   unsigned int alpha = CMSExpAvgFactor);

  // Whether or not the statistics contain valid data; higher level statistics
  // cannot be called until this returns true (they require at least one young
  // gen and one cms cycle to have completed).
  inline bool valid() const;

  // Record statistics.
  inline void record_gc0_begin();
  inline void record_gc0_end(size_t cms_gen_bytes_used);
  inline void record_cms_begin();
  inline void record_cms_end();

  // Allow management of the cms timer, which must be stopped/started around
  // yield points.
  inline elapsedTimer& cms_timer()     { return _cms_timer; }
  inline void start_cms_timer()        { _cms_timer.start(); }
  inline void stop_cms_timer()         { _cms_timer.stop(); }

  // Basic statistics; units are seconds or bytes.
  inline double gc0_period() const     { return _gc0_period; }
  inline double gc0_duration() const   { return _gc0_duration; }
  inline size_t gc0_promoted() const   { return _gc0_promoted; }

  inline double cms_period() const          { return _cms_period; }
  inline double cms_duration() const        { return _cms_duration; }
  inline double cms_duration_per_mb() const { return _cms_duration_per_mb; }
  inline size_t cms_allocated() const       { return _cms_allocated; }

  // Seconds since the last background cms cycle began or ended.
  inline double cms_time_since_begin() const;
  inline double cms_time_since_end() const;

  // Higher level statistics--caller must check that valid() returns true before
  // calling.

  // Returns bytes promoted per second of wall clock time.
  inline double promotion_rate() const;

  // Returns bytes directly allocated per second of wall clock time.
  inline double cms_allocation_rate() const;

  // Rate at which space in the cms generation is being consumed (sum of the
  // above two).
  inline double cms_consumption_rate() const;

  // Returns an estimate of the number of seconds until the cms generation will
  // fill up, assuming no collection work is done.
  double time_until_cms_gen_full() const;

  // End of higher level statistics.

  // Returns the cms incremental mode duty cycle, as a percentage (0-100).
  inline unsigned int icms_duty_cycle() const { return _icms_duty_cycle; }

  // Update the duty cycle and return the new value.
  inline unsigned int icms_update_duty_cycle();

  // Debugging.
  void print_on(outputStream* st) const PRODUCT_RETURN;
  void print() const { print_on(gclog_or_tty); }
};

class CMSCollector: public CHeapObj {
  friend class VMStructs;
  friend class ConcurrentMarkSweepThread;
  friend class ConcurrentMarkSweepGeneration;  // XXXPERM
  friend class CompactibleFreeListSpace;       // XXXYSR
  friend class CMSParRemarkTask;

 private:
  jlong _time_of_last_gc;
  void update_time_of_last_gc(jlong now) {
    _time_of_last_gc = now;
  }
  
  OopTaskQueueSet* _task_queues;

  int*             _hash_seed;


  // Performance Counters
  CollectorCounters* _gc_counters;

 protected:
  ConcurrentMarkSweepGeneration* _cmsGen;  // old gen (CMS)
  ConcurrentMarkSweepGeneration* _permGen; // perm gen
  MemRegion                      _span;    // span covering above two
  CardTableRS*                   _ct;      // card table

  // ("Weak") Reference processing support
  ReferenceProcessor*            _ref_processor;

  ConcurrentMarkSweepThread*     _cmsThread;   // the thread doing the work
  ModUnionClosure    _modUnionClosure;
  ModUnionClosurePar _modUnionClosurePar;

  // CMS abstract state machine
  // initial_state: Idling
  // next_state(Idling)    = {Marking}
  // next_state(Marking)   = {Precleaning, Sweeping}
  // next_state(Sweeping)  = {Resetting}
  // next_state(Resetting) = {Idling}
  // The numeric values below are chosen so that:
  // . _collectorState < Idling ==  post-sweep && pre-mark
  // . _collectorState in (Idling, Sweeping) ==
  //                               {initial,,final}marking || precleaning
  enum CollectorState {
    Resetting           = 0,
    Idling              = 1,
    InitialMarking      = 2,
    Marking             = 3,
    FinalMarking        = 4,
    Precleaning         = 5,
    Sweeping            = 6,
    LastCollectorState  = 7
  };
  CollectorState _collectorState;

  // State related to prologue/epilogue invocation for my generations
  bool _between_prologue_and_epilogue;

  // Signalling/State related to coordination between fore- and backgroud GC
  // Note: When the baton has been passed from background GC to foreground GC,
  // _foregroundGCIsActive is true and _foregroundGCShouldWait is false.
  bool _foregroundGCIsActive;    // true iff foreground collector is active or
                                 // wants to go active
  bool _foregroundGCShouldWait;  // true iff background GC is active and has not
                                 // yet passed the baton to the foreground GC

  int    _numYields;
  size_t _numDirtyCards;
  uint   _sweepCount;
  // number of full gc's since the last concurrent gc.
  uint	 _full_gcs_since_conc_gc;

  // if occupancy exceeds this, start a new gc cycle
  double _initiatingOccupancy;

  // timer
  elapsedTimer _timer;

  // CMS marking support structures
  CMSBitMap     _markBitMap;
  CMSBitMap     _modUnionTable;
  CMSMarkStack  _markStack;
  CMSMarkStack  _revisitStack; // used to keep track of klassKlass objects
                               // to revisit

  // Timing, allocation and promotion statistics, used for scheduling.
  CMSStats      _stats;

  // Allocation limits installed in the young gen, used only in
  // CMSIncrementalMode.  When an allocation in the young gen would cross one of
  // these limits, the cms generation is notified and the cms thread is started
  // or stopped, respectively.
  HeapWord*	_icms_start_limit;
  HeapWord*	_icms_stop_limit;

  enum CMS_op_type {
    CMS_op_checkpointRootsInitial,
    CMS_op_checkpointRootsFinal
  };

  void doCMSOperation(CMS_op_type op);
  bool stopWorldAndDo(CMS_op_type op);

  OopTaskQueueSet* task_queues() { return _task_queues; }
  int*             hash_seed(int i) { return &_hash_seed[i]; }

 private:
  // CMS work methods
  void checkpointRootsInitialWork(bool asynch);    // initial checkpoint work

  void markFromRootsWork(bool asynch);  // concurrent marking work

  // concurrent precleaning work
  size_t precleanModUnionTable(ScanMarkedObjectsAgainCarefullyClosure* cl);
  size_t precleanCardTable(ConcurrentMarkSweepGeneration* gen,
                           ScanMarkedObjectsAgainCarefullyClosure* cl);
  void precleanWork();

  // final (second) checkpoint work
  void checkpointRootsFinalWork(bool asynch, bool clear_all_soft_refs,
                                bool init_mark_was_synchronous);
  // work routine for parallel version of remark
  void do_remark_parallel();
  // work routine for non-parallel version of remark
  void do_remark_non_parallel();
  // reference processing work routine (during second checkpoint)
  void refProcessingWork(bool asynch, bool clear_all_soft_refs);

  // concurrent sweeping work
  void sweepWork(ConcurrentMarkSweepGeneration* gen, bool asynch);

  // (concurrent) resetting of support data structures
  void reset(bool asynch);

  // An auxilliary method used to record the ends of
  // used regions of each generation to limit the extent of sweep
  inline void save_sweep_limits();

  // A work method used by foreground collection to determine
  // what type of collection (compacting or not, continuing or fresh)
  // it should do.
  void decide_foreground_collection_type(bool clear_all_soft_refs,
    bool* should_compact, bool* should_start_over);

  // A work method used by the foreground collector to do
  // a mark-sweep-compact.
  void do_compaction_work(bool clear_all_soft_refs);

  // A work method used by the foreground collector to do
  // a mark-sweep, after taking over from a possibly on-going
  // concurrent mark-sweep collection.
  void do_mark_sweep_work(bool clear_all_soft_refs,
    CollectorState first_state, bool should_start_over);

  // If the backgrould GC is active, acquire control from the background
  // GC and do the collection.
  void acquire_control_and_collect(bool   full, bool clear_all_soft_refs);

  // For synchronizing passing of control from background to foreground
  // GC.  waitForForegroundGC() is called by the background
  // collector.  It if had to wait for a foreground collection,
  // it returns true and the background collection should assume
  // that the collection was finished by the foreground
  // collector.
  bool waitForForegroundGC();

  // locking checks
  bool vm_thread_has_cms_token();
  bool cms_thread_has_cms_token();

  // Incremental mode triggering:  recompute the icms duty cycle and set the
  // allocation limits in the young gen.
  void icms_update_allocation_limits();

  size_t block_size_using_printezis_bits(HeapWord* addr) const;
  HeapWord* next_card_start_after_block(HeapWord* addr) const;

 public:
  CMSCollector(ConcurrentMarkSweepGeneration* cmsGen,
               ConcurrentMarkSweepGeneration* permGen,
               CardTableRS*                   ct);
  ConcurrentMarkSweepThread* cmsThread() { return _cmsThread; }

  ReferenceProcessor* ref_processor() { return _ref_processor; }

  Mutex* bitMapLock()        const { return _markBitMap.lock();    }
  Mutex* modUnionTableLock() const { return _modUnionTable.lock(); }
  CollectorState abstract_state() const { return _collectorState;  }
  double initiatingOccupancy() const { return _initiatingOccupancy; }

  // XXXPERM bool should_collect(bool full, size_t size, bool large_noref, bool tlab);
  bool shouldConcurrentCollect();

  void collect(bool   full,
               bool   clear_all_soft_refs,
               size_t size,
               bool   large_noref,
               bool   tlab);
  void collect_in_background(bool clear_all_soft_refs);
  void collect_in_foreground(bool clear_all_soft_refs);

  void direct_allocated(HeapWord* start, size_t size);

  // Object is dead if not marked and current phase is sweeping.
  inline bool is_dead_obj(oop obj);

  // After a promotion (of "start"), do any necessary marking.
  // If "par", then it's being done by a parallel GC thread.
  void promoted(bool par, HeapWord* start);

  HeapWord* allocation_limit_reached(Space* space, HeapWord* top,
				     size_t word_size);

  void getFreelistLocks();
  void releaseFreelistLocks();
  bool haveFreelistLocks();

  // GC prologue and epilogue
  void gc_prologue(bool full);
  void gc_epilogue(bool full);

  jlong time_of_last_gc(jlong now) {
    if (_collectorState <= Idling) {
      // gc not in progress
      return _time_of_last_gc;
    } else {
      // collection in progress
      return now;
    }
  }

  CMSBitMap* markBitMap()  { return &_markBitMap; }
  void directAllocated(HeapWord* start, size_t size);

  // main CMS steps and related support
  void checkpointRootsInitial(bool asynch);
  void markFromRoots(bool asynch);
  void preclean(bool asynch);
  void checkpointRootsFinal(bool asynch, bool clear_all_soft_refs,
                            bool init_mark_was_synchronous);
  void sweep(bool asynch);

  // Check that the currently executing thread is the correct
  // thread with regard to the foreground collector or background
  // collector.
  void checkCorrectThreadExecuting()    PRODUCT_RETURN;
  // XXXPERM void print_statistics()               PRODUCT_RETURN;

  bool is_cms_reachable(HeapWord* addr);

  // Performance Counter Support
  CollectorCounters* counters()    { return _gc_counters; }

  // timer stuff
  void    startTimer() { _timer.start();   }
  void    stopTimer()  { _timer.stop();    }
  void    resetTimer() { _timer.reset();   }
  double  timerValue() { return _timer.seconds(); }

  int  yields()          { return _numYields; }
  void resetYields()     { _numYields = 0;    }
  void incrementYields() { _numYields++;      }
  void resetNumDirtyCards()               { _numDirtyCards = 0; }
  void incrementNumDirtyCards(size_t num) { _numDirtyCards += num; }
  size_t  numDirtyCards()                 { return _numDirtyCards; }

  bool foregroundGCShouldWait() const { return _foregroundGCShouldWait; }
  void set_foregroundGCShouldWait(bool v) { _foregroundGCShouldWait = v; }
  bool foregroundGCIsActive() const { return _foregroundGCIsActive; }
  void set_foregroundGCIsActive(bool v) { _foregroundGCIsActive = v; }
  uint  sweepCount() const             { return _sweepCount; }
  void incrementSweepCount()           { _sweepCount++; }

  // Timers/stats for gc scheduling and incremental mode pacing.
  CMSStats& stats() { return _stats; }

  // Convenience methods that check whether CMSIncrementalMode is enabled and
  // forward to the corresponding methods in ConcurrentMarkSweepThread.
  static inline void start_icms();
  static inline void stop_icms();    // Called at the end of the cms cycle.
  static inline void disable_icms(); // Called before a foreground collection.
  static inline void enable_icms();  // Called after a foreground collection.
  inline void icms_wait();	     // Called at yield points.

  // debugging
  void verify(bool) PRODUCT_RETURN;
  static const size_t skip_header_HeapWords() PRODUCT_RETURN0;
  void verify_ok_to_terminate() const PRODUCT_RETURN;
  HeapWord* block_start(const void* p) const PRODUCT_RETURN0;
};

class CMSExpansionCause : public AllStatic  {
 public:
  enum Cause {
    _no_expansion,
    _satisfy_free_ratio,
    _satisfy_promotion,
    _satisfy_allocation,
    _allocate_par_lab
  };
  // Return a string describing the cause of the expansion.
  static const char* to_string(CMSExpansionCause::Cause cause);
};

class ConcurrentMarkSweepGeneration: public CardGeneration {
  friend class VMStructs;
  friend class ConcurrentMarkSweepThread;
  friend class ConcurrentMarkSweep;
  friend class CMSCollector;   // XXX PERM fix this!!!
 protected:
  static CMSCollector*       _collector; // the collector that collects us
  CompactibleFreeListSpace*  _cmsSpace;  // underlying space (only one for now)

  // Performance Counters
  GenerationCounters*      _gen_counters;
  GSpaceCounters*          _space_counters;

  // Words directly allocated, used by CMSStats.
  size_t _direct_allocated_words;

  // Non-product stat counters
  NOT_PRODUCT(
    int _numObjectsPromoted;
    int _numWordsPromoted;
    int _numObjectsAllocated;
    int _numWordsAllocated;
  )

 private:
  // For parallel young-gen GC support.
  CMSParGCThreadState** _par_gc_thread_states;

  // Reason generation was expanded
  CMSExpansionCause::Cause _expansion_cause;

  // accessors
  void set_expansion_cause(CMSExpansionCause::Cause v) { _expansion_cause = v;}
  CMSExpansionCause::Cause expansion_cause() { return _expansion_cause; }

  // Grow generation by specified size (returns false if unable to grow)
  bool grow_by(size_t bytes);
  // Grow generation to reserved size.
  void grow_to_reserved();
  // Shrink generation by specified size (returns false if unable to shrink)
  void shrink_by(size_t bytes);

 public:
  ConcurrentMarkSweepGeneration(ReservedSpace rs, size_t initial_byte_size,
                                int level, CardTableRS* ct,
				bool use_adaptive_freelists,
                                FreeBlockDictionary::DictionaryChoice);

  // Accessors
  CMSCollector* collector() { return _collector; }
  static void set_collector(CMSCollector* collector) {
    assert(_collector == NULL, "already set");
    _collector = collector;
  }
  CompactibleFreeListSpace*  cmsSpace() const { return _cmsSpace;  }
  
  Mutex* freelistLock() const;

  Generation::Name kind() { return Generation::ConcurrentMarkSweep; }

  const bool refs_discovery_is_atomic() const { return false; }
  bool refs_discovery_is_mt()     const {
    // Note: CMS does MT-discovery during the parallel-remark
    // phases. Use ReferenceProcessorMTMutator to make refs
    // discovery MT-safe during such phases or other parallel
    // discovery phases in the future. This may all go away
    // if/when we decide that refs discovery is sufficiently
    // rare that the cost of the CAS's involved is in the
    // noise. That's a measurement that should be done, and
    // the code simplified if that turns out to be the case.
    return false;
  }

  // Space enquiries
  inline size_t capacity() const;
  inline size_t used() const;
  inline size_t free() const;
  double occupancy()      { return ((double)used())/((double)capacity()); }
  size_t contiguous_available() const;
  size_t unsafe_max_alloc_nogc() const;

  // over-rides
  inline MemRegion used_region() const;
  inline MemRegion used_region_at_save_marks() const;

  void space_iterate(SpaceClosure* blk, bool usedOnly = false);

  // Support for compaction
  CompactibleSpace* first_compaction_space() const;
  // Adjust quantites in the generation affected by
  // the compaction.
  void reset_after_compaction();

  // Allocation support
  HeapWord* allocate(size_t size, bool large_noref, bool tlab);
  HeapWord* have_lock_and_allocate(size_t size, bool large_noref, bool tlab);
  oop       promote(oop obj, size_t obj_size, oop* ref);
  HeapWord* par_allocate(size_t size, bool large_noref, bool tlab) {
    return allocate(size, large_noref, tlab);
  }

  // Incremental mode triggering.
  HeapWord* allocation_limit_reached(Space* space, HeapWord* top,
				     size_t word_size);

  // Used by CMSStats to track direct allocation.  The value is sampled and
  // reset after each young gen collection.
  size_t direct_allocated_words() const { return _direct_allocated_words; }
  void reset_direct_allocated_words()   { _direct_allocated_words = 0; }

  // Overrides for parallel promotion.
  virtual oop par_promote(int thread_num,
			  oop obj, markOop m, size_t word_sz);
  // This one should not be called for CMS.
  virtual void par_promote_alloc_undo(int thread_num,
				      HeapWord* obj, size_t word_sz);
  virtual void par_promote_alloc_done(int thread_num);
  virtual void par_oop_since_save_marks_iterate_done(int thread_num);

  bool should_collect(bool full, size_t size, bool large_noref, bool tlab);
    // XXXPERM
  bool shouldConcurrentCollect(double initiatingOccupancy); // XXXPERM
  void collect(bool   full,
               bool   clear_all_soft_refs,
               size_t size,
               bool   large_noref,
               bool   tlab);

  HeapWord* expand_and_allocate(size_t word_size,
				bool large_noref, bool tlab,
				bool parallel = false);

  // GC prologue and epilogue
  void gc_prologue(bool full);
  void gc_prologue_work(bool full, bool registerClosure,
                        ModUnionClosure* modUnionClosure);
  void gc_epilogue(bool full);
  void gc_epilogue_work(bool full);

  // Time since last GC of this generation
  jlong time_of_last_gc(jlong now) {
    return collector()->time_of_last_gc(now);
  }
  void update_time_of_last_gc(jlong now) {
    collector()-> update_time_of_last_gc(now);
  }

  // Allocation failure
  void expand(size_t bytes, size_t expand_bytes, 
    CMSExpansionCause::Cause cause);
  void shrink(size_t bytes);
  HeapWord* expand_and_par_lab_allocate(CMSParGCThreadState* ps, size_t word_sz);

  // Iteration support and related enquiries
  void save_marks();
  bool no_allocs_since_save_marks();
  void object_iterate_since_last_GC(ObjectClosure* cl);
  void younger_refs_iterate(OopsInGenClosure* cl);

  // Iteration support specific to CMS generations
  inline void save_sweep_limit();

  // More iteration support
  void get_locks_and_oop_iterate(OopClosure* cl);
  void get_locks_and_object_iterate(ObjectClosure* cl);

  // Need to declare the full complement of closures, whether we'll
  // override them or not, or get message from the compiler:
  //   oop_since_save_marks_iterate_nv hides virtual function...
  #define CMS_SINCE_SAVE_MARKS_DECL(OopClosureType, nv_suffix) \
    void oop_since_save_marks_iterate##nv_suffix(OopClosureType* cl);
  ALL_SINCE_SAVE_MARKS_CLOSURES(CMS_SINCE_SAVE_MARKS_DECL)

  // Smart allocation  XXX -- move to CFLSpace?
  void setNearLargestChunk();
  bool isNearLargestChunk(HeapWord* addr);

  // Debugging
  void prepare_for_verify();
  void verify(bool allow_dirty);
  void print_statistics()               PRODUCT_RETURN;

  // Performance Counters support
  void update_counters();
  void update_counters(size_t used);
  void initialize_performance_counters();
  CollectorCounters* counters()  { return collector()->counters(); }


  // Printing
  const char* name() const;
  const char* short_name() const { return "CMS"; }
  void        print() const;
  void printOccupancy(const char* s);
  bool must_be_youngest() const { return false; }
  bool must_be_oldest()   const { return true; }

  void compute_new_size();
};

//
// Closures of various sorts used by CMS to accomplish its work
//

// This closure is used to check that a certain set of oops is empty.
class FalseClosure: public OopClosure {
 public:
  void do_oop(oop* p) {
    guarantee(false, "Should be an empty set");
  }
};

// This closure is used to do concurrent marking from the roots
// following the first checkpoint. 
class MarkFromRootsClosure: public BitMapClosure {
  CMSCollector*  _collector;
  MemRegion      _span;
  CMSBitMap*     _bitMap;
  CMSMarkStack*  _markStack;
  CMSMarkStack*  _revisitStack;
  bool           _yield;
  int            _skipBits;
  HeapWord*      _finger;
 public:
  MarkFromRootsClosure(CMSCollector* collector, MemRegion span,
                       CMSBitMap* bitMap,
                       CMSMarkStack*  markStack,
                       CMSMarkStack*  revisitStack,
                       bool should_yield);
  void do_bit(size_t offset);

 private:
  void scanOopsInOop(HeapWord* ptr);

  inline void do_yield_check();

  void do_yield_work();
};

// This closure is used to check that a certain set of bits is
// "empty" (i.e. the bit vector doesn't have any 1-bits).
class FalseBitMapClosure: public BitMapClosure {
 public:
  void do_bit(size_t offset) {
    guarantee(false, "Should not have a 1 bit"); 
  }
};

// This closure is used during the concurrent marking phase
// following the first checkpoint. Its use is buried in
// the closure MarkFromRootsClosure.
class PushOrMarkClosure: public OopClosure {
  MemRegion        _span;
  CMSBitMap*       _bitMap;
  CMSMarkStack*    _markStack;
  CMSMarkStack*    _revisitStack;
  const HeapWord*  _finger;
  int              _num_klasses;
 public:
  PushOrMarkClosure(ReferenceProcessor* rp, MemRegion span,
                    CMSBitMap* bitMap,
                    CMSMarkStack*  markStack,
                    CMSMarkStack*  revisitStack,
                    HeapWord*      finger);
  void do_oop(oop* p);
  const inline bool should_remember_klasses() const {
    return CMSClassUnloadingEnabled;
  }
  void remember_klass(Klass* k) {
    _revisitStack->push(oop(k));
    _num_klasses++;
  }
};

// This closure is used during the second checkpointing phase
// to rescan the marked objects on the dirty cards in the mod
// union table and the card table proper. It's invoked via
// MarkFromDirtyCardsClosure below. It uses either
// [Par_]MarkRefsIntoAndScanClosure (Par_ in the parallel case)
// declared in genOopClosures.hpp to accomplish some of its work.
// In the parallel case the bitMap is shared, so access to
// it needs to be suitably synchronized for updates by embedded
// closures that update it; however, this closure itself only
// reads the bit_map and because it is idempotent, is immune to
// reading stale values.
class ScanMarkedObjectsAgainClosure: public UpwardsObjectClosure {
  #ifdef ASSERT
    MemRegion              _span;
    union {
      CMSMarkStack*        _mark_stack;
      OopTaskQueue*        _work_queue;
    };
  #endif // ASSERT
  bool                       _parallel;
  CMSBitMap*                 _bit_map;
  union {
    MarkRefsIntoAndScanClosure*     _scan_closure;
    Par_MarkRefsIntoAndScanClosure* _par_scan_closure;
  };

 public:
  ScanMarkedObjectsAgainClosure(MemRegion span,
                                ReferenceProcessor* rp,
                                CMSBitMap* bit_map,
                                CMSMarkStack*  mark_stack,
                                CMSMarkStack*  revisit_stack,
                                MarkRefsIntoAndScanClosure* cl):
    #ifdef ASSERT
      _span(span),
      _mark_stack(mark_stack),
    #endif // ASSERT
    _parallel(false),
    _bit_map(bit_map),
    _scan_closure(cl) { }

  ScanMarkedObjectsAgainClosure(MemRegion span,
                                ReferenceProcessor* rp,
                                CMSBitMap* bit_map,
                                OopTaskQueue* work_queue,
                                CMSMarkStack* revisit_stack,
                                Par_MarkRefsIntoAndScanClosure* cl):
    #ifdef ASSERT
      _span(span),
      _work_queue(work_queue),
    #endif // ASSERT
    _parallel(true),
    _bit_map(bit_map),
    _par_scan_closure(cl) { }
                                
  void do_object(oop p);
};

// This closure is used during the second checkpointing phase
// to rescan the marked objects on the dirty cards in the mod
// union table and the card table proper. It invokes
// ScanMarkedObjectsAgainClosure above to accomplish much of its work.
// In the parallel case, the bit map is shared and requires
// synchronized access.
class MarkFromDirtyCardsClosure: public MemRegionClosure {
  CompactibleFreeListSpace*      _space;
  ScanMarkedObjectsAgainClosure  _scan_cl;
  size_t                         _num_dirty_cards;

 public:
  MarkFromDirtyCardsClosure(CMSCollector* collector,
                            MemRegion span,
                            CompactibleFreeListSpace* space,
                            CMSBitMap* bit_map,
                            CMSMarkStack* mark_stack,
                            CMSMarkStack* revisit_stack,
                            MarkRefsIntoAndScanClosure* cl):
    _space(space),
    _num_dirty_cards(0),
    _scan_cl(span, collector->ref_processor(), bit_map,
                 mark_stack, revisit_stack, cl) { }

  MarkFromDirtyCardsClosure(CMSCollector* collector,
                            MemRegion span,
                            CompactibleFreeListSpace* space,
                            CMSBitMap* bit_map,
                            OopTaskQueue* work_queue,
                            CMSMarkStack* revisit_stack,
                            Par_MarkRefsIntoAndScanClosure* cl):
    _space(space),
    _num_dirty_cards(0),
    _scan_cl(span, collector->ref_processor(), bit_map,
             work_queue, revisit_stack, cl) { }

  void do_MemRegion(MemRegion mr);
  void set_space(CompactibleFreeListSpace* space) { _space = space; }
  size_t num_dirty_cards() { return _num_dirty_cards; }
};

// This closure is used in the non-product build to check
// that there are no MemRegions with a certain property.
class FalseMemRegionClosure: public MemRegionClosure {
  void do_MemRegion(MemRegion mr) {
    guarantee(!mr.is_empty(), "Shouldn't be empty");
    guarantee(false, "Should never be here");
  }
};

// This closure is used during the precleaning phase
// to "carefully" rescan marked objects on dirty cards.
// It uses MarkRefsIntoAndScanClosure declared in genOopClosures.hpp
// to accomplish some of its work.
class ScanMarkedObjectsAgainCarefullyClosure: public MemObjectClosureCareful {
  CMSCollector*                  _collector;
  MemRegion                      _span;
  bool                           _yield;
  Mutex*                         _freelistLock;
  CMSBitMap*                     _bitMap;
  CMSMarkStack*                  _markStack;
  MarkRefsIntoAndScanClosure*    _scanningClosure;

 public:
  ScanMarkedObjectsAgainCarefullyClosure(CMSCollector* collector,
                                         MemRegion     span,
                                         CMSBitMap* bitMap,
                                         CMSMarkStack*  markStack,
                                         CMSMarkStack*  revisitStack,
                                         MarkRefsIntoAndScanClosure* cl,
                                         bool should_yield):
    _collector(collector),
    _span(span),
    _yield(should_yield),
    _bitMap(bitMap),
    _markStack(markStack),
    _scanningClosure(cl) {
  }
  size_t      do_object_careful(oop p);

  void setFreelistLock(Mutex* m) {
    _freelistLock = m;
    _scanningClosure->set_freelistLock(m);
  }

 private:
  inline void do_yield_check();

  void do_yield_work();
};

// This closure is used to accomplish the sweeping work
// after the second checkpoint but before the concurrent reset
// phase.
// 
// Terminology
//   left hand chunk (LHC) - block of one or more chunks currently being
//     coalesced.  The LHC is available for coalescing with a new chunk.
//   right hand chunk (RHC) - block that is currently being swept that is
//     free or garbage that can be coalesced with the LHC.
// _inFreeRange is true if there is currently a LHC
// _lastFreeRangeCoalesced is true if the LHC consists of more than one chunk.
// _freeRangeInFreeLists is true if the LHC is in the free lists.
// _freeFinger is the address of the current LHC
class SweepClosure: public BlkClosureCareful {
  CMSCollector*                  _collector;  // collector doing the work
  ConcurrentMarkSweepGeneration* _g;	// Generation being swept
  CompactibleFreeListSpace*      _sp;	// Space being swept
  HeapWord*                      _limit;
  Mutex*                         _freelistLock;	// Free list lock (in space)
  CMSBitMap*                     _bitMap;	// Marking bit map (in 
						// generation)
  bool                           _inFreeRange;	// Indicates if we are in the
						// midst of a free run
  bool				 _freeRangeInFreeLists;	
					// Often, we have just found
					// a free chunk and started
					// a new free range; we do not
					// eagerly remove this chunk from
					// the free lists unless there is
					// a possibility of coalescing.
					// When true, this flag indicates
					// that the _freeFinger below
					// points to a potentially free chunk
					// that may still be in the free lists
  bool				 _lastFreeRangeCoalesced;
					// free range contains chunks
					// coalesced
  bool                           _yield;	
					// Whether sweeping should be 
					// done with yields. For instance 
					// when done by the foreground 
					// collector we shouldn't yield.
  HeapWord*                      _freeFinger;	// When _inFreeRange is set, the
						// pointer to the "left hand 
						// chunk"
  size_t			 _freeRangeSize; 
					// When _inFreeRange is set, this 
					// indicates the accumulated size 
					// of the "left hand chunk"
  NOT_PRODUCT(
    size_t		         _numObjectsFreed;
    size_t		         _numWordsFreed;
    size_t			 _numObjectsLive;
    size_t			 _numWordsLive;
    size_t			 _numObjectsAlreadyFree;
    size_t			 _numWordsAlreadyFree;
    FreeChunk*			 _last_fc;
  )
 private:
  // Code that is common to a free chunk or garbage when
  // encountered during sweeping.
  void doPostIsFreeOrGarbageChunk(FreeChunk *fc, 
				  size_t chunkSize);
  // Process a free chunk during sweeping.
  void doAlreadyFreeChunk(FreeChunk *fc);
  // Process a garbage chunk during sweeping.
  size_t doGarbageChunk(FreeChunk *fc);
  // Process a live chunk during sweeping.
  size_t doLiveChunk(FreeChunk* fc);

  // Accessors.
  HeapWord* freeFinger() const	 	{ return _freeFinger; }
  void set_freeFinger(HeapWord* v)  	{ _freeFinger = v; }
  size_t freeRangeSize() const	 	{ return _freeRangeSize; }
  void set_freeRangeSize(size_t v)  	{ _freeRangeSize = v; }
  bool inFreeRange() 	const	 	{ return _inFreeRange; }
  void set_inFreeRange(bool v)  	{ _inFreeRange = v; }
  bool lastFreeRangeCoalesced()	const	 { return _lastFreeRangeCoalesced; }
  void set_lastFreeRangeCoalesced(bool v) { _lastFreeRangeCoalesced = v; }
  bool freeRangeInFreeLists() const	{ return _freeRangeInFreeLists; }
  void set_freeRangeInFreeLists(bool v) { _freeRangeInFreeLists = v; }

  // Initialize a free range.
  void initialize_free_range(HeapWord* freeFinger, bool freeRangeInFreeLists);
  // Return this chunk to the free lists.
  void flushCurFreeChunk(HeapWord* chunk, size_t size);

  // Check if we should yield and do so when necessary.
  inline void do_yield_check(HeapWord* addr);

  // Yield
  void do_yield_work(HeapWord* addr);

  // Debugging/Printing
  void record_free_block_coalesced(FreeChunk* fc) const PRODUCT_RETURN;

 public:
  SweepClosure(CMSCollector* collector, ConcurrentMarkSweepGeneration* g,
               CMSBitMap* bitMap, bool should_yield);
  ~SweepClosure();

  size_t       do_blk_careful(HeapWord* addr);
};

// Closures related to weak references processing
// For objects in the CMS generation, this closure checks
// if the object is "live" (reachable). Used in weak
// reference processing.
class CMSIsAliveClosure: public BoolObjectClosure {
  MemRegion  _span;
  CMSBitMap* _bitMap;
 public:
  CMSIsAliveClosure(MemRegion span,
                    CMSBitMap* bitMap):
    _span(span),
    _bitMap(bitMap) { }
  void do_object(oop obj) {
    assert(false, "not to be invoked");
  }
  bool do_object_b(oop obj);
};

// For objects in CMS generation, this closure marks
// given objects (transitively) as being reachable/live.
class CMSKeepAliveClosure: public OopClosure {
  MemRegion     _span;
  CMSMarkStack* _markStack;
  CMSBitMap*    _bitMap;
 public:
  CMSKeepAliveClosure(MemRegion span,
                      CMSBitMap* bitMap, CMSMarkStack* markStack):
    _span(span),
    _bitMap(bitMap),
    _markStack(markStack) { }

  void do_oop(oop* p);
}; 

// During CMS' weak reference processing, this is a
// work-routine/closure used to complete transitive
// marking of objects as live after a certain point
// in which an initial set has been completely accumulated.
class CMSDrainMarkingStackClosure: public VoidClosure {
  MemRegion            _span;
  CMSMarkStack*        _markStack;
  CMSBitMap*           _bitMap;
  CMSKeepAliveClosure* _oopClosure;
 public:
  CMSDrainMarkingStackClosure(MemRegion span,
                      CMSBitMap* bitMap, CMSMarkStack* markStack,
                      CMSKeepAliveClosure* oopClosure):
    _span(span),
    _bitMap(bitMap),
    _markStack(markStack),
    _oopClosure(oopClosure) { }

  void do_void() {
    while (!_markStack->isEmpty()) {
      oop thisOop = _markStack->pop();
      HeapWord* addr = (HeapWord*)thisOop;
      assert(_span.contains(addr), "Should be within span");
      assert(_bitMap->isMarked(addr), "Should be marked");
      assert(thisOop->is_oop(), "Should be an oop");
      thisOop->oop_iterate(_oopClosure);
    }
  }
};
