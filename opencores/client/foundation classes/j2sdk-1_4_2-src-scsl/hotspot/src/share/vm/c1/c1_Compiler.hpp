#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_Compiler.hpp	1.34 03/01/23 11:53:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// There is one instance of the Compiler per CompilerThread.

class Compiler: public AbstractCompiler {
 public:
  // Creation
  Compiler();
  ~Compiler();

  // Name of this compiler
  virtual const char* name()                     { return "HotSpot Client Compiler"; }

  // Missing feature tests
  virtual bool supports_native()                 { return true; }
  virtual bool supports_osr   ()                 { return true; } 

  // Customization
  virtual bool needs_preloaded_classes()         { return false; }
  virtual bool needs_inlining_scope   ()         { return false; }
  virtual bool needs_adapters         ()         { return false; }
  virtual bool needs_stubs            ()         { return false; }

  // Initialization
  virtual void initialize();

  // Compilation entry point for methods
  virtual void compile_method(ciEnv* env, ciScope* scope, ciMethod* target, int entry_bci, int comp_lev);

  // Print compilation timers and statistics
  virtual void print_timers();
};

