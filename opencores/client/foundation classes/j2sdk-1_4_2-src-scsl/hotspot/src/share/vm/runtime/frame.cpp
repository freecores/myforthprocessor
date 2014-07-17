#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)frame.cpp	1.196 03/05/26 09:54:19 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_frame.cpp.incl"

RegisterMap::RegisterMap(JavaThread *thread, bool update_map) {
  _thread         = thread;
  _update_map     = update_map;      
  clear(thread->not_at_call_id());  
  debug_only(_update_for_id = NULL;)
}

RegisterMap::RegisterMap(const RegisterMap* map) {
  assert(map != this, "bad initialization parameter");
  assert(map != NULL, "RegisterMap must be present");
  _thread                = map->thread();
  _update_map            = map->update_map();
  _include_argument_oops = map->include_argument_oops();  
  _not_at_call_id        = map->_not_at_call_id;
  debug_only(_update_for_id = map->_update_for_id;)
#ifndef CORE
  pd_initialize_from(map);
  if (update_map()) {  
    for(int i = 0; i < location_valid_size; i++) {
      LocationValidType bits = !update_map() ? 0 : map->_location_valid[i];
      _location_valid[i] = bits;
      // for whichever bits are set, pull in the corresponding map->_location
      int j = i*location_valid_type_size;
      while (bits != 0) {
        if ((bits & 1) != 0) {
          assert(0 <= j && j < reg_count, "range check");
          _location[j] = map->_location[j];
        }
        bits >>= 1;
        j += 1;
      }
    }
  }
#endif
}

void RegisterMap::clear(intptr_t* not_at_call_id) {  
  set_include_argument_oops(true);
  _not_at_call_id = not_at_call_id;
#ifndef CORE
  if (_update_map) {
    for(int i = 0; i < location_valid_size; i++) {
      _location_valid[i] = 0;
    }
    pd_clear();
  } else {
    pd_initialize();
  }
#endif
}

#ifndef PRODUCT

void RegisterMap::print_on(outputStream* st) const {
#ifndef CORE
  st->print_cr("Register map");
  for(int i = 0; i < reg_count; i++) {
    
    intptr_t* src = (intptr_t*) location(VMReg::Name(i));
    if (src != NULL) {
      tty->print("  %s [" INTPTR_FORMAT "] = ", SharedInfo::regName[i], src);
      if (((uintptr_t)src & (sizeof(*src)-1)) != 0) {
	tty->print_cr("<misaligned>");
      } else {
	tty->print_cr(INTPTR_FORMAT, *src);
      }
    }
  }
#endif
}

void RegisterMap::print() const {
  print_on(tty);
}

#endif

// type testers

bool frame::is_first_frame() const {
  return is_entry_frame() && entry_frame_is_first(); 
}

bool frame::is_native_frame() const {
#ifndef CORE
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_native_method());    
#else
  return false;
#endif
}

bool frame::is_java_frame() const {
  if (is_interpreted_frame()) return true;
#ifndef CORE
  if (is_deoptimized_frame()) return true;
  if (is_compiled_frame())    return true;
#endif
  return false;
}

#ifndef CORE
bool frame::is_deoptimized_frame() const {
  CodeBlob* cb = NULL;
  COMPILER1_ONLY(cb = Runtime1::blob_for(Runtime1::deoptimization_handler_id);)
  COMPILER2_ONLY(cb = OptoRuntime::deoptimization_blob();)
  return cb == NULL ? false : cb->contains(pc());
}


bool frame::is_compiled_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_java_method());  
}


bool frame::is_c2runtime_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_runtime_stub());
}

bool frame::is_c2i_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_c2i_adapter());
}

bool frame::is_i2c_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_i2c_adapter());
}

bool frame::is_safepoint_blob_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_safepoint_stub());
}

bool frame::is_osr_adapter_frame() const {
#ifdef COMPILER1
  return Runtime1::returns_to_osr_adapter(pc());
#endif
#ifdef COMPILER2
  CodeBlob* cb = CodeCache::find_blob(pc());
  return (cb != NULL && cb->is_osr_adapter());
#endif
}

bool frame::is_glue_frame() const {
  CodeBlob* cb = CodeCache::find_blob(pc());
  if (cb == NULL ) return false;
       if (cb->is_i2c_adapter()) return true;
  else if (cb->is_c2i_adapter()) return true; 
  else if (cb->is_runtime_stub()) return true;
  else return false;
}
#endif // CORE


// testers

bool frame::is_first_java_frame() const {
  RegisterMap map(JavaThread::current(), false); // No update
  frame s;
  for (s = sender(&map); !(s.is_java_frame() || s.is_first_frame()); s = s.sender(&map));
  return s.is_first_frame();
}


