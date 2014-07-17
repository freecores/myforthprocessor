#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)interpreterRT_ia64.cpp	1.15 03/01/23 10:57:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_interpreterRT_ia64.cpp.incl"


#define __ _masm->


// Implementation of SignatureHandlerGenerator

void InterpreterRuntime::SignatureHandlerGenerator::pass_prev(int slot_offset) {
  Argument      jni_arg(_prev_jni_offset);
  Argument::Sig sig = _prev_sig;

  if (sig == Argument::no_sig) {
    return;
  }

  slot_offset += BytesPerWord;

  if (Argument::is_integral(sig)) {
    // Integral argument

    // Load either the output register or a very-local temp from the java stack
    // Bump java stack offset address if requested.
    const Register tmp = jni_arg.is_register() ? jni_arg.as_register() : GR2_SCRATCH;

    if (slot_offset == 0) {
      __ ld8(tmp, GR_I0);
    } else {
      __ ld8(tmp, GR_I0, -slot_offset);
    }

    if (Argument::is_4byte(sig)) {
      __ sxt4(tmp, tmp);
    }

    if (Argument::is_obj(sig)) {
      // Object, box if not null
      const PredicateRegister box = PR15_SCRATCH;

      __ cmp(box, PR0, 0, tmp, Assembler::notEqual);
      __ add(box, tmp, GR_I0, slot_offset);
    }

    if (!jni_arg.is_register()) {
      // Store into native memory parameter list
      __ add(GR3_SCRATCH, SP, jni_arg.jni_offset_in_frame());
      __ st8(GR3_SCRATCH, tmp);
    }

  } else {
    // Floating point argument
    const FloatRegister tmp = jni_arg.is_register() ? as_FloatRegister(FR_I0->encoding() + _prev_float_reg_offset) : FR6;

    if (jni_arg.is_register()) {
      if (Argument::is_4byte(sig)) {
	// Single precision float
	if (slot_offset == 0) {
	  __ ldfs(tmp, GR_I0);
	} else {
	  __ ldfs(tmp, GR_I0, -slot_offset);
	}
      } else {
	// Double precision float
	if (slot_offset == 0) {
	  __ ldfd(tmp, GR_I0);
	} else {
	  __ ldfd(tmp, GR_I0, -slot_offset);
	}
      }
    } else {
      if (slot_offset == 0) {
	__ ld8(GR2_SCRATCH, GR_I0);
      } else {
	__ ld8(GR2_SCRATCH, GR_I0, -slot_offset);
      }
      __ add(GR3_SCRATCH, SP, jni_arg.jni_offset_in_frame());
      __ st8(GR3_SCRATCH, GR2_SCRATCH);
    }
  }
}

void InterpreterRuntime::SignatureHandlerGenerator::pass_curr(Argument::Sig sig) {
  if ((sig == Argument::long_sig || sig == Argument::double_sig) &&
      _prev_sig == Argument::no_sig) {
    // First argument is a long or double in low addressed slot, so offset GR_I0.
    __ sub(GR_I0, GR_I0, BytesPerWord);
  }
  _prev_sig        = sig;
  _prev_jni_offset = jni_offset();
  if (Argument::is_float(sig)) {
    ++_prev_float_reg_offset;
  }
}


void InterpreterRuntime::SignatureHandlerGenerator::pass_int() {
  pass_prev(0);
  pass_curr(Argument::int_sig);
}


void InterpreterRuntime::SignatureHandlerGenerator::pass_object() {
  pass_prev(0);
  pass_curr(Argument::obj_sig);
}


void InterpreterRuntime::SignatureHandlerGenerator::pass_long() {
  pass_prev(BytesPerWord);
  pass_curr(Argument::long_sig);
}


void InterpreterRuntime::SignatureHandlerGenerator::pass_float() {
  pass_prev(0);
  pass_curr(Argument::float_sig);
}


void InterpreterRuntime::SignatureHandlerGenerator::pass_double() {
  pass_prev(BytesPerWord);
  pass_curr(Argument::double_sig);
}


void InterpreterRuntime::SignatureHandlerGenerator::generate(uint64_t fingerprint) {
  __ emit_fd();

  // generate code to handle arguments
  iterate(fingerprint);
  pass_prev(-BytesPerWord);

  // return result handler
  __ movl(GR_RET, (uint64_t)AbstractInterpreter::result_handler(method()->result_type()));
  __ ret();

  __ flush();
}


// Implementation of SignatureHandlerLibrary

class SignatureHandlerLibrary: public AllStatic {
 public:
  enum { size      =  1*K };                      // the size of the temporary code buffer
  enum { blob_size = 32*K };                      // the size of a handler code blob.
 private:
  static BufferBlob*              _handler_blob;  // the current buffer blob containing the generated handlers
  static address                  _handler;       // next available address within _handler_blob;
  static GrowableArray<uint64_t>* _fingerprints;  // the fingerprint collection
  static GrowableArray<address>*  _handlers;      // the corresponding handlers
  static u_char                   _buffer[SignatureHandlerLibrary::size];  // the temporary code buffer

  static address set_handler_blob() {
    BufferBlob* handler_blob = BufferBlob::create(blob_size, "native signature handlers");
    if (handler_blob == NULL) {
      fatal1("CodeCache: no room for %s", "native signature handlers");
    }
    address     handler      = handler_blob->instructions_begin();
    _handler_blob = handler_blob;
    _handler      = handler;
    return handler;
  }

