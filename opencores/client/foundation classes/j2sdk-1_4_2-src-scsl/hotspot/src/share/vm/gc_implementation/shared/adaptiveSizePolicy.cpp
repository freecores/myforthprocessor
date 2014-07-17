#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)adaptiveSizePolicy.cpp	1.20 03/01/23 12:03:10 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_adaptiveSizePolicy.cpp.incl"

#include <math.h>    

elapsedTimer AdaptiveSizePolicy::_minor_timer;
elapsedTimer AdaptiveSizePolicy::_major_timer;

AdaptiveSizePolicy::AdaptiveSizePolicy(size_t init_eden_size,
                                       size_t init_promo_size,
                                       size_t init_survivor_size,
                                       size_t max_young_gen_size,
                                       size_t max_old_gen_size,
                                       size_t min_heap_alignment) : 
     _eden_size(init_eden_size), 
     _promo_size(init_promo_size),
     _survivor_size(init_survivor_size),
     _threshold_tolerance_percent(1.0 + ThresholdTolerance/100.0),
     _max_young_gen_size(max_young_gen_size),
     _max_old_gen_size(max_old_gen_size),
     _min_heap_alignment(min_heap_alignment),
     _gc_time_limit_exceeded(false),
     _live_at_last_full_gc(init_promo_size)
{
  // Sizing policy statistics
  _avg_minor_pause    = new AdaptiveWeightedAverage(AdaptiveTimeWeight);
  _avg_major_pause    = new AdaptiveWeightedAverage(AdaptiveTimeWeight);
  _avg_minor_interval = new AdaptiveWeightedAverage(AdaptiveTimeWeight);
  _avg_major_interval = new AdaptiveWeightedAverage(AdaptiveTimeWeight);

  _avg_base_footprint = new AdaptiveWeightedAverage(AdaptiveSizePolicyWeight);
  _avg_young_live     = new AdaptiveWeightedAverage(AdaptiveSizePolicyWeight);
  _avg_old_live       = new AdaptiveWeightedAverage(AdaptiveSizePolicyWeight);

  _avg_survived       = new AdaptivePaddedAverage(AdaptiveSizePolicyWeight,
                                                  SurvivorPadding);
  _avg_promoted       = new AdaptivePaddedNoZeroDevAverage(
                                                  AdaptiveSizePolicyWeight,
                                                  SurvivorPadding);
  _avg_pretenured     = new AdaptivePaddedNoZeroDevAverage(
                                                  AdaptiveSizePolicyWeight,
                                                  SurvivorPadding);

  // Start the timers
  _minor_timer.start();
  _major_timer.start();
}

void AdaptiveSizePolicy::minor_collection_begin() {
  // Update the interval time
  _minor_timer.stop();
  _avg_minor_interval->sample(_minor_timer.seconds());
  _minor_timer.reset();
  _minor_timer.start();
}

void AdaptiveSizePolicy::major_collection_begin() {
  // Update the interval time
  _major_timer.stop();
  _avg_major_interval->sample(_major_timer.seconds());
  _major_timer.reset();
  _major_timer.start();
}

void AdaptiveSizePolicy::minor_collection_end() {
  // Update the pause time.
  _minor_timer.stop();
  _avg_minor_pause->sample(_minor_timer.seconds());
  // Interval times use this timer, and include the GC time, so we 
  // don't reset the timer here.
  _minor_timer.start();
}

void AdaptiveSizePolicy::major_collection_end(size_t amount_live) {
  // Update the pause time.
  _major_timer.stop();
  _avg_major_pause->sample(_major_timer.seconds());

  // Update the amount live at the end of a full GC
  _live_at_last_full_gc = amount_live;

  // Interval times use this timer, and include the GC time, so we 
  // don't reset the timer here.
  _major_timer.start();
}

bool AdaptiveSizePolicy::should_full_GC(size_t live_in_old) {
  // We need to do a full GC if the current amount of live
  // data in the old generation is greater than the upper bound
  // we've set of what was live after the last GC plus our 
  // calculated free space for the old generation.
  size_t upper_bound = _live_at_last_full_gc + _promo_size;
  return live_in_old > upper_bound;
}

