#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classFileParser.cpp	1.187 03/01/23 12:21:28 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classFileParser.cpp.incl"

// We generally try to create the oops directly when parsing, rather than allocating
// temporary data structures and copying the bytes twice. A temporary area is only
// needed when parsing utf8 entries in the constant pool and when parsing line number
// tables.


// Parse error

void classfile_parse_error(const char* msg, TRAPS) {
  THROW_MSG(vmSymbols::java_lang_ClassFormatError(), msg); 
}

void truncated_class_file_error(TRAPS) {
  classfile_parse_error("Truncated class file", CHECK);
}

// Verification of class file properties

void inline guarantee_property(bool b, const char* msg, TRAPS) {
  if (!b) { classfile_parse_error(msg, CHECK); }
}

// We are using the JDK class file format checker, so we do not have to repeat
// the format checks while parsing. We leave the checks in place in debug mode.

#ifdef ASSERT
void inline check_property(bool b, const char* msg, TRAPS) {
  if (!b) { fatal(msg); }
}
#else
#define check_property(b, msg, traps)
#endif


void ClassFileParser::parse_constant_pool_utf8_entry(constantPoolHandle cp, int index, TRAPS) {  
  u2  utf8_length = stream()->get_u2();
  u1* utf8_buffer = stream()->get_u1_buffer(utf8_length);
  if (utf8_buffer != NULL) {
    // Got utf8 string, set stream position forward
    stream()->skip_u1(utf8_length);
    // Optimistically assume that only 1 byte UTF format is used (common case)
    symbolOop result = oopFactory::new_symbol((char*)utf8_buffer, utf8_length, CHECK);
    cp->symbol_at_put(index, result);
  } else {
    truncated_class_file_error(CHECK);
  }
}



void ClassFileParser::parse_constant_pool_integer_entry(constantPoolHandle cp, int index) {
  u4 bytes = stream()->get_u4();
  cp->int_at_put(index, (jint) bytes);
}


void ClassFileParser::parse_constant_pool_float_entry(constantPoolHandle cp, int index) {
  u4 bytes = stream()->get_u4();
  cp->float_at_put(index, *(jfloat*)&bytes);      // %%%%% move cast into separate function
}


void ClassFileParser::parse_constant_pool_long_entry(constantPoolHandle cp, int index) {
  cp->long_at_put(index, stream()->get_u8());
}


void ClassFileParser::parse_constant_pool_double_entry(constantPoolHandle cp, int index) {
  u8 bytes = stream()->get_u8();
  cp->double_at_put(index, *(jdouble*)&bytes);    // %%%%% move cast into separate function
}


void ClassFileParser::parse_constant_pool_class_entry(constantPoolHandle cp, int index) {
  u2 name_index = stream()->get_u2();
  cp->klass_index_at_put(index, name_index);
}


void ClassFileParser::parse_constant_pool_string_entry(constantPoolHandle cp, int index) {
  u2 string_index = stream()->get_u2();
  cp->string_index_at_put(index, string_index);
}


void ClassFileParser::parse_constant_pool_fieldref_entry(constantPoolHandle cp, int index) {
  u2 class_index = stream()->get_u2();
  u2 name_and_type_index = stream()->get_u2();
  cp->field_at_put(index, class_index, name_and_type_index);
}


void ClassFileParser::parse_constant_pool_methodref_entry(constantPoolHandle cp, int index) {
  u2 class_index = stream()->get_u2();
  u2 name_and_type_index = stream()->get_u2();
  cp->method_at_put(index, class_index, name_and_type_index);
}


void ClassFileParser::parse_constant_pool_interfacemethodref_entry(constantPoolHandle cp, int index) {
  u2 class_index = stream()->get_u2();
  u2 name_and_type_index = stream()->get_u2();
  cp->interface_method_at_put(index, class_index, name_and_type_index);
}

void ClassFileParser::parse_constant_pool_nameandtype_entry(constantPoolHandle cp, int index) {
  u2 name_index = stream()->get_u2();
  u2 signature_index = stream()->get_u2();
  cp->name_and_type_at_put(index, name_index, signature_index);
}


void ClassFileParser::parse_constant_pool_entry(constantPoolHandle cp, int* index, TRAPS) {
  HandleMark hm(THREAD);
  constantTag tag(stream()->get_u1());
  switch (tag.value()) {
    case JVM_CONSTANT_Class :
      parse_constant_pool_class_entry(cp, *index);
      break;
    case JVM_CONSTANT_Fieldref :
      parse_constant_pool_fieldref_entry(cp, *index);
      break;
    case JVM_CONSTANT_Methodref :
      parse_constant_pool_methodref_entry(cp, *index);
      break;
    case JVM_CONSTANT_InterfaceMethodref :
      parse_constant_pool_interfacemethodref_entry(cp, *index);
      break;
    case JVM_CONSTANT_String :
      parse_constant_pool_string_entry(cp, *index);
      break;
    case JVM_CONSTANT_Integer :
      parse_constant_pool_integer_entry(cp, *index);
      break;
    case JVM_CONSTANT_Float :
      parse_constant_pool_float_entry(cp, *index);
      break;
    case JVM_CONSTANT_Long :
      parse_constant_pool_long_entry(cp, *index);
      (*index)++;   // Skip entry following eigth-byte constant, see JVM book p. 98
      break;
    case JVM_CONSTANT_Double :
      parse_constant_pool_double_entry(cp, *index);
      (*index)++;   // Skip entry following eigth-byte constant, see JVM book p. 98
      break;
    case JVM_CONSTANT_NameAndType :
      parse_constant_pool_nameandtype_entry(cp, *index);
      break;
    case JVM_CONSTANT_Utf8 :
      parse_constant_pool_utf8_entry(cp, *index, CHECK);
      break;
    default:
      classfile_parse_error("Unknown constant tag in .class file", CHECK);
      break;
  }
}


constantPoolHandle ClassFileParser::parse_constant_pool(TRAPS) {
  u2 length = stream()->get_u2();

  constantPoolHandle nullHandle;
  constantPoolOop constant_pool =
                      oopFactory::new_constantPool(length, CHECK_(nullHandle));
  constantPoolHandle cp (THREAD, constant_pool);
  
  #ifndef PRODUCT
  cp->set_partially_loaded();    // Enables heap verify to work on partial constantPoolOops
  #endif

  // parsing  Index 0 is unused
  for (int parse_index = 1; parse_index < length; parse_index++) {
    // Pass &parse_index so padding can be done after JVM_CONSTANT_Long and
    // JVM_CONSTANT_Double, see JVM book p. 98
    parse_constant_pool_entry(cp, &parse_index, CHECK_(nullHandle));
  }

  // validate cross references and fixup class and string constants
  for (int index = 1; index < length; index++) {          // Index 0 is unused
    switch (cp->tag_at(index).value()) {
      case JVM_CONSTANT_Class :
        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
        break;
      case JVM_CONSTANT_Fieldref :
        // fall through
      case JVM_CONSTANT_Methodref :
        // fall through
      case JVM_CONSTANT_InterfaceMethodref :
        check_property(cp->tag_at(cp->klass_ref_index_at(index)).is_klass_reference(), 
          "Invalid constant in .class file", CHECK_(nullHandle));
        check_property(cp->tag_at(cp->name_and_type_ref_index_at(index)).is_name_and_type(), 
          "Invalid constant in .class file", CHECK_(nullHandle));
        break;
      case JVM_CONSTANT_String :
        ShouldNotReachHere();     // Only JVM_CONSTANT_StringIndex should be present
        break;
      case JVM_CONSTANT_Integer :
        break;
      case JVM_CONSTANT_Float :
        break;
      case JVM_CONSTANT_Long :
      case JVM_CONSTANT_Double :
        index++;
        break;
      case JVM_CONSTANT_NameAndType :
        check_property(cp->tag_at(cp->name_ref_index_at(index)).is_utf8(), 
          "Invalid constant in .class file", CHECK_(nullHandle));
        check_property(cp->tag_at(cp->signature_ref_index_at(index)).is_utf8(), 
          "Invalid constant in .class file", CHECK_(nullHandle));
        break;
      case JVM_CONSTANT_Utf8 :
        break;
      case JVM_CONSTANT_UnresolvedClass :
        ShouldNotReachHere();     // Only JVM_CONSTANT_ClassIndex should be present
        break;
      case JVM_CONSTANT_ClassIndex :
        {
          int class_index = cp->klass_index_at(index);
          check_property(cp->tag_at(class_index).is_utf8(), "Invalid constant in .class file", CHECK_(nullHandle));
          cp->unresolved_klass_at_put(index, cp->symbol_at(class_index));
        }
        break;
      case JVM_CONSTANT_UnresolvedString :
        ShouldNotReachHere();     // Only JVM_CONSTANT_StringIndex should be present
        break;
      case JVM_CONSTANT_StringIndex :
        {
          int string_index = cp->string_index_at(index);
          check_property(cp->tag_at(string_index).is_utf8(), "Invalid constant in .class file", CHECK_(nullHandle));
          symbolOop sym = cp->symbol_at(string_index);
          cp->unresolved_string_at_put(index, sym);
        }
        break;
      default:
        fatal1("bad constant pool tag value %d", cp->tag_at(index).value());
        ShouldNotReachHere();
        break;
    }
  }

  return cp;
}


