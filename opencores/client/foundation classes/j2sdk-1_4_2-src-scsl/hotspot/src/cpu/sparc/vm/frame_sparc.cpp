#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)frame_sparc.cpp	1.133 03/05/14 15:04:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame_sparc.cpp.incl"

#ifndef CORE
void RegisterMap::pd_clear() {
  if (_thread->has_last_Java_frame()) {
    frame fr = _thread->last_frame(); 
    _window = fr.sp();
  } else {
    _window = NULL;
  }
  _younger_window = NULL;
}

#ifdef COMPILER2
address RegisterMap::pd_location(VMReg::Name regname) const {
  assert(0 <= regname && regname < reg_count, "sanity check");
  int enc = Matcher::_regEncode[regname];
  if( enc == 255 ) return NULL; // Unencodable high-half of doubles
  int second_word = 0;
  if( enc > RegisterImpl::number_of_registers ) {
#ifdef _LP64
    enc -= 128;                 // Other half of 64-bit long registers
    assert( enc >= 0, "sanity check on ad file register encoding" );
#else // _LP64
    int index = enc - Matcher::_regEncode[OptoReg::Name(R_O0H_num)];
    enc -= 128;                 // Other half of 64-bit long registers
    assert( enc >= 0, "sanity check on ad file register encoding" );
    assert( enc < RegisterImpl::number_of_registers, "" );
    Register reg = as_Register(enc);
    if (reg->is_out() && 0 <= index && index <= 5 && 
        _thread->is_in_compiled_safepoint()) { 
      // High 32 bits of 64-bit long output registers (O0-O5) are stored in
      // thread-local storage for the 32-bit VM build when we are
      // in a compiled safepoint.
      return _thread->o_reg_temps(index);
    }
#endif // _LP64
    second_word = sizeof(jint);
  }

  assert( enc < RegisterImpl::number_of_registers, "" );
  
  Register reg = as_Register(enc);
  if (reg->is_out()) {
    assert(_younger_window != NULL, "Younger window should be available");
    return second_word + (address)&_younger_window[reg->after_save()->sp_offset_in_saved_window()];
  }
  if (reg->is_local() || reg->is_in()) {
    assert(_window != NULL, "Window should be available");
    return second_word + (address)&_window[reg->sp_offset_in_saved_window()];
  }
  return NULL;
}
#endif // #ifdef COMPILER2


#ifdef COMPILER1
address RegisterMap::pd_location(VMReg::Name regname) const {
  assert(0 <= regname && regname < reg_count, "sanity check");
  if (regname < RegisterImpl::number_of_registers) {
    Register reg = as_Register(regname);
    if (reg->is_out()) {
      assert(_younger_window != NULL, "Younger window should be available");
      return (address)&_younger_window[reg->after_save()->sp_offset_in_saved_window()];
    }
    if (reg->is_local() || reg->is_in()) {
      assert(_window != NULL, "Window should be available");
      return (address)&_window[reg->sp_offset_in_saved_window()];
    }
  }
  return NULL;
}
#endif // COMPILER1


