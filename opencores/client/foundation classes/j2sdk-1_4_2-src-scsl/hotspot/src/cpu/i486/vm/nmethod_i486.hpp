#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod_i486.hpp	1.8 03/01/23 10:55:20 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class nmethod
 private:
  enum { pd_oops_do_enabled_permanently = 1 };
 public:
  // the oops are always exposed, as 32-bit immediates
  bool pd_oops_do_enabled() { return true; }
