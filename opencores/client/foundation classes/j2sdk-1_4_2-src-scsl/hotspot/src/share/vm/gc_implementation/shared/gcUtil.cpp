#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcUtil.cpp	1.5 03/01/23 12:03:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcUtil.cpp.incl"

// Catch-all file for utility classes

float AdaptiveWeightedAverage::compute_adaptive_average(float new_sample, 
                                                        float average) {
  // We smooth the samples by not using weight() directly until we've
  // had enough data to make it meaningful. We'd like the first weight
  // used to be 1, the second to be 1/2, etc until we have 100/weight
  // samples.
  unsigned count_weight = 100/count();
  float adaptive_weight = (float)(MAX2(weight(), count_weight))/100;

  float new_avg = (adaptive_weight * new_sample) + 
                  ((1 - adaptive_weight) * average);

  return new_avg;
}

void AdaptiveWeightedAverage::sample(float new_sample) {
  increment_count();

  // Compute the new weighted average
  float new_avg = compute_adaptive_average(new_sample, average());
  set_average(new_avg);
}

void AdaptivePaddedAverage::sample(float new_sample) {
  // Compute our parent classes sample information
  AdaptiveWeightedAverage::sample(new_sample);

  // Now compute the deviation and the new padded sample
  float new_avg = average();
  float new_dev = compute_adaptive_average(labs(new_sample - new_avg),
                                           deviation());

  set_deviation(new_dev);
  set_padded_average(new_avg + padding() * new_dev);
}

void AdaptivePaddedNoZeroDevAverage::sample(float new_sample) {
  // Compute our parent classes sample information
  AdaptiveWeightedAverage::sample(new_sample);

  float new_avg = average();
  if (new_sample != 0) {
    // We only create a new deviation if the sample is non-zero
    float new_dev = compute_adaptive_average(labs(new_sample - new_avg),
                                             deviation());

    set_deviation(new_dev);
  }
  set_padded_average(new_avg + padding() * deviation());
}