objArrayHandle ClassFileParser::parse_interfaces(constantPoolHandle cp, Handle class_loader, Handle protection_domain, TRAPS) {  
  objArrayHandle nullHandle;
  u2 length = stream()->get_u2();
  if (length == 0) {
    return objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else {        
    objArrayOop interface_oop = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
    objArrayHandle interfaces (THREAD, interface_oop);
    for (int index = 0; index < length; index++) {
      u2 interface_index = stream()->get_u2();
      symbolHandle unresolved_klass (THREAD, cp->unresolved_klass_at(interface_index));
      klassOop k = SystemDictionary::resolve_or_fail(unresolved_klass,
                    class_loader, protection_domain, true, CHECK_(nullHandle));
      KlassHandle interf (THREAD, k);
      if (!Klass::cast(interf())->is_interface()) {
        THROW_MSG_(vmSymbols::java_lang_IncompatibleClassChangeError(), "Implementing class", nullHandle);
      }
      interfaces->obj_at_put(index, interf());
    }
    return interfaces;
  }
}


// Parse attributes for a field. Currently, only the ConstantValue attribute is recognized.
// The returned value is the constant pool index of the initial field value.
// This can be a constant integer, long, float, double or string. 
// 0 means no initial value for the field.

void ClassFileParser::parse_field_attributes(constantPoolHandle cp, u2* constantvalue_index_addr, bool* is_synthetic_addr, TRAPS) {  
  u2 attributes_count = stream()->get_u2();
  u2 constantvalue_index = 0;
  bool is_synthetic = false;
  while (attributes_count--) {
    u2 attribute_name_index = stream()->get_u2();
    u4 attribute_length = stream()->get_u4();
    check_property(cp->tag_at(attribute_name_index).is_utf8(), "Invalid attribute in .class file", CHECK);
    if (cp->symbol_at(attribute_name_index) == vmSymbols::tag_constant_value()) {
      if (constantvalue_index != 0) {
        classfile_parse_error("Invalid attribute in .class file", CHECK);
      }
      check_property(attribute_length == 2, "Invalid attribute in .class file", CHECK);
      constantvalue_index = stream()->get_u2();
    } else if (cp->symbol_at(attribute_name_index) == vmSymbols::tag_synthetic()) {
      if (attribute_length != 0) {
        classfile_parse_error("Invalid synthetic attribute in .class file", CHECK);
      }
      is_synthetic = true;
    } else {
      stream()->skip_u1(attribute_length);  // Skip unknown attributes
    }
  }
  *constantvalue_index_addr = constantvalue_index;
  *is_synthetic_addr = is_synthetic;
  return;
}


// Field allocation types. Used for computing field offsets.

enum FieldAllocationType {
  STATIC_OOP,		// Oops
  STATIC_BYTE,		// Boolean, Byte, char
  STATIC_SHORT,		// shorts
  STATIC_WORD,		// ints
  STATIC_DOUBLE,	// long or double
  STATIC_ALIGNED_DOUBLE,// aligned long or double
  NONSTATIC_OOP,	 
  NONSTATIC_BYTE,
  NONSTATIC_SHORT,
  NONSTATIC_WORD,
  NONSTATIC_DOUBLE,
  NONSTATIC_ALIGNED_DOUBLE
};


struct FieldAllocationCount {
  int static_oop_count;
  int static_byte_count;
  int static_short_count;
  int static_word_count;
  int static_double_count;
  int nonstatic_oop_count;
  int nonstatic_byte_count;
  int nonstatic_short_count;
  int nonstatic_word_count;
  int nonstatic_double_count;
};

typeArrayHandle ClassFileParser::parse_fields(constantPoolHandle cp, struct FieldAllocationCount *fac, TRAPS) {
  typeArrayHandle nullHandle;
  u2 length = stream()->get_u2();   
  // 5-tuples of shorts [access, name index, sig index, initial value index, byte offset]
  typeArrayOop new_fields = oopFactory::new_permanent_shortArray(length*6, CHECK_(nullHandle));
  typeArrayHandle fields(THREAD, new_fields);
  int index = 0;
  for (int n = 0; n < length; n++) {
    AccessFlags access_flags;
    access_flags.set_flags(stream()->get_u2() & JVM_RECOGNIZED_FIELD_MODIFIERS);
    u2 name_index = stream()->get_u2();
    check_property(cp->tag_at(name_index).is_utf8(), "Invalid .class file", CHECK_(nullHandle));
    u2 signature_index = stream()->get_u2();
    check_property(cp->tag_at(signature_index).is_utf8(), "Invalid .class file", CHECK_(nullHandle));
    u2 constantvalue_index = 0;
    bool is_synthetic = false;
    parse_field_attributes(cp, &constantvalue_index, &is_synthetic, CHECK_(nullHandle));
    if (is_synthetic) {
      access_flags.set_is_synthetic();
    }
    
    fields->short_at_put(index++, access_flags.as_short());
    fields->short_at_put(index++, name_index);
    fields->short_at_put(index++, signature_index);
    fields->short_at_put(index++, constantvalue_index);	

    // Remember how many oops we encountered and compute allocation type
    BasicType type = cp->basic_type_for_signature_at(signature_index);
    FieldAllocationType atype;
    bool is_static = access_flags.is_static();
    if ( is_static ) {
      switch ( type ) {
        case  T_BOOLEAN:
  	case  T_BYTE:
          fac->static_byte_count++;
          atype = STATIC_BYTE;
	  break;
        case  T_LONG:
  	case  T_DOUBLE:
          if (Universe::field_type_should_be_aligned(type)) 
	    atype = STATIC_ALIGNED_DOUBLE;
 	  else 
	    atype = STATIC_DOUBLE;
	  fac->static_double_count++;
	  break;
  	case  T_CHAR:     
  	case  T_SHORT: 
          fac->static_short_count++;
          atype = STATIC_SHORT;
	  break;
  	case  T_FLOAT:
  	case  T_INT:
	  fac->static_word_count++;
	  atype = STATIC_WORD;
	  break;
        case  T_ARRAY: 
        case  T_OBJECT:
  	  fac->static_oop_count++;
	  atype = STATIC_OOP;
	  break;
  	case  T_ADDRESS: 
        case  T_VOID:
        default: 
	  assert(0, "bad field type");
	  break;
      }

    }
    else {
      switch ( type ) {
        case  T_BOOLEAN:
  	case  T_BYTE:
          fac->nonstatic_byte_count++;
          atype = NONSTATIC_BYTE;
	  break;
        case  T_LONG:
  	case  T_DOUBLE:
          if (Universe::field_type_should_be_aligned(type)) 
	    atype = NONSTATIC_ALIGNED_DOUBLE;
 	  else 
	    atype = NONSTATIC_DOUBLE;
	  fac->nonstatic_double_count++;
	  break;
  	case  T_CHAR:     
  	case  T_SHORT: 
          fac->nonstatic_short_count++;
          atype = NONSTATIC_SHORT;
	  break;
  	case  T_FLOAT:
  	case  T_INT:
	  fac->nonstatic_word_count++;
	  atype = NONSTATIC_WORD;
	  break;
        case  T_ARRAY: 
        case  T_OBJECT:
	  fac->nonstatic_oop_count++;
	  atype = NONSTATIC_OOP;
	  break;
  	case  T_ADDRESS: 
        case  T_VOID:
        default: 
	  assert(0, "bad field type");
	  break;
      }
    }

    // The correct offset is computed later (all oop fields will be located together)
    // We temporarily store the allocation type in the offset field
    fields->short_at_put(index++, atype);
    fields->short_at_put(index++, 0);  // Clear out high word of byte offset
  }

  return fields;
}


static void copy_u2_with_conversion(u2* dest, u2* src, int length) {
  while (length-- > 0) {
    *dest++ = Bytes::get_Java_u2((u1*) (src++));
  }
}


// Note: the parse_method below is big and clunky because all parsing of the code and exceptions
// attribute is inlined. This is curbersome to avoid since we inline most of the parts in the
// methodOop to save footprint, so we only know the size of the resulting methodOop when the
// entire method attribute is parsed.
//
// The promoted_flags parameter is used to pass relevant access_flags
// from the method back up to the containing klass. These flag values
// are added to klass's access_flags.

methodHandle ClassFileParser::parse_method(constantPoolHandle cp, AccessFlags *promoted_flags, TRAPS) {
  methodHandle nullHandle;
  ResourceMark rm(THREAD);
  // Parse fixed parts
  int flags = stream()->get_u2();    
  u2 name_index = stream()->get_u2();
  check_property(cp->tag_at(name_index).is_utf8(), "Invalid .class file", CHECK_(nullHandle));
  u2 signature_index = stream()->get_u2();
  check_property(cp->tag_at(signature_index).is_utf8(), "Invalid .class file", CHECK_(nullHandle));

  AccessFlags access_flags;  
  if (cp->symbol_at(name_index) == vmSymbols::class_initializer_name()) {
    // We ignore the access flags for a class initializer. (JVM Spec. p. 116)
    flags = JVM_ACC_STATIC;
  }
  access_flags.set_flags(flags & JVM_RECOGNIZED_METHOD_MODIFIERS);
  
  // Default values for code and exceptions attribute elements
  u2 max_stack = 0;
  u2 max_locals = 0;
  u4 code_length = 0;
  u1* code_start = 0;
  u2 exception_table_length = 0;
  typeArrayHandle exception_handlers(THREAD, Universe::the_empty_int_array());
  u2 checked_exceptions_length = 0;
  u2* checked_exceptions_start = NULL;
  int compressed_linenumber_table_size = 0;
  u_char* compressed_linenumber_table = NULL;
  int localvariable_table_length = 0;
  u2* localvariable_table_start = NULL;
  bool parsed_code_attribute = false;
  bool parsed_checked_exceptions_attribute = false;

  // Parse code and exceptions attribute
  u2 method_attributes_count = stream()->get_u2();
  while (method_attributes_count--) {   
    u2 method_attribute_name_index = stream()->get_u2();
    u4 method_attribute_length = stream()->get_u4();
    check_property(cp->tag_at(method_attribute_name_index).is_utf8(), "Invalid attribute in .class file", CHECK_(nullHandle));

    if (cp->symbol_at(method_attribute_name_index) == vmSymbols::tag_code()) {
      // Parse Code attribute
      if (parsed_code_attribute) {
        classfile_parse_error("Invalid attribute in .class file", CHECK_(nullHandle));
      }
      parsed_code_attribute = true;
      // Stack and locals size
      max_stack = stream()->get_u2();
      max_locals = stream()->get_u2();
      // Code size and pointer
      code_length = stream()->get_u4();
      code_start = stream()->get_u1_buffer(code_length);
      if (code_start == NULL) truncated_class_file_error(CHECK_(nullHandle));
      stream()->skip_u1(code_length);
      // Exception handler table
      exception_table_length = stream()->get_u2();
      if (exception_table_length > 0) {
        // 4-tuples of ints [start_pc, end_pc, handler_pc, catch_type index]
        typeArrayOop eh =oopFactory::new_permanent_intArray(exception_table_length*4, CHECK_(nullHandle));
        exception_handlers = typeArrayHandle(THREAD, eh);
      }
      int index = 0;
      for (int i = 0; i < exception_table_length; i++) {
        for (int j = 0; j < 4; j++) {
          exception_handlers->int_at_put(index++, stream()->get_u2());  
        }
      }
      // Parse additional attributes in code attribute
      u2 code_attributes_count = stream()->get_u2();
      while (code_attributes_count--) {
        u2 code_attribute_name_index = stream()->get_u2();
        u4 code_attribute_length = stream()->get_u4();
        check_property(cp->tag_at(code_attribute_name_index).is_utf8(), "Invalid attribute in .class file", CHECK_(nullHandle));
        if (LoadLineNumberTables && cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_line_number_table()) {
          // Parse and compress line number table
          int linenumber_table_length = stream()->get_u2();
          if (linenumber_table_length > 0) {
            CompressedLineNumberWriteStream c_stream(linenumber_table_length * sizeof(u2) * 2);  // initial_size large enough
            while (linenumber_table_length-- > 0) {
              u2 bci  = stream()->get_u2();
              u2 line = stream()->get_u2();
              c_stream.write_pair(bci, line);
            }
            c_stream.write_terminator();
            compressed_linenumber_table_size = c_stream.position();
            compressed_linenumber_table = c_stream.buffer();
          }
        } else if (LoadLocalVariableTables && cp->symbol_at(code_attribute_name_index) == vmSymbols::tag_local_variable_table()) {
          // Parse line number table
          localvariable_table_length = stream()->get_u2();
          int size = localvariable_table_length * sizeof(LocalVariableTableElement) / sizeof(u2);
          localvariable_table_start = stream()->get_u2_buffer(size);
          if (localvariable_table_start == NULL) truncated_class_file_error(CHECK_(nullHandle));
          stream()->skip_u2(size);
        } else {
          // Skip unknown attributes
          stream()->skip_u1(code_attribute_length);
        }
      }
    } else if (cp->symbol_at(method_attribute_name_index) == vmSymbols::tag_exceptions()) {
    // Parse Exceptions attribute
      if (parsed_checked_exceptions_attribute) {
        classfile_parse_error("Invalid attribute in .class file", CHECK_(nullHandle));
      }
      parsed_checked_exceptions_attribute = true;
      checked_exceptions_length = stream()->get_u2();
      int size = checked_exceptions_length * sizeof(CheckedExceptionElement) / sizeof(u2);
      checked_exceptions_start = stream()->get_u2_buffer(size);
      if (checked_exceptions_start == NULL) truncated_class_file_error(CHECK_(nullHandle));
      stream()->skip_u2(size);
    } else if (cp->symbol_at(method_attribute_name_index) == vmSymbols::tag_synthetic()) {
      if (method_attribute_length != 0) {
        classfile_parse_error("Invalid synthetic attribute in .class file", CHECK_(nullHandle));
      }
      // Should we check that there hasn't already been a synthetic attribute?
      access_flags.set_is_synthetic();
    } else {
      // Skip unknown attributes
      stream()->skip_u1(method_attribute_length);
    }
  }

  // All sizing information for a methodOop is finally available, now create it
  methodOop m_oop  = oopFactory::new_method(code_length, access_flags,
                               compressed_linenumber_table_size, 
                               localvariable_table_length, 
                               checked_exceptions_length, 
                               CHECK_(nullHandle));
  methodHandle m (THREAD, m_oop);

  // Fill in information from fixed part (access_flags already set)
  m->set_constants(cp());
  m->set_name_index(name_index);
  m->set_signature_index(signature_index);
#ifdef CC_INTERP
  // hmm is there a gc issue here??
  ResultTypeFinder rtf(cp->symbol_at(signature_index));
  m->set_result_index(rtf.type());
#endif
  m->compute_size_of_parameters(THREAD);
  // Fill in code atttribute information
  m->set_max_stack(max_stack);
  m->set_max_locals(max_locals);
  m->set_exception_table(exception_handlers());
  // Copy byte codes
  if (code_length > 0) {
    memcpy(m->code_base(), code_start, code_length);
  }
  // Copy line number table
  if (compressed_linenumber_table_size > 0) {
    memcpy(m->compressed_linenumber_table(), compressed_linenumber_table, compressed_linenumber_table_size);
  }
  // Copy checked exceptions
  if (checked_exceptions_length > 0) {
    int size = checked_exceptions_length * sizeof(CheckedExceptionElement) / sizeof(u2);
    copy_u2_with_conversion((u2*) m->checked_exceptions_start(), checked_exceptions_start, size);
  }
  // Copy local variable table
  if (localvariable_table_length > 0) {
    int size = localvariable_table_length * sizeof(LocalVariableTableElement) / sizeof(u2);
    copy_u2_with_conversion((u2*) m->localvariable_table_start(), localvariable_table_start, size);
    promoted_flags->set_has_localvariable_table();
  }
  return m;
}


extern "C" {
  static int method_compare(methodOop* a, methodOop* b) {
    return (*a)->name()->fast_compare((*b)->name());
  }

  typedef int (*compareFn)(const void*, const void*);
}


// The promoted_flags parameter is used to pass relevant access_flags
// from the methods back up to the containing klass. These flag values
// are added to klass's access_flags.

objArrayHandle ClassFileParser::parse_methods(constantPoolHandle cp, AccessFlags* promoted_flags, TRAPS) {
  objArrayHandle nullHandle;
  u2 length = stream()->get_u2();
    if (length == 0) {
    return objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else {
    objArrayOop m = oopFactory::new_system_objArray(length, CHECK_(nullHandle));
    objArrayHandle methods(THREAD, m);
    for (int index = 0; index < length; index++) {
      HandleMark hm(THREAD);
      methodHandle method = parse_method(cp, promoted_flags, CHECK_(nullHandle));
      methods->obj_at_put(index, method());  
    }
    return methods;
  }
}


typeArrayHandle ClassFileParser::sort_methods(objArrayHandle methods, TRAPS) {
  typeArrayHandle nullHandle;
  int length = methods()->length();
  // If JVMDI is enabled we have to remember the original class file ordering.
  // We temporarily use the vtable_index field in the methodOop to store the
  // class file index, so we can read in after calling qsort.
  if (jvmdi::enabled()) {
    for (int index = 0; index < length; index++) {
      methodOop m = methodOop(methods->obj_at(index));
      assert(m->vtable_index() == -1, "vtable index should not be set");
      m->set_vtable_index(index);
    }
  }
  // Sort method array by ascending method name (for faster lookups & vtable construction)
  // Note that the ordering is not alfabetical, see symbolOopDesc::fast_compare
  if (length > 1) qsort(methods()->obj_at_addr(0), length, oopSize, (compareFn)method_compare);
  // If JVMDI is enabled construct int array remembering the original ordering
  if (jvmdi::enabled()) {
    typeArrayOop new_ordering = oopFactory::new_permanent_intArray(length, CHECK_(nullHandle));
    typeArrayHandle method_ordering(THREAD, new_ordering);
    for (int index = 0; index < length; index++) {
      methodOop m = methodOop(methods->obj_at(index));
      int old_index = m->vtable_index();
      assert(old_index >= 0 && old_index < length, "invalid method index");
      method_ordering->int_at_put(index, old_index);
      m->set_vtable_index(-1);
    }
    return method_ordering;
  } else {
    return typeArrayHandle(THREAD, Universe::the_empty_int_array());
  }
}


void ClassFileParser::parse_classfile_sourcefile_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  u2 sourcefile_index = stream()->get_u2();
  k->set_source_file_name(cp->symbol_at(sourcefile_index));
}



void ClassFileParser::parse_classfile_source_debug_extension_attribute(constantPoolHandle cp, instanceKlassHandle k, int length, TRAPS) {
  u1* sde_buffer = stream()->get_u1_buffer(length);
  if (sde_buffer != NULL) {
    // Don't bother storing it if there is no way to retrieve it
    if (jvmdi::enabled()) {
      // Optimistically assume that only 1 byte UTF format is used
      // (common case)
      symbolOop sde_symbol = oopFactory::new_symbol((char*)sde_buffer, 
                                                    length, CHECK);
      k->set_source_debug_extension(sde_symbol);
    }
    // Got utf8 string, set stream position forward
    stream()->skip_u1(length);
  } else {
    truncated_class_file_error(CHECK);
  }
}



// Inner classes can be static, private or protected (classic VM does this)
#define RECOGNIZED_INNER_CLASS_MODIFIERS (JVM_RECOGNIZED_CLASS_MODIFIERS | JVM_ACC_PRIVATE | JVM_ACC_PROTECTED | JVM_ACC_STATIC)


void ClassFileParser::parse_classfile_inner_classes_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {  
  u2 length = stream()->get_u2();

  // 4-tuples of shorts [inner_class_info_index, outer_class_info_index, inner_name_index, inner_class_access_flags]
  typeArrayOop ic = oopFactory::new_permanent_shortArray(length*4, CHECK);  
  typeArrayHandle inner_classes(THREAD, ic);
  int index = 0;
  for (int n = 0; n < length; n++) {
    // Inner class index
    u2 inner_class_info_index = stream()->get_u2();
    check_property(inner_class_info_index == 0 || cp->tag_at(inner_class_info_index).is_klass_reference(), "Invalid .class file", CHECK);
    // Outer class index
    u2 outer_class_info_index = stream()->get_u2();
    check_property(outer_class_info_index == 0 || cp->tag_at(outer_class_info_index).is_klass_reference(), "Invalid .class file", CHECK);
    // Inner class name
    u2 inner_name_index = stream()->get_u2();
    check_property(inner_name_index == 0 || cp->tag_at(inner_name_index).is_utf8(), "Invalid .class file", CHECK);    
    // Access flags
    AccessFlags inner_access_flags;
    jint flags = stream()->get_u2() & RECOGNIZED_INNER_CLASS_MODIFIERS;
    if ((flags & JVM_ACC_INTERFACE) != 0) {
      // Set the abstract bit explicitly for interface classes (Classic VM does this)
      flags |= JVM_ACC_ABSTRACT;
    }
    inner_access_flags.set_flags(flags);

    inner_classes->short_at_put(index++, inner_class_info_index);
    inner_classes->short_at_put(index++, outer_class_info_index);
    inner_classes->short_at_put(index++, inner_name_index);	
    inner_classes->short_at_put(index++, inner_access_flags.as_short());
  }

  // Update instanceKlass with inner class info.  
  k->set_inner_classes(inner_classes());
}

void ClassFileParser::parse_classfile_synthetic_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  k->set_is_synthetic();
}

