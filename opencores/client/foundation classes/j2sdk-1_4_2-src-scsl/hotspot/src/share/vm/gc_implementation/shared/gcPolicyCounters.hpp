#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcPolicyCounters.hpp	1.2 03/01/23 12:03:37 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// GCPolicyCounters is a holder class for performance counters
// that track a generation

class GCPolicyCounters: public CHeapObj {
  friend class VMStructs;

  private:

    // Constant PerfData types don't need to retain a reference.
    // However, it's a good idea to document them here.
    // PerfStringConstant*     _name;
    // PerfStringConstant*     _collector_size;
    // PerfStringConstant*     _generation_size;

    const char* _name_space;

  public:

    GCPolicyCounters(const char* name_space, const char* name,
                     int collectors, int generations);

    const char* name_space() const            { return _name_space; }
};
