#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)inlining.cpp	1.87 03/01/23 12:01:22 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_inlining.cpp.incl"

const char* InliningPolicy::msg = 0;
int         InliningPolicy::cost = 0;
int         InliningPolicy::_cumulCost = 0;


const char* InliningPolicy::shouldInline(methodHandle m, float freq, int cnt) {
  // Allows targeted inlining
#ifndef COMPILER1
  if(CompilerOracle::should_inline(m)) {
    return NULL;
  }
#endif

  // positive filter: should send be inlined?  returns NULL (--> yes)
  // or rejection msg
  int max_size = MaxInlineSize;
  cost = m->code_size();

#ifdef COMPILER2
  // Check for too many throws (and not too huge)
  if(m->interpreter_throwout_count() > InlineThrowCount && cost < InlineThrowMaxSize ) {
    return NULL;
  }
#endif

  // bump the max size if the call is frequent
  if ((freq >= InlineFrequencyRatio) || (cnt >= InlineFrequencyCount)) {
    if (TraceFrequencyInlining) {
      tty->print("Inlined frequent method\n");
      m->print();
    }
    max_size = FreqInlineSize;
  }
  if (cost > max_size) {
    return (msg = "too big");
  }
  return NULL;
}


const char* InliningPolicy::shouldNotInline(methodHandle m, Scope*const caller) {
  // negative filter: should send NOT be inlined?  returns NULL (--> inline) or rejection msg 
  cost = 0;     // clear previous cost estimate
  if (m->is_abstract()) return (msg = "abstract method");
  // note: we allow ik->is_abstract()
  if (!instanceKlass::cast(m->method_holder())->is_initialized()) return (msg = "method holder not initialized");
  if (m->is_native()) return (msg = "native method"); 
  nmethod* m_code = m->code();
  if( m_code != NULL && m_code->instructions_size() > InlineSmallCode ) 
    return (msg = "already compiled into a big method");

  // don't inline exception code unless the top method belongs to an
  // exception class
  if (caller != NULL &&
      m->method_holder()->klass_part()->is_subclass_of(SystemDictionary::throwable_klass())) {
    Scope* top = caller;
    while (top->caller() != NULL) top = top->caller();
    Klass* k = top->method()->method_holder()->klass_part();
    if (!k->is_subclass_of(SystemDictionary::throwable_klass()))
      return (msg = "exception method");
  }

  // use frequency-based objections only for non-trivial methods
  if (m->code_size() <= MaxTrivialSize) return NULL;    
  if (UseInterpreter) {     // don't use counts with -Xcomp
    if ((m->code() == NULL) && m->was_never_executed()) return (msg = "never executed");
    if (!m->was_executed_more_than(MIN2(MinInliningThreshold, CompileThreshold >> 1))) return (msg = "executed < MinInliningThreshold times");
  }
  if (methodOopDesc::has_unloaded_classes_in_signature(m, JavaThread::current())) return (msg = "unloaded signature classes");

  return NULL;
}


// ---------------------------------------- inliner --------------------------------------

// The inliner builds up the scope tree as it iterates over them; since new scopes are created
// in the pre_* methods, the iterator will subsequently enter them.

// if a subscope exists, the (preliminary) decision was to inline it, so always enter subscopes
bool InliningClosure::should_enter_scope(Scope* s, int bci) { return true; }


ScopeClosure* InliningClosure::clone_for_subiteration(Scope* subscope) {
  return new InliningClosure(subscope);
}


void InliningClosure::pre_invoke_interface_by_index     (int idx, symbolHandle name, symbolHandle signature, int nofArgs) {
  resolve_call(idx, name, signature);
}


void InliningClosure::pre_invoke_nonvirtual_by_index    (int idx, symbolHandle name, symbolHandle signature) {
  resolve_direct_call(idx, name, signature, true);
}


void InliningClosure::pre_invoke_static_by_index        (int idx, symbolHandle name, symbolHandle signature) {
  resolve_direct_call(idx, name, signature, false);
}


void InliningClosure::pre_invoke_virtual_klass_by_index (int idx, symbolHandle name, symbolHandle signature) {
  resolve_call(idx, name, signature);
}

void InliningClosure::resolve_direct_call(int index, symbolHandle name, symbolHandle signature, bool is_special) {
  // try to inline this call (but only if class is loaded already)
  assert(Inline || InlineAccessors, "shouldn't call");
  KlassHandle klass = get_klass_or_NULL(index);
  if (klass.is_null()) return;
  KlassHandle caller = _scope->method()->method_holder();
  methodHandle method;
  if (is_special) {
    // invokespecial
    method = LinkResolver::resolve_special_call_or_null(klass, name, signature, caller);
  } else {
    // invokestatic
    method = LinkResolver::resolve_static_call_or_null(klass, name, signature, caller);
  }

  if (method.is_null()) return;
  const char* msg = try_to_inline( method );
  if (PrintInlining) printInline(method, msg ? msg : "inlining");
}