void AdaptiveSizePolicy::compute_generation_free_space(size_t young_live,
                                                       size_t old_live,
                                                       size_t perm_live,
                                                       bool   is_full_gc) {
  // Update statistics
  // Time statistics are updated as we go, update footprint stats here
  _avg_base_footprint->sample(BaseFootPrintEstimate + perm_live);

  // Note we count both survivor spaces as live, rather than
  // using the argument passed in.
  // NEEDS_CLEANUP explain why, exactly. Why not use used()?
  young_live = 2*calculated_survivor_size_in_bytes();
  _avg_young_live->sample(young_live);
  _avg_old_live->sample(old_live);

  // Until we know better, desired sizes are same as last calculation
  size_t desired_promo_size = _promo_size;
  size_t desired_eden_size  = _eden_size;
  
  // Cache some values. There's a bit of work getting these, so
  // we might save a little time.
  const double major_time = major_gc_time();
  const double minor_time = minor_gc_time();

  // Guard against division by zero through if tests below.
  guarantee(major_time > 0.0, "major time is <= 0.0");
  // Mutator time can go negative if, for example, an application
  // calls System.gc() many times in a row. Because we're dealing
  // with averages, we have to handle growing a little out of bounds.
  if (_promo_size != 0 && mutator_time() > 0.0) {
    desired_promo_size = _promo_size *
                           sqrt(major_time * live_space() / 
                                (_promo_size * mutator_time()));
  }
  // Note that we don't worry about any padding for promotion here.
  // Because this math assumes steady state applications, the computed
  // desired_promo_size should be optimal.

  guarantee(minor_time > 0.0, "minor time is <= 0.0");
  if (_eden_size != 0 && mutator_time() > 0.0) {
    desired_eden_size  = _eden_size *
                           sqrt(minor_time * live_space() / 
                                (_eden_size * mutator_time()));
  }

  // Limits on our growth
  size_t promo_limit = _max_old_gen_size - old_live;
  size_t eden_limit  = _max_young_gen_size - young_live;

  const double gc_time_limit = GCTimeLimit/100.0;

  // Note we make the same tests as in the code block below;  the code
  // seems a little easier to read with the printing in another block.
  if (PrintAdaptiveSizePolicy) {
    if (desired_promo_size > promo_limit)  {
      tty->print_cr("AdaptiveSizePolicy::compute_generation_free_space limits:"
                    " desired_promo_size: " SIZE_FORMAT
                    " promo_limit:" SIZE_FORMAT,
                    desired_promo_size, promo_limit);
    }
    if (desired_eden_size > eden_limit) {
      tty->print_cr("AdaptiveSizePolicy::compute_generation_free_space limits:"
                    " desired_eden_size: " SIZE_FORMAT
                    " old_eden_size: " SIZE_FORMAT
                    " eden_limit:" SIZE_FORMAT,
                    desired_eden_size, _eden_size, eden_limit);
    }
    if (gc_time() > gc_time_limit) {
      tty->print_cr("AdaptiveSizePolicy::compute_generation_free_space limits:"
                    " gc_time: %f "
                    " GCTimeLimit: %d",
                    gc_time(), GCTimeLimit);
    }
  }

  // Adjust our desired promo size and eden size according to some limits
  // Note the printing code above:  it should be changed in synch with
  // any limit test changes.
  // NEEDS_CLEANUP    Handle printing better? Awkward to have block above.
  if (desired_promo_size > promo_limit) {
    if (desired_eden_size > eden_limit) {
      desired_eden_size = eden_limit;

      const size_t total_free_limit = promo_limit + eden_limit;
      const size_t total_mem = _max_old_gen_size + _max_young_gen_size;
      const double mem_free_limit = total_mem * (GCHeapFreeLimit/100.0);
      if (is_full_gc &&
          gc_time() > gc_time_limit && 
          total_free_limit < mem_free_limit) {
        // Our collections, on average, are taking too much time, and
        // we have too little space available after a full gc.
        // The heap can ask us if for the value of this variable when deciding
        // whether to thrown an OutOfMemory error.
        _gc_time_limit_exceeded = true;
      }
    } else {
      // There's still room in Eden.
      // Are we at risk of dividing by zero during the eden size calculation
      //  (the "y" calculation)?
      // Mutator time can go negative if, for example, an application
      // calls System.gc() many times in a row. Because we're dealing
      // with averages, we have to handle growing a little out of bounds.
      if (_eden_size != 0 && mutator_time() > 0.0) {
        // an x or y of 1.0 means don't change. Add 0.0 so we do a floating divide.
        double x = ((_promo_size == 0) ? 1.0 : ((promo_limit + 0.0) / _promo_size));
        double y = sqrt(minor_time * x * (live_space() + _promo_size * x) /
                       (_eden_size * (major_time + x * mutator_time())));
        desired_eden_size = _eden_size * y;
      } else {
        // Leave well enough alone.
        desired_eden_size = _eden_size;
      }
    }
    desired_promo_size = promo_limit;

  } else if (desired_eden_size > eden_limit) {
    // Are we at risk of dividing by zero during the promo size calculation
    //  (the "x" calculation)?
    if (_promo_size != 0 && mutator_time() > 0.0) {
      // an x or y of 1.0 means don't change
      double y = ((_eden_size == 0) ? 1.0 : ((eden_limit + 0.0) / _eden_size));
      double x = sqrt(major_time * y * (live_space() + _eden_size * y) /
                     (_promo_size * (minor_time + y * mutator_time())));
      desired_promo_size = _promo_size * x;
    } else {
      // Leave it unchanged.
      desired_promo_size = _promo_size;
    }
    desired_eden_size  = eden_limit;
  } 

  // Align everything and make a final limit check
  desired_eden_size  = align_size_up(desired_eden_size, _min_heap_alignment);
  desired_eden_size  = MAX2(desired_eden_size, _min_heap_alignment);
  desired_promo_size = align_size_up(desired_promo_size, _min_heap_alignment);
  desired_promo_size = MAX2(desired_promo_size, _min_heap_alignment);

  eden_limit  = align_size_down(eden_limit, _min_heap_alignment);
  promo_limit = align_size_down(promo_limit, _min_heap_alignment);

  // And one last limit check, now that we've aligned things.
  desired_eden_size  = MIN2(desired_eden_size, eden_limit);
  desired_promo_size = MIN2(desired_promo_size, promo_limit);

  if (PrintAdaptiveSizePolicy) {
    // Timing stats
    tty->print("AdaptiveSizePolicy::compute_generation_free_space: "
               " minor_time: %f"
               " major_time: %f", 
               minor_gc_time(), major_gc_time());

    // We give more details if Verbose is set
    if (Verbose) {
      tty->print( " minor_pause: %f"
                  " major_pause: %f"
                  " minor_interval: %f"
                  " major_interval: %f",
                  _avg_minor_pause->average(), 
                  _avg_major_pause->average(),
                  _avg_minor_interval->average(), 
                  _avg_major_interval->average());
    }

    // Footprint stats
    tty->print( " live_space: " SIZE_FORMAT
                " free_space: " SIZE_FORMAT,
                live_space(), free_space());
    // More detail
    if (Verbose) {
      tty->print( " base_footprint: " SIZE_FORMAT
                  " young_live: " SIZE_FORMAT
                  " old_live: " SIZE_FORMAT,
                  (size_t)_avg_base_footprint->average(), 
                  (size_t)_avg_young_live->average(), 
                  (size_t)_avg_old_live->average());
    }
    
    // And finally, our old and new sizes.
    tty->print(" old_promo_size: " SIZE_FORMAT
               " old_eden_size: " SIZE_FORMAT
               " desired_promo_size: " SIZE_FORMAT
               " desired_eden_size: " SIZE_FORMAT,
               _promo_size, _eden_size, 
               desired_promo_size, desired_eden_size);
    tty->cr();
  }

  set_promo_size(desired_promo_size);
  set_eden_size(desired_eden_size);
}


