#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)thread_solaris_i486.hpp	1.10 03/01/23 11:10:10 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  void pd_initialize()                           { _anchor.clear(); }

  frame pd_last_frame() {
    assert(has_last_Java_frame(), "must have last_Java_sp() when suspended");
    if (_anchor.last_native_pc() != NULL) {
      return frame(_anchor.last_Java_sp(), _anchor.last_Java_fp(), _anchor.last_native_pc());
    } else {
      // This will pick up pc from sp
      return frame(_anchor.last_Java_sp(), _anchor.last_Java_fp());
    }
  }

 public:
  // Mutators are highly dangerous....
  jint* last_Java_fp()                           { return _anchor.last_Java_fp(); }
  void  set_last_Java_fp(jint* fp)               { _anchor.set_last_Java_fp(fp);   }
  address last_native_pc()                       { return _anchor.last_native_pc(); }
  void  set_last_native_pc(address pc)           { _anchor.set_last_native_pc(pc);   }

  void set_base_of_stack_pointer(jint* base_sp)  {}

  static ByteSize last_Java_fp_offset()          { 
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_Java_fp_offset();
  }
  static ByteSize last_native_pc_offset()        {
    return byte_offset_of(JavaThread, _anchor) + JavaFrameAnchor::last_native_pc_offset();
  }

  jint* base_of_stack_pointer()                  { return NULL; }
  void record_base_of_stack_pointer()            {}

  // debugging support
  static frame current_frame_guess() { 
    jint* fp = (*CAST_TO_FN_PTR( jint* (*)(void), StubRoutines::i486::get_previous_fp_entry()))();
    // fp points to the frame of the ps stub routine
    frame f(NULL, fp, (address)NULL);
    RegisterMap map(JavaThread::current(), false);
    return f.sender(&map);
  }

  int pd_get_fast_thread_id() {
    return (int) ThreadLocalStorage::thread();
  }

  // These routines are only used on cpu architectures that
  // have separate register stacks (Itanium).
  static bool register_stack_overflow() { return false; }
  static void enable_register_stack_guard() {}
  static void disable_register_stack_guard() {}

