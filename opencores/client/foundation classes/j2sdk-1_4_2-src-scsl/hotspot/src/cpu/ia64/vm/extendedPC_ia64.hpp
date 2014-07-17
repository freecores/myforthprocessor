#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC_ia64.hpp	1.3 03/01/23 10:57:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:
  ExtendedPC(address pc) { _pc  = pc;   }
  ExtendedPC()           { _pc  = NULL; }
