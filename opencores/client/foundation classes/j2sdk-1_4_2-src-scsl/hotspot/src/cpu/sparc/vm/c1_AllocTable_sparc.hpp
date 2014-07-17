#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_AllocTable_sparc.hpp	1.6 03/01/23 10:59:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:
  enum {
    max = 32
  };

  static void init_tables ();

 private:
  // the statics are initialized with init_reg_table
  static intx _reg_mask[max];

  void reg_check (int rnr) const      { assert(0 <= rnr && rnr < _size, "wrong register number"); }

  bool pd_has_double_free () const    { return has_pair_free(); }
  void pd_set_double_locked (int rnr) { set_pair_locked(rnr); }
  int pd_get_double_free ()           { return get_pair_free(); }

