#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)scopeIterator.cpp	1.27 03/01/23 12:01:44 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_scopeIterator.cpp.incl"

// ------------------------------------ ScopeClosure ---------------------------------------------------

bool check_call(methodHandle m, int index, bool is_static) {
  // Check that klass is both loaded and resolved in the constantpool. 
  // Otherwise, the LinkResolver call will fail
  if (!m->is_klass_loaded(index, true)) return false;  
  EXCEPTION_MARK;
  HandleMark hm(THREAD);
  constantPoolHandle pool (THREAD, m->constants());
  methodHandle spec_method;
  KlassHandle spec_klass;
  LinkResolver::resolve_method(spec_method, spec_klass, pool, index, THREAD);
  if (HAS_PENDING_EXCEPTION) {
    CLEAR_PENDING_EXCEPTION;
    return false;
  } else {
    return (spec_method->is_static() == is_static);    
  }
}

void ScopeClosure::invoke_nonvirtual_by_index(int klass_index, symbolHandle name, symbolHandle signature) {
  // Make sure to itereate through calls that do not resolve
  if (check_call(method_handle(), klass_index, false)) {  
    pre_invoke_nonvirtual_by_index(klass_index, name, signature);
    iterate_over_subscope();    
  }
}

void ScopeClosure::invoke_static_by_index(int klass_index, symbolHandle name, symbolHandle signature) {
  if (check_call(method_handle(), klass_index, true)) {  
    pre_invoke_static_by_index(klass_index, name, signature);
    iterate_over_subscope();
  }
}

void ScopeClosure::invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature) {
  if (check_call(method_handle(), klass_index, false) ) {  
    pre_invoke_virtual_klass_by_index(klass_index, name, signature);
    iterate_over_subscope();
  }
}

void ScopeClosure::iterate_over_subscope() {
  // the current bci is a call; determine if it has been inlined, and iterate
  // over the inlined scope if desired
  int call = bci();
  int nsub = _scope->nof_subscopes_at(call);
  for (int i = 0; i < nsub; i++) {
    Scope* ssub = _scope->subscope_at_bci(call, i);
    if (ssub != NULL) {
      if (should_enter_scope(ssub, call)) {
        ScopeIterator* iter = _it->clone_for_subiteration(ssub);
        ScopeClosure* c = clone_for_subiteration(ssub);
        iter->iterate(c);
      }
    }
  }
}


// ------------------------------------ ScopeIterator ---------------------------------------------------

ScopeIterator::ScopeIterator(Scope* s, AbstractMethodIntervalFactory* f) :
               MethodIterator(s->method(), f) {}
ScopeIterator::ScopeIterator(MethodInterval* i, AbstractMethodIntervalFactory* f) : MethodIterator(i, f) {}

ScopeIterator* ScopeIterator::clone_for_subiteration(Scope* s) {
  return new ScopeIterator(s);
}

void ScopeIterator::iterate(ScopeClosure* c) {
  c->set_iterator(this);
  c->enter_scope();
  MethodIterator::iterate(c);  
  c->exit_scope();
}