void InliningClosure::resolve_call(int index, symbolHandle name, symbolHandle signature) {
  // try to resolve this call; if successful, decide if it should be inlined
  assert(Inline || InlineAccessors, "shouldn't call");
  KlassHandle klass = get_klass_or_NULL(index);
  if (klass.is_null()) return;
  Thread *thread = Thread::current();
  KlassHandle mh (thread, _scope->method()->method_holder());
  CHAResult* cha = CHA::analyze_call(mh, klass, klass, name, signature);
  if (PrintCHA && UseCHA) {
    if (!cha->is_monomorphic()) cha->print();
  }
  
  // test for monomorphic case first (this will catch final methods
  // even if CHA is turned off)
  if (cha->is_monomorphic()) {
    const char* msg = try_to_inline( cha->monomorphic_target() );
    if (PrintInlining) printInline(cha->monomorphic_target(), msg ? msg : "inlining");
    return;
  } 

#ifdef COMPILER2
  if (ProfileInterpreter && UseTypeProfile) {
    methodDataOop mdo = method_handle()->method_data();
    ProfileData* data = mdo ? mdo->bci_to_data(bci()) : NULL;
    if (data != NULL && data->is_VirtualCallData()) {
      VirtualCallData* call = data->as_VirtualCallData();
      KlassHandle prof_klass;
      for (uint i = 0; i < call->row_limit(); i++) {
	klassOop receiver = call->receiver(i);
	if (receiver == NULL)  continue;
	if (!receiver->klass_part()->oop_is_instance())  continue;
	if (prof_klass.is_null()) {
	  prof_klass = KlassHandle(thread, receiver);
	}
	if (prof_klass->as_klassOop() != receiver) {
	  prof_klass = KlassHandle();  // conflicting receivers
	  break;
	}
      }
      if (prof_klass.not_null()) {
	KlassHandle link_klass = prof_klass;  // do not bother w/ arrays now
	KlassHandle mh (thread, _scope->method()->method_holder());
	methodHandle prof_method = LinkResolver::resolve_virtual_call_or_null(link_klass, link_klass, name, signature, mh);
	if (prof_method.not_null()) {
	  const char* msg = try_to_inline( prof_method );
	  if (PrintInlining) printInline(prof_method, msg ? msg : "inlining");
#ifndef PRODUCT
	  if (PrintInlining) tty->print_cr("%*s>> type profile target=%s", _scope->inlining_depth()+1, " ", prof_klass->internal_name());
#endif
	  return;
	}
      }
    }
  }
#endif

  // general case -- don't inline
  return;
}


const char* InliningClosure::try_to_inline( methodHandle m ) {
  // return NULL if ok, reason for not inlining otherwise
  if (ClipInlining && _scope->size() >= DesiredMethodLimit) {
    return "size > DesiredMethodLimit";
  }

  int cnt    = 0;
#ifdef COMPILER2
  if (ProfileInterpreter) {
    methodDataOop mdo = method_handle()->method_data();
    ProfileData* data = mdo ? mdo->bci_to_data(bci()) : NULL;
    if (data != NULL && data->is_CounterData()) {
      cnt = data->as_CounterData()->count();
    }
  }
#endif

  // compute call frequency to aid in inlining decision
  int invcnt = 0;
#ifdef COMPILER2
  if (ProfileInterpreter) {
    invcnt = method_handle()->interpreter_invocation_count();
    // if this method has not been seen yet just set value to one
    if( invcnt == 0 ) invcnt++;
  }
#endif
  int freq = (invcnt) ? cnt/invcnt : cnt;
  const char* msg;
  if ((msg = InliningPolicy::shouldInline(m, freq, cnt)) != NULL) return msg;
  if ((msg = InliningPolicy::shouldNotInline(m, _scope)) != NULL) return msg;

  if (m->code_size() > MaxTrivialSize) {
    if (!InlineUnreachedCalls && AbstractInterpreter::is_not_reached(method_handle(), bci())) return (msg = "call site not reached");
  }

  if (!Inline && InlineAccessors) {
    if (!m->is_accessor()) return "not an accessor";
  }

  if( _scope->inlining_depth() > MaxInlineLevel ) 
    return "inlining too deep";
  if( _scope->method() == m() &&
      _scope->inlining_depth() > MaxRecursiveInlineLevel ) 
    return "recursively inlining too deep";

  int size = m->code_size();
  if (ClipInlining && _scope->size() + size >= DesiredMethodLimit) {
    return "size > DesiredMethodLimit";
  } 
  
  // ok, inline this method
  // construction of CompilationScope adds it to the list of caller subscopes
  CompilationScope* callee = new CompilationScope(m, _scope, bci());

  return NULL;
}


// return klass (if loaded), NULL otherwise
KlassHandle InliningClosure::get_klass_or_NULL(int klass_index) const {
  Thread *thread = Thread::current();
  constantPoolHandle constants (thread, _scope->method()->constants());
  assert (_scope->method() == method_handle()(), "just checking");
  KlassHandle kh (thread, constantPoolOopDesc::klass_at_if_loaded(constants, constants->klass_ref_index_at(klass_index)));
  return kh;
}


#ifndef PRODUCT
void InliningClosure::printInline(methodHandle method, const char* msg) {
  tty->print("%*s", _scope->inlining_depth(), " ");
  method->print_short_name(tty);
  tty->print_cr(": %s (size: %d/%d)", msg, _scope->size(), method->code_size());
}
#endif

