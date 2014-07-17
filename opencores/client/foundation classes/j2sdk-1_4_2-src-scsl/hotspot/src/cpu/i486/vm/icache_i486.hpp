#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icache_i486.hpp	1.10 03/01/23 10:54:40 JVM"
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


class ICache : public AbstractICache {
 public:
  static void invalidate_word (u_char* addr)  {};    // for writes up to 4 bytes
  static void invalidate_range(u_char* start, int length) {}

  static void initialize() {}
};