// We are shifting windows.  That means we are moving all %i to %o,
// getting rid of all current %l, and keeping all %g.  This is only
// complicated if any of the location pointers for these are valid.
// The normal case is that everything is in its standard register window
// home, and _location_valid[0] is zero.  In that case, this routine
// does exactly nothing.
void RegisterMap::shift_individual_registers() {
  if (!update_map())  return;  // this only applies to maps with locations

  LocationValidType lv = _location_valid[0];
  LocationValidType lv0 = lv;
  int i;

#ifdef COMPILER1
  const static int R_L_nums[] = {0+020,1+020,2+020,3+020,4+020,5+020,6+020,7+020};
  const static int R_I_nums[] = {0+030,1+030,2+030,3+030,4+030,5+030,6+030,7+030};
  const static int R_O_nums[] = {0+010,1+010,2+010,3+010,4+010,5+010,6+010,7+010};
  const static int R_G_nums[] = {0+000,1+000,2+000,3+000,4+000,5+000,6+000,7+000};
#endif

#ifdef COMPILER2
  const static int R_L_nums[] = {R_L0_num,R_L1_num,R_L2_num,R_L3_num,R_L4_num,R_L5_num,R_L6_num,R_L7_num};
  const static int R_I_nums[] = {R_I0_num,R_I1_num,R_I2_num,R_I3_num,R_I4_num,R_I5_num,R_FP_num,R_I7_num};
  const static int R_O_nums[] = {R_O0_num,R_O1_num,R_O2_num,R_O3_num,R_O4_num,R_O5_num,R_SP_num,R_O7_num};
  const static int R_G_nums[] = {R_G0_num,R_G1_num,R_G2_num,R_G3_num,R_G4_num,R_G5_num,R_G6_num,R_G7_num};
#endif

#ifdef ASSERT
  static int once = 0;
  if (!once++) {
    for (i = 0; i < 8; i++) {
      assert(R_L_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_I_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_O_nums[i] < location_valid_type_size, "in first chunk");
      assert(R_G_nums[i] < location_valid_type_size, "in first chunk");
    }
  }

#ifdef COMPILER2
  static LocationValidType bad_mask = 0;
  if (bad_mask == 0) {
    LocationValidType m = 0;
    m |= (1LL << R_O_nums[6]); // SP
    m |= (1LL << R_O_nums[7]); // cPC
    m |= (1LL << R_I_nums[6]); // FP
    m |= (1LL << R_I_nums[7]); // rPC
    m |= (1LL << R_G_nums[2]); // TLS
    m |= (1LL << R_G_nums[7]); // reserved by libthread
    bad_mask = m;
  }
  assert((lv & bad_mask) == 0, "cannot have special locations for SP,FP,TLS,etc.");
#endif // COMPILER2
#endif // ASSERT

  static LocationValidType R_LIO_mask = 0;
  LocationValidType mask = R_LIO_mask;
  if (mask == 0) {
    for (i = 0; i < 8; i++) {
      mask |= (1LL << R_L_nums[i]);
      mask |= (1LL << R_I_nums[i]);
      mask |= (1LL << R_O_nums[i]);
    }
    R_LIO_mask = mask;
  }

  lv &= ~mask;  // clear %l, %o, %i regs

  // if we cleared some non-%g locations, we may have to do some shifting
  if (lv != lv0) {
    // copy %i0-%i5 to %o0-%o5, if they have special locations
    // This can happen in within stubs which spill argument registers
    // around a dynamic link operation, such as resolve_opt_virtual_call.
    for (i = 0; i < 8; i++) {
      if (lv0 & (1LL << R_I_nums[i])) {
        _location[R_O_nums[i]] = _location[R_I_nums[i]];
        lv |=  (1LL << R_O_nums[i]);
      }
    }
  }

  _location_valid[0] = lv;
}
#endif  // !CORE


// Profiling/safepoint support
bool JavaThread::get_top_frame(frame* _fr, ExtendedPC* _addr, bool for_profile_only) {
  intptr_t*   sp;
  intptr_t*   younger_sp;
  u_char* pc;

  ExtendedPC addr = os::fetch_top_frame(this, &younger_sp, &sp);

  if ((addr.contained_pc() == NULL) || (sp == NULL))
    return false;

  if ((addr.npc() != addr.pc() + 4)) {
    // caught a signal at a delay slot -- bail for safepoints (ok for profiling)
    if (!for_profile_only)
      return false;
  }

  if (_addr != NULL) *_addr = addr;

  frame fr(sp, frame::unpatchable, addr.pc());
  *_fr = fr;

  return true;
}

