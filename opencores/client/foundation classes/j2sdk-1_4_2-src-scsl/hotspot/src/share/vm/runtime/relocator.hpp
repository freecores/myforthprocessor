#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)relocator.hpp	1.17 03/01/23 12:24:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This code has been converted from the 1.1E java virtual machine
// Thanks to the JavaTopics group for using the code

class ChangeItem;

// Callback object for code relocations
class RelocatorListener : public StackObj {
 public:
  RelocatorListener() {};
  virtual void relocated(int bci, int delta) = 0;
};


class Relocator : public ResourceObj {  
 public:
  Relocator(methodHandle method, RelocatorListener* listener);
  methodHandle insert_space_at(int bci, int space, u_char inst_buffer[], TRAPS);  

  // Callbacks from ChangeItem's
  bool handle_code_changes();
  bool handle_widen       (int bci, int new_ilen, u_char inst_buffer[]);  // handles general instructions
  bool handle_jump_widen  (int bci, int delta);     // handles jumps
  bool handle_switch_pad  (int bci, int old_pad, bool is_lookup_switch); // handles table and lookup switches
  
 private:
  unsigned char* _code_array;
  int            _code_array_length;
  int            _code_length;
  unsigned char* _compressed_line_number_table;
  int            _compressed_line_number_table_size;
  methodHandle   _method;
  u_char         _overwrite[3];             // stores overwritten bytes for shrunken instructions

  GrowableArray<ChangeItem*>* _changes;

  unsigned char* code_array() const         { return _code_array; }
  void set_code_array(unsigned char* array) { _code_array = array; }

  int code_length() const                   { return _code_length; }
  void set_code_length(int length)          { _code_length = length; }

  int code_array_length() const             { return _code_array_length; }
  void set_code_array_length(int length)    { _code_array_length = length; }

  unsigned char* compressed_line_number_table() const         { return _compressed_line_number_table; }
  void set_compressed_line_number_table(unsigned char* table) { _compressed_line_number_table = table; }

  int compressed_line_number_table_size() const               { return _compressed_line_number_table_size; }
  void set_compressed_line_number_table_size(int size)        { _compressed_line_number_table_size = size; }

  methodHandle method() const               { return _method; }
  void set_method(methodHandle method)      { _method = method; }
  
  Bytecodes::Code code_at(int bci) const          { return (Bytecodes::Code) code_array()[bci]; }
  void code_at_put(int bci, Bytecodes::Code code) { code_array()[bci] = (char) code; }
  
  // get and set signed integers in the code_array 
  inline int   int_at(int bci) const               { return Bytes::get_Java_u4(&code_array()[bci]); }
  inline void  int_at_put(int bci, int value)      { Bytes::put_Java_u4(&code_array()[bci], value); }

  // get and set signed shorts in the code_array
  inline short short_at(int bci) const            { return (short)Bytes::get_Java_u2(&code_array()[bci]); }
  inline void  short_at_put(int bci, short value) { Bytes::put_Java_u2((address) &code_array()[bci], value); }

  // get the address of in the code_array
  inline char* addr_at(int bci) const             { return (char*) &code_array()[bci]; }

  int  instruction_length_at(int bci)             { return Bytecodes::length_at(code_array() + bci); }  
  
  // Helper methods
  int  align(int n) const                          { return (n+3) & ~3; }
  int  code_slop_pct() const                       { return 25; }  
  bool is_opcode_lookupswitch(Bytecodes::Code bc);

  // basic relocation methods
  bool relocate_code         (int bci, int ilen, int delta);  
  void change_jumps          (int break_bci, int delta);
  void change_jump           (int bci, int offset, bool is_short, int break_bci, int delta);  
  void adjust_exception_table(int bci, int delta);  
  void adjust_line_no_table  (int bci, int delta);
  void adjust_local_var_table(int bci, int delta);
  int  get_orig_switch_pad   (int bci, bool is_lookup_switch);
  int  rc_instr_len          (int bci);  
  bool expand_code_array     (int delta);

  // Callback support
  RelocatorListener *_listener;
  void notify(int bci, int delta) { if (_listener != NULL) _listener->relocated(bci, delta); }
};

