#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c2compiler.cpp	1.12 03/01/23 12:15:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c2compiler.cpp.incl"


void C2Compiler::initialize() {
  mark_initialized();
}

void C2Compiler::compile_method(ciEnv* env,
				ciScope* scope,
				ciMethod* target,
				int entry_bci,
                                int comp_lev) {
  if (target->flags().is_native()) {
    Compile C(env, target);
  } else {
    bool subsume_loads = true;
    bool retry;
    do {
      // Attempt to compile while subsuming loads into machine instructions.
      Compile C(env, scope, target, entry_bci, subsume_loads, comp_lev);

      // Check result and retry if appropriate.
      switch (C.result()) {
      case Compile::Comp_subsumed_load_conflict:
	// Retry, without subsuming loads.
	assert(subsume_loads, "unexpected recompilation");
	retry = true;
	subsume_loads = false;
	break;
      default:
	retry = false;
	break;
      }
    } while (retry);
  }
}

void C2Compiler::compile_adapter(ciEnv* env,
				 ciMethod* callee,
				 int/*AdapterType*/ type) {
  Compile C(env, callee, type);
  if (!C.has_root()) {
    if (PrintCompilation && (Verbose || WizardMode)) {
      tty->print("COMPILE FAILED for adapter of ");
      callee->print_name();
      tty->cr();
    }
    callee->set_not_compilable();
  }
}

void C2Compiler::print_timers() {
  // do nothing
}
