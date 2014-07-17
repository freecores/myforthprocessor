#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaCalls_ia64.hpp	1.5 03/01/23 10:57:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public:
  // %%% will clean this up with a pd-specific abstraction in thread.hpp
  intptr_t*        _last_Java_fp;           // the saved last_Java_fp
  address          _last_Java_pc;           // the saved last_Java_pc
  intptr_t*        last_Java_fp() const       { return _last_Java_fp; }
  address          last_Java_pc() const       { return _last_Java_pc; }
  void             set_last_Java_fp(intptr_t* fp) { _last_Java_fp = fp;   }
  void             set_last_Java_pc(address pc) { _last_Java_pc = pc;   }

private:
  // For the profiler, the last_Java_frame information in thread must always be in
  // legal state. Illegal state is if last_Java_sp != NULL and last_Java_fp == NULL.
  // To make the transition state also legal, we either set first last_Java_sp (to NULL)
  // or set last_Java_fp first (to non NULL)
  // set the cached state after setting the last java sp
  // because setting the last java sp clears the cached state

  // Platform dependent thread state push/pop for constructor/destructor use
  //
  void pd_wrapper_push(void) {
    assert(_thread != NULL, "_thread must already be set");
    _last_Java_pc = _thread->last_Java_pc();
    _thread->set_last_Java_pc(NULL);
  }
  void pd_wrapper_pop(void) {
    assert(_thread != NULL, "_thread must already be set");
    _thread->set_last_Java_pc(_last_Java_pc);
    _last_Java_pc = NULL;
  }
