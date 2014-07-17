#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC_sparc.hpp	1.6 03/01/23 11:00:54 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  address _npc;

 public:
  address npc() { return _npc; }
  ExtendedPC(address pc, address npc) {
    _pc  = pc;
    _npc = npc;
  }
  ExtendedPC() {
    _pc  = NULL;
    _npc = NULL;
  }
  static bool is_contained_in(address pc, address begin, address end);
