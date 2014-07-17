#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo_ia64.hpp	1.3 03/01/23 10:58:27 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // IA64 instructions are 128bit-aligned.
    offset_unit        =  16,

    // Encodes Instruction slot (at least, may need nore bits)
    format_width       =  2
  };
