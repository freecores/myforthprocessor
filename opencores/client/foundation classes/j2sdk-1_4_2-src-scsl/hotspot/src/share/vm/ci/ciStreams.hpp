#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciStreams.hpp	1.14 03/02/28 09:18:07 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciFieldLayoutStream
//
// The class is used to iterate over a ciFieldLayout.
class ciFieldLayoutStream : public StackObj {
private:
  ciFieldLayout* _layout;
  int            _pos;
public:
  ciFieldLayoutStream(ciFieldLayout* layout) {
    _layout = layout;
    _pos    = 0;
  }

  bool is_done() { return _pos >= _layout->count(); }
  bool is_header_done() { return _pos >= _layout->header_count(); }

  BasicType type() {
    return _layout->type_at(_pos);
  }

  void next() {
    assert(!is_done(), "out of bounds");
    _pos++;
  }

  void next_non_pad() {
    do {
      next();
    } while (!is_done() && type() == T_VOID);
  }

  void skip_header() {
    assert(_pos < _layout->header_count(), "illegal skip_header");
    while (!is_header_done()) {
      next();
    }
  }
  
  int offset_in_bytes()  { return ( _layout->offset_at(_pos) ); }
};

// ciByteCodeStream
//
// The class is used to iterate over the bytecodes of a method.
// It hides the details of constant pool structure/access by
// providing accessors for constant pool items.
class ciByteCodeStream : StackObj {
private:
 // Handling for the weird bytecodes
  Bytecodes::Code EOBCs();	// End-of-ByteCodes
  Bytecodes::Code wide();	// Handle wide bytecode
  Bytecodes::Code table(Bytecodes::Code); // Handle complicated inline table
  Bytecodes::Code java(Bytecodes::Code); // Handle java-only fast bytecodes

  ciMethod* _method;           // the method
  ciInstanceKlass* _holder;
  address _bc_start;	       	// Start of current bytecode for table
  address _was_wide;		// Address past last wide bytecode
  address _restore_pc;		// Address to restore pc when running out of
  address _restore_end;		//   a nested bytecode buffer
  jint* _table_base;		// Aligned start of last table or switch 

  u_char java_buf[4];
  void init_java_buf() {
    java_buf[0] = Bytecodes::_aload_0; 
    java_buf[1] = Bytecodes::_fast_igetfield; 
    java_buf[2] = 0; 
    java_buf[3] = 0; 
  }

public:
  enum {EOBC=257};
  ciByteCodeStream(ciMethod* m) {
    init_java_buf();
    reset_to_method(m);
  }

  ciByteCodeStream() {
    init_java_buf();
    reset_to_method(NULL);
  }

  void Reset( address base, unsigned int size ) { 
    _bc_start =_was_wide =_restore_pc = 0; 
    _start = _pc = base; _end = base + size; }

  void reset_to_method(ciMethod* m) {
    _method = m;
    if (m == NULL) {
      _holder = NULL;
      Reset(NULL, 0);
    } else {
      _holder = m->holder();
      Reset(m->code(), m->code_size());
    }
  }

  void reset_to_bci( int bci );

  // Force the iterator to report a certain bci.
  void force_bci(int bci);

  void set_max_bci( int max ) {
    _end = _start + max;
  }
 
  address _start;		   // Start of bytecodes
  address _end;			   // Past end of bytecodes
  address _pc;			   // Current PC
  Bytecodes::Code _bc;             // Current bytecode

  address cur_bcp()             { return _bc_start; }  // Returns bcp to current instruction
  int next_bci() const          { return _pc -_start; }
  int cur_bci() const           { return _bc_start - _start; }

  Bytecodes::Code cur_bc() const{ return _bc; }
  Bytecodes::Code next_bc()     { return (Bytecodes::Code)* _pc; }
  
