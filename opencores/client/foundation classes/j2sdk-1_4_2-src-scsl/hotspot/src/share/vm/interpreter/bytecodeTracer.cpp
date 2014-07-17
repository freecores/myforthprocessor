#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)bytecodeTracer.cpp	1.33 03/01/23 12:04:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_bytecodeTracer.cpp.incl"


// Standard closure for BytecodeTracer: prints the current bytecode
// and its attributes using bytecode-specific information.

class PrintingClosure: public BytecodeClosure {
 private:
  methodOop _current_method;
  bool      _previous_bytecode;

  void print_attributes(address bcp, int bci, const char* format) {
    address     b = bcp;
    const char* f = format;
    // iterate through format
    while (*f != '\0') {
      // remember start position and form of sub-interval in format
      address p =  b++;
      char form = *f++;
      // parse to the end of this interval
      while (form == *f) { b++; f++; }
      // get values depending on length
      int c, i, j;
      switch (b - p) {
        case  1: c = (jbyte )*p                   ; i = *p                   ; j = i                      ; break;
        case  2: c = (jshort)Bytes::get_Java_u2(p); i = Bytes::get_Java_u2(p); j = Bytes::get_native_u2(p); break;
        case  4: c = (jint  )Bytes::get_Java_u4(p); i = Bytes::get_Java_u4(p); j = Bytes::get_native_u4(p); break;
        default: ShouldNotReachHere();
      }
      // print value depending on format
      switch (form) {
        case 'c': tty->print(" %d",       c); break;
        case 'i': tty->print(" %d",       i); break;
        case 'j': tty->print(" %d",       j); break;
        case 'o': tty->print(" %d", bci + c); break;
      }
    }
  }

 public:
  PrintingClosure() {
    _current_method    = NULL;
    _previous_bytecode = Bytecodes::_nop;
  }

  void trace(methodOop method, address bcp, uintptr_t tos, uintptr_t tos2) {
#ifndef PRODUCT
    MutexLocker ml(BytecodeTrace_lock);
    if (_current_method != method) {
      // Note 1: This code will not work as expected with true MT/MP.
      //         Need an explicit lock or a different solution.
      ResourceMark rm;
      tty->cr();
      tty->print("[%d] ", (int) Thread::current()->osthread()->thread_id());
      method->print_name(tty);
      tty->cr();
      _current_method = method;
    }
    if (Verbose) {
      const char* format;
      switch (Bytecodes::length_at(bcp)) {
        case  1: format = "%x  %02x         "    ; break;
        case  2: format = "%x  %02x %02x      "  ; break;
        case  3: format = "%x  %02x %02x %02x   "; break;
        default: format = "%x  %02x %02x %02x .."; break;
      }
      tty->print(format, bcp, *bcp, *(bcp+1), *(bcp+2));
    }
    Bytecodes::Code code;
    if (_previous_bytecode == Bytecodes::_wide) {
      code = Bytecodes::cast(*(bcp+1));
    } else {
      code = Bytecodes::cast(*bcp);
    }
    int bci = bcp - method->code_base();
    const char* format = _previous_bytecode == Bytecodes::_wide ? Bytecodes::wide_format(code) : Bytecodes::format(code);
    tty->print("[%d] ", (int) Thread::current()->osthread()->thread_id());
    if (Verbose) {
      tty->print("%8d  %4d  0x%016lx 0x%016lx %s", 
	   BytecodeCounter::counter_value(), bci, tos, tos2, Bytecodes::name(code));
    } else {
      tty->print("%8d  %4d  %s", 
	   BytecodeCounter::counter_value(), bci, Bytecodes::name(code));
    }
    print_attributes(bcp, bci, format);
    tty->cr();
    _previous_bytecode = code;
#endif
  }
};


// Implementation of BytecodeTracer

BytecodeClosure* BytecodeTracer::_closure = NULL;

static PrintingClosure std_closure;
BytecodeClosure* BytecodeTracer::std_closure() {
  return &::std_closure;
}


void BytecodeTracer::trace(methodOop method, address bcp, uintptr_t tos, uintptr_t tos2 ) {
  if (TraceBytecodes && BytecodeCounter::counter_value() >= TraceBytecodesAt) _closure->trace(method, bcp, tos, tos2);
}