int AdaptiveSizePolicy::compute_survivor_space_size_and_threshold(
                                             size_t survived, 
                                             size_t promoted,
                                             bool is_survivor_overflow,
                                             int tenuring_threshold,
                                             size_t survivor_limit) {
  assert(survivor_limit >= _min_heap_alignment, "survivor_limit too small");
  assert((size_t)align_size_down(survivor_limit, _min_heap_alignment) ==
	 survivor_limit, "survivor_limit not aligned");

  // We'll decide whether to increase or decrease the tenuring
  // threshold based partly on the newly computed survivor size
  // (if we hit the maximum limit allowed, we'll always choose to
  // decrement the threshold).
  bool incr_tenuring_threshold = false;
  bool decr_tenuring_threshold = false;

  if (!is_survivor_overflow) {
    // Keep running averages on how much survived 
    _avg_survived->sample(survived);

    // We use the tenuring threshold to equalize the cost of major
    // and minor collections.
    // ThresholdTolerance is used to indicate how sensitive the 
    // tenuring threshold is to differences in cost betweent the
    // collection types.

    // Get the times of interest. This involves a little work, so
    // we cache the values here.
    const double major_time = major_gc_time();
    const double minor_time = minor_gc_time();

    if (minor_time > major_time * _threshold_tolerance_percent) {
      // Minor times are getting too long;  lower the threshold so
      // less survives and more is promoted.
      decr_tenuring_threshold = true;
    } else if (major_time > minor_time * _threshold_tolerance_percent) {
      // Major times are too long, so we want less promotion.
      incr_tenuring_threshold = true;
    }

  } else {
    // Survivor space overflow occurred, so promoted and survived are
    // not accurate. We'll make our best guess by combining survived
    // and promoted and count them as survivors.
    // 
    // We'll lower the tenuring threshold to see if we can correct
    // things. Also, set the survivor size conservatively. We're
    // trying to avoid many overflows from occurring if defnew size
    // is just too small.
    size_t survived_guess = survived + promoted;
    _avg_survived->sample(survived_guess);

    decr_tenuring_threshold = true;
  }

  // The padded average also maintains a deviation from the average;
  // we use this to see how good of an estimate we have of what survived.
  // We're trying to pad the survivor size as little as possible without
  // overflowing the survivor spaces.
  size_t target_size = align_size_up(_avg_survived->padded_average(), 
                                     _min_heap_alignment);
  target_size = MAX2(target_size, _min_heap_alignment);

  if (target_size > survivor_limit) {
    // Target size is bigger than we can handle. Let's also reduce
    // the tenuring threshold.
    target_size = survivor_limit;
    decr_tenuring_threshold = true;
  }

  // Finally, increment or decrement the tenuring threshold, as decided above.
  // We test for decrementing first, as we might have hit the target size
  // limit.
  if (decr_tenuring_threshold && !(AlwaysTenure || NeverTenure)) {
    if (tenuring_threshold > 1) {
      tenuring_threshold--;
    }
  } else if (incr_tenuring_threshold && !(AlwaysTenure || NeverTenure)) {
    if (tenuring_threshold < MaxTenuringThreshold) {
      tenuring_threshold++;
    }
  }

  // We keep a running average of the amount promoted which is used
  // to decide when we should collect the old generation (when 
  // the amount of old gen free space is less than what we expect to
  // promote).
  _avg_promoted->sample(promoted + _avg_pretenured->padded_average());
 
  if (PrintAdaptiveSizePolicy) {
    tty->print("AdaptiveSizePolicy::compute_survivor_space_size_and_thresh:"
                  "  survived: "  SIZE_FORMAT
                  "  promoted: "  SIZE_FORMAT
                  "  overflow: %s",
                  survived, promoted, is_survivor_overflow ? "true" : "false");
   
    // A little more detail if Verbose is on
    if (Verbose) { 
      tty->print( "  avg_survived: %f"
                  "  avg_deviation: %f",
                  _avg_survived->average(), 
                  _avg_survived->deviation());
    }

    tty->print( "  avg_survived_padded_avg: %f",
                _avg_survived->padded_average());

    if (Verbose) { 
      tty->print( "  avg_promoted_avg: %f"
                  "  avg_promoted_dev: %f",
                  _avg_promoted->average(),
                  _avg_promoted->deviation());
    }

    tty->print( "  avg_promoted_padded_avg: %f"
                "  avg_pretenured_padded_avg: %f"
                "  tenuring_thresh: %d"
                "  target_size: " SIZE_FORMAT,
                _avg_promoted->padded_average(),
                _avg_pretenured->padded_average(),
                tenuring_threshold, target_size);
    tty->cr();
  }

  set_survivor_size(target_size);

  return tenuring_threshold;
}