void ClassFileParser::parse_classfile_attributes(constantPoolHandle cp, instanceKlassHandle k, TRAPS) {
  // Set inner classes attribute to default sentinel
  k->set_inner_classes(Universe::the_empty_short_array());
  u2 attributes_count = stream()->get_u2();
  // Iterate over attributes
  while (attributes_count--) {    
    u2 attribute_name_index = stream()->get_u2();
    u4 attribute_length = stream()->get_u4();
    check_property(cp->tag_at(attribute_name_index).is_utf8(), "Invalid attribute in .class file", CHECK);
    symbolOop tag = cp->symbol_at(attribute_name_index);
    if (tag == vmSymbols::tag_source_file()) {
      // Check for SourceFile tag
      parse_classfile_sourcefile_attribute(cp, k, CHECK);
    } else if (tag == vmSymbols::tag_source_debug_extension()) {
      // Check for SourceDebugExtension tag
      parse_classfile_source_debug_extension_attribute(cp, k, (int)attribute_length, CHECK);
    } else if (tag == vmSymbols::tag_inner_classes()) {
      // Check for InnerClasses tag
      parse_classfile_inner_classes_attribute(cp, k, CHECK);
    } else if (tag == vmSymbols::tag_synthetic()) {
      // Check for Synthetic tag
      // Shouldn't we check that the synthetic flags wasn't already set?
      if (attribute_length != 0) {
        classfile_parse_error("Invalid synthetic attribute in .class file", CHECK);
      }
      parse_classfile_synthetic_attribute(cp, k, CHECK);
    } else {
      // Unknown attribute
      stream()->skip_u1(attribute_length);
    }
  }
}


