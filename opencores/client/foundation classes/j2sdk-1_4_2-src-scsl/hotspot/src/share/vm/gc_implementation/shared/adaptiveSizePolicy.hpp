#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)adaptiveSizePolicy.hpp	1.18 03/01/23 12:03:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The AdaptiveSizePolicy class implements the adaptive heap size
// policy designed by David Stoutamire.
// The design document is at doc/gc-interface/AdaptivePolicy.Design.txt
// in this workspace.
//
// This class keeps statistical information and computes the
// optimal free space for both the young and old generation
// based on current application characteristics (based on gc cost
// and application footprint).
//
// It also computes an optimal tenuring threshold between the young
// and old generations, so as to equalize the cost of collections
// of those generations, as well as optimial survivor space sizes
// for the young generation.
//
// While this class is specifically intended for a generational system
// consisting of a young gen (containing an Eden and two semi-spaces)
// and a tenured gen, as well as a perm gen for reflective data, it
// makes NO references to specific generations.

// Forward decls
class elapsedTimer;

class AdaptiveSizePolicy : public CHeapObj {
 private:
  // Last calculated sizes, in bytes, and aligned
  // NEEDS_CLEANUP should use sizes.hpp,  but it works in ints, not size_t's
  size_t _eden_size;        // calculated eden free space in bytes
  size_t _promo_size;       // calculated old gen free space in bytes
  size_t _survivor_size;    // calculated survivor size in bytes

  // Major and minor collection timers, used to determine both
  // pause and interval times for collections
  static elapsedTimer _minor_timer; 
  static elapsedTimer _major_timer;

  // Time statistics
  AdaptiveWeightedAverage* _avg_minor_pause;
  AdaptiveWeightedAverage* _avg_major_pause;
  AdaptiveWeightedAverage* _avg_minor_interval;
  AdaptiveWeightedAverage* _avg_major_interval;

  // Footprint statistics
  AdaptiveWeightedAverage* _avg_base_footprint;
  AdaptiveWeightedAverage* _avg_young_live;
  AdaptiveWeightedAverage* _avg_old_live;

  // Statistics for survivor space calculation for young generation
  AdaptivePaddedAverage*   _avg_survived;
 
  // Avg amount promoted; used for avoiding promotion undo
  // This class does not update deviations if the sample is zero.
  AdaptivePaddedNoZeroDevAverage*   _avg_promoted;
  // Objects that have been directly allocated in the old generation.
  AdaptivePaddedNoZeroDevAverage*   _avg_pretenured;

  size_t _survivor_size_limit;   // Limit in bytes of survivor size
  // Allowed difference between major and minor gc times, used
  // for computing tenuring_threshold.
  const double _threshold_tolerance_percent;

  const size_t _min_heap_alignment;    // min heap alignment for this collector
  const size_t _max_young_gen_size;    // max young gen size for this collector
  const size_t _max_old_gen_size;      // max old gen size for this collector

  // This is a hint for the heap:  we've detected that gc times
  // are taking longer than GCTimeLimit allows.
  bool _gc_time_limit_exceeded; 

  // The amount of live data in the heap at the last full GC, used
  // as a baseline to help us determine when we need to perform the
  // next full GC.
  size_t _live_at_last_full_gc;

 protected:
  // Time accessors

  // The value returned is unitless:  it's the proportion of time
  // spent in a particular collection type.
  // Never allow an estimate to go below 1%, in case of long pauses
  // (perhaps idle time at a GUI app?) or when one type of collection
  // dominates (perhaps because of many successive System.gc() calls).
  // An interval time will be 0.0 if a collection type hasn't occurred yet.
  double minor_gc_time() const {
    const double minor_interval = _avg_minor_interval->average();
    double result =  (minor_interval == 0.0) ? 0.01 :
                        _avg_minor_pause->average() / minor_interval;
    return MAX2(0.01, result);
  }
  double major_gc_time() const {
    const double major_interval = _avg_major_interval->average();
    double result = (major_interval == 0.0) ? 0.01 :
                       _avg_major_pause->average() / major_interval;
    return MAX2(0.01, result);
  }