bool frame::entry_frame_is_first() const {
  return entry_frame_call_wrapper()->anchor()->last_Java_sp() == NULL;
}


bool frame::should_be_deoptimized() const {
#ifndef CORE
  if (!is_compiled_frame()) return false;
  CodeBlob *cb = CodeCache::find_blob(pc());
  assert(cb != NULL && cb->is_nmethod(), "must be an nmethod");
  nmethod* nm = (nmethod *)cb;
  if (TraceDependencies) {
    tty->print("checking (%s) ", nm->is_marked_for_deoptimization() ? "true" : "false"); 
    nm->print_value_on(tty);
    tty->cr();
  }
  return nm->is_marked_for_deoptimization();
#else
  return false;
#endif
}

bool frame::can_be_deoptimized() const {
#ifndef CORE
  if (!is_compiled_frame()) return false;
  nmethod* nm = CodeCache::find_nmethod(pc());  
  return nm->can_be_deoptimized();
#else
  return false;
#endif
}

frame frame::java_sender() const {
  RegisterMap map(JavaThread::current(), false);
  frame s;
  for (s = sender(&map); !(s.is_java_frame() || s.is_first_frame()); s = s.sender(&map)) ;
  guarantee(s.is_java_frame(), "tried to get caller of first java frame");
  return s;
}

frame frame::real_sender(RegisterMap* map) const {
#ifndef CORE
  frame result = sender(map);
  while (result.is_glue_frame()) {
    result = result.sender(map);
  }
  return result;
#else
  return sender(map);
#endif
}

// Note: called by profiler - NOT for current thread
frame frame::profile_find_Java_sender_frame(JavaThread *thread) {
// If we don't recognize this frame, walk back up the stack until we do
  RegisterMap map(thread, false);
  frame first_java_frame = frame();

  // Find the first Java frame on the stack starting with input frame
  if (is_java_frame()) {
    // top frame is compiled frame or deoptimized frame
    first_java_frame = *this;
  } else if (safe_for_sender(thread)) {
    for (frame sender_frame = sender(&map);
      sender_frame.safe_for_sender(thread) && !sender_frame.is_first_frame(); 
      sender_frame = sender_frame.sender(&map)) {
      if (sender_frame.is_java_frame()) {
        first_java_frame = sender_frame;
        break;
      }
    }
  }
  return first_java_frame;
}

// Interpreter frames

 
void frame::interpreter_frame_set_locals(intptr_t* locs)  { 
  assert(is_interpreted_frame(), "Not an interpreted frame");
  *interpreter_frame_locals_addr() = locs; 
}

methodOop frame::interpreter_frame_method() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  methodOop m = *interpreter_frame_method_addr();
  assert(m->is_perm(), "bad methodOop in interpreter frame");
  assert(m->is_method(), "not a methodOop");
  return m;
}

void frame::interpreter_frame_set_method(methodOop method) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  *interpreter_frame_method_addr() = method;
}

void frame::interpreter_frame_set_bcx(intptr_t bcx) {
  assert(is_interpreted_frame(), "Not an interpreted frame");
#ifdef CORE
  *interpreter_frame_bcx_addr() = bcx;
#else  // !CORE
  if (ProfileInterpreter) {
    bool formerly_bci = is_bci(interpreter_frame_bcx());
    bool is_now_bci = is_bci(bcx);
    *interpreter_frame_bcx_addr() = bcx;

    intptr_t mdx = interpreter_frame_mdx();

    if (mdx != 0) {
      if (formerly_bci) {
        if (!is_now_bci) {
          // The bcx was just converted from bci to bcp.
          // Convert the mdx in parallel.
          methodDataOop mdo = interpreter_frame_method()->method_data();
          assert(mdo != NULL, "");
          int mdi = mdx - 1; // We distinguish valid mdi from zero by adding one.
          address mdp = mdo->di_to_dp(mdi);
          interpreter_frame_set_mdx((intptr_t)mdp);
        }
      } else {
        if (is_now_bci) {
          // The bcx was just converted from bcp to bci.
          // Convert the mdx in parallel.
          methodDataOop mdo = interpreter_frame_method()->method_data();
          assert(mdo != NULL, "");
          int mdi = mdo->dp_to_di((address)mdx);
          interpreter_frame_set_mdx((intptr_t)mdi + 1); // distinguish valid from 0.
        }
      }
    }
  } else {
    *interpreter_frame_bcx_addr() = bcx;
  }
#endif // CORE
}

