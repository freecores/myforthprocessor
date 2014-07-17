#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Compiler.cpp	1.85 03/01/23 11:53:44 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_Compiler.cpp.incl"


Compiler::Compiler() {
}


Compiler::~Compiler() {
  Unimplemented();
}


void Compiler::initialize() {
  // install exception filters that are called when an implicit exception happens outside the interpreter
  NEEDS_CLEANUP // the compiler interface should support setting these SharedRuntime values, which should be
                // done once for all (C1) Compilers instead of once for every (C1) Compiler!
  SharedRuntime::_implicit_null_exception_filter = Runtime1::entry_for(Runtime1::implicit_null_exception_id);
  SharedRuntime::_implicit_div0_exception_filter = Runtime1::entry_for(Runtime1::implicit_div0_exception_id);
  SharedRuntime::_stack_ovrflow_exception_filter = Runtime1::entry_for(Runtime1::stack_overflow_exception_id);
  // done with installation
  mark_initialized();
}


void Compiler::compile_method(ciEnv* env, ciScope* scope, ciMethod* method, int entry_bci, int comp_lev) {
  CodeBuffer* code = Runtime1::new_code_buffer();
  C1_MacroAssembler* masm = new C1_CodePatterns(code);
  // invoke compilation
  Compilation c(env, method, entry_bci, masm);
}


void Compiler::print_timers() {
  Compilation::print_timers();
}


void compiler1_init() {
  // This is needed as runtime1 stubs are generated always
  SharedInfo::set_stack0(FrameMap::stack0);
  // No initialization needed if interpreter only
  if (Arguments::mode() != Arguments::_int) {
    RegAlloc::init_register_allocation();
  }
  Runtime1::initialize();
}