  // Because we're dealing with averages, gc_time() can be
  // larger than 1.0 (thus mutator_time() can go negative).
  double gc_time() const { 
    return minor_gc_time() + major_gc_time();
  }

  double mutator_time() const { 
    return 1.0 - gc_time(); 
  }

  // Footprint accessors
  size_t live_space() const { 
    return _avg_base_footprint->average() +
           _avg_young_live->average() +
           _avg_old_live->average();  
  }
  size_t free_space() const { 
    return _eden_size + _promo_size; 
  }

  void set_eden_size(size_t new_size) { 
    _eden_size = new_size; 
  }
  void set_promo_size(size_t new_size) { 
    _promo_size = new_size;
  }
  void set_survivor_size(size_t new_size) { 
    _survivor_size = new_size; 
  }

 public:
  // Input arguments are initial free space sizes for young and old
  // generations, the initial survivor space size, and the minimum
  // heap alignment required for this collector.
  //
  // NEEDS_CLEANUP this is a singleton object
  AdaptiveSizePolicy(size_t init_eden_size, 
                     size_t init_promo_size, 
                     size_t init_survivor_size, 
                     size_t max_young_gen_size,
                     size_t max_old_gen_size,
                     size_t min_heap_alignment);

  // Methods indicating events of interest to the adaptive size policy,
  // called by GC algorithms. It is the responsibility of users of this
  // policy to call these methods at the correct times!
  void minor_collection_begin();
  void major_collection_begin();
  void minor_collection_end();
  void major_collection_end(size_t amount_live);

  // 
  void tenured_allocation(size_t size) {
    _avg_pretenured->sample(size);
  }

  // Accessors
  // NEEDS_CLEANUP   should use sizes.hpp
  size_t calculated_eden_size_in_bytes() const { 
    return _eden_size; 
  }
  size_t calculated_promo_size_in_bytes() const { 
    return _promo_size;
  }
  size_t calculated_survivor_size_in_bytes() const { 
    return _survivor_size; 
  }

  size_t calculated_old_free_size_in_bytes() const {
    return _promo_size + _avg_promoted->padded_average();
  }

  size_t average_promoted_in_bytes() const {
    return (size_t)_avg_promoted->average();
  }

  // Given the amount of live data in the heap, should we
  // perform a Full GC?
  bool should_full_GC(size_t live_in_old_gen);

  // Calculates optimial free space sizes for both the old and young
  // generations.  Stores results in _eden_size and _promo_size.
  // Takes current used space in all generations as input, as well
  // as an indication if a full gc has just been performed, for use
  // in deciding if an OOM error should be thrown.
  void compute_generation_free_space(size_t young_live,
                                     size_t old_live,
                                     size_t perm_live,
                                     bool   is_full_gc);

  // Calculates new survivor space size;  returns a new tenuring threshold
  // value. Stores new survivor size in _survivor_size.
  int compute_survivor_space_size_and_threshold(size_t survived, 
                                                size_t promoted,
                                                bool   is_survivor_overflow,
                                                int    tenuring_threshold,
                                                size_t survivor_limit);

  // Return the maximum size of a survivor space if the young generation were of
  // size gen_size.
  size_t max_survivor_size(size_t gen_size) {
    // Never allow the target survivor size to grow more than MinSurvivorRatio
    // of the young generation size.  We cannot grow into a two semi-space
    // system, with Eden zero sized.  Even if the survivor space grows, from()
    // might grow by moving the bottom boundary "down" -- so from space will
    // remain almost full anyway (top() will be near end(), but there will be a
    // large filler object at the bottom).
    const size_t sz = gen_size / MinSurvivorRatio;
    const size_t alignment = _min_heap_alignment;
    return sz > alignment ? align_size_down(sz, alignment) : alignment;
  }

  // This is a hint for the heap:  we've detected that gc times
  // are taking longer than GCTimeLimit allows.
  // Most heaps will choose to throw an OutOfMemoryError when
  // this occurs but it is up to the heap to request this information
  // of the policy
  bool gc_time_limit_exceeded() {
    return _gc_time_limit_exceeded;
  }
};
