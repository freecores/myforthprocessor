#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)frame_ia64.cpp	1.23 03/02/07 11:29:33 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame_ia64.cpp.incl"


void RegisterMap::pd_clear() {
  if (_thread->has_last_Java_frame()) {
    frame fr = _thread->last_frame(); 
    _bsp = fr.fp();
  } else {
    _bsp = NULL;
  }
  _extra1 = NULL;
  _extra2 = NULL;
  _extra3 = NULL;
#ifdef ASSERT
    for(int i = 0; i < reg_count; i++) {
      _location[i] = NULL;
    }
#endif
}

void RegisterMap::pd_initialize_from(const RegisterMap* map) {
  _bsp = map->_bsp;
  _extra1 = map->_extra1;
  _extra2 = map->_extra2;
  _extra3 = map->_extra3;

  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::pd_initialize() {
  _bsp = NULL;
  _extra1 = NULL;
  _extra2 = NULL;
  _extra3 = NULL;

  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::make_integer_regs_unsaved(void) {
  // We have more 96 stacked + 32 unstacked register to worry about.
  _location_valid[0] = 0;
  _location_valid[1] = 0;
}

void RegisterMap::shift_window(frame& caller, frame &callee) {
  _bsp = caller.fp();
  // This will cause is to 
  if (_location_valid[0] != 0 || _location_valid[1] != 0) shift_individual_registers(caller, callee);
}

// We are shifting windows.  That means we are moving all INs to OUTs.
// This is simple by comparison on sparc where the sizes of the windows
// is fixed. On IA64 the INs of the callee frame become OUTs in the caller
// but the register number is determined by the value in the frame
// mask cfm/pfs. Basically we always hope that _location_valid[0..1]
// are zero so we can skip this entire action.
void RegisterMap::shift_individual_registers(frame& caller, frame& callee) {
  if (!update_map())  return;  // this only applies to maps with locations

  uintptr_t lv[2];
  lv[0] = (uintptr_t) _location_valid[0];
  lv[1] = (uintptr_t) _location_valid[1];
  int i;
  BitMap orig(lv, 128);

  // The number of in's in the callee (current map) is the
  // number of out's in the caller
  int caller_size_of_frame = frame::size_of_frame((uint64_t) *callee.register_addr(GR_Lsave_PFS));
  int caller_size_of_locals = frame::size_of_locals((uint64_t) *callee.register_addr(GR_Lsave_PFS));
  int out_count = caller_size_of_frame - caller_size_of_locals;
  
  // Assume that reg 0..31 are already valid (or clear). Need a mask to clear
  // the rest

  static uintptr_t R_static_mask = 0;
  uintptr_t mask1 = R_static_mask;
  if (mask1 == 0) {
    BitMap bits(&mask1, 64);
    for (i = 0; i < 32; i++) {
      bits.set_bit(i);
    }
    R_static_mask = mask1;
  }

  _location_valid[0] &= mask1;
  _location_valid[1] = 0;
  BitMap updated((uintptr_t*) _location_valid, 128);

  // Get the first register number of the new output registers

  int out_base_idx = 32 + caller_size_of_locals;
  address locations[96]; // At most 96 stacked locations

  // Copy the output locations to temporary space while setting the
  // new valid bits
  for ( i = 0 ; i < out_count ; i++ ) {
    // Is the input register valid??
    if (orig.at(i + 32)) {
      // Save the location and mark output register as valid
      locations[i] = _location[i + 32];
      updated.set_bit(out_base_idx + i);
    }
  }

  // Now move the locations to their final spot

  for ( i = 0 ; i < out_count ; i++ ) {
    if (updated.at(out_base_idx + i)) {
      _location[out_base_idx + i] = locations[i];
    }
  }

#ifdef ASSERT
  // Verify no valid bits are on in the stacked register bit range except the in's -> out's
  // we just moved
  for ( i = 32 ; i < out_base_idx ; i++) {
    assert(!updated.at(i), "Invalid bit");
  }

  for ( i = out_count ; i < 128 - out_base_idx ; i ++ ) {
    assert(!updated.at( i+ out_base_idx), "Invalid bit");
  }
#endif    
}

#ifndef CORE
address RegisterMap::pd_location(VMReg::Name regname) const {
  //
  // If regname is a windowed register find the rse location of it.
  // If not windowed then return NULL.
  // Unlike sparc where the window storage location is split across
  // caller and callee (i.e. to find a caller's outs we must find
  // the callee's ins). On ia64 the this is simpler (well something
  // had to be) since all the windowed registers are contiguous
  // (well except for NATs) so we can find any frames registers
  // just by knowing its bsp.
  //
  assert(0 <= regname && regname < reg_count, "sanity check");
  if (regname < RegisterImpl::number_of_registers) {
    Register reg = as_Register((int)Matcher::_regEncode[regname]);
    if (reg->is_stacked()) {
      assert(_bsp != NULL, "Window should be available");
      return (address) frame::register_addr(reg, _bsp);
    }
    return NULL;
  }
  return NULL;
}


#endif /* !CORE */


// Profiling/safepoint support
bool JavaThread::get_top_frame(frame* _fr, ExtendedPC* _addr, bool for_profile_only) {

  intptr_t*   sp;
  intptr_t*   fp;
  address*    interp_pc;

  // TODO:  For now, only return top frames for profiling.  We don't
  //        have support for compiled safepoints so this false return
  //        is guarding that path.
  if (!for_profile_only)
    return false;

  ExtendedPC addr = os::fetch_top_frame(this, &sp, &fp);

  if ((addr.contained_pc() == NULL) || (sp == NULL))
    return false;

  if (_addr != NULL) *_addr = addr;

  // We now need to determine if we were in the interpreter or not.
  // The check for is_interpreted_frame returns false for time
  // spent in interpreter subroutines outside of the interpreter main
  // loop.  To work around this, we check for all other conditions
  // and then adjust the reported pc to trick the fprofiler code.
  //
  // If it is an interpreter frame, we need to get the thread saved
  // fp since we can't access istate asynchronously.

  if ( InterpreterGenerator::is_interpreter_return(addr.pc()) ) {
    fp = last_interpreter_fp();  
    if ( fp == NULL ) return false;
    frame fr(sp, fp);
    fr.set_pc(addr.pc());
    *_fr = fr;
    return true;
  }

#ifndef CORE
  if (CodeCache::contains(addr.pc()) || 
      (VtableStubs::stub_containing(addr.pc()) != NULL))  {
    frame fr(sp, fp);
    fr.set_pc(addr.pc());
    *_fr = fr;
    return true;
  }
#endif

  // All other tests failed so assume that we were in the 
  // cInterpreter called subroutines.
  //
  // By setting the newly created frame's pc to an interpreter
  // address, the is_interpreted_frame routine will return true
  // and the profile tick will appear in the correct bucket.
  // See interpreter_ia64.hpp  "Interpreter::contains(pc)" routine.

  fp = last_interpreter_fp();  
  if ( fp == NULL ) return false;
  frame fr(sp, fp);
  interp_pc = CAST_FROM_FN_PTR(address*, InterpretMethodDummy);
  fr.set_pc((*interp_pc) + frame::pc_return_offset);
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
  return false;
#if 0
  bool gotframe = false;;
  ThreadState state = osthread()->state();

  // self suspension saves needed state.
  if ((state == SUSPENDED || state == CONDVAR_WAIT) 
     && has_last_Java_frame() && pd_post_Java_state_has_flushed()) {
     *_fr = pd_last_frame();
     gotframe = true;
     
  }
  return gotframe;
#endif
}

bool JavaThread::profile_last_Java_frame(frame* _fr, ExtendedPC* addr) {
  if (frame_anchor()->walkable()) {
      frame fr = pd_last_frame();
      *_fr = fr;
      ExtendedPC epc(fr.pc());
      *addr = epc;
      return true;
    }
    return false;
}

bool frame::safe_for_sender(JavaThread *thread) {
  bool safe = false;
  address   cursp = (address)sp();
  address   curfp = (address)fp();
  if ((cursp != NULL && curfp != NULL && 
      (cursp <= thread->stack_base() && cursp >= thread->stack_base() - thread->stack_size())) &&
      (curfp <= thread->stack_base() && curfp >= thread->stack_base() - thread->stack_size())) {
      safe = true;
  }
  return safe;
}

bool frame::is_interpreted_frame() const  {
  return InterpreterGenerator::is_interpreter_return(pc());
}

bool frame::is_entry_frame() const {
  return StubRoutines::returns_to_call_stub(pc());
}



// constructor

#if 0
frame::frame(intptr_t* sp, intptr_t* younger_sp, intptr_t pc_adjustment, bool younger_frame_is_interpreted) { 
  _raw_sp = sp;
  _younger_sp = younger_sp;
  if (younger_sp == NULL) {
    // make a deficient frame which doesn't know where its PC is
    _pc = NULL;
  } else {
    _pc = (address)younger_sp[I7->sp_offset_in_saved_window()] + pc_return_offset + pc_adjustment;
    assert( (intptr_t*)younger_sp[FP->sp_offset_in_saved_window()] == (intptr_t*)((intptr_t)sp - STACK_BIAS), "younger_sp must be valid");
    // In case of native stubs, the pc retreived here might be 
    // wrong. (the _last_native_pc will have the right value)
    // So do not put add any asserts on the _pc here.
  }
  if (younger_frame_is_interpreted) {
#ifdef CC_INTERP
    // compute adjustment to this frame's SP made by its interpreted callee
    interpreterState istate = (interpreterState) younger_sp[Lstate->sp_offset_in_saved_window()];
    // unfortunately in regular interpreted frame (i.e. not native entry code) the caller's original
    // sp while it is stored in the initial state object it isn't reachable here because we have
    // the initial "ignored" dummy frame. So we have to grab it from a register we know about.
    // QQQ we hate this.
    intptr_t* orig_sp = (intptr_t*)younger_sp[I4->sp_offset_in_saved_window()]; // QQQ remove at some point
//     assert(istate->_save_sp == orig_sp, " bad saved sp");
//    _interpreter_sp_adjustment = istate->_saved_sp - sp;
    _interpreter_sp_adjustment = orig_sp -  sp;
    
#else
#ifdef COMPILER1
    // compute adjustment to this frame's SP made by its interpreted callee
    _interpreter_sp_adjustment = (intptr_t*)younger_sp[IsavedSP->sp_offset_in_saved_window()] - sp;
#endif // COMPILER1
#ifdef COMPILER2
    // adjustment to this frame's SP was saved by its interpreted callee
    _interpreter_sp_adjustment = (intptr_t*)sp[IsavedSP->sp_offset_in_saved_window()] - sp;
#endif
#endif /* CC_INTERP */
  } else {
    _interpreter_sp_adjustment = 0;
  }
}

#endif /* 0 */


// sender_sp

intptr_t* frame::interpreter_frame_sender_sp() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  return (intptr_t*) (*register_addr(GR_Lsave_SP));
}

void frame::set_interpreter_frame_sender_sp(intptr_t* sender_sp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  Unimplemented(); // From SPARC, we can do this...
  // x86 would do this
  *register_addr(GR_Lsave_SP) = (intptr_t) sender_sp;
}

static bool contains_nat(intptr_t* from, intptr_t* to) { return frame::contains_nat_collection(from, to); }

intptr_t* frame::link() const {
  // This ought to be doable for all of our frames
  intptr_t* bsp = (intptr_t*) *register_addr(GR_Lsave_BSP);
  assert(bsp == fp(), "Bad bsp?");
  uint64_t fsize = size_of_locals((uint64_t) *register_addr(GR_Lsave_PFS));
  intptr_t* result = bsp - fsize;
  if (fsize > 63) {
    assert(false, "2 nat collections possible !");
  } else {
    if (contains_nat(result, bsp)) result--;
  }

  return result;

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
  intptr_t* last_Java_sp = jfa->last_Java_sp();
  intptr_t* last_Java_fp = jfa->last_Java_fp();
  address last_Java_pc = jfa->last_Java_pc();
  assert(last_Java_sp > _raw_sp, "must be above this frame on stack");
  assert(last_Java_fp < _bsp, "RSE must be below this frame on stack");
  map->clear(jfa->not_at_call_id());  
  frame caller_frame(last_Java_sp, last_Java_fp, NULL, last_Java_pc);
#ifndef CORE
  map->make_integer_regs_unsaved();
  // HACK
  frame callee(*this);
  map->shift_window(caller_frame, callee);
#endif
  assert(map->include_argument_oops(), "should be set by clear");
  return caller_frame;
}

frame frame::sender_for_interpreter_frame(RegisterMap *map) const {

  // SPARC did not call this because of interpreter_sp_adjustment concerns
  
  frame caller(sender_sp(), link(), sender_pc_addr(), sender_pc());

  // What about the map? seems like at the very least we should do
  map->make_integer_regs_unsaved();
  // HACK
  frame callee(*this);
  map->shift_window(caller, callee);
  return caller;
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

  assert(map != NULL, "map must be set");

  // frame owned by  compiler 

  address* pc_addr = compiled_sender_pc_addr(cb);
  address pc = *pc_addr;

  if (adjusted) {
    // Adjust the sender_pc if it points into a temporary codebuffer.      
    pc = map->thread()->safepoint_state()->compute_adjusted_pc(pc);
  }

  frame caller(compiled_sender_sp(cb), compiled_sender_fp(cb), pc_addr, pc);

  // Now adjust the map

  // HACK
  frame callee(*this);
  // Move callee ins to caller outs
  map->shift_window(caller, callee);

  // Get the rest
  if (map->update_map()) {
    if (is_deoptimized_frame()) {
      map->thread()->vframe_array_for(this)->update_register_map(map);
      map->make_integer_regs_unsaved();
    } else if (cb->oop_maps() != NULL) {
      OopMapSet::update_register_map(this, cb, map);
    }
  }
  //  if (map->update_map() && cb->oop_maps() != NULL) {
  //    OopMapSet::update_register_map(this, cb, map);
  //  }

  return caller;
}

frame frame::sender_for_deoptimized_frame(RegisterMap *map, CodeBlob* cb) const {
  // SPARC does this
#if 0
  ShouldNotCallThis();
  return sender(map);
#endif
  // Until I know better.
  return sender_for_compiled_frame(map, cb, false);
}

intptr_t* frame::compiled_sender_sp(CodeBlob* cb) const {
  // SPARC/X86 don't do this
  if (is_deoptimized_frame()) {
    // QQQ really needs to find the original cb and use it's frame
    return (intptr_t*) (*register_addr(GR_Lsave_SP));
  } else {
    return sp() + cb->frame_size();
  }
  // return (intptr_t*) (*register_addr(GR_Lsave_SP));
}

address*   frame::compiled_sender_pc_addr(CodeBlob* cb) const {
  // SPARC/X86 don't do this
  return (address*) (register_addr(GR_Lsave_RP));
}

intptr_t* frame::compiled_sender_fp(CodeBlob* cb) const {
  // This needs to be in link() once we're convinced it works properly here
  // and for all of our frames.

  // intptr_t* bsp = (intptr_t*) *register_addr(GR_Lsave_BSP);
  intptr_t* bsp = fp();
  uint64_t fsize = size_of_locals((uint64_t) *register_addr(GR_Lsave_PFS));
  intptr_t* sender_bsp = bsp - fsize;
  if (fsize > 63) {
    assert(false, "2 nat collections possible !");
  } else {
    if (contains_nat(sender_bsp, bsp)) sender_bsp--;
  }
  return sender_bsp;
}

#endif

frame frame::sender(RegisterMap* map, CodeBlob* cb) const {

  // Default is we done have to follow them. The sender_for_xxx will
  // update it accordingly
  map->set_include_argument_oops(false);

  if (is_entry_frame())       return sender_for_entry_frame(map);
  if (is_interpreted_frame()) return sender_for_interpreter_frame(map);
#ifdef COMPILER1
  // Note: this test has to come before CodeCache::find_blob(pc())
  //       call since the code for osr adapter frames is contained
  //       in the code cache, too!
  if (is_osr_adapter_frame()) return sender_for_interpreter_frame(map);
#endif // COMPILER1
#ifndef CORE  
  if(cb == NULL) {
    cb = CodeCache::find_blob(pc());
  } else {
    assert(cb == CodeCache::find_blob(pc()),"Must be the same");
  }

  if (cb  != NULL) {
    // Deoptimized frame?
    if (is_deoptimized_frame()) {
      return sender_for_deoptimized_frame(map, cb);
    } else {
      // Returns adjusted pc if it was pointing into a temp. safepoint codebuffer.
      return sender_for_compiled_frame(map, cb, true);      
    }
  }
#endif // CORE
  // Must be native-compiled frame, i.e. the marshaling code for native
  // methods that exists in the core system.
  // link() will blow here since not interpreted....
  return frame(sender_sp(), link(), NULL, sender_pc());
}

frame frame::sender_with_pc_adjustment(RegisterMap* map, CodeBlob* cb, bool adjust_pc) const {

  // QQQ expect to delete this

  ShouldNotCallThis();
#if 0
  // SPARC only
  assert(map != NULL, "map must be set");

  // Default is not to follow arguments; sender_for_xxx will update it accordingly
  map->set_include_argument_oops(false);

  if (cb == NULL && is_entry_frame()) return sender_for_entry_frame(map);

  return frame(sender_sp(), link(), sender_pc());

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
#ifdef CC_INTERP
  /* the normally ignored frame has to be recognized here because it is the frame where the sp adjustment 
     happened. Yuck */
  is_interpreted = is_interpreted_frame() || is_ignored_frame() COMPILER1_ONLY(|| is_osr_adapter_frame());
#else
  is_interpreted =  is_interpreted_frame() COMPILER1_ONLY(|| is_osr_adapter_frame());
#endif /* CC_INTERP */
  if (is_interpreted) {
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
#else
  // The Windows compiler insists that we return something.
  return frame(NULL, NULL, NULL, NULL);
#endif /* 0 */
}


// Adjust index for the way a compiler frame is layed out in SPARC

#ifndef CORE
int frame::adjust_offset(methodOop method, int index) {
  // SPARC
#if 0

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
#else
return 0;
#endif
#if 0
  // X86 version
  int size_of_parameters = method->size_of_parameters();
  assert (size_of_parameters >= 0, "illegal number of parameters in method");
  return (size_of_parameters - index + (index < size_of_parameters ? 1 : -1));
#endif /* 0 */
}
#endif

#ifdef CC_INTERP
bool frame::is_ignored_frame() const  {
  return InterpreterGenerator::is_dummy_frame(pc());
}
#endif /* CC_INTERP */

int frame::pd_compute_variable_size(int frame_size_in_words, CodeBlob *code) {
   // Don't think we have to do anything special
   assert(!code->is_osr_adapter(), "check this code");
#if 0
  // X86 version
  if (code->is_osr_adapter()) {
    intptr_t* sender_sp = sp() + code->frame_size();
    // See in frame_i486.hpp the interpreter's frame layout.
    // Currently, sender's sp points just past the 'return pc' like normal.
    // We need to load up the real 'sender_sp' from the interpreter's frame.
    // This is different from normal, because the current interpreter frame
    // (which has been mangled into an OSR-adapter) pushed a bunch of space
    // on the caller's frame to make space for Java locals.
    jint* sp_addr = (sender_sp - frame::sender_sp_offset) + frame::interpreter_frame_sender_sp_offset;
    intptr_t* new_sp    = (intptr_t*)*sp_addr;
    frame_size_in_words = new_sp - sp();
  }
  return frame_size_in_words;
#endif

  return frame_size_in_words;
}

void frame::patch_pc(Thread* thread, address pc) {
  guarantee(_pc_address != NULL, "Must have address to patch");
  // We should croak if _pc_address is in the register window area and the flushed
  // bit is not set (so should sparc) QQQ
  if(thread == Thread::current()) {
   (void)StubRoutines::ia64::flush_register_stack()();
  } 
  if (TracePcPatching) {
    assert(_pc == *_pc_address + pc_return_offset, "frame has wrong pc");
    tty->print_cr("patch_pc at address  " INTPTR_FORMAT " [" INTPTR_FORMAT " -> " INTPTR_FORMAT "] ",
                 _pc_address, _pc, pc);
  }
  _pc = pc;
  *_pc_address = pc - pc_return_offset;
}

#if 0
// SPARC only
static bool sp_is_valid(intptr_t* old_sp, intptr_t* young_sp, intptr_t* sp) {
  return (((intptr_t)sp & (2*wordSize-1)) == 0 && 
          sp <= old_sp && 
          sp >= young_sp);
}

// SPARC only
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
  // SPARC version
  assert(is_interpreted_frame(), "must be interpreter frame");
  return this->fp() == fp;
#if 0
  // X86 version
  assert(is_interpreted_frame(), "must be interpreter frame");
  methodOop method = interpreter_frame_method();
  // When unpacking an optimized frame the frame pointer is
  // adjusted with: 
  int diff = method->max_locals() - method->size_of_parameters();
  return _fp == (fp - diff);
#endif /* 0 */
}

#endif /* 0 */

void frame::pd_gc_epilog() {
  // QQQ why is this needed on sparc but not x86???
  if (is_interpreted_frame()) {
    // set constant pool cache entry for interpreter
    methodOop m = interpreter_frame_method();

    *interpreter_frame_cpoolcache_addr() = m->constants()->cache();
  }
}


bool frame::is_interpreted_frame_valid() const {
  // Is there anything to do?
  assert(is_interpreted_frame(), "Not an interpreted frame");
#if 0
  // These are reasonable sanity checks
  if (fp() == 0 || (intptr_t(fp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  if (sp() == 0 || (intptr_t(sp()) & (2*wordSize-1)) != 0) {
    return false;
  }
  const int interpreter_frame_initial_sp_offset = interpreter_frame_vm_local_words;
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
#endif /* CC_INTERP */
  return true;
}


#ifndef CORE
void frame::set_sender_pc_for_unpack(frame caller, int exec_mode) {
  // This routine ought to go away.
  // With the walkable skeletal frames changes the pc has already been
  // written in unpack_vframe_on_stack so this is just a nop.
  assert(caller.pc() == *caller.pc_addr(), "pc not updated");
}


#ifdef COMPILER1
void frame::patch_for_deoptimization(JavaThread* thread, frame callee, address deopt_handler) {
#ifdef CC_INTERP
  ShouldNotReachHere();
#else
  // This frame is treated like an interpreter frame now, so record a valid value for IsavedSP.
  // Since it didn't start out as an interpreter frame, the IsavedSP is simply the callers real sp
  // which is the fp of this frame.
  sp()[IsavedSP->sp_offset_in_saved_window()] = (intptr_t)fp();
  
  // Return to the deoptimization blob
  patch_pc(thread, deopt_handler);
#endif /* CC_INTERP */

#if 0
  // x86 version
  if (callee.is_interpreted_frame()) {
    assert(callee.sp() != sp(), "can be only for top frame");
    callee.set_sender_pc(deopt_handler);
  } else {
    patch_pc(thread, deopt_handler); 
  }
#endif /* 0 */
}


address frame::frameless_stub_return_addr() {
  // SPARC
  return (address)(sp()[I5->sp_offset_in_saved_window()]);
  // X86
#if 0
  // To call the runtime, the stub uses call_RT, which pushes thread, so the stub's
  // return address is stored one word below its frame's top-of-stack.
  return (address)*(sp() + 1);
#endif /* 0 */
}


void frame::patch_frameless_stub_return_addr(Thread* thread, address return_addr) {
  // SPARC version
  if (thread == Thread::current()) {
    StubRoutines::sparc::flush_callers_register_windows_func()();
  }
  sp()[I5->sp_offset_in_saved_window()] = (intptr_t)return_addr;
#if 0
  // x86 version
  *(address *)(sp() + 1) = return_addr;
#endif /* 0 */
}
#endif // COMPILER1
#endif // 0

void JavaFrameAnchor::make_walkable(bool is_definitely_current_thread, JavaThread* thread) {
  if (!has_last_Java_frame()) {
    // we have no state to cache -- return
    return;
  }
  if (is_definitely_current_thread) {
    if ( !_flushed_windows) {
      // Eventually make an assert
      guarantee(Thread::current() == (Thread*)thread, "only current thread can flush its registers");
      (void)StubRoutines::ia64::flush_register_stack()();
      _flushed_windows = true;
    }
  } else if ( !_flushed_windows ) {
      // Eventually make an assert
      guarantee(Thread::current() == (Thread*)thread, "only current thread can flush its registers");
      (void)StubRoutines::ia64::flush_register_stack()();
      _flushed_windows = true;
  }
}
