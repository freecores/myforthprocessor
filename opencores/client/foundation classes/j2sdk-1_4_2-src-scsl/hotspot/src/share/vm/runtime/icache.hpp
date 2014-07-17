#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icache.hpp	1.9 03/01/23 12:22:58 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for updating the instruction cache.  Whenever the VM modifies
// code, part of the processor instruction cache potentially has to be flushed.

// On the x86, this is a no-op -- the I-cache is guaranteed to be consistent
// after the next jump, and the VM never modifies instructions directly ahead
// of the instruction fetch path.

// Nevertheless, use this interface from all code that is not x86-specific.


// default implementations
class AbstractICache : AllStatic {
 public:
  static void invalidate_word (u_char* addr);    // for writes up to 4 bytes
  static void invalidate_range(u_char* start, int nBytes);

  static void initialize() {}
};

// overriden per platform, e.g.,
//
// 
//class ICache : public AbstractICache {
// public:
//  static void invalidate_word (u_char* addr);    // for writes up to 4 bytes
//  static void invalidate_range(u_char* start, int nBytes);
//
//  static void initialize();
//};
//
//
// The code must be in machine-specific file, i.e., icache_<arch>.hpp/.cpp

#include "incls/_icache_pd.hpp.incl"