// Called by the jvmpi profiler to get the sp and pc
// for a thread_in_Java
// Assumes we have already self-suspended the thread so it saved state
// Solaris can not call fetch_top_frame 
// - if called on other  threads will call _thr_getstate 
// which assumes thread is t_stop or stopallmutators, 
// and it is not
bool JavaThread::profile_top_frame(frame* _fr, ExtendedPC* _addr) {
  bool gotframe = false;;
  ThreadState state = osthread()->get_state();

  // self suspension saves needed state.
  if ((is_any_suspended() || state == CONDVAR_WAIT || state == OBJECT_WAIT) 
     && has_last_Java_frame() && _anchor.walkable()) {
     *_fr = pd_last_frame();
     gotframe = true;
     
  }
  return gotframe;
}

bool JavaThread::profile_last_Java_frame(frame* _fr, ExtendedPC* addr) {
  frame youngest;

  // Try to bypass Solaris libthread Bug Id: 4207397 (signals may get lost if
  // the target thread is doing poll()). If in native, get the pd cached state
  // if available instead of interrupting the thread, etc.
  
  if(ProfilerLight) {
    if(thread_state() == _thread_in_native && post_Java_state_is_pc()) {
      // this JavaThread is guaranteed to have been stopped by the profiler
      // so the register windows have been flushed and it is ok to use the 
      // cached state
      _anchor.make_walkable(false, this);
      *_fr = pd_last_frame();
      address    pc = post_Java_pc();
      // Don't really care about npc
      ExtendedPC epc(pc, pc+4);
      *addr = epc;
      return true;
    }
  }

  if (!get_top_frame(&youngest, addr, true))
    return false;

  intptr_t* sp = frame::next_younger_sp_or_null(last_Java_sp(), youngest.sp());
  if (sp == NULL) {
    if (thread_state() == _thread_in_native) {
      // There are two cases: we may or may not have already called the actual
      // native (C) method when the signal was delivered
      if(last_Java_sp() == youngest.sp()) {
        // the native method implementation has not been called yet
        frame fr(last_Java_sp(), frame::unpatchable, addr->pc());
        *_fr = fr;
        return true;
      }
      else {
        // We were interrupted inside C code.
        frame fr(last_Java_sp(), frame::unpatchable, post_Java_pc());
        *_fr = fr;
        return true;
      }
    }
    else {
      return false;
    }
  }

  if (_fr != NULL) { 
    frame fr(last_Java_sp(), sp);
    *_fr = fr;
  }

  return true;
}

bool frame::safe_for_sender(JavaThread *thread) {
  bool safe = false;
  address   sp = (address)_sp;
  if (sp != NULL && 
      (sp <= thread->stack_base() && sp >= thread->stack_base() - thread->stack_size())) {
      safe = true;
  }
  return safe;
}

bool frame::is_interpreted_frame() const  {
  return Interpreter::contains(pc());
}


bool frame::is_entry_frame() const {
  return StubRoutines::returns_to_call_stub(pc());
}


// constructor

frame::frame(intptr_t* sp, intptr_t* younger_sp, intptr_t pc_adjustment, bool younger_frame_is_interpreted) { 
  _sp = sp;
  _younger_sp = younger_sp;
  if (younger_sp == NULL) {
    // make a deficient frame which doesn't know where its PC is
    _pc = NULL;
  } else {
    _pc = (address)younger_sp[I7->sp_offset_in_saved_window()] + pc_return_offset + pc_adjustment;
    assert( (intptr_t*)younger_sp[FP->sp_offset_in_saved_window()] == (intptr_t*)((intptr_t)sp - STACK_BIAS), "younger_sp must be valid");
    // In case of native stubs, the pc retrieved here might be 
    // wrong.  (the _last_native_pc will have the right value)
    // So do not put add any asserts on the _pc here.
  }
  if (younger_frame_is_interpreted) {
#ifdef COMPILER1
    // compute adjustment to this frame's SP made by its interpreted callee
    _interpreter_sp_adjustment = (intptr_t*)younger_sp[IsavedSP->sp_offset_in_saved_window()] - sp;
#endif // COMPILER1
#ifdef COMPILER2
    // adjustment to this frame's SP was saved by its interpreted callee
    _interpreter_sp_adjustment = (intptr_t*)sp[IsavedSP->sp_offset_in_saved_window()] - sp;
#endif
  } else {
    _interpreter_sp_adjustment = 0;
  }
}


