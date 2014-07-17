#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodPrinter.cpp	1.38 03/01/23 12:28:36 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_methodPrinter.cpp.incl"

#ifndef PRODUCT

const char* MethodPrinterClosure::type_name_print_string(BasicType t) {
  const char* name = type2name(t);
  if (name == NULL) {
    assert(t == T_ILLEGAL, "");
    return "##ILLEGAL##";
  }
  return name;
}

void MethodPrinterClosure::bytecode_prolog(Bytecodes::Code code) { 
  tty->indent();
  if (_nop_compress && code != Bytecodes::_nop) {
    tty->print_cr("<...more nops....>");
    tty->indent();
    _nop_compress = false;
  }                                                
}

void MethodPrinterClosure::bytecode_epilog() {
#ifndef CORE
  methodDataOop mdo = method_handle()->method_data();
  if (mdo != NULL) {
    ProfileData* data = mdo->bci_to_data(bci());
    if (data != NULL) {
      tty->print("  %d", mdo->dp_to_di(data->dp()));
      tty->fill_to(6);
      data->print_data_on(tty);
    }
  }
#endif
}

// Loads & stores

void MethodPrinterClosure::push_int(jint value) { 
  tty->print_cr("%d push_int " INT32_FORMAT " ", bci(), value);
}

void MethodPrinterClosure::push_long(jlong value) { 
  tty->print_cr("%d push_long " INT64_FORMAT " ", bci(), value);
}

void MethodPrinterClosure::push_float(jfloat value) {  
  tty->print_cr("%d push_float %f ", bci(), value); 
}

void MethodPrinterClosure::push_double(jdouble value) {
  tty->print_cr("%d push_double %f ", bci(), value); 
}

void MethodPrinterClosure::push_object(oop value){       
  tty->print("%d push_object ", bci());
  if (value == NULL) {
    tty->print("NULL");
  } else {
    EXCEPTION_MARK;
    Handle h_value (THREAD, value);
    symbolHandle sym = java_lang_String::as_symbol(h_value, CATCH);
    if (sym->utf8_length() > 32) {
      tty->print("....");
    } else {
      sym->print();
    }
  }
  tty->cr();
}

void MethodPrinterClosure::push_unresolved_string_at(int i) {       
  tty->print_cr("%d push <unresolved string at %d>", bci(), i);  
}


// to do: print out name of local variable, rather than just index
void MethodPrinterClosure::load_local_int(int index) {			
  tty->print_cr("%d load_local_int #%d ", bci(),  index); 
}

void MethodPrinterClosure::load_local_long(int index) {			
  tty->print_cr("%d load_local_long #%d ", bci(),  index); 
}

void MethodPrinterClosure::load_local_float(int index) {
  tty->print_cr("%d load_local_float #%d ", bci(),  index); 
}

void MethodPrinterClosure::load_local_double(int index) {
 tty->print_cr("%d load_local_double #%d ", bci(),  index); 
}

void MethodPrinterClosure::load_local_object(int index) {
  tty->print_cr("%d load_local_object #%d ", bci(),  index); 
}

void MethodPrinterClosure::store_local_int(int index) {
  tty->print_cr("%d store_local_int #%d ", bci(),  index); 
}
void MethodPrinterClosure::store_local_long  (int index) {
  tty->print_cr("%d store_local_long #%d ", bci(),  index); 
}

void MethodPrinterClosure::store_local_float(int index) {
  tty->print_cr("%d store_local_float #%d ", bci(),  index); 
}

void MethodPrinterClosure::store_local_double(int index) {
  tty->print_cr("%d store_local_double #%d ", bci(),  index); 
}

void MethodPrinterClosure::store_local_object(int index) {
  tty->print_cr("%d store_local_object #%d ", bci(),  index); 
}

void MethodPrinterClosure::increment_local_int(int index, jint offset) {
  tty->print_cr("%d increment_local_int #%d " INT32_FORMAT, bci(),  index, offset); 
}

