#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_solaris_sparc.hpp	1.57 03/05/09 11:28:12 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
private:

  void pd_initialize() {
    _anchor.clear();
    _base_of_stack_pointer        = NULL;
  }

  inline intptr_t* post_Java_sp() { return _anchor.post_Java_sp(); }
  inline address   post_Java_pc() { return _anchor.post_Java_pc(); }

  inline bool post_Java_state_is_pc() { return _anchor.post_Java_state_is_pc(); }

  frame pd_last_frame() {
    assert(has_last_Java_frame(), "must have last_Java_sp() when suspended");
    assert(_anchor.walkable(), "thread has not dumped its register windows yet");

    if (_anchor.post_Java_state_is_pc())
      return frame(last_Java_sp(), frame::unpatchable, post_Java_pc());
    else
      return frame(last_Java_sp(), post_Java_sp());
  }

  // Sometimes the trap handler needs to record both PC and NPC.
  // This is a SPARC-specific companion to Thread::set_saved_exception_pc.
  address _saved_exception_npc;

  // In illegal_instruction_handler_blob we have to tail call other
  // blobs without blowing any registers.  A tail call requires some
  // register to jump with and we can't blow any registers, so it must
  // be restored in the delay slot.  'restore' cannot be used as it
  // will chop the heads off of 64-bit %o registers in the 32-bit
  // build.  Instead we reload the registers using G2_thread and this
  // location.  Must be 64bits in the 32-bit LION build.
  jdouble _o_reg_temps[6];
  jlong   _ccr_temp;

  // a stack pointer older than any java frame stack pointer.  It is
  // used to validate stack pointers in frame::next_younger_sp (it
  // provides the upper bound in the range check).  This is necessary
  // on Solaris/SPARC since the ucontext passed to a signal handler is
  // sometimes corrupt and we need a way to check the extracted sp.
  intptr_t* _base_of_stack_pointer;

public:

  static ByteSize post_Java_state_offset_in_bytes() { 
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::post_Java_state_offset();
  }

  static int o_reg_temps_offset_in_bytes() { return (intptr_t)&((JavaThread*)NULL)->_o_reg_temps; }
  static int ccr_temp_offset_in_bytes() { return (intptr_t)&((JavaThread*)NULL)->_ccr_temp; }

#ifdef COMPILER2
#ifndef _LP64
  address o_reg_temps(int i) { return (address)&_o_reg_temps[i]; }
#endif
#endif

  static int saved_exception_npc_offset_in_bytes() { return (intptr_t)&((JavaThread*)NULL)->_saved_exception_npc; }

  address  saved_exception_npc()             { return _saved_exception_npc; }
  void set_saved_exception_npc(address a)    { _saved_exception_npc = a; }


public:

  intptr_t* base_of_stack_pointer() { return _base_of_stack_pointer; }

  void set_base_of_stack_pointer(intptr_t* base_sp) { 
    _base_of_stack_pointer = base_sp; 
  }

  void record_base_of_stack_pointer() { 
    intptr_t *sp = (intptr_t *)(((intptr_t)StubRoutines::sparc::flush_callers_register_windows_func()()) + STACK_BIAS);  
    intptr_t *ysp;
    while((ysp = (intptr_t*)sp[FP->sp_offset_in_saved_window()]) != NULL) {
      sp = (intptr_t *)((intptr_t)ysp + STACK_BIAS);
    }
    _base_of_stack_pointer = sp;
  }

  static frame current_frame_guess() { 
    intptr_t* sp = StubRoutines::sparc::flush_callers_register_windows_func()(); 
    sp = (intptr_t *)((intptr_t)sp + STACK_BIAS);
    frame fr(sp, frame::unpatchable, (address)NULL); 
    return fr; 
  }

# ifdef ASSERT
  virtual bool state_is_saved_before_block() {
    return  (jvmdi::enabled()) || 
            (thread_state() == _thread_new_trans) || 
            (thread_state() == _thread_new) || 
	    // This seems redundant...
            (_anchor.has_cached_state() && _anchor.post_Java_state_has_flushed());
  }
# endif

  intptr_t pd_get_fast_thread_id() {
    return (intptr_t) ThreadLocalStorage::thread();
  }

  // For Forte Analyzer AsyncGetCallTrace profiling support - thread is currently interrupted by SIGPROF
  bool pd_get_top_frame_from_ucontext(frame* fr_addr, ucontext_t* uc, bool youngest_frame) {
    intptr_t* youngest_sp;
    assert(Thread::current() == this, "only current thread can flush its own register windows");

    ExtendedPC addr = os::Solaris::fetch_frame_from_ucontext(this, uc, &youngest_sp);
    if (addr.contained_pc() != NULL && youngest_sp != NULL) {
      frame youngest(youngest_sp, frame::unpatchable, addr.pc());
      if (youngest_frame) {
        *fr_addr = youngest;
        return true;
      }

      intptr_t* sp = frame::next_younger_sp_or_null(last_Java_sp(), youngest.sp());
      if (sp == NULL) {
        if (thread_state() == _thread_in_native) {
          // There are two cases: we may or may not have already called the actual
          // native (C) method when the signal was delivered
          if (last_Java_sp() == youngest.sp()) {
            // the native method implementation has not been called yet
            frame fr(last_Java_sp(), frame::unpatchable, addr.pc());
            *fr_addr = fr;
            return true;
          } else {
            // We were interrupted inside C code.
            frame fr(last_Java_sp(), frame::unpatchable, post_Java_pc());
            *fr_addr = fr;
            return true;
          }
        }
      } else {
        frame fr(last_Java_sp(), sp);
        *fr_addr = fr;
        return true;
      }
    }

    // Fail to fetch top frame from ucontext;
    // Return the top Java frame if available
    if (has_last_Java_frame()) {
      _anchor.make_walkable(true, (JavaThread*) this);
      *fr_addr = pd_last_frame();
      return true;
    }
    return false;
  }

  // These routines are only used on cpu architectures that
  // have separate register stacks (Itanium).
  static bool register_stack_overflow() { return false; }
  static void enable_register_stack_guard() {}
  static void disable_register_stack_guard() {}

