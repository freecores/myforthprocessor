#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)stubRoutines_i486.hpp	1.55 03/01/23 10:55:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This file holds the platform specific parts of the StubRoutines
// definition. See stubRoutines.hpp for a description on how to
// extend it.

enum platform_dependent_constants {
  code_size1 =  9000,		// simply increase if too small (assembler will crash if too small)
  code_size2 = 22000		// simply increase if too small (assembler will crash if too small)
};

class i486 {
 friend class StubGenerator;

 private:
  static address _handler_for_divide_by_zero_entry;
  static address _handler_for_null_exception_entry;
  static address _handler_for_stack_overflow_entry;  
  static address _handler_for_unsafe_access_entry;  
  static address _get_previous_fp_entry;  

 public:
  // Call back points for traps in compiled code
  static address handler_for_divide_by_zero()                { return _handler_for_divide_by_zero_entry; }
  static address handler_for_null_exception()                { return _handler_for_null_exception_entry; }
  static address handler_for_stack_overflow()                { return _handler_for_stack_overflow_entry; }
  static address handler_for_unsafe_access()                 { return _handler_for_unsafe_access_entry; }
  static address get_previous_fp_entry()                     { return _get_previous_fp_entry; }
};