jint frame::interpreter_frame_bci() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  intptr_t bcx = interpreter_frame_bcx();
  return is_bci(bcx) ? bcx : interpreter_frame_method()->bci_from((address)bcx);
}

void frame::interpreter_frame_set_bci(jint bci) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  assert(!is_bci(interpreter_frame_bcx()), "should not set bci during GC");
  interpreter_frame_set_bcx((intptr_t)interpreter_frame_method()->bcp_from(bci));
}

address frame::interpreter_frame_bcp() const {
  assert(is_interpreted_frame(), "interpreted frame expected");
  intptr_t bcx = interpreter_frame_bcx();
  return is_bci(bcx) ? interpreter_frame_method()->bcp_from(bcx) : (address)bcx;
}

void frame::interpreter_frame_set_bcp(address bcp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  assert(!is_bci(interpreter_frame_bcx()), "should not set bcp during GC");
  interpreter_frame_set_bcx((intptr_t)bcp);
}

#ifndef CORE
void frame::interpreter_frame_set_mdx(intptr_t mdx) {
  assert(is_interpreted_frame(), "Not an interpreted frame");
  assert(ProfileInterpreter, "must be profiling interpreter");
  *interpreter_frame_mdx_addr() = mdx;
}

address frame::interpreter_frame_mdp() const {
  assert(ProfileInterpreter, "must be profiling interpreter");
  assert(is_interpreted_frame(), "interpreted frame expected");
  intptr_t bcx = interpreter_frame_bcx();
  intptr_t mdx = interpreter_frame_mdx();

  assert(!is_bci(bcx), "should not access mdp during GC");
  return (address)mdx;
}

void frame::interpreter_frame_set_mdp(address mdp) {
  assert(is_interpreted_frame(), "interpreted frame expected");
  if (mdp == NULL) {
    // Always allow the mdp to be cleared.
    interpreter_frame_set_mdx((intptr_t)NULL);
  }
  intptr_t bcx = interpreter_frame_bcx();
  assert(!is_bci(bcx), "should not set mdp during GC");
  interpreter_frame_set_mdx((intptr_t)mdp);
}
#endif // !CORE

BasicObjectLock* frame::next_monitor_in_interpreter_frame(BasicObjectLock* current) const { 
  assert(is_interpreted_frame(), "Not an interpreted frame");
#ifdef ASSERT
  interpreter_frame_verify_monitor(current);
#endif
  BasicObjectLock* next = (BasicObjectLock*) (((intptr_t*) current) + interpreter_frame_monitor_size());
  return next;
}

BasicObjectLock* frame::previous_monitor_in_interpreter_frame(BasicObjectLock* current) const { 
  assert(is_interpreted_frame(), "Not an interpreted frame");
#ifdef ASSERT
//   // This verification needs to be checked before being enabled
//   interpreter_frame_verify_monitor(current);
#endif
  BasicObjectLock* previous = (BasicObjectLock*) (((intptr_t*) current) - interpreter_frame_monitor_size());
  return previous;
}


// (frame::interpreter_frame_sender_sp accessor is in frame_<arch>.cpp)

const char* frame::print_name() const {
  if (is_native_frame())      return "Native";
  if (is_interpreted_frame()) return "Interpreted";
#ifdef CC_INTERP
  if (is_ignored_frame()) return "Ignored";
#endif
#ifndef CORE
  if (is_deoptimized_frame()) return "Deoptimized";
  if (is_compiled_frame())    return "Compiled";
#endif
  if (sp() == NULL)            return "Empty";
  return "C";
}

void frame::print_value_on(outputStream* st, JavaThread *thread) const {
  NOT_PRODUCT(address begin = pc()-40;)
  NOT_PRODUCT(address end   = NULL;)

  st->print("%s frame (sp=%#lx", print_name(), sp());
  if (sp() != NULL)
    st->print(", fp=%#lx, pc=%#lx", fp(), pc());

  if (StubRoutines::contains(pc())) {
    StubCodeDesc* desc = StubCodeDesc::desc_for(pc());
    st->print("~Stub::%s", desc->name());
    NOT_PRODUCT(begin = desc->begin(); end = desc->end();)
  } else if (Interpreter::contains(pc())) {
    InterpreterCodelet* desc = Interpreter::codelet_containing(pc());
    if (desc != NULL) {
      st->print("~");
      desc->print();
      NOT_PRODUCT(begin = desc->code_begin(); end = desc->code_end();)
    } else {
      st->print("~interpreter"); 
    }
  }
  st->print_cr(")");

#ifndef CORE
  CodeBlob* cb = CodeCache::find_blob(pc());
  if (cb != NULL) {
    st->print("     ");
    const DeoptimizationBlob *db = cb->as_deoptimization_stub();
    if( db )
      db->print_value_on_fancy(st,thread,*this);
    else
      cb->print_value_on(st);
    st->cr();
#ifndef PRODUCT
    if (end == NULL) {
      begin = cb->instructions_begin();
      end = cb->instructions_end();
    }
#endif
  }
#endif
  NOT_PRODUCT(if (WizardMode && Verbose) Disassembler::decode(begin, end);)
}


