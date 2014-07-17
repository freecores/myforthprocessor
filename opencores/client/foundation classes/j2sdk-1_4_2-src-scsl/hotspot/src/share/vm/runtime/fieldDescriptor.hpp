#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)fieldDescriptor.hpp	1.36 03/01/23 12:22:05 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A fieldDescriptor describes the attributes of a single field (instance or class variable).
// It needs the class constant pool to work (because it only holds indices into the pool
// rather than the actual info).

class fieldDescriptor VALUE_OBJ_CLASS_SPEC {
 private:
  AccessFlags         _access_flags;
  int                 _name_index;
  int                 _signature_index;
  int                 _initial_value_index;
  int                 _offset;
  int                 _index; // index into fields() array
  constantPoolHandle  _cp;

 public:
  symbolOop name() const               { return _cp->symbol_at(_name_index); }
  symbolOop signature() const          { return _cp->symbol_at(_signature_index); }
  klassOop field_holder() const        { return _cp->pool_holder(); }
  constantPoolOop constants() const    { return _cp(); }
  AccessFlags access_flags() const     { return _access_flags; }
  oop loader() const;
  // Offset (in words) of field from start of instanceOop / klassOop
  int offset() const                   { return _offset; }
  int index() const                    { return _index; }

  // Initial field value
  bool has_initial_value() const          { return _initial_value_index != 0; }
  constantTag initial_value_tag() const;  // The tag will return true on one of is_int(), is_long(), is_single(), is_double()
  jint        int_initial_value() const;
  jlong       long_initial_value() const;
  jfloat      float_initial_value() const;
  jdouble     double_initial_value() const;
  oop         string_initial_value(TRAPS) const;

  // Field signature type
  BasicType field_type() const            { return FieldType::basic_type(signature()); }

  // Access flags
  bool is_public() const                  { return _access_flags.is_public(); }
  bool is_private() const                 { return _access_flags.is_private(); }
  bool is_protected() const               { return _access_flags.is_protected(); }
  bool is_package_private() const         { return !is_public() && !is_private() && !is_protected(); }

  bool is_static() const                  { return _access_flags.is_static(); }
  bool is_final() const                   { return _access_flags.is_final(); }
  bool is_volatile() const                { return _access_flags.is_volatile(); }
  bool is_transient() const               { return _access_flags.is_transient(); }

  bool is_synthetic() const               { return _access_flags.is_synthetic(); }

  bool is_field_access_watched() const    { return _access_flags.is_field_access_watched(); }
  bool is_field_modification_watched() const
                                          { return _access_flags.is_field_modification_watched(); }
  void set_is_field_access_watched(const bool value)
                                          { _access_flags.set_is_field_access_watched(value); }
  void set_is_field_modification_watched(const bool value)
                                          { _access_flags.set_is_field_modification_watched(value); }

  // Initialization
  void initialize(klassOop k, int index);

  // Print
  void print_on(outputStream* st) const         PRODUCT_RETURN;
  void print_on_for(outputStream* st, oop obj)  PRODUCT_RETURN;
};