// sender_sp

intptr_t* frame::interpreter_frame_sender_sp() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  return fp();
}

void frame::set_interpreter_frame_sender_sp(intptr_t* sender_sp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  Unimplemented();
}


#ifdef ASSERT
// Debugging aid
static frame nth_sender(int n) {
  frame f = JavaThread::current()->last_frame();

  for(int i = 0; i < n; ++i)
    f = f.sender((RegisterMap*)NULL);

  printf("first frame %d\n",          f.is_first_frame()       ? 1 : 0);
  printf("interpreted frame %d\n",    f.is_interpreted_frame() ? 1 : 0);
  printf("java frame %d\n",           f.is_java_frame()        ? 1 : 0);
  printf("entry frame %d\n",          f.is_entry_frame()       ? 1 : 0);
  printf("native frame %d\n",         f.is_native_frame()      ? 1 : 0);
#ifndef CORE
  printf("compiled frame %d\n",       f.is_compiled_frame()    ? 1 : 0);
  printf("deoptimized frame %d\n",    f.is_deoptimized_frame() ? 1 : 0);
#endif

  return f;
}
#endif


frame frame::sender_for_entry_frame(RegisterMap *map) const {
  assert(map != NULL, "map must be set");
  // Java frame called from C; skip all C frames and return top C
  // frame of that chunk as the sender
  JavaFrameAnchor* jfa = entry_frame_call_wrapper()->anchor();
  assert(!entry_frame_is_first(), "next Java fp must be non zero");
  assert(jfa->last_Java_sp() > _sp, "must be above this frame on stack");
  intptr_t* last_Java_sp = jfa->last_Java_sp();
  intptr_t* younger_sp = next_younger_sp(last_Java_sp, sp());
  map->clear(jfa->not_at_call_id());  
#ifndef CORE
  map->make_integer_regs_unsaved();
  map->shift_window(last_Java_sp, younger_sp);
#endif
  assert(map->include_argument_oops(), "should be set by clear");
  return frame(last_Java_sp, younger_sp);
}

frame frame::sender_for_interpreter_frame(RegisterMap *map) const {
  ShouldNotCallThis();
  return sender(map);
}

frame frame::sender_for_raw_compiled_frame(RegisterMap* map) const {
#ifndef CORE
  CodeBlob* stub_cb = CodeCache::find_blob(pc());
  assert(stub_cb != NULL, "wrong pc");

  // Do not adjust the sender_pc if it points into a temporary codebuffer.

  return sender_for_compiled_frame(map, stub_cb, false);
#else
  ShouldNotReachHere();
  return frame();
#endif
}

#ifndef CORE
frame frame::sender_for_compiled_frame(RegisterMap *map, CodeBlob* cb, bool adjusted) const {  
  return sender_with_pc_adjustment(map, NULL, adjusted);
}

frame frame::sender_for_deoptimized_frame(RegisterMap *map, CodeBlob* cb) const {
  ShouldNotCallThis();
  return sender(map);
}
#endif

frame frame::sender(RegisterMap* map, CodeBlob* cb) const {
  return sender_with_pc_adjustment(map, cb, true);
}