  static void initialize() {
    if (_fingerprints != NULL) return;
    (void)set_handler_blob();
    _fingerprints = new(ResourceObj::C_HEAP)GrowableArray<uint64_t>(32, true);
    _handlers     = new(ResourceObj::C_HEAP)GrowableArray<address>(32, true);
  }

  static address set_handler(CodeBuffer* buffer) {
    address handler = _handler;
    int     size    = buffer->code_size();
    if (handler + size > _handler_blob->instructions_end()) {
      // get a new handler blob
      handler = set_handler_blob();
    }
    memcpy(handler, buffer->code_begin(), size);
    ((FuncDesc*)handler)->set_entry(handler + sizeof(FuncDesc));
    ICache::invalidate_range(handler, size);
    _handler = handler + size;
    return handler;
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
	  ptrdiff_t align_offset = (address)round_to((intptr_t)_buffer, CodeEntryAlignment) - (address)_buffer;
          CodeBuffer* buffer = new CodeBuffer((address)(_buffer + align_offset),
					      SignatureHandlerLibrary::size - align_offset);
          InterpreterRuntime::SignatureHandlerGenerator(method, buffer).generate(fingerprint);
          // copy into code heap
	  address handler = set_handler(buffer);
          // debugging suppport
          if (PrintSignatureHandlers) {
            tty->cr();
            tty->print_cr(
              "argument handler #%d for: %s %s (%d parameters, fingerprint = " PTR64_FORMAT ", %d bytes generated)",
              _handlers->length(),
	      (method->is_static() ? "static" : "receiver"),
	      method->signature()->as_C_string(),
	      method->size_of_parameters(),
	      fingerprint,
	      buffer->code_size());
            Disassembler::decode(handler + sizeof(FuncDesc), handler + buffer->code_size());
            #ifndef PRODUCT
            tty->print_cr(" --- associated result handler ---");
            address rh_begin = AbstractInterpreter::result_handler(method()->result_type());
            address rh_end = rh_begin;
            while (*(address*)rh_end != NULL) rh_end += sizeof(address);
            Disassembler::decode(rh_begin + sizeof(FuncDesc), rh_end);
            #endif
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


BufferBlob*              SignatureHandlerLibrary::_handler_blob = NULL;
address                  SignatureHandlerLibrary::_handler      = NULL;
GrowableArray<uint64_t>* SignatureHandlerLibrary::_fingerprints = NULL;
GrowableArray<address>*  SignatureHandlerLibrary::_handlers     = NULL;
u_char                   SignatureHandlerLibrary::_buffer[SignatureHandlerLibrary::size];


IRT_ENTRY(void, InterpreterRuntime::prepare_native_call(JavaThread* thread, methodOop method))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // lookup native function entry point if it doesn't exist
  bool in_base_library;
  if (!m->has_native_function()) NativeLookup::lookup(m, in_base_library, CHECK);
  // make sure signature handler is installed
  SignatureHandlerLibrary::add(m);

  // The interpreter entry point checks the signature handler first,
  // before trying to fetch the native entry point and klass mirror.
  // We must set the signature handler last, so that multiple processors
  // preparing the same method will be sure to see non-null entry & mirror.
IRT_END


class SlowSignatureHandler: public NativeSignatureIterator {
 private:
  intptr_t* _from;
  intptr_t* _to;
  intptr_t  *_RegArgSignature; // Signature of first Arguments to be passed in Registers
  int _argcount;

  // sign-extends to intptr_t
  virtual void pass_int()    { *_to++ = *((jint *)_from); _from--; 
                               add_signature( Argument::int_sig ); }

  // longs are in the lower addressed java stack slot
  virtual void pass_long()   { _from--; *_to++ = *_from--;
                               add_signature( Argument::long_sig ); }

  // box if not null
  virtual void pass_object() { *_to++ = (*_from == 0) ? NULL : (intptr_t)_from; _from--; 
                               add_signature( Argument::long_sig ); }

  // floats end up in low half of *_to
  virtual void pass_float()  { *_to++ = *((jint *)_from); _from--; 
                               add_signature( Argument::float_sig ); }

  // doubles are in the lower addressed java stack slot
  virtual void pass_double() { _from--; *_to++ = *_from--;
                               add_signature( Argument::double_sig ); }

  virtual void add_signature( intptr_t sig_type ) {
    // Four 2-bit slots per byte of _RegArgSignature
    if ( _argcount < sizeof(intptr_t)*4 ) {
      *_RegArgSignature |= (sig_type << (_argcount*2));
      _argcount++;
    }
  }
  
 public:
  SlowSignatureHandler(methodHandle method, intptr_t* from, intptr_t* to, intptr_t *RegArgSig) : NativeSignatureIterator(method) {
    _from = from;
    _to   = to;
    _RegArgSignature = RegArgSig;	
    *_RegArgSignature = 0;
    _argcount = method->is_static() ? 2 : 1;
  }
};


IRT_ENTRY(address, InterpreterRuntime::slow_signature_handler(JavaThread* thread, methodOop method, intptr_t* from, intptr_t* to ))
  methodHandle m(thread, method);
  assert(m->is_native(), "sanity check");
  // handle arguments
  // Warning: We use reg arg slot 00 temporarily to return the RegArgSignature
  // back to the code that pops the arguments into the CPU registers
  SlowSignatureHandler(m, from, m->is_static() ? to+2 : to+1, to).iterate(CONST64(-1));
  // return result handler
  return AbstractInterpreter::result_handler(m->result_type());
IRT_END
