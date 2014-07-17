#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)sharedRuntime.hpp	1.113 03/01/23 12:25:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Runtime is the base class for various runtime interfaces
// (InterpreterRuntime, CompilerRuntime, etc.). It provides
// shared functionality such as exception forwarding (C++ to
// Java exceptions), locking/unlocking mechanisms, statistical
// information, etc.

class SharedRuntime: AllStatic {
 private:
  // The following function pointers provide access to the
  // corresponding StrictMath functions - used for intrinsics
  // inlined by the compiler and special-cased by the interpreter
  // (must match compiled implementation).
  typedef jdouble (JNICALL *StrictMathFunction_DD_D)(JNIEnv* env_unused, jclass class_unused, jdouble x, jdouble y);
  static void lookup_function_DD_D(StrictMathFunction_DD_D& f, const char* fname);
  static StrictMathFunction_DD_D java_lang_strict_math_pow;

 public:
  // The following entry points are used to handle implicit
  // exceptions outside the interpreter - currently they are
  // setup via the JVMCI and only used by C1.
  static address _implicit_null_exception_filter;
  static address _implicit_div0_exception_filter;
  static address _stack_ovrflow_exception_filter;

  // The following arithmetic routines are used on platforms that do
  // not have machine instructions to implement their functionality.
  // Do not remove these.

  // long arithmetics
  static jlong   lmul(jlong y, jlong x);
  static jlong   ldiv(jlong y, jlong x);
  static jlong   lrem(jlong y, jlong x);

  // float and double remainder
  static jfloat  frem(jfloat  x, jfloat  y);
  static jdouble drem(jdouble x, jdouble y);

  // float conversion (needs to set appropriate rounding mode)
  static jint    f2i (jfloat  x);
  static jlong   f2l (jfloat  x);
  static jint    d2i (jdouble x);
  static jlong   d2l (jdouble x);
  static jfloat  d2f (jdouble x);
  static jfloat  l2f (jlong   x);
  static jdouble l2d (jlong   x);

  // double trigonometrics
  static jdouble dsin(jdouble x);
  static jdouble dcos(jdouble x);
  static jdouble dpow(jdouble x, jdouble y);
  
  // exception handling accross interpreter/compiler boundaries
  static address raw_exception_handler_for_return_address(address return_address);
  static address exception_handler_for_return_address(address return_address);

  // exception handling
  static int     is_interpreter_exception();
  static address compute_exception_return_address(jint is_implicit_null);
  static address handle_array_index_check(JavaThread* thread, jint index);

  // To be used as the entry point for unresolved native methods.
  static address native_method_throw_unsatisfied_link_error_entry();

  // bytecode tracing
  static int trace_bytecode(JavaThread* thread, intptr_t preserve_this_value, intptr_t tos, intptr_t tos2);
  
  // Used to back off a spin lock that is under heavy contention
  static void yield_all(JavaThread* thread, int attempts = 0);

  // Returns null-exception object and sets the correct handler_entry for given return address.
  // Note: oop is returned through the thread object and handler_entry as return-parameter
  static address get_null_exception_entry_and_oop (JavaThread *thread);

  static oop retrieve_receiver( symbolHandle sig, frame caller );  

  // called when interpreter vtable call needs an adapter before it continues
  static address create_i2c_adapter(JavaThread* thread, methodOop method);

  // return -1 if array was not copied; returns 0 if it was copied
  static int  simple_array_copy_no_checks (int length, int dst_pos, oop dst, int src_pos, oop src);

  static void verify_caller_frame(frame caller_frame, methodHandle callee_method) PRODUCT_RETURN;
  static methodHandle find_callee_method_inside_interpreter(frame caller_frame, methodHandle caller_method, int bci) PRODUCT_RETURN_(return methodHandle(););

  // jvmpi
  static void jvmpi_method_entry_work(JavaThread* thread, methodOop method, oop receiver);
  static void jvmpi_method_entry(JavaThread* thread, methodOop method, oop receiver);
  static void jvmpi_method_exit(JavaThread* thread, methodOop method);

  // intialization
  static void initialize_StrictMath_entry_points();

  // Resets a call-site in compiled code so it will get resolved again.
  NOT_CORE(static void reresolve_call_site(frame caller);)

  // used by native wrappers to reenable yellow if overflow happened in native code
  static void reguard_yellow_pages();
};
