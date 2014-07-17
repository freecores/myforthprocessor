#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaFrameAnchor_i486.hpp	1.6 03/01/23 10:55:03 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

private:

  // FP value associated with _last_Java_sp:
  jint* volatile        _last_Java_fp;           // pointer is volatile not what it points to
  volatile address      _last_native_pc;         // Contains address of last_Java_pc when calling out to native

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
    _last_native_pc = NULL;
  }

  void copy(JavaFrameAnchor* src) {
    // In order to make sure the transition state is valid for "this" 
    // We must clear _last_Java_sp before copying the rest of the new data
    //
    // Hack Alert: Temporary bugfix for 4717480/4721647
    // To act like previous version (pd_cache_state) don't NULL _last_Java_sp
    // unless the value is changing
    //
    if (_last_Java_sp != src->_last_Java_sp)
      _last_Java_sp = NULL;

    _not_at_call_id = src->_not_at_call_id;
    _last_Java_fp = src->_last_Java_fp;
    _last_native_pc = src->_last_native_pc;
    // Must be last so profiler will always see valid frame if has_last_frame() is true
    _last_Java_sp = src->_last_Java_sp;
  }

  // Always walkable
  bool walkable(void) { return true; }
  // Never any thing to do since we are always walkable and can find address of return addresses
  void make_walkable(bool is_definitely_current_thread, JavaThread* thread) { }
  void set_flushed_windows(bool flushed) { }

  // Never let anyone see the bit we hide in the sp
  intptr_t* last_Java_sp(void) const             { return (intptr_t*) ((intptr_t) _last_Java_sp & ~((intptr_t) not_at_call_bit)); }

  address last_Java_pc(void) const               {
						   if (_last_native_pc) return _last_native_pc;
						   if (last_Java_sp()) return (address)(last_Java_sp()[-1]);
						   assert(false, "NULL PC");
						   return NULL;
						 }


  // These are only used by friends. Using the mutators in the wrong order is dangerous.

private: 

  static ByteSize last_Java_fp_offset()          { return byte_offset_of(JavaFrameAnchor, _last_Java_fp); }
  static ByteSize last_native_pc_offset()        { return byte_offset_of(JavaFrameAnchor, _last_native_pc); }

  // Assert (last_Java_sp == NULL || pc == NULL)
  address last_native_pc(void)                   { return _last_native_pc; }
  void set_last_native_pc(address pc)            { _last_native_pc = pc; }

public:


  jint*   last_Java_fp(void)                     { return _last_Java_fp; }
  // Assert (last_Java_sp == NULL || fp == NULL)
  void set_last_Java_fp(jint* fp)                { _last_Java_fp = fp; }



