#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRT_i486.cpp	1.39 03/01/23 10:54:49 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRT_i486.cpp.incl"


#define __ _masm->


// Implementation of SignatureHandlerGenerator

void InterpreterRuntime::SignatureHandlerGenerator::move(int from_offset, int to_offset) {
  __ movl(temp(), Address(from(), from_offset * wordSize));
  __ movl(Address(to(), to_offset * wordSize), temp());
}


void InterpreterRuntime::SignatureHandlerGenerator::box(int from_offset, int to_offset) {
  __ leal(temp(), Address(from(), from_offset * wordSize));
  __ cmpl(Address(from(), from_offset * wordSize), 0); // do not use temp() to avoid AGI
  Label L;
  __ jcc(Assembler::notZero, L);
  __ movl(temp(), 0);
  __ bind(L);
  __ movl(Address(to(), to_offset * wordSize), temp());
}


void InterpreterRuntime::SignatureHandlerGenerator::generate( uint64_t fingerprint) {
  // generate code to handle arguments
  iterate(fingerprint);
  // return result handler
  __ movl(eax, (int)AbstractInterpreter::result_handler(method()->result_type()));
  // return
  __ ret(0);
  __ flush();
}


Register InterpreterRuntime::SignatureHandlerGenerator::from()       { return edi; }
Register InterpreterRuntime::SignatureHandlerGenerator::to()         { return esp; }
Register InterpreterRuntime::SignatureHandlerGenerator::temp()       { return ecx; }


// Implementation of SignatureHandlerLibrary

class SignatureHandlerLibrary: public AllStatic {
 public:
  enum   { size = 1024 };                        // the size of the temporary code buffer
 private:
  static GrowableArray<uint64_t>* _fingerprints;  // the fingerprint collection
  static GrowableArray<address >* _handlers;      // the corresponding handlers
  static u_char                  _buffer[SignatureHandlerLibrary::size]; // the temporary code buffer

  static void initialize() {
    if (_fingerprints != NULL) return;
    _fingerprints = new(ResourceObj::C_HEAP)GrowableArray<uint64_t>(32, true);
    _handlers     = new(ResourceObj::C_HEAP)GrowableArray<address >(32, true);
  }

 public:
  static void add(methodHandle method) {
    if (method->signature_handler() == NULL) {
      // check if we can use customized (fast) signature handler
      if (UseFastSignatureHandlers && method->size_of_parameters() <= Fingerprinter::max_size_of_parameters) {
        // use customized signature handler
        MutexLocker mu(SignatureHandlerLibrary_lock);
        // make sure data structure is initialized
        initialize();
        // lookup method signature's fingerprint
        uint64_t fingerprint = Fingerprinter(method).fingerprint();
        int index = _fingerprints->find(fingerprint);
        // create handler if necessary
        if (index < 0) {
          ResourceMark rm;
          CodeBuffer* buffer = new CodeBuffer(_buffer, SignatureHandlerLibrary::size);
          InterpreterRuntime::SignatureHandlerGenerator(method, buffer).generate(fingerprint);
          // copy into C-heap allocated memory location
          address handler = (address)NEW_C_HEAP_ARRAY(u_char, buffer->code_size());
          memcpy(handler, buffer->code_begin(), buffer->code_size());
          // debugging suppport
          if (PrintSignatureHandlers) {
            tty->cr();
            tty->print_cr(
              "argument handler #%d for: %s %s (fingerprint = 0x%x, %d bytes generated)",
              _handlers->length(), (method->is_static() ? "static" : "receiver"), method->signature()->as_C_string(), fingerprint, buffer->code_size()
            );
            Disassembler::decode(handler, handler + buffer->code_size());
          }
          // add handler to library
          _fingerprints->append(fingerprint);
          _handlers->append(handler);
          // set handler index
          assert(_fingerprints->length() == _handlers->length(), "sanity check");
          index = _fingerprints->length() - 1;
        }
        // set handler
        method->set_signature_handler(_handlers->at(index));
      } else {
        // use generic signature handler
        method->set_signature_handler(AbstractInterpreter::slow_signature_handler());
      }
    }
    assert(
      method->signature_handler() == AbstractInterpreter::slow_signature_handler() ||
      _handlers->find(method->signature_handler()) == _fingerprints->find(Fingerprinter(method).fingerprint()),
      "sanity check"
    );
  }
};


GrowableArray<uint64_t>* SignatureHandlerLibrary::_fingerprints = NULL;
GrowableArray<address >* SignatureHandlerLibrary::_handlers     = NULL;
u_char                   SignatureHandlerLibrary::_buffer[SignatureHandlerLibrary::size];


IRT_ENTRY(void, InterpreterRuntime::prepare_native_call(JavaThread* thread, methodOop method))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // make sure signature handler is installed
  SignatureHandlerLibrary::add(m);
  // lookup native function entry point if it doesn't exist
  bool in_base_library;
  if (!m->has_native_function()) NativeLookup::lookup(m, in_base_library, CHECK);
IRT_END


class SlowSignatureHandler: public NativeSignatureIterator {
 private:
  jint* _from;
  jint* _to;

  virtual void pass_int()                        { *_to++ = *_from--; }
  virtual void pass_long()                       { _to[0] = _from[-1]; _to[1] = _from[0]; _to += 2; _from -= 2; }
  virtual void pass_object()                     { *_to++ = (*_from == 0) ? 0 : (jint)_from; _from--; }

 public:
  SlowSignatureHandler(methodHandle method, jint* from, jint* to) : NativeSignatureIterator(method) {
    _from = from;
    _to   = to;
  }
};


IRT_ENTRY(address, InterpreterRuntime::slow_signature_handler(JavaThread* thread, methodOop method, jint* from, jint* to))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // handle arguments
  SlowSignatureHandler(m, from, to + 1).iterate(CONST64(-1));
  // return result handler
  return AbstractInterpreter::result_handler(m->result_type());
IRT_END



