#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaFrameAnchor_ia64.hpp	1.4 03/01/23 10:57:54 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
private:

  // FP (BSP) value associated with _last_Java_sp:
  intptr_t* volatile _last_Java_fp;
  // return link associated with _last_Java_sp:
  volatile address _last_Java_pc;
  volatile jint _flushed_windows;	// Have the register windows been flushed

public:


  // Each arch must define reset, save, restore 
  // These are used by objects that only care about:
  //  1 - initializing a new state (thread creation, javaCalls)
  //  2 - saving a current state (javaCalls, StateSaver)
  //  3 - restoring an old state (javaCalls, StateSaver)

  void clear(void) {
    // clearing _last_Java_sp must be first
    _last_Java_sp = NULL;
    // fence?
    _not_at_call_id = NULL;
    _last_Java_fp = NULL;
    _last_Java_pc = NULL;
    _flushed_windows = false;
  }

  void copy(JavaFrameAnchor* src) {
    // In order to make sure the transition state is valid for "this" 
    // We must clear _last_Java_sp before copying the rest of the new data
    // Hack Alert: Temporary bugfix for 4717480/4721647
    // To act like previous version (pd_cache_state) don't NULL _last_Java_sp
    // unless the value is changing
    //
    if (_last_Java_sp != src->_last_Java_sp)
      _last_Java_sp = NULL;
    _not_at_call_id = src->_not_at_call_id;
    _last_Java_fp = src->_last_Java_fp;
    _last_Java_pc = src->_last_Java_pc;
    _flushed_windows = src->_flushed_windows;
    // Must be last so profiler will always see valid frame if has_last_frame() is true
    _last_Java_sp = src->_last_Java_sp;
  }

  // Not always walkable
  bool walkable(void) { return _flushed_windows; }

  void make_walkable(bool is_definitely_current_thread, JavaThread* thread);

  // These are only used by friends
private: 

  static ByteSize last_Java_fp_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_fp); }
  static ByteSize last_Java_pc_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_pc); }
  static ByteSize flushed_windows_offset()       { return byte_offset_of(JavaFrameAnchor, _flushed_windows); }

public:

  void set_last_Java_sp(intptr_t* sp)            { _last_Java_sp = sp; }
  intptr_t* last_Java_sp(void) const             { return (intptr_t*) _last_Java_sp; }

  void set_last_Java_pc(address pc)              { _last_Java_pc = pc; }
  address last_Java_pc(void)                     { return _last_Java_pc; }

  void set_last_Java_fp(intptr_t* fp)            { _last_Java_fp = fp; }
  intptr_t* last_Java_fp(void)                   { return _last_Java_fp; }

  void set_flushed_windows(bool flushed)         { _flushed_windows = flushed; }
  bool flushed_windows(void)                     { return _flushed_windows; }