void frame::print_on(outputStream* st) const {
  print_value_on(st,NULL);
  if (is_interpreted_frame()) {
    interpreter_frame_print_on(st);
  }
}


void frame::interpreter_frame_print_on(outputStream* st) const {
#ifndef PRODUCT
  assert(is_interpreted_frame(), "Not an interpreted frame");
  for (jint i = interpreter_frame_expression_stack_size() - 1; i >= 0; --i ) {
    intptr_t x = interpreter_frame_expression_stack_at(i);
    st->print(" - stack  [%#lx]", x);
    st->fill_to(23);
    st->print_cr("; #%d", i);
  }
  // locks for synchronization
  for (BasicObjectLock* current = interpreter_frame_monitor_end();
       current < interpreter_frame_monitor_begin();
       current = next_monitor_in_interpreter_frame(current)) {
    st->print_cr(" [ - obj ");
    current->obj()->print_value_on(st);
    st->cr();
    st->print_cr(" - lock ");
    current->lock()->print_on(st);
    st->cr();
  }
  // monitor
  st->print_cr(" - monitor[%#lx]", interpreter_frame_monitor_begin());
  // bcp
  st->print(" - bcp    [%#lx]", interpreter_frame_bcp());
  st->fill_to(23);
  st->print_cr("; @%d", interpreter_frame_bci());
  // locals
  st->print_cr(" - locals [%#lx]", &interpreter_frame_local_at(0));
  // method
  st->print(" - method [%#lx]", interpreter_frame_method());
  st->fill_to(23);
  st->print("; ");
  interpreter_frame_method()->print_name(st);
  st->cr();
#endif
}

/*
  The interpreter_frame_expression_stack_at method in the case of SPARC needs the
  max_stack value of the method in order to compute the expression stack address.
  It uses the methodOop in order to get the max_stack value but during GC this
  methodOop value saved on the frame is changed by reverse_and_push and hence cannot
  be used. So we save the max_stack value in the FrameClosure object and pass it
  down to the interpreter_frame_expression_stack_at method
*/
class InterpreterFrameClosure : public OffsetClosure {
 private:
  frame* _fr;
  OopClosure* _f;
  int    _max_locals;
  int    _max_stack;

 public:
  InterpreterFrameClosure(frame* fr, int max_locals, int max_stack,
			  OopClosure* f) {
    _fr         = fr;
    _max_locals = max_locals;
    _max_stack  = max_stack;
    _f          = f;
  }

  void offset_do(int offset) {
    oop* addr;
    if (offset < _max_locals) {
      addr = (oop*) &_fr->interpreter_frame_local_at(offset);
      assert((intptr_t*)addr >= _fr->sp(), "must be inside the frame");
      _f->do_oop(addr);
    } else {
      addr = (oop*) &_fr->interpreter_frame_expression_stack_at((offset - _max_locals));
      // In case of exceptions, the expression stack is invalid and the esp will be reset to express
      // this condition. Therefore, we call f only if addr is 'inside' the stack (i.e., addr >= esp for Intel).
      bool in_stack;
      if (frame::interpreter_frame_expression_stack_direction() > 0) {
	in_stack = (intptr_t*)addr <= _fr->interpreter_frame_tos_address();
      } else {
	in_stack = (intptr_t*)addr >= _fr->interpreter_frame_tos_address();
      }
      if (in_stack) {
	_f->do_oop(addr);
      }
    }
  }

  int max_locals()  { return _max_locals; }
  frame* fr()       { return _fr; }
};


class ArgumentOopFinder: public SignatureInfo {
 private:
  OopClosure* _f;      // Closure to invoke
  int    _offset;      // TOS-relative offset, decremented with each argument
  bool   _is_static;   // true if the callee is a static method
  frame* _fr;

  void set(int size, BasicType type) {
    _offset -= size;
    if (type == T_OBJECT || type == T_ARRAY) oop_offset_do();    
  }

  void oop_offset_do() {
    oop* addr = (oop*)&_fr->interpreter_frame_tos_at(_offset);
    _f->do_oop(addr);
  }