// Array operations

const char* array_type_print_string(int atype)  {
  const char* str = type2name((BasicType)atype);
  if (str == NULL || atype == T_OBJECT || atype == T_ARRAY) {
    assert(false, "Unidentified basic type");
  }
  return str;
}

void MethodPrinterClosure::new_basic_array (int type) {
  tty->print_cr("%d new_basic_array %s ", bci(),  array_type_print_string(type)); 
}

void MethodPrinterClosure::new_object_array(klassOop klass) {
  tty->print("%d new_object_array_by_index ", bci());
  klass->klass_part()->name()->print_symbol_on(tty);
  tty->cr();
}

void MethodPrinterClosure::new_multi_array (klassOop klass, int nof_dims) {
  tty->print("%d new_multi_array_by_index ", bci());
  klass->klass_part()->name()->print_symbol_on(tty);
  tty->print_cr(" %d ", nof_dims);
}

void MethodPrinterClosure::new_object_array_by_index(int klass_index) {
  tty->print_cr("%d new_object_array_by_index %d ", bci(),  klass_index); 
}

void MethodPrinterClosure::new_multi_array_by_index (int klass_index, int nof_dims) {
  tty->print_cr("%d new_multi_array_by_index %d %d ", bci(),  klass_index, nof_dims); 
}

void MethodPrinterClosure::array_length() {
  tty->print_cr("%d array_length ", bci()); 
}

void MethodPrinterClosure::load_array_byte() {
  tty->print_cr("%d load_array_byte ", bci()); 
}

void MethodPrinterClosure::load_array_char() {
  tty->print_cr("%d load_array_char ", bci()); 
}

void MethodPrinterClosure::load_array_short() {
  tty->print_cr("%d load_array_short  ", bci()); 
}

void MethodPrinterClosure::load_array_int() {
  tty->print_cr("%d load_array_int  ", bci()); 
}

void MethodPrinterClosure::load_array_long() {
  tty->print_cr("%d load_array_long  ", bci()); 
}

void MethodPrinterClosure::load_array_float() {	
  tty->print_cr("%d load_array_float  ", bci()); 
}

void MethodPrinterClosure::load_array_double() {    
  tty->print_cr("%d load_array_double  ", bci());
}

void MethodPrinterClosure::load_array_object() {    
  tty->print_cr("%d load_array_object  ", bci()); 
}

void MethodPrinterClosure::store_array_byte()  {    
  tty->print_cr("%d store_array_byte  ", bci()); 
}

void MethodPrinterClosure::store_array_char() {    
  tty->print_cr("%d store_array_char ", bci()); 
}

void MethodPrinterClosure::store_array_short() {    
  tty->print_cr("%d store_array_short ", bci()); 
}

void MethodPrinterClosure::store_array_int() {   
  tty->print_cr("%d store_array_int ", bci()); 
}

void MethodPrinterClosure::store_array_long() {    
  tty->print_cr("%d store_array_long ", bci()); 
}

void MethodPrinterClosure::store_array_float() {    
  tty->print_cr("%d store_array_float ", bci()); 
}

void MethodPrinterClosure::store_array_double() {    
  tty->print_cr("%d store_array_double ", bci()); 
}

void MethodPrinterClosure::store_array_object() {    
  tty->print_cr("%d store_array_object ", bci()); 
}

// Stack operations
void MethodPrinterClosure::nop() {      
  if (!_nop_compress) {
    tty->print_cr("%d nop ", bci()); 
    _nop_compress = true;
  }  
}

void MethodPrinterClosure::pop() {      
  tty->print_cr("%d pop ", bci()); 
}

void MethodPrinterClosure::pop2() {      
  tty->print_cr("%d pop2 ", bci()); 
}

void MethodPrinterClosure::dup() {      
  tty->print_cr("%d dup ", bci()); 
}

