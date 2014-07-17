#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocInfo_i486.hpp	1.11 03/01/23 10:55:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // machine-dependent parts of class relocInfo
 private:
  enum {
    // Intel instructions are byte-aligned.
    offset_unit        =  1,

    // Encodes Assembler::disp32_operand vs. Assembler::imm32_operand.
    format_width       =  1
  };
