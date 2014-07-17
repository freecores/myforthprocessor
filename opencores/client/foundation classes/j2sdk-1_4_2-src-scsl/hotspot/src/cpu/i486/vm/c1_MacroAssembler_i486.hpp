#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_MacroAssembler_i486.hpp	1.21 03/01/23 10:53:21 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// C1_MacroAssembler contains high-level macros for C1

class C1_CodePatterns;

class C1_MacroAssembler: public MacroAssembler {
 private:
  int _esp_offset;    // track esp changes
 public:
  // creation
  C1_MacroAssembler(CodeBuffer* code) : MacroAssembler(code), _esp_offset(0) {}

  // locking
  // hdr     : must be eax, contents destroyed
  // obj     : must point to the object to lock, contents preserved
  // disp_hdr: must point to the displaced header location, contents preserved
  void lock_object  (Register swap, Register obj, Register lock, Label& slow_case);

  // unlocking
  // hdr     : contents destroyed
  // obj     : must point to the object to lock, contents preserved
  // disp_hdr: must be eax & must point to the displaced header location, contents destroyed
  void unlock_object(Register swap, Register obj, Register lock, Label& slow_case);

  // allocation of fixed-size objects
  // (can also be used to allocate fixed-size arrays, by setting
  // hdr_size correctly and storing the array length afterwards)
  // obj        : must be eax, will contain pointer to allocated object
  // t1, t2     : scratch registers - contents destroyed
  // header_size: size of object header in words
  // object_size: total size of object in words
  // slow_case  : exit to slow case implementation if fast allocation fails
  void allocate_object(Register obj, Register t1, Register t2, int header_size, int object_size, Register klass, Label& slow_case);

  // allocation of arrays
  // obj        : must be eax, will contain pointer to allocated object
  // len        : array length in number of elements
  // t          : scratch register - contents destroyed
  // header_size: size of object header in words
  // f          : element scale factor
  // slow_case  : exit to slow case implementation if fast allocation fails
  void allocate_array(Register obj, Register len, Register t, Register t2, int header_size, Address::ScaleFactor f, Register klass, Label& slow_case);

  virtual C1_CodePatterns* as_CodePatterns()        { return NULL; }

  int  esp_offset() const { return _esp_offset; } 
  void set_esp_offset(int n) { _esp_offset = n; }

  // Note: NEVER push values directly, but only through following push_xxx functions;
  //       This helps us to track the esp changes compared to the entry esp (->_esp_offset)

  void push_jint (jint i)     { _esp_offset++; pushl(i); }
  void push_oop  (jobject o)  { _esp_offset++; pushl(o); }
  void push_addr (Address a)  { _esp_offset++; pushl(a); }
  void push_reg  (Register r) { _esp_offset++; pushl(r); }
  void pop       (Register r) { _esp_offset--; popl (r); assert(_esp_offset >= 0, "stack offset underflow"); }

  void dec_stack (int nof_words) {
    _esp_offset -= nof_words;
    assert(_esp_offset >= 0, "stack offset underflow");
    addl(esp, wordSize * nof_words);
  }

  void dec_stack_after_call (int nof_words) {
    _esp_offset -= nof_words;
    assert(_esp_offset >= 0, "stack offset underflow");
  }
};