void MethodPrinterClosure::dup2()  {      
  tty->print_cr("%d dup2 ", bci()); 
}

void MethodPrinterClosure::dup_x1() {     
  tty->print_cr("%d dup_x1 ", bci()); 
}

void MethodPrinterClosure::dup2_x1() {    
  tty->print_cr("%d dup2_x1 ", bci()); 
}

void MethodPrinterClosure::dup_x2()  {    
  tty->print_cr("%d dup_x2 ", bci()); 
}

void MethodPrinterClosure::dup2_x2() {    
  tty->print_cr("%d dup2_x2 ", bci()); 
}

void MethodPrinterClosure::swap() {     
  tty->print_cr("%d swap ", bci()); 
}

// Unary arithmetic operations
void MethodPrinterClosure::neg_int() {     
  tty->print_cr("%d neg_int ", bci()); 
}

void MethodPrinterClosure::neg_long() {     
  tty->print_cr("%d neg_long ", bci()); 
}

void MethodPrinterClosure::neg_float() {     
  tty->print_cr("%d neg_float ", bci()); 
}

void MethodPrinterClosure::neg_double() {     
  tty->print_cr("%d neg_double ", bci()); 
}

 // Binary arithmetic operations

const char* MethodPrinterClosure::arithmetic_op_print_string(arithmetic_op op)  {
  switch (op) {
    case add: return "add";
    case sub: return "sub";
    case mul: return "mul";
    case div: return "div";
    case rem: return "rem";
    case shr: return "shr";
    default: assert(false, "Unidentified arithmetic-op");
  }
  return "";
};

void MethodPrinterClosure::arithmetic_int   (arithmetic_op op) {  
  tty->print_cr("%d arithmetic_int_%s ", bci(), arithmetic_op_print_string(op)); 
}

void MethodPrinterClosure::arithmetic_long  (arithmetic_op op) {  
  tty->print_cr("%d arithmetic_long_%s ", bci(), arithmetic_op_print_string(op)); 
}

void MethodPrinterClosure::arithmetic_float(arithmetic_op op) {  
  tty->print_cr("%d arithmetic_float_%s ", bci(), arithmetic_op_print_string(op)); 
}

void MethodPrinterClosure::arithmetic_double(arithmetic_op op) {  
  tty->print_cr("%d arithmetic_double_%s ", bci(), arithmetic_op_print_string(op)); 
}

// Binary logical operations

const char* MethodPrinterClosure::logical_op_print_string(logical_op op) {
  switch (op) {
    case _shl:  return "shl";
    case _ushr: return "ushr";
    case _and:  return "and";
    case _or:   return "or";
    case _xor:  return "xor";
    default: assert(false, "Unidentified logical-op");
  }
  return "";
}

void MethodPrinterClosure::logical_int (logical_op op) {   
  tty->print_cr("%d logical_int_%s ", bci(), logical_op_print_string(op)); 
}

void MethodPrinterClosure::logical_long(logical_op op) {   
  tty->print_cr("%d logical_long_%s ", bci(), logical_op_print_string(op)); 
}

// Conversion operations
void MethodPrinterClosure::int_2_long() {     
  tty->print_cr("%d int_2_long ", bci()); 
}

void MethodPrinterClosure::int_2_float() {     
  tty->print_cr("%d int_2_float ", bci()); 
}

void MethodPrinterClosure::int_2_double() {     
  tty->print_cr("%d int_2_double ", bci()); 
}

void MethodPrinterClosure::long_2_int() {     
  tty->print_cr("%d long_2_int ", bci()); 
}
void MethodPrinterClosure::long_2_float() {     
  tty->print_cr("%d long_2_float ", bci()); 
}
void MethodPrinterClosure::long_2_double() {     
  tty->print_cr("%d long_2_double ", bci()); 
}

void MethodPrinterClosure::float_2_int()  {     
  tty->print_cr("%d float_2_int ", bci()); 
}

