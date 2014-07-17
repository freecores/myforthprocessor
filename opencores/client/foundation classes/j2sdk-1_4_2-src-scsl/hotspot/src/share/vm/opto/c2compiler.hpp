#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c2compiler.hpp	1.10 03/01/23 12:15:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class C2Compiler : public AbstractCompiler {
public:
  // Name
  const char *name() { return "opto"; }

  // Customization
  bool needs_preloaded_classes() { return false; }
  bool needs_inlining_scope   () { return !UseOptoInlining; }
  bool needs_adapters         () { return true; }
  bool needs_stubs            () { return true; }
  
  void initialize();

  // Compilation entry point for methods
  void compile_method(ciEnv* env,
		      ciScope* scope,
		      ciMethod* target,
		      int entry_bci,
                      int comp_lev); 
  
  // Compilation entry point for adapters
  void compile_adapter(ciEnv* env,
		       ciMethod* callee,
		       int/*AdapterType*/ type);

  // Print compilation timers and statistics
  void print_timers();
};







