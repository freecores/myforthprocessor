#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)javaFrameAnchor_sparc.hpp	1.5 03/01/23 11:01:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

public:

  // Encodings for the value we store in _post_Java_state as part of the
  // frame anchor. At some point we're going to remove  this overloaded
  // bit banging stuff but now is not the time.

  // Modified for _LP64 to tolerate biased and unbiased stack pointers.  
  // Encodings for _post_Java_state.  LSBs are discriminated union.
  // As usual, we depend on the STACK_BIAS value, the fact that %pc values
  // are aligned on 4-byte boundaries and that unbiased %sp, %fp values 
  // are aligned on 16-byte boundaries.
  // X000	: stack value, unbiased, not flushed
  // X001	: stack value, biased, not flushed
  // X100	: stack value, unbiased, flushed
  // X101   : stack value, biased, flushed
  // XX10   : pc value, unflushed
  // XX11	: pc value, flushed
  // -0-    : vacant

  enum {
    is_pc       	= 2,		// discriminates sp vs pc -- visible outside this file!
    sp_has_flushed 	= 4,		// flushed indication for sp values
    pc_has_flushed 	= 1 		// flushed indication for pc values
  };


private:

  intptr_t* volatile _post_Java_state;

public:


  // Each arch must define clear, copy
  // These are used by objects that only care about:
  //  1 - initializing a new state (thread creation, javaCalls)
  //  2 - saving a current state (javaCalls, StateSaver)
  //  3 - restoring an old state (javaCalls, StateSaver)

  void clear(void) {
    // clearing _last_Java_sp must be first
    _last_Java_sp = NULL;
    // fence?
    _not_at_call_id = NULL;
    _post_Java_state = NULL;
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
    _post_Java_state = src->_post_Java_state;
    // Must be last so profiler will always see valid frame if has_last_frame() is true
    _last_Java_sp = src->_last_Java_sp;
  }

  // Is stack walkable
  inline bool walkable( void) {
	uintptr_t js = (uintptr_t) _post_Java_state; 
	return (js & is_pc) ? (js & pc_has_flushed) : (js & sp_has_flushed) ; 
  }

  void make_walkable(bool is_definitely_current_thread, JavaThread* thread);

  inline intptr_t* post_Java_sp() {
    assert(!post_Java_state_is_pc(), "must be a stack pointer");
    intptr_t sp = ((intptr_t) _post_Java_state) & ~(is_pc|sp_has_flushed) ; 
	if (sp & 1) { 		// handle either flavor of stack pointer: biased or unbiased
		sp += STACK_BIAS ; 
	}
    return (intptr_t*) sp ; 
  }

  // Because sparc threads need to scan for a younger sp
  // It is hard to get the last_frame of another thread,
  // so when blocking, cause state to be saved, since it
  // is relatively easy from inside the blocking thread.

  inline address post_Java_pc() {
    assert(post_Java_state_is_pc(), "must be a program counter");
    return (address)((uintptr_t)_post_Java_state & ~(is_pc|pc_has_flushed));
  }

  inline bool post_Java_state_is_pc() {
    return ((uintptr_t)_post_Java_state & is_pc) != 0;
  }

  inline bool post_Java_state_has_flushed() {
	uintptr_t js = (uintptr_t) _post_Java_state ; 
	return (js & is_pc) ? (js & pc_has_flushed) : (js & sp_has_flushed) ; 
  }

  inline void set_post_Java_state_has_flushed(bool flushed) {
    uintptr_t msk = (((uintptr_t) _post_Java_state) & is_pc) ? pc_has_flushed : sp_has_flushed; 
    if (flushed)
      _post_Java_state = ((intptr_t*)((uintptr_t)_post_Java_state | msk));
    else
      _post_Java_state = ((intptr_t*)((uintptr_t)_post_Java_state & ~msk));
  }


  #ifdef ASSERT
  bool has_cached_state() {
    if (post_Java_state_is_pc()) {
      assert(post_Java_pc() != NULL || !post_Java_state_has_flushed(), "bad pc state in post_Java_state");
    }
    else {
      assert(post_Java_sp() != NULL || !post_Java_state_has_flushed(), "bad sp state in post_Java_state");
    }

    return _post_Java_state != NULL && post_Java_state_has_flushed();
  }
  #endif

  // These are only used by friends
private:

  static ByteSize post_Java_state_offset(void) { return byte_offset_of(JavaFrameAnchor, _post_Java_state); }

 // Encodings ... changed needed to tolerate both 64-bit biased and 
 // 64-bit unbiased stack pointer values.
 // Note that we depend on the V9 ABI constraint that stack addresses must 
 // be aligned on 16-byte boundaries.
 // 001 = stack pointer, biased
 // 000 = stack pointer, unbiased
 // x1x = not_at_call marker

  intptr_t* last_Java_sp() const {
    intptr_t sp = (intptr_t) _last_Java_sp;
    if (sp != (intptr_t)NULL) {
      sp &= ~ ((intptr_t) not_at_call_bit);
      if ((sp & 0xF) == 1) {  // raw "biased" stack pointer ?
	sp += STACK_BIAS ;    // convert to address
      }
    }
    return (intptr_t*) sp;
  }