 public:
  ArgumentOopFinder(symbolHandle signature, bool is_static, frame* fr, OopClosure* f) : SignatureInfo(signature) {
    // compute size of arguments
    int args_size = ArgumentSizeComputer(signature).size() + (is_static ? 0 : 1);
    assert(!fr->is_interpreted_frame() ||     
            args_size <= fr->interpreter_frame_expression_stack_size(), "args cannot be on stack anymore");
    // initialize ArgumentOopFinder
    _f         = f;
    _fr        = fr;    
    _offset    = args_size;
    _is_static = is_static;
  }

  void oops_do() {
    if (!_is_static) {
      --_offset;
      oop_offset_do();
    }
    iterate_parameters();
  }
};


// Entry frame has following form (n arguments)
//         +-----------+
//   sp -> |  last arg |
//         +-----------+
//         :    :::    :
//         +-----------+
// (sp+n)->|  first arg|
//         +-----------+

 

// visits and GC's all the arguments in entry frame
class EntryFrameOopFinder: public SignatureInfo {
 private:
  bool   _is_static;
  int    _offset;
  frame* _fr;
  OopClosure* _f;

  void set(int size, BasicType type) {
    assert (_offset >= 0, "illegal offset");
    if (type == T_OBJECT || type == T_ARRAY) oop_at_offset_do(_offset);
    _offset -= size;
  }

  void oop_at_offset_do(int offset) {
    assert (offset >= 0, "illegal offset")
    oop* addr = (oop*) _fr->entry_frame_argument_at(offset);
    _f->do_oop(addr);
  }

 public:
   EntryFrameOopFinder(frame* frame, symbolHandle signature, bool is_static) : SignatureInfo(signature) {
     _f = NULL; // will be set later
     _fr = frame;
     _is_static = is_static;
     _offset = ArgumentSizeComputer(signature).size() - 1; // last parameter is at index 0
   }

  void arguments_do(OopClosure* f) {
    _f = f;
    if (!_is_static) oop_at_offset_do(_offset+1); // do the receiver
    iterate_parameters();
  }

};

oop* frame::interpreter_callee_receiver_addr(symbolHandle signature) {
  ArgumentSizeComputer asc(signature);
  int size = asc.size();  
  return (oop *)&interpreter_frame_tos_at(size); 
}


void frame::oops_interpreted_do(OopClosure* f, const RegisterMap* map) {
  assert(is_interpreted_frame(), "Not an interpreted frame");
  assert(map != NULL, "map must be set");
  methodOop m   = interpreter_frame_method();
  jint      bci = interpreter_frame_bci();
  Thread *thread = Thread::current();

  assert(Universe::heap()->is_in(m), "must be valid oop");
  assert(m->is_method(), "checking frame value");
  assert((m->is_native() && bci == 0)  || (!m->is_native() && bci >= 0 && bci < m->code_size()), "invalid bci value");

  // Handle the monitor elements in the activation
  for (
    BasicObjectLock* current = interpreter_frame_monitor_end();
    current < interpreter_frame_monitor_begin();
    current = next_monitor_in_interpreter_frame(current)
  ) {
#ifdef ASSERT
    interpreter_frame_verify_monitor(current);
#endif    
    current->oops_do(f);
  }

  // process fixed part
  f->do_oop((oop*)interpreter_frame_method_addr());
  f->do_oop((oop*)interpreter_frame_cache_addr());

  // Hmm what about the mdp?
#ifdef CC_INTERP
  // Interpreter frame in the midst of a call have a methodOop within the
  // object. 
  interpreterState istate = get_interpreterState();
  if (istate->msg() == cInterpreter::call_method) {
    f->do_oop((oop*)&istate->_result._to_call._callee);
  }

#endif /* CC_INTERP */

  if (m->is_native() && m->is_static()) {
#ifdef CC_INTERP
    f->do_oop((oop*)&istate->_native_mirror);
#else
    f->do_oop((oop*)( fp() + interpreter_frame_mirror_offset ));
#endif /* CC_INTERP */
  }

  int max_locals = m->is_native() ? m->size_of_parameters() : m->max_locals();
  InterpreterFrameClosure blk(this, max_locals, m->max_stack(), f);
  
  // process locals & expression stack
  InterpreterOopMap mask;
  m->mask_for(bci, &mask);
  mask.iterate_oop(&blk);

  // process a callee's arguments if we are at a call site
  // (i.e., if we are at an invoke bytecode)  
  if (map->include_argument_oops() && !m->is_native()) {
    ResourceMark rm(thread);
    methodHandle method (thread, m);
    Bytecode_invoke* call = Bytecode_invoke_at_check(method, bci);
    if (call != NULL && interpreter_frame_expression_stack_size() > 0) {
      // we are at a call site & the expression stack is not empty
      // => process callee's arguments
      //
      // Note: The expression stack can be empty if an exception
      //       occured during method resolution/execution. In all
      //       cases we empty the expression stack completely be-
      //       fore handling the exception (the exception handling
      //       code in the interpreter calls a blocking runtime
      //       routine which can cause this code to be executed).
      //       (was bug gri 7/27/98)      
      symbolHandle signature (thread, call->signature());
      oops_interpreted_arguments_do(signature, call->is_invokestatic(), f);      
    }
  }
}

