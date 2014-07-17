#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)extendedPC_i486.hpp	1.5 03/01/23 10:54:19 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 public:
  ExtendedPC(address pc) { _pc  = pc;   }
  ExtendedPC()           { _pc  = NULL; }