void MethodPrinterClosure::float_2_long() {     
  tty->print_cr("%d float_2_long ", bci()); 
}

void MethodPrinterClosure::float_2_double() {   
  tty->print_cr("%d float_2_double ", bci()); 
}

void MethodPrinterClosure::double_2_int() {     
  tty->print_cr("%d double_2_int ", bci()); 
}

void MethodPrinterClosure::double_2_long() {     
  tty->print_cr("%d double_2_long ", bci()); 
}

void MethodPrinterClosure::double_2_float() {   
  tty->print_cr("%d double_2_float ", bci()); 
}

void MethodPrinterClosure::int_2_byte() {     
  tty->print_cr("%d int_2_byte ", bci()); 
}

void MethodPrinterClosure::int_2_char() {     
  tty->print_cr("%d int_2_char ", bci()); 
}

void MethodPrinterClosure::int_2_short() {     
  tty->print_cr("%d int_2_short ", bci()); 
}

// Control transfer operations

const char* MethodPrinterClosure::cond_op_print_string(cond_op op) {
  switch (op) {
    case null: return "null";
    case nonnull: return "nonnull";
    case eq: return "eq";
    case ne: return "ne";
    case lt: return "lt";
    case le: return "le";
    case gt: return "gt";
    case ge: return "ge";
    default: assert(false, "Unidentified cond-op");
  }
  return "";
}

void MethodPrinterClosure::branch_if(cond_op cond, int dest) {
  tty->print_cr("%d branch_if_%s %d", bci(), cond_op_print_string(cond), dest); 
}
void MethodPrinterClosure::branch_if_icmp(cond_op cond, int dest) { 
  tty->print_cr("%d branch_if_icmp_%s %d", bci(), cond_op_print_string(cond), dest); 
}

void MethodPrinterClosure::branch_if_acmp(cond_op cond, int dest) { 
  tty->print_cr("%d branch_if_acmp_%s %d", bci(), cond_op_print_string(cond), dest); 
}

void MethodPrinterClosure::long_cmp() {
  tty->print_cr("%d long_cmp ", bci()); 
}

void MethodPrinterClosure::float_cmpl() {     
  tty->print_cr("%d float_cmpl ", bci()); 
}

void MethodPrinterClosure::float_cmpg() {     
  tty->print_cr("%d float_cmpg ", bci()); 
}

void MethodPrinterClosure::double_cmpl() {     
  tty->print_cr("%d double_cmpl ", bci()); 
}

void MethodPrinterClosure::double_cmpg() {     
  tty->print_cr("%d double_cmpg ", bci()); 
}

void MethodPrinterClosure::branch(int dest)  {    
  tty->print_cr("%d branch %d ", bci(), dest); 
}

void MethodPrinterClosure::jsr(int dest) {     
  tty->print_cr("%d jsr %d", bci(), dest); 
}

void MethodPrinterClosure::ret(int index)  { // to do: print name of local var at index     
  tty->print_cr("%d ret %d ", bci(), index);
}

void MethodPrinterClosure::return_int()  {     
  tty->print_cr("%d return_int ", bci()); 
}

void MethodPrinterClosure::return_long() {     
  tty->print_cr("%d return_long ", bci()); 
}

void MethodPrinterClosure::return_float() {     
  tty->print_cr("%d return_float ", bci()); 
}

void MethodPrinterClosure::return_double() {     
  tty->print_cr("%d return_double ", bci()); 
}

void MethodPrinterClosure::return_object() {     
  tty->print_cr("%d return_object ", bci()); 
}

void MethodPrinterClosure::return_void() {     
  tty->print_cr("%d return_void ", bci()); 
}