void frame::oops_interpreted_arguments_do(symbolHandle signature, bool is_static, OopClosure* f) {  
  ArgumentOopFinder finder(signature, is_static, this, f);
  finder.oops_do();
}

void frame::oops_code_blob_do(OopClosure* f, const RegisterMap* reg_map) {
#ifndef CORE
  CodeBlob* cb = CodeCache::find_blob(pc());
  assert(cb != NULL, "sanity check");
  if (cb->oop_maps() != NULL) {
    OopMapSet::oops_do(this, cb, reg_map, f);

    // Preserve potential arguments for a callee. We handle this by dispatching
    // on the codeblob. For c2i, we do
    if (reg_map->include_argument_oops()) {
      cb->preserve_callee_argument_oops(*this, reg_map, f);      
    }
  }
  // In cases where perm gen is collected, GC will want to mark
  // oops referenced from nmethods active on thread stacks so as to
  // prevent them from being collected. However, this visit should be
  // restricted to certain phases of the collection only. The
  // closure answers whether it wants nmethods to be traced.
  // (All CodeBlob subtypes other than NMethod currently have
  // an empty oops_do() method.
  if (f->do_nmethods()) {
    cb->oops_do(f);
  }
#endif
}


void frame::nmethods_code_blob_do() {
#ifndef CORE
  CodeBlob* cb = CodeCache::find_blob(pc());
  assert(cb != NULL, "sanity check");

  // If we see an activation belonging to a non_entrant nmethod, we mark it.
  if (cb->is_nmethod() && ((nmethod *)cb)->is_not_entrant()) {
    ((nmethod*)cb)->mark_as_seen_on_stack();
  }
#endif
}


#ifdef COMPILER1
void frame::oops_compiled_arguments_do(symbolHandle signature, bool is_static, const RegisterMap* reg_map, OopClosure* f) {
  NEEDS_CLEANUP // consolidate these
#ifdef SPARC
  GC_Support::preserve_callee_argument_oops(*this, signature, is_static, f);
#else
  oops_interpreted_arguments_do(signature, is_static, f);
#endif
}
#endif


#ifdef COMPILER2
class CompiledArgumentOopFinder: public SignatureInfo {
 protected:
  OopClosure*     _f;  
  int             _offset;      // the current offset, incremented with each argument
  bool            _is_static;   // true if the callee is a static method
  frame           _fr;
  RegisterMap*    _reg_map;      
  int             _arg_size;
  OptoRegPair*    _regs;        // VMReg::Name list of arguments
  
  void set(int size, BasicType type) {
    if (type == T_OBJECT || type == T_ARRAY) handle_oop_offset();
    _offset += size;
  }

  virtual void handle_oop_offset() {
    // Extract low order register number from register array.
    // In LP64-land, the high-order bits are valid but unhelpful.
    VMReg::Name reg = VMReg::Name(_regs[_offset].lo());
    oop *loc = _fr.oopmapreg_to_location(reg, _reg_map);
    _f->do_oop(loc); 
  }

 public:
  CompiledArgumentOopFinder(symbolHandle signature, bool is_static, OopClosure* f, frame fr,  const RegisterMap* reg_map) 
    : SignatureInfo(signature) {

    // initialize CompiledArgumentOopFinder
    _f         = f;
    _offset    = 0;
    _is_static = is_static;    
    _fr        = fr;    
    _reg_map   = (RegisterMap*)reg_map;
    _arg_size  = ArgumentSizeComputer(signature).size() + (is_static ? 0 : 1);

    int arg_size;
    _regs = Matcher::find_callee_arguments(signature(), is_static, &arg_size);
    assert(arg_size == _arg_size, "wrong arg size");
  }

  void oops_do() {
    if (!_is_static) {
      handle_oop_offset(); 
      _offset++;
    }
    iterate_parameters();
  }  
};

void frame::oops_compiled_arguments_do(symbolHandle signature, bool is_static, const RegisterMap* reg_map, OopClosure* f) {
  ResourceMark rm;
  CompiledArgumentOopFinder finder(signature, is_static, f, *this, reg_map);
  finder.oops_do();
}
#endif // COMPILER2


