#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)classFileParser.hpp	1.57 03/01/23 12:21:31 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Parser for for .class files
//
// The bytes describing the class file structure is read from a Stream object


class ClassFileParser VALUE_OBJ_CLASS_SPEC {
 private:
  ClassFileStream* _stream;              // Actual input stream

  // Accessors
  ClassFileStream* stream()                        { return _stream; }
  void set_stream(ClassFileStream* st)             { _stream = st; }

  // Constant pool parsing
  void parse_constant_pool_utf8_entry              (constantPoolHandle cp, int index, TRAPS);
  void parse_constant_pool_integer_entry           (constantPoolHandle cp, int index);
  void parse_constant_pool_float_entry             (constantPoolHandle cp, int index);
  void parse_constant_pool_long_entry              (constantPoolHandle cp, int index);
  void parse_constant_pool_double_entry            (constantPoolHandle cp, int index);
  void parse_constant_pool_class_entry             (constantPoolHandle cp, int index);
  void parse_constant_pool_string_entry            (constantPoolHandle cp, int index);
  void parse_constant_pool_fieldref_entry          (constantPoolHandle cp, int index);
  void parse_constant_pool_methodref_entry         (constantPoolHandle cp, int index);
  void parse_constant_pool_interfacemethodref_entry(constantPoolHandle cp, int index);
  void parse_constant_pool_nameandtype_entry       (constantPoolHandle cp, int index);
  void parse_constant_pool_entry                   (constantPoolHandle cp, int* index, TRAPS);
  constantPoolHandle parse_constant_pool           (TRAPS);

  // Interface parsing
  objArrayHandle parse_interfaces(constantPoolHandle cp, Handle class_loader, Handle protection_domain, TRAPS);

  // Field parsing
  void parse_field_attributes(constantPoolHandle cp, u2* constantvalue_index_addr, bool* is_synthetic_addr, TRAPS);
  typeArrayHandle parse_fields(constantPoolHandle cp, struct FieldAllocationCount *fac, TRAPS);

  // Method parsing
  methodHandle parse_method                             (constantPoolHandle cp, AccessFlags* promoted_flags, TRAPS);
  objArrayHandle parse_methods                          (constantPoolHandle cp, AccessFlags* promoted_flags, TRAPS);
  typeArrayHandle sort_methods                          (objArrayHandle methods, TRAPS);

  // Classfile attribute parsing
  void parse_classfile_sourcefile_attribute   (constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_source_debug_extension_attribute   (constantPoolHandle cp, instanceKlassHandle k, int length, TRAPS);
  void parse_classfile_inner_classes_attribute(constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_attributes             (constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  void parse_classfile_synthetic_attribute    (constantPoolHandle cp, instanceKlassHandle k, TRAPS);
  
  // Final setup
  int  compute_oop_map_size(instanceKlassHandle super, int nonstatic_oop_count, int first_nonstatic_oop_offset);
  void fill_oop_maps(instanceKlassHandle k, int nonstatic_oop_count, int first_nonstatic_oop_offset);
  void set_precomputed_flags(instanceKlassHandle k);
  objArrayHandle compute_transitive_interfaces(instanceKlassHandle super, objArrayHandle local_ifs, TRAPS);

  // Special handling for certain classes.
  // Add the "discovered" field to java.lang.ref.Reference if
  // it does not exist.
  void java_lang_ref_Reference_fix_pre(typeArrayHandle* fields_ptr, 
    constantPoolHandle cp, FieldAllocationCount *fac_ptr, TRAPS);
  // Adjust the field allocation counts for java.lang.Class to add
  // fake fields.
  void java_lang_Class_fix_pre(objArrayHandle* methods_ptr,
    FieldAllocationCount *fac_ptr, TRAPS);
  // Adjust the next_nonstatic_oop_offset to place the fake fields
  // before any Java fields.
  void java_lang_Class_fix_post(int* next_nonstatic_oop_offset);
 public:
  // Constructor
  ClassFileParser(ClassFileStream* st) { set_stream(st); }

  // Parse .class file and return new klassOop. The klassOop is not hooked up
  // to the system dictionary or any other structures, so a .class file can 
  // be loaded several times if desired. 
  // The system dictionary hookup is done by the caller.
  //
  // "parsed_name" is updated by this method, and is the name found
  // while parsing the stream.
  instanceKlassHandle parseClassFile(symbolHandle name, 
                                     Handle class_loader, 
                                     Handle protection_domain, 
                                     symbolHandle& parsed_name,
                                     TRAPS);

  // Verifier checks
  static void check_super_class_access(instanceKlassHandle this_klass, TRAPS);
  static void check_super_interface_access(instanceKlassHandle this_klass, TRAPS);
  static void check_final_method_override(instanceKlassHandle this_klass, TRAPS);
  static void check_illegal_static_method(instanceKlassHandle this_klass, TRAPS);
};