static void initialize_static_field(fieldDescriptor* fd, TRAPS) {
  KlassHandle h_k (THREAD, fd->field_holder());
  assert(h_k.not_null() && fd->is_static(), "just checking");
  if (fd->has_initial_value()) {
    BasicType t = fd->field_type();
    switch (t) {
      case T_BYTE:
        h_k()->byte_field_put(fd->offset(), fd->int_initial_value());
	break;
      case T_BOOLEAN:
        h_k()->bool_field_put(fd->offset(), fd->int_initial_value());
	break;
      case T_CHAR:
        h_k()->char_field_put(fd->offset(), fd->int_initial_value());
	break;
      case T_SHORT:
        h_k()->short_field_put(fd->offset(), fd->int_initial_value());
	break;
      case T_INT:
        h_k()->int_field_put(fd->offset(), fd->int_initial_value());
        break;
      case T_FLOAT:
        h_k()->float_field_put(fd->offset(), fd->float_initial_value());
        break;
      case T_DOUBLE:
        h_k()->double_field_put(fd->offset(), fd->double_initial_value());
        break;
      case T_LONG:
        h_k()->long_field_put(fd->offset(), fd->long_initial_value());
        break;
      case T_OBJECT:
        {
          #ifdef ASSERT      
          symbolOop sym = oopFactory::new_symbol("Ljava/lang/String;", CHECK);
          assert(fd->signature() == sym, "just checking");      
          #endif
          oop string = fd->string_initial_value(CHECK);
          h_k()->obj_field_put(fd->offset(), string);
        }
        break;
      default:
        classfile_parse_error("Illegal ConstantValue attribute", CHECK);
    }
  }
}