frame frame::sender_with_pc_adjustment(RegisterMap* map, CodeBlob* cb, bool adjust_pc) const {
  assert(map != NULL, "map must be set");

  // Default is not to follow arguments; sender_for_xxx will update it accordingly
  map->set_include_argument_oops(false);

  if (cb == NULL && is_entry_frame()) return sender_for_entry_frame(map);

  intptr_t* younger_sp     = sp();
  intptr_t* sp             = sender_sp();
  intptr_t  pc_adjustment  = 0;     // not used in CORE system
  bool      is_interpreted = false; // not used in CORE system

  // Note:  The version of this operation on any platform with callee-save
  //        registers must update the register map (if not null).
  //        In order to do this correctly, the various subtypes of
  //        of frame (interpreted, compiled, glue, native),
  //        must be distinguished.  There is no need on SPARC for
  //        such distinctions, because all callee-save registers are
  //        preserved for all frames via SPARC-specific mechanisms.
  //
  //        *** HOWEVER, *** if and when we make any floating-point
  //        registers callee-saved, then we will have to copy over
  //        the RegisterMap update logic from the Intel code.

#ifndef CORE
  // The constructor of the sender must know whether this frame is interpreted so it can set the
  // sender's _interpreter_sp_adjustment field.  For Compiler1, an osr adapter frame was originally
  // interpreted but its pc is in the code cache (osr_frame_return_id stub), so it must be
  // explicitly recognized. 
  if (Interpreter::contains(pc()) COMPILER1_ONLY(|| is_osr_adapter_frame())) {
    is_interpreted = true;
    COMPILER1_ONLY(map->make_integer_regs_unsaved();
                   map->shift_window(sp, younger_sp);)
  } else {
    // Find a CodeBlob containing this frame's pc or elide the lookup and use the
    // supplied blob which is already known to be associated with this frame.
    assert(cb == NULL || (!cb->caller_must_gc_arguments(map->thread()) && !map->include_argument_oops()),
           "Assumed that we don't need "
           "map->set_include_argument_oops(cb->caller_must_gc_arguments(map->thread())) "
           "(for sake of performance)");
    if (cb == NULL) {
      cb = CodeCache::find_blob(pc());
    }
    if (cb != NULL) {
      if (adjust_pc) {
        address sender_pc_0 = this->sender_pc();
        address sender_pc = map->thread()->safepoint_state()->compute_adjusted_pc(sender_pc_0);
        // Adjust the sender_pc if it points into a temporary codebuffer.      
        pc_adjustment = sender_pc - sender_pc_0;
      }
      if (cb->caller_must_gc_arguments(map->thread())) {
        map->set_include_argument_oops(true);
      }

      // Update the locations of implicitly saved registers to be their addresses in the register save area.
      // For %o registers, the addresses of %i registers in the next younger frame are used.
      map->shift_window(sp, younger_sp);
      if (map->update_map()) {
        if (is_deoptimized_frame()) {
          map->thread()->vframe_array_for(this)->update_register_map(map);
          map->make_integer_regs_unsaved();
        } else if (cb->oop_maps() != NULL) {
          OopMapSet::update_register_map(this, cb, map);
        }
      }
    }
  }
#endif
  return frame(sp, younger_sp, pc_adjustment, is_interpreted);
}

// Adjust index for the way a compiler frame is layed out in SPARC

#ifndef CORE
int frame::adjust_offset(methodOop method, int index) {

  int size_of_parameters = method->size_of_parameters();
  assert (size_of_parameters >= 0, "illegal number of parameters in method");

  nmethod *code = method->code();

  if (method->is_native())
  {
     return local_offset_for_compiler(index, size_of_parameters, code->max_nof_locals(), code->max_nof_monitors());
  }
  else  {
    if ( index >= code->max_nof_locals() ) {    // local bound to a register
      Register reg = as_Register((index - code->max_nof_locals()) + RegisterImpl::obase);

      assert(!reg->is_global(), "invalid register type");
      if ( !reg->is_out() )     // %l or %i register
        return (reg->sp_offset_in_saved_window() - (fp() - sp()));
      else                      // %o register
        Unimplemented();
    }
    else
      return local_offset_for_compiler(index, size_of_parameters, code->max_nof_locals(), code->max_nof_monitors());
  }
}
#endif


