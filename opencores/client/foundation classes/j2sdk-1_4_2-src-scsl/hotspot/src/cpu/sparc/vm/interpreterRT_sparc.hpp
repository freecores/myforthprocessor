#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)interpreterRT_sparc.hpp	1.19 03/01/23 11:01:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

static int binary_search(int key, LookupswitchPair* array, int n);

static address iload (JavaThread* thread);
static address aload (JavaThread* thread);
static address istore(JavaThread* thread);
static address astore(JavaThread* thread);
static address iinc  (JavaThread* thread);



// native method calls

class SignatureHandlerGenerator: public NativeSignatureIterator {
 private:
  MacroAssembler* _masm;

  void pass_word(int size_of_arg, int offset_in_arg);
  void pass_int()    { pass_word(1, 0); }
  void pass_long();
  void pass_double();
  void pass_float();
  void pass_object();

 public:
  // Creation
  SignatureHandlerGenerator(methodHandle method, CodeBuffer* buffer) : NativeSignatureIterator(method) {
    _masm = new MacroAssembler(buffer);
  }

  // Code generation
  void generate( uint64_t fingerprint );
};


static void    prepare_native_call   (JavaThread* thread, methodOop method);
static address slow_signature_handler(JavaThread* thread, methodOop method, intptr_t* from, intptr_t* to);


// Return type for InterpreterRuntime::invocation_counter_overflow, such
// that the results are returned in O0 and O1.

#ifdef _LP64

// v9 ABI says that 16-byte structs are returned in O0 and O1

struct IcoResult {
  address osr_adapter_frame_address;
  address osr_code;

  IcoResult(address osr_adapter_frame_address, nmethod *osr_code)
    : osr_adapter_frame_address(osr_adapter_frame_address),
      osr_code((address)osr_code)
    {}

  IcoResult(address osr_code)
    : osr_adapter_frame_address(0),
      osr_code(osr_code)
    {}
};

static inline IcoResult makeIcoResult(address osr_adapter_frame_address, nmethod *osr_code)
{ return IcoResult(osr_adapter_frame_address, osr_code); }

static inline IcoResult makeIcoResult(address osr_code)
{ return IcoResult(osr_code); }

#else

// High half of a long is returned in O0, low half in O1

typedef jlong IcoResult;

static inline IcoResult makeIcoResult(address osr_adapter_frame_address, nmethod *osr_code)
{ return jlong_from((jint)osr_adapter_frame_address, (jint)osr_code); }

static inline IcoResult makeIcoResult(address osr_code)
{ return (jlong)(osr_code); }

#endif


//Reconciliation History
// 1.9 98/05/05 16:49:15 interpreterRT_i486.hpp
// 1.10 98/05/29 09:28:04 interpreterRT_i486.hpp
// 1.11 98/06/23 18:19:25 interpreterRT_i486.hpp
// 1.13 99/06/22 16:37:44 interpreterRT_i486.hpp
//End