void ClassFileParser::java_lang_ref_Reference_fix_pre(typeArrayHandle* fields_ptr,
  constantPoolHandle cp, FieldAllocationCount *fac_ptr, TRAPS) {
  // This code is for compatibility with earlier jdk's that do not
  // have the "discovered" field in java.lang.ref.Reference.  For 1.5
  // the check for the "discovered" field should issue a warning if
  // the field is not found.  For 1.6 this code should be issue a
  // fatal error if the "discovered" field is not found.
  //
  // Increment fac.nonstatic_oop_count so that the start of the
  // next type of non-static oops leaves room for the fake oop.
  // Do not increment next_nonstatic_oop_offset so that the
  // fake oop is place after the java.lang.ref.Reference oop
  // fields.
  //
  // Check the fields in java.lang.ref.Reference for the "discovered"
  // field.  If it is not present, artifically create a field for it.
  // This allows this VM to run on early JDK where the field is not
  // present.

  //
  // Increment fac.nonstatic_oop_count so that the start of the 
  // next type of non-static oops leaves room for the fake oop.
  // Do not increment next_nonstatic_oop_offset so that the
  // fake oop is place after the java.lang.ref.Reference oop
  // fields.
  //
  // Check the fields in java.lang.ref.Reference for the "discovered"
  // field.  If it is not present, artifically create a field for it.
  // This allows this VM to run on early JDK where the field is not
  // present.
  int reference_sig_index = 0;
  int reference_name_index = 0;
  int reference_index = 0;
  int extra = java_lang_ref_Reference::number_of_fake_oop_fields;
  const int n = (*fields_ptr)()->length();
  for (int i = 0; i < n; i += instanceKlass::next_offset ) {
    int name_index = 
    (*fields_ptr)()->ushort_at(i + instanceKlass::name_index_offset);
    int sig_index  = 
      (*fields_ptr)()->ushort_at(i + instanceKlass::signature_index_offset);
    symbolOop f_name = cp->symbol_at(name_index);
    symbolOop f_sig  = cp->symbol_at(sig_index);
    if (f_sig == vmSymbols::reference_signature() && reference_index == 0) {
      // Save the index for reference signature for later use.
      // The fake discovered field does not entries in the
      // constant pool so the index for its signature cannot
      // be extracted from the constant pool.  It will need 
      // later, however.  It's signature is vmSymbols::reference_signature()
      // so same an index for that signature.
      reference_sig_index = sig_index;
      reference_name_index = name_index;
      reference_index = i;
    }
    if (f_name == vmSymbols::reference_discovered_name() &&
      f_sig == vmSymbols::reference_signature()) {
      // The values below are fake but will force extra
      // non-static oop fields and a corresponding non-static 
      // oop map block to be allocated.
      extra = 0;
      break;
    }
  }
  if (extra != 0) { 
    fac_ptr->nonstatic_oop_count += extra;
    // Add the additional entry to "fields" so that the klass
    // contains the "discoverd" field and the field will be initialized
    // in instances of the object.
    int fields_with_fix_length = (*fields_ptr)()->length() + 
      instanceKlass::next_offset;
    typeArrayOop ff = oopFactory::new_permanent_shortArray(
                                                fields_with_fix_length, CHECK);
    typeArrayHandle fields_with_fix(THREAD, ff);

    // Take everything from the original but the length.
    for (int idx = 0; idx < (*fields_ptr)->length(); idx++) {
      fields_with_fix->ushort_at_put(idx, (*fields_ptr)->ushort_at(idx));
    }

    // Add the fake field at the end.
    int i = (*fields_ptr)->length();
    // There is no name index for the fake "discovered" field nor 
    // signature but a signature is needed so that the field will
    // be properly initialized.  Use one found for
    // one of the other reference fields. Be sure the index for the
    // name is 0.  In fieldDescriptor::initialize() the index of the
    // name is checked.  That check is by passed for the last nonstatic
    // oop field in a java.lang.ref.Reference which is assumed to be
    // this artificial "discovered" field.  An assertion checks that
    // the name index is 0.
    assert(reference_index != 0, "Missing signature for reference");

    int j;
    for (j = 0; j < instanceKlass::next_offset; j++) {
      fields_with_fix->ushort_at_put(i + j, 
	(*fields_ptr)->ushort_at(reference_index +j));
    }
    // Clear the public access flag and set the private access flag.
    short flags;
    flags = 
      fields_with_fix->ushort_at(i + instanceKlass::access_flags_offset);
    assert(!(flags & JVM_ACC_FIELD_FLAGS), "Unexpected access flags set");
    flags = flags & (~JVM_ACC_PUBLIC);
    flags = flags | JVM_ACC_PRIVATE;
    AccessFlags access_flags;
    access_flags.set_field_flags(flags);
    assert(!access_flags.is_public(), "Failed to clear public flag");
    assert(access_flags.is_private(), "Failed to set private flag");
    fields_with_fix->ushort_at_put(i + instanceKlass::access_flags_offset, 
      flags);

    assert(fields_with_fix->ushort_at(i + instanceKlass::name_index_offset) 
      == reference_name_index, "The fake reference name is incorrect");
    assert(fields_with_fix->ushort_at(i + instanceKlass::signature_index_offset)
      == reference_sig_index, "The fake reference signature is incorrect");
    // The type of the field is stored in the low_offset entry during
    // parsing.
    assert(fields_with_fix->ushort_at(i + instanceKlass::low_offset) ==
      NONSTATIC_OOP, "The fake reference type is incorrect");

    // "fields" is allocated in the permanent generation.  Disgard
    // it and let it be collected.
    (*fields_ptr) = fields_with_fix;
  }
  return;
}


void ClassFileParser::java_lang_Class_fix_pre(objArrayHandle* methods_ptr, 
  FieldAllocationCount *fac_ptr, TRAPS) {
  // Add fake fields for java.lang.Class instances
  //
  // This is not particularly nice. We should consider adding a
  // private transient object field at the Java level to
  // java.lang.Class. Alternatively we could add a subclass of
  // instanceKlass which provides an accessor and size computer for
  // this field, but that appears to be more code than this hack.
  //
  // NOTE that we wedge these in at the beginning rather than the
  // end of the object because the Class layout changed between JDK
  // 1.3 and JDK 1.4 with the new reflection implementation; some
  // nonstatic oop fields were added at the Java level. The offsets
  // of these fake fields can't change between these two JDK
  // versions because when the offsets are computed at bootstrap
  // time we don't know yet which version of the JDK we're running in.

  // The values below are fake but will force two non-static oop fields and 
  // a corresponding non-static oop map block to be allocated.
  const int extra = java_lang_Class::number_of_fake_oop_fields;
  fac_ptr->nonstatic_oop_count += extra;
}


void ClassFileParser::java_lang_Class_fix_post(int* next_nonstatic_oop_offset_ptr) {
  // Cause the extra fake fields in java.lang.Class to show up before
  // the Java fields for layout compatibility between 1.3 and 1.4
  // Incrementing next_nonstatic_oop_offset here advances the 
  // location where the real java fields are placed.
  const int extra = java_lang_Class::number_of_fake_oop_fields;
  (*next_nonstatic_oop_offset_ptr) += (extra * wordSize);
}


