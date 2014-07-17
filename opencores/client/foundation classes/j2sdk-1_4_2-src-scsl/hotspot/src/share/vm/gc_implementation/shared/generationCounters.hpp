#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)generationCounters.hpp	1.2 03/01/23 12:03:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A GenerationCounter is a holder class for performance counters
// that track a generation

class GenerationCounters: public CHeapObj {
  friend class VMStructs;

 private:
  PerfVariable*      _current_size;
  VirtualSpace*      _virtual_space;

  // Constant PerfData types don't need to retain a reference.
  // However, it's a good idea to document them here.
  // PerfStringConstant*     _name;
  // PerfConstant*           _min_size;
  // PerfConstant*           _max_size;
  // PerfConstant*           _spaces;

  const char*        _name_space;

 public:

  GenerationCounters(const char* name_space, const char* name,
                     int ordinal, int spaces, VirtualSpace* v);

  void update_all() {
    _current_size->set_value(_virtual_space->committed_size());
  }

  const char* name_space() const        { return _name_space; }
};
