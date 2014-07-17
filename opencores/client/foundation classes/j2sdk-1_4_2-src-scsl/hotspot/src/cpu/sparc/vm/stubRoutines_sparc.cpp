#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stubRoutines_sparc.cpp	1.42 03/01/23 11:02:24 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_stubRoutines_sparc.cpp.incl"

// Implementation of the platform-specific part of StubRoutines - for
// a description of how to extend it, see the stubRoutines.hpp file.

address StubRoutines::sparc::_test_stop_entry = NULL;
address StubRoutines::sparc::_stop_subroutine_entry = NULL;
address StubRoutines::sparc::_flush_callers_register_windows_entry = NULL;

address StubRoutines::sparc::_copy_words_aligned8_lower_entry = NULL;
address StubRoutines::sparc::_copy_words_aligned8_higher_entry = NULL;
address StubRoutines::sparc::_set_words_aligned8_entry = NULL;
address StubRoutines::sparc::_zero_words_aligned8_entry = NULL;

// %%%%% move these to the common code
address StubRoutines::sparc::_handler_for_divide_by_zero_entry = NULL;
address StubRoutines::sparc::_handler_for_null_exception_entry = NULL;
address StubRoutines::sparc::_handler_for_stack_overflow_entry = NULL;
address StubRoutines::sparc::_handler_for_unsafe_access_entry = NULL;

address StubRoutines::sparc::_partial_subtype_check = NULL;

int StubRoutines::sparc::_atomic_memory_operation_lock = StubRoutines::sparc::unlocked;

int StubRoutines::sparc::_v8_oop_lock_cache[StubRoutines::sparc::nof_v8_oop_lock_cache_entries];


//Reconciliation History
// 1.30 97/12/04 18:27:03 stubRoutines_i486.cpp
// 1.31 98/01/30 16:00:50 stubRoutines_i486.cpp
// 1.36 98/03/10 08:25:26 stubRoutines_i486.cpp
// 1.37 98/04/09 16:49:24 stubRoutines_i486.cpp
// 1.38 98/07/07 13:27:28 stubRoutines_i486.cpp
// 1.40 98/07/31 14:08:19 stubRoutines_i486.cpp
// 1.41 98/08/19 14:30:21 stubRoutines_i486.cpp
// 1.40 98/10/06 14:41:10 stubRoutines_i486.cpp
// 1.42 99/06/22 16:37:57 stubRoutines_i486.cpp
// 1.44 99/08/06 15:47:24 stubRoutines_i486.cpp
//End
