#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)templateTable_sparc.hpp	1.12 03/01/23 11:02:34 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

  // helper function
  static void invokevfinal_helper(Register Rcache, Register Rret);
  static void invokeinterface_object_method(Register RklassOop, Register Rcall,
                                            Register Rret,
                                            Register Rflags);
  static void generate_vtable_call(Register Rrecv, Register Rindex, Register Rret);
  static void volatile_barrier(Assembler::Membar_mask_bits order_constraint);
