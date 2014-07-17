#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodPrinter.hpp	1.25 03/01/23 12:28:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// MethodClosure for printing byte codes of a method

#ifndef PRODUCT
class MethodPrinterClosure : public MethodClosure {
protected:

  static const char* cond_op_print_string(cond_op op);
  static const char* logical_op_print_string(logical_op op);
  static const char* arithmetic_op_print_string(arithmetic_op op);
  static const char* type_name_print_string(BasicType t);

  bool   _nop_compress; // Used to avoid printing out a line for each nop.

public:
  void bytecode_prolog(Bytecodes::Code code);
  void bytecode_epilog();

  virtual void	push_int   (jint value);		
  virtual void  push_long  (jlong value);			
  virtual void  push_float (jfloat value);			
  virtual void  push_double(jdouble value);			
  virtual void  push_object(oop value);	        		
  virtual void  push_unresolved_string_at(int i);	        		
  
  virtual void  load_local_int   (int index);			
  virtual void  load_local_long  (int index);			
  virtual void  load_local_float(int index);			
  virtual void  load_local_double(int index);			
  virtual void  load_local_object(int index);			

  virtual void  store_local_int   (int index);			
  virtual void  store_local_long  (int index);			
  virtual void  store_local_float(int index);			
  virtual void  store_local_double(int index);			
  virtual void  store_local_object(int index);			
  virtual void  increment_local_int(int index, jint offset);	


  // Array operations

  virtual void  new_basic_array (int type);			
  virtual void  new_object_array(klassOop klass);	
  virtual void  new_multi_array (klassOop klass, int nof_dims);

  virtual void  new_object_array_by_index(int klass_index);	
  virtual void  new_multi_array_by_index (int klass_index, int nof_dims);

  virtual void  array_length();					
  virtual void  load_array_byte();				
  virtual void  load_array_char();				
  virtual void  load_array_short();				
  virtual void  load_array_int();			
  virtual void  load_array_long();				
  virtual void  load_array_float();				
  virtual void  load_array_double();				
  virtual void  load_array_object();				
  
  virtual void  store_array_byte();				
  virtual void  store_array_char();				
  virtual void  store_array_short();				
  virtual void  store_array_int();				
  virtual void  store_array_long();				
  virtual void  store_array_float();				
  virtual void  store_array_double();				
  virtual void  store_array_object();				

  // Stack operations
  virtual void  nop();						
  virtual void  pop();						
  virtual void  pop2();						
  virtual void  dup();						
  virtual void  dup2();						
  virtual void  dup_x1();				
  virtual void  dup2_x1();					
  virtual void  dup_x2();				
  virtual void  dup2_x2(); 
  virtual void  swap();						

  // Unary arithmetic operations
  virtual void  neg_int();					
  virtual void  neg_long();					
  virtual void  neg_float();					
  virtual void  neg_double();					

  // Binary arithmetic operations

  virtual void  arithmetic_int   (arithmetic_op op);		
  virtual void  arithmetic_long  (arithmetic_op op);		
  virtual void  arithmetic_float(arithmetic_op op);		
  virtual void  arithmetic_double(arithmetic_op op);		

  // Binary logical operations

  virtual void  logical_int (logical_op op);			
  virtual void  logical_long(logical_op op);			

  // Conversion operations
  virtual void  int_2_long();					
  virtual void  int_2_float();					
  virtual void  int_2_double();					

  virtual void  long_2_int();					
  virtual void  long_2_float();					
  virtual void  long_2_double();					

  virtual void  float_2_int();					
  virtual void  float_2_long();					
  virtual void  float_2_double();				

  virtual void  double_2_int();					
  virtual void  double_2_long();					
  virtual void  double_2_float();				


  virtual void  int_2_byte();					
  virtual void  int_2_char();					
  virtual void  int_2_short();					

  // Control transfer operations

  virtual void  branch_if     (cond_op cond, int dest);		
  virtual void  branch_if_icmp(cond_op cond, int dest);		
  virtual void  branch_if_acmp(cond_op cond, int dest);		

  virtual void  long_cmp();					
  virtual void  float_cmpl();					
  virtual void  float_cmpg();					
  virtual void  double_cmpl();					
  virtual void  double_cmpg();					

  virtual void  branch(int dest);			
  virtual void  jsr(int dest);					
  virtual void  ret(int index); 
  virtual void  return_int();					
  virtual void  return_long();					
  virtual void  return_float();					
  virtual void  return_double();					
  virtual void  return_object();					
  virtual void  return_void();					

  virtual void  table_switch(jint lo, jint hi, jint dest[], int default_dest);
  virtual void  lookup_switch(int len, jint key[], jint dest[], int default_dest);

  // Object field manipulation
  virtual void  get_field_by_index (int klass_index, symbolOop field);
  virtual void  put_field_by_index (int klass_index, symbolOop field);
  virtual void  get_field(BasicType field_type, int field_offset);
  virtual void  put_field(BasicType field_type, int field_offset);
  virtual void  get_static_by_index (int index);
  virtual void  put_static_by_index (int index);


  // Method invocation
  virtual void  invoke_interface_by_index (int index, symbolHandle name, symbolHandle signature, int nofArgs);	
  
  // Method invocations for dynamic loading of classes
  virtual void	invoke_nonvirtual_by_index   (int klass_index, symbolHandle name, symbolHandle signature);
  virtual void	invoke_static_by_index       (int klass_index, symbolHandle name, symbolHandle signature);
  virtual void  invoke_virtual_klass_by_index(int klass_index, symbolHandle name, symbolHandle signature);

  // Exception handling
  virtual void  throw_exception();				
  
  // Object allocation
  virtual void  new_object_by_index (int klass_index);			
  virtual void  check_cast_by_index (int klass_index);			 
  virtual void  instance_of_by_index(int klass_index);			
  
  // Fast bytecodes
  virtual void fast_iaccess_0(int field_offset);
  virtual void fast_aaccess_0(int field_offset);

  // Monitors
  virtual void  monitor_enter();				
  virtual void  monitor_exit();					

  virtual void	unknown_bytecode()            { tty->print_cr("unknown bytecode"); }
  virtual void  bytecode_default()            { ShouldNotCallThis(); }

  MethodPrinterClosure() { _nop_compress = false; }
};
#endif




