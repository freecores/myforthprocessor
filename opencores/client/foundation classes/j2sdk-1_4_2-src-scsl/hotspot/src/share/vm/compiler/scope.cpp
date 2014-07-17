#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)scope.cpp	1.28 03/01/23 12:01:39 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_scope.cpp.incl"


Scope::Scope(methodHandle method, Scope* caller, int bci) 
  : _method(method), _caller(caller), _callerBCI(bci) {
  assert(caller == NULL || bci != IllegalBCI, "bad parameters");
  if (caller != NULL) {
    caller->add_subscope(this);
  }
  _subscopes = NULL;    // create subscope list lazily
  _num_bytecodes = method->code_size();
  // by default, inherit caller's frequency, or assume 1.0
  _frequency = _caller == NULL ? (float)1.0 : _caller->frequency();
}


const Scope* Scope::root() const {
  for (const Scope* p = this; ; p = p->caller()) {
    if (!p->is_inlined())  return p;
  }
}


void Scope::add_subscope(Scope* callee) {
  // Subscopes are kept sorted by ascending caller bci (for more efficient iteration).
  // Right now, code ensures they are added in the correct order; if
  // this is too restrictive, change code below to explicitly sort.  -Urs 4/97
  if (_subscopes == NULL) _subscopes = new GrowableArray<Scope*>(5);
  assert(_subscopes->is_empty() || _subscopes->last()->callerBCI() <= callee->callerBCI(),
         "inserting scopes out of bci order");
  assert(_subscopes->find(callee) < 0, "subscope already in list");
  _subscopes->append(callee);
  int size = callee->method()->code_size();
  for (Scope* s = this; s != NULL; s = s->caller()) {
    s->_num_bytecodes += size;
  }
}

GrowableArray<Scope*>* Scope::all_scopes(GrowableArray<Scope*>* result) {
  if (result == NULL) {
    result = new GrowableArray<Scope*>(10);
  }
  result->push(this);  
  if (_subscopes != NULL) {
    for (int index = 0; index < _subscopes->length(); index++) {
      _subscopes->at(index)->all_scopes(result);
    }
  }
  return result;
}

int Scope::nof_subscopes() const {
  return _subscopes != NULL ? _subscopes->length() : 0;
}


int Scope::nof_subscopes_at(int bci) const {
  int i = index_of_subscope_at_bci(bci);
  if (i == -1) return 0;
  const int len = _subscopes->length();
  int n = 1;
  while (++i < len && _subscopes->at(i)->callerBCI() == bci) n++;
  return n;
}


Scope* Scope::subscope_at(int i) const {
  assert(_subscopes, "has no subscopes");
  return _subscopes->at(i);
}


Scope* Scope::subscope_at_bci(int bci) const {
  int i = index_of_subscope_at_bci(bci);
  if (i == -1) return NULL;
  assert(i == _subscopes->length() - 1 || _subscopes->at(i+1)->callerBCI() != bci,
         "> 1 subscopes at this bci");
  return _subscopes->at(i);
}


Scope* Scope::subscope_at_bci(int bci, int which) const {
  int i = index_of_subscope_at_bci(bci);
  if (i == -1) return NULL;
  assert(i + which < _subscopes->length(), "invalid argument");
  assert(_subscopes->at(i + which)->callerBCI() == bci, "invalid argument");
  return _subscopes->at(i + which);
}


int Scope::index_of_subscope_at_bci(int bci) const {
  if (_subscopes == NULL) return -1;
  const int len = _subscopes->length();
  for (int i = 0; i < len; i++) {
    Scope* s = _subscopes->at(i);
    if (s->callerBCI() == bci) return i;
  }
  return -1;
}


void Scope::iterate(ScopeClosure* c) {
  ScopeIterator it(this);
  it.iterate(c);
}


int Scope::inlining_depth() {
  int d = 0;
  Scope* c = caller();
  while (c != NULL) {
    c = c->caller();
    d++;
  }
  return d;
}

//---------------------------------------------------------------------------------------
#ifndef PRODUCT

class PrintScopeClosure : public ScopeClosure {
 private:
  int _nesting;

 public:
  PrintScopeClosure(Scope* s, int n = 0) : ScopeClosure(s) { _nesting = n; }
  void enter_scope ()                                      { _scope->print(_nesting); } 
  ScopeClosure* clone_for_subiteration(Scope* subscope)    { return new PrintScopeClosure(subscope, _nesting + 1); }
};


void Scope::print_tree() {
  tty->cr();
  iterate(new PrintScopeClosure(this));
}




#endif

#ifndef PRODUCT
//============================================================================
class Scope_MethodPrinterClosure : public MethodPrinterClosure {
public:
  const Scope *_scope;
  const int _indent;

  Scope_MethodPrinterClosure( const Scope *s, int i ) : _scope(s), _indent(i) {}

  void bytecode_prolog(Bytecodes::Code code);

  void invoke_interface_by_index    (int index, symbolHandle n, symbolHandle sig, int nofArgs);
  void invoke_nonvirtual_by_index   (int index, symbolHandle n, symbolHandle sig);
  void invoke_static_by_index       (int index, symbolHandle n, symbolHandle sig);
  void invoke_virtual_klass_by_index(int index, symbolHandle n, symbolHandle sig);
  
};

void Scope_MethodPrinterClosure::bytecode_prolog(Bytecodes::Code code) { 
  int indent = _indent;
  if (AbstractInterpreter::is_not_reached(method_handle(), bci())) {
    tty->print("- ");
    --indent;
  }
  for( int i = 0 ; i < indent; i++ )  tty->print("  ");
}

void Scope_MethodPrinterClosure::invoke_interface_by_index (int index, symbolHandle name, symbolHandle signature, int nofArgs) { 
  tty->print_cr("%d invoke_interface %d <%s> <%s> %d ", bci(), index, name->as_C_string(), signature->as_C_string(), nofArgs); 

  Scope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

// Method invocations for dynamic loading of classes
void Scope_MethodPrinterClosure::invoke_nonvirtual_by_index(int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_special %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  Scope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

void Scope_MethodPrinterClosure::invoke_static_by_index    (int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_static %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  Scope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}

void Scope_MethodPrinterClosure::invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature) {   
  tty->print_cr("%d invoke_virtual %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 

  Scope *ssub = _scope->subscope_at_bci(bci());
  if( ssub ) ssub->print_codes(_indent+1);
}
#endif // PRODUCT


void Scope::print_codes( int indent ) const {
#ifndef PRODUCT
  Scope_MethodPrinterClosure ps(this,indent);
  MethodIterator iter(method());
  iter.iterate(&ps);
#endif
}

