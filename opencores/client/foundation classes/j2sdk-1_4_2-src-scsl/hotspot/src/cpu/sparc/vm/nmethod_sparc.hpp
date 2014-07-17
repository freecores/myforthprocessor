#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)nmethod_sparc.hpp	1.8 03/01/23 11:01:56 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class nmethod
 private:
  enum { pd_oops_do_enabled_permanently = 0 };
 public:
  bool pd_oops_do_enabled() { return _oops_do_enabled; }