instanceKlassHandle ClassFileParser::parseClassFile(symbolHandle name, 
                                                    Handle class_loader, 
                                                    Handle protection_domain, 
                                                    symbolHandle& parsed_name,
                                                    TRAPS) {
  // Timing
  TraceTime timer(NULL, ClassLoader::accumulated_time(), TraceClassLoadingTime, false);
  PerfTraceTime vmtimer(ClassLoader::perf_accumulated_time());

  if (jvmpi::is_event_enabled(JVMPI_EVENT_CLASS_LOAD_HOOK)) {
    ClassFileStream* cfs = stream();
    unsigned char* ptr = cfs->buffer();
    unsigned char* end_ptr = cfs->buffer() + cfs->length();

    jvmpi::post_class_load_hook_event(&ptr, &end_ptr, jvmpi::jvmpi_alloc);

    if (ptr != cfs->buffer()) {
      cfs = new ClassFileStream(ptr, end_ptr - ptr, cfs->source());
      set_stream(cfs);
    }
  }

  instanceKlassHandle nullHandle;
  // Verify the class file format
  Verifier::verify_class_format(name, class_loader, stream(), CHECK_(nullHandle));
  
  // Magic value
  u4 magic = stream()->get_u4();
  guarantee_property(magic == 0xCAFEBABE, "Incompatible magic value in .class file", CHECK_(nullHandle));

  // Version numbers  
  u2 minor_version = stream()->get_u2();
  u2 major_version = stream()->get_u2();
  // version numbers checked by class format checker

  // Constant pool
  constantPoolHandle cp = parse_constant_pool(CHECK_(nullHandle));
  
  // Access flags
  AccessFlags access_flags;
  jint flags = stream()->get_u2() & JVM_RECOGNIZED_CLASS_MODIFIERS;
  if ((flags & JVM_ACC_INTERFACE) != 0) {
    // Set the abstract bit explicitly for interface classes (Classic VM does this)
    flags |= JVM_ACC_ABSTRACT;
  }
  access_flags.set_flags(flags);

  // This class and superclass
  instanceKlassHandle super_klass;
  u2 this_class_index = stream()->get_u2();
  check_property(cp->tag_at(this_class_index).is_unresolved_klass(), "Invalid .class file", CHECK_(nullHandle));

  symbolHandle class_name (THREAD, cp->unresolved_klass_at(this_class_index));
  // It's important to set parsed_name *before* resolving the super class.
  // (it's used for cleanup by the caller if parsing fails)
  parsed_name = class_name;
  
  klassOop preserve_this_klass;   // for storing result across HandleMark

  // release all handles when parsing is done
  { HandleMark hm(THREAD);

    // Checks if name in class file matches requested name
    if (name.not_null() && class_name() != name()) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(
        THREAD_AND_LOCATION,
        vmSymbolHandles::java_lang_NoClassDefFoundError(), 
        "%s (wrong name: %s)", 
        name->as_C_string(), 
        class_name->as_C_string()
      );
    }

    if (TraceClassLoadingPreorder) {
      tty->print("[Loading %s", name()->as_klass_external_name());
      if (stream()->source() != NULL) tty->print(" from %s", stream()->source());
      tty->print_cr("]");
    }

    u2 super_class_index = stream()->get_u2();
    if (super_class_index == 0) {
      check_property(class_name() == vmSymbols::java_lang_Object(), "Invalid superclass", CHECK_(nullHandle));
    } else {
      check_property(cp->tag_at(super_class_index).is_unresolved_klass(), "Invalid superclass", CHECK_(nullHandle));
      timer.suspend();  // do not count recursive loading twice
      vmtimer.suspend();  // do not count recursive loading twice
      symbolHandle sk (THREAD, cp->unresolved_klass_at(super_class_index));
      klassOop k =
        SystemDictionary::resolve_super_or_fail(class_name,
                                                sk, 
                                                class_loader, 
                                                protection_domain, 
                                                CHECK_(nullHandle));
      KlassHandle kh (THREAD, k);
      super_klass = instanceKlassHandle(THREAD, kh());
      timer.resume();
      vmtimer.resume();
      // Checks below not done by format checker
      if (super_klass->is_final()) {
        THROW_MSG_(vmSymbols::java_lang_VerifyError(), "Cannot inherit from final class", nullHandle);
      }
      if (access_flags.is_interface() && super_klass() != SystemDictionary::object_klass()) {
        classfile_parse_error("Interfaces must have java.lang.Object as superclass", CHECK_(nullHandle));
      }
    }

    // Interfaces
    objArrayHandle local_interfaces = parse_interfaces(cp, class_loader, protection_domain, CHECK_(nullHandle));

    // Compute the transitive list of all unique interfaces implemented by this class
    objArrayHandle transitive_interfaces = compute_transitive_interfaces(super_klass, local_interfaces, CHECK_(nullHandle));

    // Fields (offsets are filled in later)
    struct FieldAllocationCount fac = {0,0,0,0,0,0,0,0,0,0};
    typeArrayHandle fields = parse_fields(cp, &fac, CHECK_(nullHandle));
    // Methods
    AccessFlags promoted_flags;
    promoted_flags.set_flags(0);
    objArrayHandle methods = parse_methods(cp, &promoted_flags, CHECK_(nullHandle));
    typeArrayHandle method_ordering = sort_methods(methods, CHECK_(nullHandle));

    // promote flags from parse_methods() to the klass' flags
    access_flags.add_promoted_flags(promoted_flags.as_int());

    // Size of Java vtable (in words)
    int vtable_size = 0;    
    int itable_size = 0;
    int num_miranda_methods = 0;

    klassVtable::compute_vtable_size_and_num_mirandas(vtable_size, 
  						      num_miranda_methods, 
						      super_klass(),
						      methods(),
						      access_flags,
						      class_loader(),
						      class_name(), 
						      local_interfaces());  
       
    // Size of Java itable (in words)
    itable_size = access_flags.is_interface() ? 0 : klassItable::compute_itable_size(transitive_interfaces);  
    
    // Field size and offset computation
    int nonstatic_field_size = super_klass() == NULL ? 0 : super_klass->nonstatic_field_size();
    int static_field_size = 0;
    int next_static_oop_offset;
    int next_static_double_offset;
    int next_static_word_offset;
    int next_static_short_offset;
    int next_static_byte_offset;
    int next_static_type_offset;
    int next_nonstatic_oop_offset;
    int next_nonstatic_double_offset;
    int next_nonstatic_word_offset;
    int next_nonstatic_short_offset;
    int next_nonstatic_byte_offset;
    int next_nonstatic_type_offset;
    int first_nonstatic_oop_offset;

    // Calculate the starting byte offsets
    next_static_oop_offset      = (instanceKlass::header_size() + 
		 		  align_object_offset(vtable_size) + 
				  align_object_offset(itable_size)) * wordSize;
    next_static_double_offset   = next_static_oop_offset + 
			 	  (fac.static_oop_count * oopSize);
    if ( fac.static_double_count && 
	 (Universe::field_type_should_be_aligned(T_DOUBLE) || 
 	  Universe::field_type_should_be_aligned(T_LONG)) ) {
      next_static_double_offset = align_size_up(next_static_double_offset, BytesPerLong);
    }

    next_static_word_offset     = next_static_double_offset + 
				  (fac.static_double_count * BytesPerLong);
    next_static_short_offset    = next_static_word_offset + 
				  (fac.static_word_count * BytesPerInt);
    next_static_byte_offset     = next_static_short_offset + 
				  (fac.static_short_count * BytesPerShort);
    next_static_type_offset     = align_size_up((next_static_byte_offset +
			          fac.static_byte_count ), wordSize );
    static_field_size 	        = (next_static_type_offset - 
			          next_static_oop_offset) / wordSize;
    next_nonstatic_oop_offset   = (instanceOopDesc::header_size() + 
				  nonstatic_field_size) * wordSize;

    // Add fake fields for java.lang.Class instances (also see below)
    if (class_name() == vmSymbols::java_lang_Class() && class_loader.is_null()) {
      java_lang_Class_fix_pre(&methods, &fac, CHECK_(nullHandle));
    }

    // Add a fake "discovered" field if it is not present 
    // for compatibility with earlier jdk's.
    if (class_name() == vmSymbols::java_lang_ref_Reference() 
      && class_loader.is_null()) {
      java_lang_ref_Reference_fix_pre(&fields, cp, &fac, CHECK_(nullHandle));
    }
    // end of "discovered" field compactibility fix

    next_nonstatic_double_offset= next_nonstatic_oop_offset + 
			 	  (fac.nonstatic_oop_count * oopSize);
    if ( fac.nonstatic_double_count && 
         (Universe::field_type_should_be_aligned(T_DOUBLE) || 
          Universe::field_type_should_be_aligned(T_LONG)) ) {
      next_nonstatic_double_offset = align_size_up(next_nonstatic_double_offset, BytesPerLong); 
    }
    next_nonstatic_word_offset  = next_nonstatic_double_offset + 
				  (fac.nonstatic_double_count * BytesPerLong);
    next_nonstatic_short_offset = next_nonstatic_word_offset + 
				  (fac.nonstatic_word_count * BytesPerInt);
    next_nonstatic_byte_offset  = next_nonstatic_short_offset + 
				  (fac.nonstatic_short_count * BytesPerShort);
    next_nonstatic_type_offset  = align_size_up((next_nonstatic_byte_offset +
			          fac.nonstatic_byte_count ), wordSize );
    nonstatic_field_size = nonstatic_field_size + ((next_nonstatic_type_offset - 
			          next_nonstatic_oop_offset) / wordSize );
 
    first_nonstatic_oop_offset  = next_nonstatic_oop_offset;

    // Add fake fields for java.lang.Class instances (also see above)
    if (class_name() == vmSymbols::java_lang_Class() && class_loader.is_null()) {
      java_lang_Class_fix_post(&next_nonstatic_oop_offset);
    }

    // Iterate over fields again and compute correct offsets.
    // The field allocation type was temporarily stored in the offset slot.
    // oop fields are located before non-oop fields (static and non-static).
    int len = fields->length();
    for (int i = 0; i < len; i += 6) {
      int real_offset;
      FieldAllocationType atype = (FieldAllocationType) fields->ushort_at(i+4);
      switch (atype) {
        case STATIC_OOP:
          real_offset = next_static_oop_offset;
	  next_static_oop_offset += oopSize;
          break;
        case STATIC_BYTE:
          real_offset = next_static_byte_offset;
	  next_static_byte_offset += 1;
          break;
        case STATIC_SHORT:
          real_offset = next_static_short_offset;
	  next_static_short_offset += BytesPerShort;
          break;
        case STATIC_WORD:
          real_offset = next_static_word_offset;
	  next_static_word_offset += BytesPerInt;
          break;
        case STATIC_ALIGNED_DOUBLE:
        case STATIC_DOUBLE:
          real_offset = next_static_double_offset;
          next_static_double_offset += BytesPerLong;
          break;
        case NONSTATIC_OOP:
          real_offset = next_nonstatic_oop_offset;
	  next_nonstatic_oop_offset += oopSize;
          break;
        case NONSTATIC_BYTE:
          real_offset = next_nonstatic_byte_offset;
	  next_nonstatic_byte_offset += 1;
          break;
        case NONSTATIC_SHORT:
          real_offset = next_nonstatic_short_offset;
	  next_nonstatic_short_offset += BytesPerShort;
          break;
        case NONSTATIC_WORD:
          real_offset = next_nonstatic_word_offset;
	  next_nonstatic_word_offset += BytesPerInt;
          break;
        case NONSTATIC_ALIGNED_DOUBLE:
        case NONSTATIC_DOUBLE:
          real_offset = next_nonstatic_double_offset;
          next_nonstatic_double_offset += BytesPerLong;
          break;
        default:
          ShouldNotReachHere();
      }
      fields->short_at_put(i+4, extract_low_short_from_int(real_offset) );
      fields->short_at_put(i+5, extract_high_short_from_int(real_offset) ); 
    }

    // Size of instances
    int instance_size;

    instance_size = align_object_size(next_nonstatic_type_offset / wordSize);

    // Size of non-static oop map blocks (in words) allocated at end of klass
    int nonstatic_oop_map_size = compute_oop_map_size(super_klass, fac.nonstatic_oop_count, first_nonstatic_oop_offset);

    // Compute reference type
    ReferenceType rt;
    if (super_klass() == NULL) {
      rt = REF_NONE;
    } else {
      rt = super_klass->reference_type();
    }

    // We can now create the basic klassOop for this klass    
    klassOop ik = oopFactory::new_instanceKlass(
                                    vtable_size, itable_size, 
                                    static_field_size, nonstatic_oop_map_size, 
                                    rt, CHECK_(nullHandle));
    instanceKlassHandle this_klass (THREAD, ik); 

    assert(this_klass->static_field_size() == static_field_size && 
           this_klass->nonstatic_oop_map_size() == nonstatic_oop_map_size, "sanity check");
    
    // Fill in information already parsed
    this_klass->set_access_flags(access_flags);
    this_klass->set_size_helper(instance_size);
    // Not yet: supers are done below to support the new subtype-checking fields
    //this_klass->set_super(super_klass());  
    this_klass->set_class_loader(class_loader());    
    this_klass->set_nonstatic_field_size(nonstatic_field_size);
    this_klass->set_static_oop_field_size(fac.static_oop_count);       
    cp->set_pool_holder(this_klass());
    this_klass->set_constants(cp());
    this_klass->set_local_interfaces(local_interfaces());
    this_klass->set_fields(fields());
    this_klass->set_methods(methods());
    this_klass->set_method_ordering(method_ordering());
    this_klass->set_name(cp->unresolved_klass_at(this_class_index));  // An unresolved klass entry contains a symbol
    this_klass->set_protection_domain(protection_domain());

    // Miranda methods
    if ((num_miranda_methods > 0) || 
	// if this class introduced new miranda methods or
	(super_klass.not_null() && (super_klass->has_miranda_methods()))
	// super class exists and this class inherited miranda methods
	) {
      this_klass->set_has_miranda_methods(); // then set a flag
    }

    // Additional attributes
    parse_classfile_attributes(cp, this_klass, CHECK_(nullHandle));

    // Initialize static fields
    this_klass->do_local_static_fields(&initialize_static_field, CHECK_(nullHandle));

    // VerifyOops believes that once this has been set, the object is completely loaded.
    // Compute transitive closure of interfaces this class implements
    this_klass->set_transitive_interfaces(transitive_interfaces());    

    // Fill in information needed to compute superclasses.
    this_klass->initialize_supers(super_klass(), CHECK_(nullHandle));

    // Initialize itable offset tables
    klassItable::setup_itable_offset_table(this_klass);

    // Do final class setup
    fill_oop_maps(this_klass, fac.nonstatic_oop_count, first_nonstatic_oop_offset);

    set_precomputed_flags(this_klass);

    // reinitialize modifiers, using the InnerClasses attribute
    int computed_modifiers = this_klass->compute_modifier_flags(CHECK_(nullHandle));
    this_klass->set_modifier_flags(computed_modifiers);

    // the following checks should really be done by the verifier.

    // check if this class can access its super class
    check_super_class_access(this_klass, CHECK_(nullHandle));

    // check if this class can access its superinterfaces
    check_super_interface_access(this_klass, CHECK_(nullHandle));

    //check if this class overrides any final method
    check_final_method_override(this_klass, CHECK_(nullHandle));

    // check that if this class is an interface then it doesn't have static methods
    if (this_klass->is_interface()) {
      check_illegal_static_method(this_klass, CHECK_(nullHandle));
    }

    ClassLoader::notify_class_loaded(instanceKlass::cast(this_klass()));
	  
    if (TraceClassLoading) {
      // print in a single call to reduce interleaving of output
      if (stream()->source() != NULL) {
        tty->print("[Loaded %s from %s]\n", this_klass->external_name(), stream()->source());
      } else {
        tty->print("[Loaded %s]\n", this_klass->external_name());
      }
    }

    if (TraceClassResolution) {
      // print out the superclass.
      const char * from = Klass::cast(this_klass())->external_name();
      if (this_klass->java_super() != NULL) {
        tty->print("RESOLVE %s %s\n", from, instanceKlass::cast(this_klass->java_super())->external_name());
      }
      // print out each of the interface classes referred to by this class.
      objArrayHandle local_interfaces = this_klass->local_interfaces();
      if (!local_interfaces.is_null()) {
        int length = local_interfaces->length();
        for (int i = 0; i < length; i++) {
          klassOop k = klassOop(local_interfaces->obj_at(i)); 
          instanceKlass* to_class = instanceKlass::cast(k);
          const char * to = to_class->external_name();
          tty->print("RESOLVE %s %s\n", from, to);
        }
      }
    }

    // preserve result across HandleMark  
    preserve_this_klass = this_klass();    
  }

  // Create new handle outside HandleMark
  instanceKlassHandle this_klass (THREAD, preserve_this_klass);
  debug_only(this_klass->as_klassOop()->verify();)

  return this_klass;
}


