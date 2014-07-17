#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)icache_ia64.hpp	1.5 03/01/23 10:57:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface for updating the instruction cache.  Whenever the VM modifies
// code, part of the processor instruction cache potentially has to be flushed.

class ICache : public AbstractICache {
 public:
  static void invalidate_word(u_char* addr);
  static void invalidate_range(u_char* start, int nBytes);
  static void initialize();
};