void frame::patch_pc(Thread* thread, address pc) {
  if(thread == Thread::current()) {
   StubRoutines::sparc::flush_callers_register_windows_func()();
  } 
  if (TracePcPatching) {
    assert(_pc == *O7_addr() + pc_return_offset, "frame has wrong pc");
    tty->print_cr("patch_pc at address  0x%x [0x%x -> 0x%x] ", O7_addr(), _pc, pc);
  }
  _pc = pc;
  *O7_addr() = pc - pc_return_offset;
}


static bool sp_is_valid(intptr_t* old_sp, intptr_t* young_sp, intptr_t* sp) {
  return (((intptr_t)sp & (2*wordSize-1)) == 0 && 
          sp <= old_sp && 
          sp >= young_sp);
}

intptr_t* frame::next_younger_sp_or_null(intptr_t* old_sp, intptr_t* young_sp, intptr_t* sp) {
  if (young_sp == NULL) {
    assert(Thread::current()->is_Java_thread(), "Must be Java thread");
    young_sp = StubRoutines::sparc::flush_callers_register_windows_func()();
    young_sp = (intptr_t *)((intptr_t)young_sp + STACK_BIAS);
  }

  if (sp == NULL)
    sp = young_sp;

  intptr_t* previous_sp = NULL;

  int max_frames = (old_sp - sp) / 16; // Minimum frame size is 16
  while(sp != old_sp && sp_is_valid(old_sp, young_sp, sp)) {
    if (max_frames-- <= 0) 
      // too many frames have gone by; invalid parameters given to this function 
      break; 
    previous_sp = sp;
    sp = (intptr_t*)sp[FP->sp_offset_in_saved_window()];
    sp = (intptr_t*)((intptr_t)sp + STACK_BIAS);
  }

  return (sp == old_sp ? previous_sp : NULL);
}


bool frame::interpreter_frame_equals_unpacked_fp(intptr_t* fp) {
  assert(is_interpreted_frame(), "must be interpreter frame");
  return this->fp() == fp;
}


void frame::pd_gc_epilog() {
  if (is_interpreted_frame()) {
    // set constant pool cache entry for interpreter
    methodOop m = interpreter_frame_method();

    *interpreter_frame_cpoolcache_addr() = m->constants()->cache();
  }
}


