#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compilationScope.cpp	1.160 03/01/23 12:00:52 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_compilationScope.cpp.incl"


CompilationScope::CompilationScope(methodHandle method, Scope* caller, int callerBCI) : Scope(method, caller, callerBCI) {
  assert(!method->is_abstract(), "cannot generate code for abstract method"); 
  assert(!method->is_native(), "no scopes for native methods");
}


void CompilationScope::inline_scopes() {
  // Determine (recursively) which callees should be inlined.
  // After this pass, all inlining decisions have been made, and
  // subsequent passes obey these decisions.
#ifdef COMPILER2
  if (Inline || InlineAccessors) {
    InliningClosure c(this);
    iterate(&c);
    if (PrintScopeTree) {
      print_tree();
    }
  }
#else
  Unimplemented();
#endif
}


class ResolveStringsClosure: public ScopeClosure {
 private:
  int       _nesting;
  Thread*   _thread;

 public:
  ResolveStringsClosure(Scope* s, Thread* thread, int n = 0) : ScopeClosure(s) { _nesting = n; _thread = thread; }
  void enter_scope () {         
    if (!_thread->has_pending_exception()) {
      _scope->method()->constants()->resolve_string_constants(_thread);
    }
  } 
  ScopeClosure* clone_for_subiteration(Scope* subscope) { return new ResolveStringsClosure(subscope, _thread, _nesting + 1); }
};


// TEMP METHOD: This should be removed when JVMCI is in place.
void CompilationScope::resolve_string_constants(TRAPS) {  
  ResolveStringsClosure rsc(this, THREAD);
  iterate(&rsc);
  // Could have a pending exception at this point
}

//--------------------------------------------------------------------------------------------
#ifndef PRODUCT

void CompilationScope::print(int nesting) {
  tty->print("%*s", nesting * 2, " ");
  method()->print_name(tty);
  tty->fill_to(68);
  tty->print("(CompilationScope*)%#x", this);
  tty->cr();
}

#endif

