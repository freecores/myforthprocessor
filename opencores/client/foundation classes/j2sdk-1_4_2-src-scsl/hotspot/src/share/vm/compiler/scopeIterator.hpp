#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)scopeIterator.hpp	1.19 03/01/23 12:01:46 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Iterator for iterating over scopes and subscopes
class ScopeIterator : public MethodIterator {
 public:
  ScopeIterator(Scope* s, AbstractMethodIntervalFactory* factory = MethodIterator::default_factory());
  ScopeIterator(MethodInterval* interval, AbstractMethodIntervalFactory* factory = MethodIterator::default_factory());

  virtual void iterate(ScopeClosure* c);    // start iteration

  virtual ScopeIterator* clone_for_subiteration(Scope* subscope);
    // return the iterator to be used for iterating over an inlined subscope
};

// Closure for iterating over scopes and subscopes
class ScopeClosure : public MethodClosure {
 protected:
  ScopeIterator* _it;                       // our iterator
  Scope*const  _scope;                      // scope over which to iterate
 public:
  ScopeClosure(Scope* s) : _scope(s)        { _it = NULL; }

  void set_iterator(ScopeIterator* it)      { _it = it; }
  void unknown_bytecode()                   { ShouldNotReachHere(); }

 private:
  // The call bytecodes are overridden to iterate over inlined methods.
  // No matter whether there are any inlined scopes or not, the pre_... methods
  // further below are called before anything happens.
  // DO NOT OVERRIDE these in subclasses.
  void invoke_nonvirtual_by_index    (int klass_index, symbolHandle name, symbolHandle signature);
  void invoke_static_by_index        (int klass_index, symbolHandle name, symbolHandle signature);
  void invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature);

 protected:
  // override these if needed
  virtual void pre_invoke_interface_by_index     (int intfc_index, symbolHandle name, symbolHandle signature, int nofArgs) { bytecode_default(); }
  virtual void pre_invoke_nonvirtual_by_index    (int klass_index, symbolHandle name, symbolHandle signature) { bytecode_default(); }
  virtual void pre_invoke_static_by_index        (int klass_index, symbolHandle name, symbolHandle signature) { bytecode_default(); }
  virtual void pre_invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature) { bytecode_default(); }

  // iteration control
  virtual bool should_enter_scope(Scope* s, int bci)  { return true; }   
    // called before entering (sub)scope; if returning false, will not include
    // this subscope in the iteration

  virtual void enter_scope ()                         {} 
  virtual void exit_scope  ()                         {} 
    // called before entering/leaving (sub)scope

  virtual ScopeClosure* clone_for_subiteration(Scope* subscope) = 0;
    // return the closure to be used for iterating over an inlined subscope
    // (if closure has state, it usually is a good idea to clone it for each subscope)

 private:
  void iterate_over_subscope();
  friend class ScopeIterator;
};

