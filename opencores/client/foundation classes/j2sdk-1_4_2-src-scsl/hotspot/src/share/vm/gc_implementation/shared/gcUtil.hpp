#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcUtil.hpp	1.5 03/01/23 12:03:42 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Catch-all file for utility classes

// A weighted average maintains a running, weighted average
// of some float value (templates would be handy here if we
// need different types).
//
// The average is adaptive in that we smooth it for the 
// initial samples; we don't use the weight until we have
// enough samples for it to be meaningful.
//
// This serves as our best estimate of a future unknown.
//
class AdaptiveWeightedAverage : public CHeapObj {
 private: 
  float            _average;        // The last computed average
  unsigned         _sample_count;   // How often we've sampled this average
  const unsigned   _weight;         // The weight used to smooth the averages
                                    //   A higher weight favors the most
                                    //   recent data.

 protected:
  void  increment_count()       { _sample_count++;       }
  void  set_average(float avg)  { _average = avg;        }

  // Helper function, computes an adaptive weighted average 
  // given a sample and the last average
  float compute_adaptive_average(float new_sample, float average);

 public:
  // Input weight must be between 0 and 100
  AdaptiveWeightedAverage(unsigned weight) : 
    _average(0.0), _sample_count(0), _weight(weight) {
    assert (weight >= 0 && weight <= 100, "invalid argument");
  }

  // Accessors
  float    average() const       { return _average;       }
  unsigned weight()  const       { return _weight;        }
  unsigned count()   const       { return _sample_count;  }

  // Update data with a new sample.
  void sample(float new_sample);
};


// A weighted average that includes a deviation from the average,
// some multiple of which is added to the average.
//
// This serves as our best estimate of an upper bound on a future
// unknown.
class AdaptivePaddedAverage : public AdaptiveWeightedAverage {
 private:
  float    _padded_avg;           // The last computed padded average 
  float    _deviation;            // Running deviation from the average
  const unsigned _padding;        // A multiple which, added to the average,
                                  // gives us an upper bound guess.

 protected:
  void set_padded_average(float avg)  { _padded_avg = avg;  }
  void set_deviation(float dev)       { _deviation  = dev;  }

 public:
  AdaptivePaddedAverage(unsigned weight, unsigned padding) :
    AdaptiveWeightedAverage(weight), 
    _padded_avg(0.0), _deviation(0.0), _padding(padding) {}

  // Accessor
  float padded_average() const         { return _padded_avg; }
  float deviation()      const         { return _deviation;  }
  unsigned padding()     const         { return _padding;    }

  // Override
  void  sample(float new_sample);
};

// A weighted average that includes a deviation from the average,
// some multiple of which is added to the average.
//
// This serves as our best estimate of an upper bound on a future
// unknown.
// A special sort of padded average:  it doesn't update deviations
// if the sample is zero. The average is allowed to change. We're
// preventing the zero samples from drastically changing our padded
// average.
class AdaptivePaddedNoZeroDevAverage : public AdaptivePaddedAverage {
public:
  AdaptivePaddedNoZeroDevAverage(unsigned weight, unsigned padding) :
    AdaptivePaddedAverage(weight, padding)  {}
  // Override
  void  sample(float new_sample);
};
