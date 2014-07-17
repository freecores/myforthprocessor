#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compilationScope.hpp	1.87 03/01/23 12:00:55 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The class CompilationScope contains variables that are typical for each
// compiled method (or method scope)

class CompilationScope: public Scope {
 public:
  CompilationScope(methodHandle method, Scope* caller = NULL, int callerBCI = IllegalBCI);

  void inline_scopes();
  void resolve_string_constants(TRAPS);

  // redefined accessors (to centralize casts to CompilationScope
  CompilationScope* caller() const                         { return (CompilationScope*)_caller; }
  CompilationScope* subscope_at(int i) const               { return (CompilationScope*)Scope::subscope_at(i); }
  CompilationScope* subscope_at_bci(int bci) const         { return (CompilationScope*)Scope::subscope_at_bci(bci); }
  CompilationScope* subscope_at_bci(int bci, int i) const  { return (CompilationScope*)Scope::subscope_at_bci(bci, i); }

  void print(int indent) PRODUCT_RETURN;
}; 