  // Return current ByteCode and increment PC to next bytecode, skipping all
  // intermediate constants.  Returns EOBC at end.
  // Expected usage:
  //     while( (bc = iter.next()) != EndOfByteCodes ) { ... }
  Bytecodes::Code next() {
    _bc_start = _pc;		            // Capture start of bc
    if( _pc >= _end ) return EOBCs();       // End-Of-Bytecodes
    _bc = (Bytecodes::Code)*_pc;            // Fetch bytecode
    int csize = Bytecodes::length_for(_bc); // Expected size
    if( _bc == Bytecodes::_wide ) 
      return (_bc=wide());	            // Handle wide bytecode
    if( csize == 0 ) return table(_bc);	    // Handle inline tables
    if( _bc > Bytecodes::_jsr_w)            // 
      return (_bc=java(_bc));	            // Java-fast BCs
    _pc += csize;		            // Bump PC past bytecode
    return _bc; 
  }

  bool is_wide()  { return ( _pc == _was_wide ); }

  // Get a byte index following this bytecode.
  // If prefixed with a wide bytecode, get a wide index.
  int get_index() const {
    return (_pc == _was_wide)	// was widened?
      ? Bytes::get_Java_u2(_bc_start+2)	// yes, return wide index
      : _bc_start[1];		// no, return narrow index
  }

  // Set a byte index following this bytecode.
  // If prefixed with a wide bytecode, get a wide index.
  void put_index(int idx) {
      if (_pc == _was_wide)	// was widened?
         Bytes::put_Java_u2(_bc_start+2,idx);	// yes, set wide index
      else
         _bc_start[1]=idx;		// no, set narrow index
  }

  // Get 2-byte index (getfield/putstatic/etc)
  int get_index_big() const { return Bytes::get_Java_u2(_bc_start+1); }

  // Get dimensions byte (multinewarray)
  int get_dimensions() const { return *(unsigned char*)(_pc-1); }

  // Get unsigned index fast
  int get_index_fast() const { return Bytes::get_native_u2(_pc-2); }

  // Sign-extended index byte/short, no widening
  int get_byte() const { return (int8_t)(_pc[-1]); }
  int get_short() const { return (int16_t)Bytes::get_Java_u2(_pc-2); }
  int get_long() const  { return (int32_t)Bytes::get_Java_u4(_pc-4); }

  // Get a byte signed constant for "iinc".  Invalid for other bytecodes.
  // If prefixed with a wide bytecode, get a wide constant
  int get_iinc_con() const {return (_pc==_was_wide) ? get_short() :get_byte();}

  // 2-byte branch offset from current pc
  int get_dest( ) const {
    assert( Bytecodes::length_at(_bc_start) == sizeof(jshort)+1,  "dest2 called with bad bytecode" );
    return _bc_start-_start + (short)Bytes::get_Java_u2(_pc-2);
  }

  // 4-byte branch offset from current pc
  int get_far_dest( ) const {
    assert( Bytecodes::length_at(_bc_start) == sizeof(jint)+1, "dest4 called with bad bytecode" );
    return _bc_start-_start + (int)Bytes::get_Java_u4(_pc-4);
  }

  // For a lookup or switch table, return target destination 
  int get_int_table( int index ) const { 
    return Bytes::get_Java_u4((address)&_table_base[index]); }

  // For tableswitch - get length of offset part
  int get_tableswitch_length()  { return get_int_table(2)-get_int_table(1)+1; }

  int get_dest_table( int index ) const {
    return cur_bci() + get_int_table(index); }

  // --- Constant pool access ---
  
private:
  int get_constant_index() const;
  int get_field_index();
  int get_method_index();

public:

  // If this bytecode is a new, newarray, multianewarray, instanceof,
  // or checkcast, get the referenced klass.
  ciKlass* get_klass(bool& will_link);
  int get_klass_index();

  // If this bytecode is one of the ldc variants, get the referenced
  // constant
  ciConstant get_constant();
  // True if the ldc variant points to an unresolved string
  bool is_unresolved_string() const;

