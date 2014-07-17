#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciScope.cpp	1.10 03/01/23 11:58:11 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciScope.cpp.incl"

// ciScope
//
// This class represents a tree of potentially inlineable
// methods.
//
// At this point, a ciScope is just a copy of the information
// from an already created CompilationScope.  As our inlining
// policy changes (presumably by moving into the compiler) this
// class may be able to be eliminated.

// ------------------------------------------------------------------
// ciScope::ciScope
//
// Top level ciScope constructor
ciScope::ciScope(CompilationScope* cscope) {
  // Build the ciScope from the CompilationScope.
  VM_ENTRY_MARK;
  _method_handle = JNIHandles::make_local(THREAD, cscope->method()());
  _method = NULL;
  _caller = NULL;
  _caller_bci = cscope->callerBCI();
  int nof_subscopes = cscope->nof_subscopes();
  Arena* arena = CURRENT_THREAD_ENV->arena();
  _subscopes =
    new (arena) GrowableArray<ciScope*>(arena, nof_subscopes,
					  nof_subscopes, NULL);
  for (int i = 0; i < nof_subscopes; i++) {
    _subscopes->at_put(i, new ciScope(this, cscope->subscope_at(i)));
  }
}

// ------------------------------------------------------------------
// ciScope::ciScope
//
// Recursive subscope constructor
ciScope::ciScope(ciScope* caller, CompilationScope* cscope) {
  ASSERT_IN_VM;
  _method = NULL;
  _method_handle = JNIHandles::make_local(cscope->method()());
  _caller = caller;
  _caller_bci = cscope->callerBCI();
  int nof_subscopes = cscope->nof_subscopes();
  Arena* arena = CURRENT_ENV->arena();
  _subscopes =
    new (arena) GrowableArray<ciScope*>(arena, nof_subscopes,
					  nof_subscopes, NULL);
  for (int i = 0; i < nof_subscopes; i++) {
    _subscopes->at_put(i, new ciScope(this, cscope->subscope_at(i)));
  }
}

// ------------------------------------------------------------------
// ciScope::method
//
// What method does this scope correspond to?
ciMethod* ciScope::method() {
  if (_method == NULL) {
    VM_ENTRY_MARK;
    _method = CURRENT_THREAD_ENV->get_object(JNIHandles::resolve(_method_handle))->as_method();
  }
  return _method;
}

// ------------------------------------------------------------------
// ciScope::index_of_subscope_at_bci
//
// Look through our list of subscopes and find the one with the
// matching bci.
//
// Returns -1 if there is no subscope with the given bci.
int ciScope::index_of_subscope_at_bci(int bci) const {
  if (_subscopes == NULL) return -1;
  const int len = _subscopes->length();
  for (int i = 0; i < len; i++) {
    ciScope* s = _subscopes->at(i);
    if (s->caller_bci() == bci) return i;
  }
  return -1;
}

// ------------------------------------------------------------------
// ciScope::subscope_at_bci
//
// Find the scope associated with some call site.
ciScope* ciScope::subscope_at_bci(int bci) const {
  int i = index_of_subscope_at_bci(bci);
  if (i == -1) return NULL;
  assert(i == _subscopes->length() - 1 ||
	 _subscopes->at(i+1)->caller_bci() != bci, "> 1 subscopes at this bci");
  return _subscopes->at(i);
}

#ifndef PRODUCT
//============================================================================
class Scope_MethodPrinterClosure2 : public MethodPrinterClosure {
public:
  ciScope *_scope;
  const int _indent;

  Scope_MethodPrinterClosure2( ciScope *s, int i ) : _scope(s), _indent(i) {}

  void bytecode_prolog(Bytecodes::Code code);

  void invoke_interface_by_index    (int index, symbolHandle n, symbolHandle sig, int nofArgs);
  void invoke_nonvirtual_by_index   (int index, symbolHandle n, symbolHandle sig);
  void invoke_static_by_index       (int index, symbolHandle n, symbolHandle sig);
  void invoke_virtual_klass_by_index(int index, symbolHandle n, symbolHandle sig);
  
};

void Scope_MethodPrinterClosure2::bytecode_prolog(Bytecodes::Code code) { 
  int indent = _indent;
  if (AbstractInterpreter::is_not_reached(method_handle(), bci())) {
    tty->print("- ");
    --indent;
  }
  for( int i = 0 ; i < indent; i++ )  tty->print("  ");
}

void Scope_MethodPrinterClosure2::invoke_interface_by_index (int index, symbolHandle name, symbolHandle signature, int nofArgs) { 
  tty->print_cr("%d invoke_interface %d <%s> <%s> %d ", bci(), index, name->as_C_string(), signature->as_C_string(), nofArgs); 

  ciScope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

// Method invocations for dynamic loading of classes
void Scope_MethodPrinterClosure2::invoke_nonvirtual_by_index(int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_special %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  ciScope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

void Scope_MethodPrinterClosure2::invoke_static_by_index    (int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_static %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  ciScope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

void Scope_MethodPrinterClosure2::invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature) {   
  tty->print_cr("%d invoke_virtual %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  ciScope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}
#endif // PRODUCT

// ------------------------------------------------------------------
// ciScope::print_codes
//
// Print the bytecodes for all methods in the scope tree.
// indent recursively for nested scopes.
void ciScope::print_codes( int indent ) {
#ifndef PRODUCT
  GUARDED_VM_ENTRY(
    Scope_MethodPrinterClosure2 ps(this,indent);
    MethodIterator iter((methodOop)JNIHandles::resolve(_method_handle));
    iter.iterate(&ps);
  )
#endif
}
