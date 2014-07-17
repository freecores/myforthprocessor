#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaFrameAnchor.hpp	1.5 03/01/23 12:23:26 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
//
// An object for encapsulating the machine/os dependent part of a JavaThread frame state
//
class JavaThread;

class JavaFrameAnchor VALUE_OBJ_CLASS_SPEC {
// Too many friends...
friend class JavaThread;
friend class frame;
friend class VMStructs;
friend class cInterpreter;
friend class JavaCallWrapper;

 private:
   enum Constants {
     not_at_call_bit  = 2                        // Used to mark existent of a not_at_call frame
   };

  //
  // Whenever _last_Java_sp != NULL other anchor fields MUST be valid!
  // The stack may not be walkable [check with walkable() ] but the values must be valid.
  // The profiler apparently depends on this.
  //
  intptr_t* volatile _last_Java_sp;
  intptr_t* volatile _not_at_call_id;

  // tells whether the last Java frame is set
  // It is important that when last_Java_sp != NULL that the rest of the frame
  // anchor (including platform specific) all be valid.

  bool has_last_Java_frame() const                   { return _last_Java_sp != NULL; }
  // This is very dangerous unless sp == NULL
  // Invalidate the anchor so that has_last_frame is false
  // and no one should look at the other fields.
  void zap(void)                                     { _last_Java_sp = NULL; }
  // Only implement on ia64 for c++ interpreter. Every other use ought to be zap/clear
  // void  set_last_Java_sp(intptr_t* sp)               {  _last_Java_sp = sp;  _cache.reset(); }

  
  // scope-desc/oopmap lookup 
  void  set_pc_not_at_call_for_frame(intptr_t* id)      {
    _not_at_call_id = id;
    set_bits((intptr_t&)_last_Java_sp, not_at_call_bit);
  }

  bool  not_at_call_frame_exists() const                { return ((intptr_t)_last_Java_sp) & (intptr_t)not_at_call_bit; }

  intptr_t* not_at_call_id() const                      { return not_at_call_frame_exists() ? (intptr_t*)_not_at_call_id : NULL; }

  // Clear an anchor state this must be done in an order so that we never report
  // a valid frame (has_last_frame) while the clear is in progress
  // inline void clear(void);
  // inline void copy(JavaFrameAnchor* src);

  // Is the stack described by this anchor walkable
  // inline bool walkable(void);
  // make  the anchor walkable based on the current thread's execution stack (a sparc convenience)
  // void make_walkable(bool is_definitely_current_thread, JavaThread* thread);


  // Assembly stub generation helpers

  static ByteSize last_Java_sp_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_sp); }

#include "incls/_javaFrameAnchor_pd.hpp.incl"

public:
  JavaFrameAnchor()                              { clear(); }
  JavaFrameAnchor(JavaFrameAnchor *src)          { copy(src); }
};

