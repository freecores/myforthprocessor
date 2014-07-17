#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytecodeTracer.hpp	1.14 03/01/23 12:04:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The BytecodeTracer is a helper class used by the interpreter for run-time
// bytecode tracing. If bytecode tracing is turned on, trace() will be called
// for each bytecode.
//
// By specialising the BytecodeClosure, all kinds of bytecode traces can
// be done.

class BytecodeClosure;
class BytecodeTracer: AllStatic {
 private:
  static BytecodeClosure* _closure;

 public:
  static BytecodeClosure* std_closure();                        // a printing closure
  static BytecodeClosure* closure()				                      { return _closure; }
  static void             set_closure(BytecodeClosure* closure)	{ _closure = closure; }

  static void             trace(methodOop method, address bcp, uintptr_t tos, uintptr_t tos2);
};


// For each bytecode, a BytecodeClosure's trace() routine will be called.

class BytecodeClosure {
 public:
  virtual void trace(methodOop method, address bcp, uintptr_t tos, uintptr_t tos2) = 0;
};