// Ideally, we should keep track of the current byte code index, and use it to compute
// the actual targets of these instructions
void MethodPrinterClosure::table_switch(jint lo, jint hi, jint dest[], int default_dest)  { 
  tty->print_cr("%d table_switch %d " INT32_FORMAT " " INT32_FORMAT " ", bci(), default_dest, lo, hi); 
  for (int ll = lo; ll <= hi; ll++)  {
    int idx = ll - lo;
    tty->print_cr("%d %d : " INT32_FORMAT " (delta: %d)", bci(), ll, dest[idx], dest[idx]-bci());
  }
}

void MethodPrinterClosure::lookup_switch(int len, jint key[], jint dest[], int default_dest) { 
  tty->print_cr("%d lookup_switch %d %d ", bci(), default_dest, len); 
  for (int ll = 0; ll < len; ll++)  {
    tty->print_cr("%d " INT32_FORMAT " : " INT32_FORMAT, bci(), key[ll], dest[ll]);
  }
}

// Object field manipulation
void MethodPrinterClosure::get_field_by_index (int klass_index, symbolOop field) { 
  tty->print_cr("%d get_field %d <%s>", bci(), klass_index, field->as_C_string()); 
}

void MethodPrinterClosure::put_field_by_index (int klass_index, symbolOop field) { 
  tty->print_cr("%d put_field %d <%s> ", bci(), klass_index, field->as_C_string()); 
}

void MethodPrinterClosure::get_field(BasicType field_type, int field_offset) {
  tty->print_cr("%d get_field %s %d", bci(), type_name_print_string(field_type), field_offset);
}

void MethodPrinterClosure::put_field(BasicType field_type, int field_offset) {
  tty->print_cr("%d put_field %s %d", bci(), type_name_print_string(field_type), field_offset);
}

void MethodPrinterClosure::get_static_by_index (int index) {
  tty->print_cr("%d get_static %d ", bci(), index); 
}

void MethodPrinterClosure::put_static_by_index (int index) { 
  tty->print_cr("%d set_static %d ", bci(), index); 
}

// Method invocation
void MethodPrinterClosure::invoke_interface_by_index (int index, symbolHandle name, symbolHandle signature, int nofArgs) { 
  tty->print_cr("%d invoke_interface %d <%s> <%s> %d ", bci(), index, name->as_C_string(), signature->as_C_string(), nofArgs); 
}

// Method invocations for dynamic loading of classes
void MethodPrinterClosure::invoke_nonvirtual_by_index(int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_special %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 
}

void MethodPrinterClosure::invoke_static_by_index    (int klass_index, symbolHandle name, symbolHandle signature)  {
  tty->print_cr("%d invoke_static %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 
}

void MethodPrinterClosure::invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature) {   
  tty->print_cr("%d invoke_virtual %d <%s> <%s> ", bci(), klass_index, name->as_C_string(), signature->as_C_string()); 
}

 // Exception handling
void MethodPrinterClosure::throw_exception() {   
  tty->print_cr("%d throw_exception ", bci()); 
}

// Object allocation
void MethodPrinterClosure::new_object_by_index(int klass_index) {   
  tty->print_cr("%d new_object %d ", bci(), klass_index); 
}

void MethodPrinterClosure::check_cast_by_index(int klass_index) {   
  tty->print_cr("%d check_cast %d ", bci(), klass_index);
}
void MethodPrinterClosure::instance_of_by_index(int klass_index) {   
  tty->print_cr("%d instance_of %d ", bci(), klass_index);
}

// Fast bytecodes
void MethodPrinterClosure::fast_iaccess_0(int field_offset) {
  tty->print_cr("%d fast_iaccess_0 %d ", bci(), field_offset);
}

void MethodPrinterClosure::fast_aaccess_0(int field_offset) {
  tty->print_cr("%d fast_aaccess_0 %d ", bci(), field_offset);
}

// Monitors
void MethodPrinterClosure::monitor_enter() {     
  tty->print_cr("%d monitor_enter ", bci()); 
}

void MethodPrinterClosure::monitor_exit()  {     
  tty->print_cr("%d monitor_exit ", bci()); 
}

#endif