  // If this bytecode is one of get_field, get_static, put_field,
  // or put_static, get the referenced field.
  ciField* get_field(bool& will_link);
  ciInstanceKlass* get_declared_field_holder();
  int      get_field_holder_index();
  int      get_field_signature_index();

  // If this is a method invocation bytecode, get the invoked method.
  ciMethod* get_method(bool& will_link);
  ciInstanceKlass* get_declared_method_holder();
  int       get_method_holder_index();
  int       get_method_signature_index();
};


// ciSignatureStream
//
// The class is used to iterate over the elements of a method signature.
class ciSignatureStream : public StackObj {
private:
  ciSignature* _sig;
  int    _pos;
public:
  ciSignatureStream(ciSignature* signature) {
    _sig = signature;
    _pos = 0;
  }

  bool at_return_type() { return _pos == _sig->count(); }

  bool is_done() { return _pos > _sig->count(); }

  void next() {
    if (_pos <= _sig->count()) {
      _pos++;
    }
  }

  ciType* type() {
    if (at_return_type()) {
      return _sig->return_type();
    } else {
      return _sig->type_at(_pos);
    }
  }
};


// ciExceptionHandlerStream
//
// The class is used to iterate over the exception handlers of
// a method.
class ciExceptionHandlerStream : public StackObj {
private:
  // The method whose handlers we are traversing
  ciMethod* _method;

  // Our current position in the list of handlers
  int        _pos;
  int        _end;

  ciInstanceKlass*  _exception_klass;
  int        _bci;
  bool       _is_exact;

public:
  ciExceptionHandlerStream(ciMethod* method) {
    _method = method;

    // Force loading of method code and handlers.
    _method->code();

    _pos = 0;
    _end = _method->_handler_count;
    _exception_klass = NULL;
    _bci    = -1;
    _is_exact = false;
  }

  ciExceptionHandlerStream(ciMethod* method, int bci,
                           ciInstanceKlass* exception_klass = NULL,
                           bool is_exact = false) {
    _method = method;

    // Force loading of method code and handlers.
    _method->code();

    _pos = -1;
    _end = _method->_handler_count + 1; // include the rethrow handler
    _exception_klass = (exception_klass != NULL && exception_klass->is_loaded()
                          ? exception_klass
                          : NULL);
    _bci = bci;
    assert(_bci >= 0, "bci out of range");
    _is_exact = is_exact;
    next();
  }

  // These methods are currently implemented in an odd way.
  // Count the number of handlers the iterator has ever produced
  // or will ever produce.  Do not include the final rethrow handler.
  // That is, a trivial exception handler stream will have a count
  // of zero and produce just the rethrow handler.
  int count();

  // Count the number of handlers this stream will produce from now on.
  // Include the current handler, and the final rethrow handler.
  // The remaining count will be zero iff is_done() is true, 
  int count_remaining();

  bool is_done() {
    return (_pos >= _end);
  }

  void next() {
    _pos++;
    if (_bci != -1) {
      // We are not iterating over all handlers...
      while (!is_done()) {
        ciExceptionHandler* handler = _method->_exception_handlers[_pos];
        if (handler->is_in_range(_bci)) {
          if (handler->is_catch_all()) {
            // Found final active catch block.
            _end = _pos+1;
            return;
          } else if (_exception_klass == NULL || !handler->catch_klass()->is_loaded()) {
            // We cannot do any type analysis here.  Must conservatively assume
            // catch block is reachable.
            return;
          } else if (_exception_klass->is_subtype_of(handler->catch_klass())) {
            // This catch clause will definitely catch the exception.
            // Final candidate.
            _end = _pos+1;
            return;
          } else if (!_is_exact &&
                     handler->catch_klass()->is_subtype_of(_exception_klass)) {
            // This catch block may be reachable.
            return;
          }
        }

        // The catch block was not pertinent.  Go on.
        _pos++;
      }
    } else {
      // This is an iteration over all handlers.
      return;
    }
  }

  ciExceptionHandler* handler() {
    return _method->_exception_handlers[_pos];
  }
};