int ClassFileParser::compute_oop_map_size(instanceKlassHandle super, int nonstatic_oop_count, int first_nonstatic_oop_offset) {
  int map_size = super.is_null() ? 0 : super->nonstatic_oop_map_size();
  if (nonstatic_oop_count > 0) {
    // We have oops to add to map
    if (map_size == 0) {
      map_size++;
    } else {
      // Check whether we should add a new map block or whether the last one can be extended
      OopMapBlock* first_map = super->start_of_nonstatic_oop_maps();
      OopMapBlock* last_map = first_map + map_size - 1;

      int next_offset;
      next_offset = last_map->offset() + (last_map->length() * oopSize);

      if (next_offset != first_nonstatic_oop_offset) {
        // Superklass didn't end with a oop field, add extra map
        assert(next_offset<first_nonstatic_oop_offset, "just checking");
        map_size++;
      }
    }
  }
  return map_size;
}


void ClassFileParser::fill_oop_maps(instanceKlassHandle k, int nonstatic_oop_count, int first_nonstatic_oop_offset) {
  OopMapBlock* this_oop_map = k->start_of_nonstatic_oop_maps();
  OopMapBlock* last_oop_map = this_oop_map + k->nonstatic_oop_map_size() - 1;
  instanceKlass* super = k->superklass();
  if (super != NULL) {
    int super_oop_map_size     = super->nonstatic_oop_map_size();
    OopMapBlock* super_oop_map = super->start_of_nonstatic_oop_maps();
    // Copy maps from superklass
    while (super_oop_map_size-- > 0) {
      *this_oop_map++ = *super_oop_map++;
    }
  }
  if (nonstatic_oop_count > 0) {
    if (this_oop_map == last_oop_map) {
      // We added a new map block, fill it
      last_oop_map->set_offset(first_nonstatic_oop_offset);
      last_oop_map->set_length(nonstatic_oop_count);
    } else {
      // We should extend the last map block copied from the superklass
      assert(this_oop_map == last_oop_map + 1, "just checking");
      last_oop_map->set_length(last_oop_map->length() + nonstatic_oop_count);
    }
  }
}