#ifndef CORE
oop* frame::oopmapreg_to_location(VMReg::Name reg, const RegisterMap* reg_map) const {
  if(reg < SharedInfo::stack0) {    
    // If it is passed in a register, it got spilled in the stub frame.  
    return (oop *)reg_map->location(reg);
  } else {			        
    int sp_offset_in_stack_slots = reg - SharedInfo::stack0 + pd_oop_map_offset_adjustment();
    int sp_offset = sp_offset_in_stack_slots >> (LogBytesPerWord - LogBytesPerInt);
    return (oop *)&sp()[sp_offset];
  }
}
#endif

void frame::oops_entry_do(OopClosure* f, const RegisterMap* map) {
  assert(map != NULL, "map must be set");
  if (map->include_argument_oops()) {
    // must collect argument oops, as nobody else is doing it
    Thread *thread = Thread::current();
    methodHandle m (thread, entry_frame_call_wrapper()->callee_method());
    symbolHandle signature (thread, m->signature());
    EntryFrameOopFinder finder(this, signature, m->is_static());
    finder.arguments_do(f);
  }
  // Traverse the Handle Block saved in the entry frame
  entry_frame_call_wrapper()->oops_do(f);
}


void frame::oops_do(OopClosure* f, RegisterMap* map) {
         if (is_interpreted_frame())    { oops_interpreted_do(f, map);
  } else if (is_entry_frame())          { oops_entry_do      (f, map);
#ifdef COMPILER1
  // Note: this test has to come before the CodeCache::contains(pc())
  //       check since the code for osr adapter frames is contained
  //       in the code cache, too!
  } else if (is_osr_adapter_frame())    { // nothing to do
#endif
#ifdef CC_INTERP
  } else if (is_ignored_frame())        { // nothing to do
#endif
#ifndef CORE  
  } else if (CodeCache::contains(pc())) { oops_code_blob_do  (f, map);
#endif
  } else { 
    ShouldNotReachHere();
  }  
}


void frame::nmethods_do() {
#ifndef CORE  
  if (   !is_interpreted_frame() 
      && !is_entry_frame()
#ifdef COMPILER1
      // Note: this test has to come before the CodeCache::contains(pc())
      //       check since the code for osr adapter frames is contained
      //       in the code cache, too!
      && !is_osr_adapter_frame()
#endif
      && CodeCache::contains(pc())
      ) { 
    nmethods_code_blob_do();
  } 
#endif
}


void frame::gc_prologue() {  
  if (is_interpreted_frame()) {
    // set bcx to bci to become methodOop position independent during GC
    interpreter_frame_set_bcx(interpreter_frame_bci());
  }
}


void frame::gc_epilogue() {
  if (is_interpreted_frame()) {
    // set bcx back to bcp for interpreter
    interpreter_frame_set_bcx((intptr_t)interpreter_frame_bcp());
  }
  // call processor specific epilog function
  pd_gc_epilog();
}


# ifdef ENABLE_ZAP_DEAD_LOCALS

void frame::CheckValueClosure::do_oop(oop* p) {
  if (CheckOopishValues && Universe::heap()->is_in_reserved(*p)) {
    warning("value @ " INTPTR_FORMAT " looks oopish (" INTPTR_FORMAT ") (thread = " INTPTR_FORMAT ")", p, *p, Thread::current());
  }
}
frame::CheckValueClosure frame::_check_value;


void frame::CheckOopClosure::do_oop(oop* p) {
  if (*p != NULL && !(*p)->is_oop()) {
    warning("value @ " INTPTR_FORMAT " should be an oop (" INTPTR_FORMAT ") (thread = " INTPTR_FORMAT ")", p, *p, Thread::current());
 }
}
frame::CheckOopClosure frame::_check_oop;

void frame::check_derived_oop(oop* base, oop* derived) {
  _check_oop.do_oop(base);
}


void frame::ZapDeadClosure::do_oop(oop* p) {
  if (TraceZapDeadLocals) tty->print_cr("zapping @ " INTPTR_FORMAT " containing " INTPTR_FORMAT, p, *p);
  *p = (oop)0xbabebabe; 
}
frame::ZapDeadClosure frame::_zap_dead;

