#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines_ia64.cpp	1.7 03/01/23 10:58:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines_ia64.cpp.incl"


// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

address StubRoutines::ia64::_flush_register_stack_entry                      = NULL;

address StubRoutines::ia64::_handler_for_divide_by_zero_entry                = NULL;
address StubRoutines::ia64::_handler_for_null_exception_entry                = NULL;
address StubRoutines::ia64::_handler_for_stack_overflow_entry                = NULL;

address StubRoutines::ia64::_get_previous_fp_entry                           = NULL;
address StubRoutines::ia64::_partial_subtype_check                           = NULL;
address StubRoutines::ia64::_jni_call_trampoline                             = NULL;
address StubRoutines::ia64::_get_backing_store_pointer                       = NULL;