void ClassFileParser::set_precomputed_flags(instanceKlassHandle k) {
  // Check if this klass has an empty finalize method (i.e. one with return bytecode only),
  // in which case we don't have to register objects as finalizable
  methodOop m = k->lookup_method(vmSymbols::finalize_method_name(), 
                                 vmSymbols::void_method_signature());
  if (m != NULL && !m->is_empty_method()) {
    k->set_has_finalizer();
  }

  // Check if this klass supports the java.lang.Cloneable interface
  if (SystemDictionary::cloneable_klass_loaded()) {
    if (k->is_subtype_of(SystemDictionary::cloneable_klass())) {
      k->set_is_cloneable();
    }
  }

  // Check if this klass has a vanilla default constructor
  klassOop super = k->super();
  if (super == NULL) {
    // java.lang.Object has empty default constructor
    k->set_has_vanilla_constructor();
  } else {
    if (Klass::cast(super)->has_vanilla_constructor()) {
      methodOop constructor = k->find_method(vmSymbols::object_initializer_name(), vmSymbols::void_method_signature());
      if (constructor != NULL && constructor->is_vanilla_constructor()) {
        k->set_has_vanilla_constructor();
      }
    }
  }

  if (!k->has_finalizer() && !k->is_abstract() && !k->is_interface()) {
    k->set_can_be_fastpath_allocated(); // Allow fast-path allocation
  }
}


// utility method for appending and array with check for duplicates

void append_interfaces(objArrayHandle result, int& index, objArrayOop ifs) {
  // iterate over new interfaces
  for (int i = 0; i < ifs->length(); i++) {
    oop e = ifs->obj_at(i);
    assert(e->is_klass() && instanceKlass::cast(klassOop(e))->is_interface(), "just checking");
    // check for duplicates
    bool duplicate = false;
    for (int j = 0; j < index; j++) {
      if (result->obj_at(j) == e) {
        duplicate = true;
        break;
      }
    }
    // add new interface
    if (!duplicate) {
      result->obj_at_put(index++, e);
    }
  }
}

objArrayHandle ClassFileParser::compute_transitive_interfaces(instanceKlassHandle super, objArrayHandle local_ifs, TRAPS) {
  // Compute maximum size for transitive interfaces
  int max_transitive_size = 0;
  int super_size = 0;
  // Add superclass transitive interfaces size
  if (super.not_null()) {
    super_size = super->transitive_interfaces()->length();
    max_transitive_size += super_size;
  }
  // Add local interfaces' super interfaces  
  int local_size = local_ifs->length();
  for (int i = 0; i < local_size; i++) {
    klassOop l = klassOop(local_ifs->obj_at(i));
    max_transitive_size += instanceKlass::cast(l)->transitive_interfaces()->length();
  }
  // Finally add local interfaces
  max_transitive_size += local_size;
  // Construct array
  objArrayHandle result;
  if (max_transitive_size == 0) {
    // no interfaces, use canonicalized array
    result = objArrayHandle(THREAD, Universe::the_empty_system_obj_array());
  } else if (max_transitive_size == super_size) {
    // no new local interfaces added, share superklass' transitive interface array
    result = objArrayHandle(THREAD, super->transitive_interfaces());
  } else if (max_transitive_size == local_size) {
    // only local interfaces added, share local interface array
    result = local_ifs;
  } else {
    objArrayHandle nullHandle;
    objArrayOop new_objarray = oopFactory::new_system_objArray(max_transitive_size, CHECK_(nullHandle));
    result = objArrayHandle(THREAD, new_objarray);
    int index = 0;
    // Copy down from superclass
    if (super.not_null()) {
      append_interfaces(result, index, super->transitive_interfaces());
    }    
    // Copy down from local interfaces' superinterfaces
    for (int i = 0; i < local_ifs->length(); i++) {
      klassOop l = klassOop(local_ifs->obj_at(i));
      append_interfaces(result, index, instanceKlass::cast(l)->transitive_interfaces());
    }
    // Finally add local interfaces
    append_interfaces(result, index, local_ifs());

    // Check if duplicates were removed
    if (index != max_transitive_size) {
      assert(index < max_transitive_size, "just checking");
      objArrayOop new_result = oopFactory::new_system_objArray(index, CHECK_(nullHandle));
      for (int i = 0; i < index; i++) {
        oop e = result->obj_at(i);
        assert(e != NULL, "just checking");
        new_result->obj_at_put(i, e);
      }
      result = objArrayHandle(THREAD, new_result);
    }
  }
  return result;  
}


void ClassFileParser::check_super_class_access(instanceKlassHandle this_klass, TRAPS) {
  klassOop super = this_klass->super();
  if ((super != NULL) &&
      (!Reflection::verify_class_access(this_klass->as_klassOop(), super, false))) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(  
      THREAD_AND_LOCATION,
      vmSymbolHandles::java_lang_IllegalAccessError(),
      "class %s cannot access its superclass %s",
      this_klass->external_name(),
      instanceKlass::cast(super)->external_name()
      );
  }
}


void ClassFileParser::check_super_interface_access(instanceKlassHandle this_klass, TRAPS) {
  objArrayHandle local_interfaces (THREAD, this_klass->local_interfaces());
  int lng = local_interfaces->length();
  for (int i = lng - 1; i >= 0; i--) {
    klassOop k = klassOop(local_interfaces->obj_at(i)); 
    assert (k != NULL && Klass::cast(k)->is_interface(), "invalid interface");
    if (!Reflection::verify_class_access(this_klass->as_klassOop(), k, false)) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(  
        THREAD_AND_LOCATION,
        vmSymbolHandles::java_lang_IllegalAccessError(),
        "class %s cannot access its superinterface %s",
        this_klass->external_name(),
        instanceKlass::cast(k)->external_name()
        );
    }
  }
}


void ClassFileParser::check_final_method_override(instanceKlassHandle this_klass, TRAPS) {
  objArrayHandle methods (THREAD, this_klass->methods());
  int num_methods = methods->length();
  
  // go thru each method and check if it overrides a final method
  for (int index = 0; index < num_methods; index++) {
    methodOop m = (methodOop)methods->obj_at(index);

    // skip private, static and <init> methods
    if ((!m->is_private()) &&
	(!m->is_static()) &&
	(m->name() != vmSymbols::object_initializer_name())) {
	
      symbolOop name = m->name();
      symbolOop signature = m->signature();
      klassOop k = this_klass->super();
      methodOop super_m = NULL;
      while (k != NULL) {
	// lookup a matching method in the super class hierarchy
        super_m = instanceKlass::cast(k)->lookup_method(name, signature); 
	if (super_m == NULL) {
	  break; // didn't find any match; get out
	}

	if (super_m->is_final() &&
	    // matching method in super is final
	    (Reflection::verify_field_access(this_klass->as_klassOop(), 
					     super_m->method_holder(),
					     super_m->method_holder(),
					     super_m->access_flags(),
					     false))
	    // this class can access super final method and therefore override
	    ) {
	  ResourceMark rm(THREAD);
	  Exceptions::fthrow(  
	    THREAD_AND_LOCATION,
	    vmSymbolHandles::java_lang_VerifyError(),
	    "class %s overrides final method %s.%s",
	    this_klass->external_name(),
	    name,
	    signature
	    );
	}

        k = instanceKlass::cast(super_m->method_holder())->super(); // continue to look
      }
    }
  }
}


// assumes that this_klass is an interface
void ClassFileParser::check_illegal_static_method(instanceKlassHandle this_klass, TRAPS) {
  assert(this_klass->is_interface(), "not an interface");
  objArrayHandle methods (THREAD, this_klass->methods());
  int num_methods = methods->length();

  for (int index = 0; index < num_methods; index++) {
    methodOop m = (methodOop)methods->obj_at(index);
    // if m is static and not the init method, throw a verify error
    if ((m->is_static()) && (m->name() != vmSymbols::class_initializer_name())) {
      ResourceMark rm(THREAD);
      Exceptions::fthrow(  
	THREAD_AND_LOCATION,
	vmSymbolHandles::java_lang_VerifyError(),
	"Illegal static method %s in interface %s",
	m->name(),
	this_klass->external_name()
	);
    }
  }
}