void frame::zap_dead_locals(JavaThread* thread, const RegisterMap* map) {
  assert(thread == Thread::current(), "need to synchronize to do this to another thread");
  // Tracing - part 1
  if (TraceZapDeadLocals) {
    ResourceMark rm(thread);
    tty->print_cr("--------------------------------------------------------------------------------");
    tty->print("Zapping dead locals in ");
    print_on(tty);
    tty->cr();
  }
  // Zapping
       if (is_entry_frame      ()) zap_dead_entry_locals      (thread, map);
  else if (is_interpreted_frame()) zap_dead_interpreted_locals(thread, map);
#ifndef CORE
  else if (is_compiled_frame   ()) zap_dead_compiled_locals   (thread, map);
  else if (is_deoptimized_frame()) zap_dead_deoptimized_locals(thread, map);
#endif

  else
    // could be is_c2i_frame, is_c2runtime_frame, is_i2c_frame
    // so remove error: ShouldNotReachHere();
    ;
  // Tracing - part 2
  if (TraceZapDeadLocals) {
    tty->cr();
  }
}


void frame::zap_dead_interpreted_locals(JavaThread *my_thread, const RegisterMap* map) {
  // get current interpreter 'pc'
  assert(is_interpreted_frame(), "Not an interpreted frame");
  methodOop m   = interpreter_frame_method();
  int       bci = interpreter_frame_bci();

  int max_locals = m->is_native() ? m->size_of_parameters() : m->max_locals();

  // process dynamic part
  InterpreterFrameClosure value_blk(this, max_locals, m->max_stack(),
				    &_check_value);
  InterpreterFrameClosure   oop_blk(this, max_locals, m->max_stack(),
				    &_check_oop  );
  InterpreterFrameClosure  dead_blk(this, max_locals, m->max_stack(),
				    &_zap_dead   );

  // get frame map
  InterpreterOopMap mask;
  m->mask_for(bci, &mask);
  mask.iterate_all( &oop_blk, &value_blk, &dead_blk); 
}


void frame::zap_dead_compiled_locals(JavaThread* thread, const RegisterMap* reg_map) {

# ifndef CORE
  ResourceMark rm(thread);
  CodeBlob* cb = CodeCache::find_blob(pc());
  assert(cb != NULL, "sanity check");
  if (cb->oop_maps() != NULL) {
    OopMapSet::all_do(this, cb, reg_map, &_check_oop, check_derived_oop,
		      &_check_value, &_zap_dead);
  }
# endif
}


void frame::zap_dead_entry_locals(JavaThread*, const RegisterMap*) {
  if (TraceZapDeadLocals) warning("frame::zap_dead_entry_locals unimplemented");
}


void frame::zap_dead_deoptimized_locals(JavaThread*, const RegisterMap*) {
  if (TraceZapDeadLocals) warning("frame::zap_dead_deoptimized_locals unimplemented");
}

# endif // ENABLE_ZAP_DEAD_LOCALS

void frame::verify(const RegisterMap* map) {
  // for now make sure receiver type is correct
  if (is_interpreted_frame()) {
    methodOop method = interpreter_frame_method();
    guarantee(method->is_method(), "method is wrong in frame::verify");
    if (!method->is_static()) {
      // fetch the receiver
      oop* p = (oop*) &interpreter_frame_local_at(0);
      // make sure we have the right receiver type
    }
  }
#ifndef PRODUCT
  COMPILER2_ONLY(assert(DerivedPointerTable::is_empty(), "must be empty before verify");)
  NOT_CORE(oops_do(&VerifyOopClosure::verify_oop, (RegisterMap*)map);)
#endif
}


#ifdef ASSERT
bool frame::verify_return_pc(address x) {
  if (StubRoutines::returns_to_call_stub(x)) {
    return true;
  }  
#ifndef CORE
  if (CodeCache::contains(x)) {
    return true; 
  }
#endif
  if (Interpreter::contains(x)) {
    return true;
  }
  return false;
}
#endif


#ifdef ASSERT
void frame::interpreter_frame_verify_monitor(BasicObjectLock* value) const {
  assert(is_interpreted_frame(), "Not an interpreted frame");
  // verify that the value is in the right part of the frame
  address low_mark  = (address) interpreter_frame_monitor_end();
  address high_mark = (address) interpreter_frame_monitor_begin();
  address current   = (address) value;

  const int monitor_size = frame::interpreter_frame_monitor_size();
  guarantee((high_mark - current) % monitor_size  ==  0         , "invalid BasicObjectLock*");
  guarantee( high_mark > current                                , "invalid BasicObjectLock*");

  guarantee((current - low_mark) % monitor_size  ==  0         , "invalid BasicObjectLock*");
  guarantee( current >= low_mark                               , "invalid BasicObjectLock*");
}
#endif


//-----------------------------------------------------------------------------------
// StackFrameStream implementation

StackFrameStream::StackFrameStream(JavaThread *thread, bool update) : _reg_map(thread, update) {
  assert(thread->has_last_Java_frame(), "sanity check");  
  _fr = thread->last_frame(); 
  _is_done = false;
}