bool frame::is_interpreted_frame_valid() const {
  assert(is_interpreted_frame(), "Not an interpreted frame");
  // These are reasonable sanity checks
  if (fp() == 0 || (intptr_t(fp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  if (sp() == 0 || (intptr_t(sp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  const interpreter_frame_initial_sp_offset = interpreter_frame_vm_local_words;
  if (fp() + interpreter_frame_initial_sp_offset < sp()) {
    return false;
  }
  // These are hacks to keep us out of trouble.
  // The problem with these is that they mask other problems
  if (fp() <= sp()) {        // this attempts to deal with unsigned comparison above
    return false;
  }
  if (fp() - sp() > 4096) {  // stack frames shouldn't be large.
    return false;
  }
  return true;
}


#ifndef CORE
void frame::set_sender_pc_for_unpack(frame caller, int exec_mode) {
  set_sender_pc(caller.pc());
}


#ifdef COMPILER1
void frame::patch_for_deoptimization(JavaThread* thread, frame callee, address deopt_handler) {
  
  // Return to the deoptimization blob
  patch_pc(thread, deopt_handler);

  // This frame is treated like an interpreter frame now, so record a valid value for IsavedSP.
  // Since it didn't start out as an interpreter frame, the IsavedSP is simply the callers real sp
  // which is the fp of this frame.
  // 
  // NOTE: must be done after patch_pc since patch_pc will force a late window flush
  // and that is needed to make sure a stale IsavedSP doesn't overwrite the updated
  // value.

  sp()[IsavedSP->sp_offset_in_saved_window()] = (intptr_t)fp();
}


address frame::frameless_stub_return_addr() {
  return (address)(sp()[I5->sp_offset_in_saved_window()]);
}


void frame::patch_frameless_stub_return_addr(Thread* thread, address return_addr) {
  if (thread == Thread::current()) {
    StubRoutines::sparc::flush_callers_register_windows_func()();
  }
  sp()[I5->sp_offset_in_saved_window()] = (intptr_t)return_addr;
}
#endif // COMPILER1
#endif

#ifdef COMPILER2
int frame::pd_compute_variable_size(int frame_size_in_words, CodeBlob *code) {
  return frame_size_in_words;
}
#endif


//Reconciliation History
// 1.3 97/06/27 08:21:54 frame_i486.cpp
// 1.27 97/10/02 16:13:28 frame_i486.cpp
// 1.35 97/11/06 17:14:27 frame_i486.cpp
// 1.36 97/12/11 15:57:26 frame_i486.cpp
// 1.37 98/02/27 15:52:50 frame_i486.cpp
// 1.38 98/03/05 17:16:54 frame_i486.cpp
// 1.41 98/03/27 14:47:21 frame_i486.cpp
// 1.43 98/04/30 16:37:39 frame_i486.cpp
// 1.56 98/05/06 13:47:01 frame_i486.cpp
// 1.57 98/05/07 12:17:24 frame_i486.cpp
// 1.59 98/05/11 16:35:53 frame_i486.cpp
// 1.63 98/05/21 13:46:20 frame_i486.cpp
// 1.67 98/05/26 15:54:03 frame_i486.cpp
// 1.70 98/05/28 12:53:39 frame_i486.cpp
// 1.71 98/06/01 12:43:04 frame_i486.cpp
// 1.84 98/06/12 16:05:30 frame_i486.cpp
// 1.85 98/06/20 12:03:48 frame_i486.cpp
// 1.87 98/06/23 14:25:20 frame_i486.cpp
// 1.88 98/06/26 09:48:39 frame_i486.cpp
// 1.92 98/07/24 15:29:47 frame_i486.cpp
// 1.93 98/08/13 13:24:30 frame_i486.cpp
// 1.94 98/08/24 16:21:02 frame_i486.cpp
// 1.95 98/09/03 19:04:09 frame_i486.cpp
// 1.97 98/10/02 10:58:45 frame_i486.cpp
// 1.97 98/10/06 14:22:37 frame_i486.cpp
// 1.100 98/10/23 16:18:40 frame_i486.cpp
// 1.101 98/11/09 20:24:28 frame_i486.cpp
// 1.107 98/11/10 14:35:33 frame_i486.cpp
// 1.108 98/11/11 08:52:20 frame_i486.cpp
// 1.110 98/11/15 18:22:54 frame_i486.cpp
// 1.111 98/11/20 14:56:00 frame_i486.cpp
// 1.113 98/11/23 11:23:40 frame_i486.cpp
// 1.114 99/01/05 13:55:55 frame_i486.cpp
// 1.116 99/01/11 14:47:47 frame_i486.cpp
// 1.117 99/01/20 17:39:24 frame_i486.cpp
// 1.118 99/01/31 18:57:46 frame_i486.cpp
// 1.121 99/03/10 15:39:59 frame_i486.cpp
// 1.124 99/06/02 18:02:05 frame_i486.cpp
// 1.127 99/06/28 09:57:42 frame_i486.cpp
// 1.124 99/04/01 16:52:27 frame_i486.cpp
// 1.126 99/04/13 11:57:12 frame_i486.cpp
// 1.128 99/07/06 16:49:15 frame_i486.cpp
// 1.130 99/08/05 16:48:25 frame_i486.cpp
// 1.130 99/09/04 16:05:25 frame_i486.cpp
//End
