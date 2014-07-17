/*
 * @(#)bands.h	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
// -*- C++ -*-
struct entry;
struct coding;

struct band {
#ifndef PRODUCT
  const char* name;
#endif
  coding* c;
  entry** ix;             // CP entry mapping, if CPRefBand
  //      (end of initialized fields)
  int     length;         // expected # values
  int	  dbase[1];       // delta state
  byte*   rp;             // read-pointer
#ifndef PRODUCT
  byte*   rpBase;         // initial value of read-pointer
  byte*   rpLimit;        // limit of read-pointer
  bool	  isDone;
#endif

  void init();

  band* nextBand() { return this+1; }
  band* prevBand() { return this-1; }

  void expectLength(int l) {
    assert(length == 0);
    length = l;
  }
  void expectMoreLength(int l) {
    assert((uint)l < 0x10000000);
    length += l;
  }
  void setIndex(entry** ix_) { ix = ix_; }
  void setIndexByTag(byte tag);
  void readFrom(byte* &rpVar);
  void setInputFrom(byte* &rpVar) {
    assert(length == 0);
    assert(!c->isSigned);
    rp = rpVar;
  }
  void decideCoding();

  int    getByte() { assert(!isDone); return *rp++ & 0xFF; }
  bytes  getBytes(int len) { bytes b; b.set(rp, len); rp += len; return b; }
  int    getValue();
  int*   getValueArray();
  int    getUnsigned() {
    int x = getValue();
    assert(x >= 0);
    return x;
  }
  int    getFlags() { return getUnsigned(); }
  entry* getRef();
  entry* getRefUsing(entry** ix);
  void done() { assert(assertDone()); }
  static void initStatics();
  static void initIndexes();

#ifndef PRODUCT
  bool assertDone();
#endif
};

extern band all_bands[];

#define BAND_LOCAL \
  band* band_temp = all_bands; \
  band* all_bands = band_temp

#ifdef PRODUCT
#define BAND_INIT(name, c, ix) \
  {       (coding*)(c), ix }
#else
#define BAND_INIT(name, c, ix) \
  { name, (coding*)(c), ix }
#endif

// bundle of bands which supply a sequence of attrs
struct attr_bands {
  // initialized data:
  band* count;
  band* name;
  band* size;
  band* bits;

  // zero-initialized data:
  ptrlist clients;

  void expectClient(void* client);
  void readCountsFrom(byte* &rpVar);
  void readNamesFrom(byte* &rpVar);
  void readSizesFrom(byte* &rpVar);
  void setBitsInputFrom(byte* &rpVar);
  void done(byte* &rpVar);
};

extern attr_bands all_attr_bands[];
// these are in the order their components appear in the package file:
enum {
  e_code_attrs,
  e_method_attrs,
  e_field_attrs,
  e_class_attrs,
  e_package_attrs,
  ATTR_BAND_LIMIT
};
#define ATTR_BAND_CLIENT_KINDS "cmfkp"
#define code_attrs    all_attr_bands[e_code_attrs]
#define method_attrs  all_attr_bands[e_method_attrs]
#define field_attrs   all_attr_bands[e_field_attrs]
#define class_attrs   all_attr_bands[e_class_attrs]
#define package_attrs all_attr_bands[e_package_attrs]

#define ATTR_BAND_LOCAL \
  attr_bands* attr_band_temp = all_attr_bands; \
  attr_bands* all_attr_bands = attr_band_temp


extern entry** getCPIndex(byte tag);

// Band schema:
enum {
    e_package_magic,
    e_package_header,

    // constant pool contents
    e_cp_Utf8_prefix,
    e_cp_Utf8_suffix,
    e_cp_Utf8_chars,
    e_cp_Utf8_big_length,
    e_cp_Utf8_big_chars,
    e_cp_Int,
    e_cp_Float,
    e_cp_Long_hi,
    e_cp_Long_lo,
    e_cp_Double_hi,
    e_cp_Double_lo,
    e_cp_String,
    e_cp_Class,
    e_cp_Signature_form,
    e_cp_Signature_classes,
    e_cp_Descr_name,
    e_cp_Descr_type,
    e_cp_Field_class,
    e_cp_Field_desc,
    e_cp_Method_class,
    e_cp_Method_desc,
    e_cp_Imethod_class,
    e_cp_Imethod_desc,

    // bands for carrying class schema information:
    e_class_flags,
    e_class_this,
    e_class_super,
    e_class_interface_count,
    e_class_interface,

    // band for hardwired InnerClasses attribute (shared across the package)
    e_ic_this_class,
    e_ic_flags,
    // These bands contain data only where flags sets ACC_IC_LONG_FORM:
    e_ic_outer_class,
    e_ic_name,

    // bands for class members
    e_class_field_count,
    e_class_method_count,
    e_field_flags,
    e_method_flags,
    e_field_descr,
    e_method_descr,

    // band for hardwired ConstantValue attribute
    e_field_constant_value,

    // band for hardwired Exceptions attribute
    e_method_exception_count,
    e_method_exceptions,

    e_code_headers,
    e_code_max_stack,
    e_code_max_locals,
    e_code_handler_count,
    e_code_handler_start,
    e_code_handler_end,
    e_code_handler_catch,
    e_code_handler_class,

    // bands for generic attributes
    e_code_attr_count,
    e_method_attr_count,
    e_field_attr_count,
    e_class_attr_count,
    //e_package_attr_count => fake_package_attr_count

    e_code_attr_name,
    e_method_attr_name,
    e_field_attr_name,
    e_class_attr_name,
    e_package_attr_name,

    e_code_attr_size,
    e_method_attr_size,
    e_field_attr_size,
    e_class_attr_size,
    e_package_attr_size,

    e_code_attr_bits,
    e_method_attr_bits,
    e_field_attr_bits,
    e_class_attr_bits,
    e_package_attr_bits,

    // bands for bytecodes
    e_bc_codes,
    // remaining bands provide typed opcode fields required by the bc_codes

    e_bc_case_count,
    e_bc_case_value,
    e_bc_byte,
    e_bc_short,
    e_bc_local,
    e_bc_label,

    // ldc* operands:
    e_bc_intref,
    e_bc_floatref,
    e_bc_longref,
    e_bc_doubleref,
    e_bc_stringref,

    e_bc_classref,
    e_bc_fieldref,
    e_bc_methodref,
    e_bc_imethodref,

    // _self_linker_op family
    e_bc_thismethod,
    e_bc_supermethod,
    e_bc_thisfield,
    e_bc_superfield,

    // bc_invokeinit family:
    e_bc_initref,

    e_fake_package_attr_count,

    BAND_LIMIT
};

// Symbolic names for bands, as if in a giant global struct:
#define package_magic all_bands[e_package_magic]
#define package_header all_bands[e_package_header]
#define cp_Utf8_prefix all_bands[e_cp_Utf8_prefix]
#define cp_Utf8_suffix all_bands[e_cp_Utf8_suffix]
#define cp_Utf8_chars all_bands[e_cp_Utf8_chars]
#define cp_Utf8_big_length all_bands[e_cp_Utf8_big_length]
#define cp_Utf8_big_chars all_bands[e_cp_Utf8_big_chars]
#define cp_Int all_bands[e_cp_Int]
#define cp_Float all_bands[e_cp_Float]
#define cp_Long_hi all_bands[e_cp_Long_hi]
#define cp_Long_lo all_bands[e_cp_Long_lo]
#define cp_Double_hi all_bands[e_cp_Double_hi]
#define cp_Double_lo all_bands[e_cp_Double_lo]
#define cp_String all_bands[e_cp_String]
#define cp_Class all_bands[e_cp_Class]
#define cp_Signature_form all_bands[e_cp_Signature_form]
#define cp_Signature_classes all_bands[e_cp_Signature_classes]
#define cp_Descr_name all_bands[e_cp_Descr_name]
#define cp_Descr_type all_bands[e_cp_Descr_type]
#define cp_Field_class all_bands[e_cp_Field_class]
#define cp_Field_desc all_bands[e_cp_Field_desc]
#define cp_Method_class all_bands[e_cp_Method_class]
#define cp_Method_desc all_bands[e_cp_Method_desc]
#define cp_Imethod_class all_bands[e_cp_Imethod_class]
#define cp_Imethod_desc all_bands[e_cp_Imethod_desc]
#define class_flags all_bands[e_class_flags]
#define class_this all_bands[e_class_this]
#define class_super all_bands[e_class_super]
#define class_interface_count all_bands[e_class_interface_count]
#define class_interface all_bands[e_class_interface]
#define ic_this_class all_bands[e_ic_this_class]
#define ic_flags all_bands[e_ic_flags]
#define ic_outer_class all_bands[e_ic_outer_class]
#define ic_name all_bands[e_ic_name]
#define class_field_count all_bands[e_class_field_count]
#define class_method_count all_bands[e_class_method_count]
#define field_flags all_bands[e_field_flags]
#define method_flags all_bands[e_method_flags]
#define field_descr all_bands[e_field_descr]
#define method_descr all_bands[e_method_descr]
#define field_constant_value all_bands[e_field_constant_value]
#define method_exception_count all_bands[e_method_exception_count]
#define method_exceptions all_bands[e_method_exceptions]
#define code_headers all_bands[e_code_headers]
#define code_max_stack all_bands[e_code_max_stack]
#define code_max_locals all_bands[e_code_max_locals]
#define code_handler_count all_bands[e_code_handler_count]
#define code_handler_start all_bands[e_code_handler_start]
#define code_handler_end all_bands[e_code_handler_end]
#define code_handler_catch all_bands[e_code_handler_catch]
#define code_handler_class all_bands[e_code_handler_class]
#define code_attr_count all_bands[e_code_attr_count]
#define method_attr_count all_bands[e_method_attr_count]
#define field_attr_count all_bands[e_field_attr_count]
#define class_attr_count all_bands[e_class_attr_count]
#define code_attr_name all_bands[e_code_attr_name]
#define method_attr_name all_bands[e_method_attr_name]
#define field_attr_name all_bands[e_field_attr_name]
#define class_attr_name all_bands[e_class_attr_name]
#define package_attr_name all_bands[e_package_attr_name]
#define code_attr_size all_bands[e_code_attr_size]
#define method_attr_size all_bands[e_method_attr_size]
#define field_attr_size all_bands[e_field_attr_size]
#define class_attr_size all_bands[e_class_attr_size]
#define package_attr_size all_bands[e_package_attr_size]
#define code_attr_bits all_bands[e_code_attr_bits]
#define method_attr_bits all_bands[e_method_attr_bits]
#define field_attr_bits all_bands[e_field_attr_bits]
#define class_attr_bits all_bands[e_class_attr_bits]
#define package_attr_bits all_bands[e_package_attr_bits]
#define bc_codes all_bands[e_bc_codes]
#define bc_case_count all_bands[e_bc_case_count]
#define bc_case_value all_bands[e_bc_case_value]
#define bc_byte all_bands[e_bc_byte]
#define bc_short all_bands[e_bc_short]
#define bc_local all_bands[e_bc_local]
#define bc_label all_bands[e_bc_label]
#define bc_intref all_bands[e_bc_intref]
#define bc_floatref all_bands[e_bc_floatref]
#define bc_longref all_bands[e_bc_longref]
#define bc_doubleref all_bands[e_bc_doubleref]
#define bc_stringref all_bands[e_bc_stringref]
#define bc_classref all_bands[e_bc_classref]
#define bc_fieldref all_bands[e_bc_fieldref]
#define bc_methodref all_bands[e_bc_methodref]
#define bc_imethodref all_bands[e_bc_imethodref]
#define bc_thismethod all_bands[e_bc_thismethod]
#define bc_supermethod all_bands[e_bc_supermethod]
#define bc_thisfield all_bands[e_bc_thisfield]
#define bc_superfield all_bands[e_bc_superfield]
#define bc_initref all_bands[e_bc_initref]
#define fake_package_attr_count all_bands[e_fake_package_attr_count]
