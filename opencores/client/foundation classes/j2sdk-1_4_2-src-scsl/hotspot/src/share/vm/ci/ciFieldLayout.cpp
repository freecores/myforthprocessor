#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciFieldLayout.cpp	1.13 03/01/23 11:56:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciFieldLayout.cpp.incl"

// ciFieldLayout
//
// This class represents the layout of the fields of an instanceKlass.

// ------------------------------------------------------------------
// ciFieldLayout::ciFieldLayout
ciFieldLayout::ciFieldLayout(ciInstanceKlass* my_klass) {
  assert(my_klass->is_loaded(), "must be loaded");
  ASSERT_IN_VM;

  klassOop klass = my_klass->get_klassOop();

  Arena* arena = CURRENT_ENV->arena();
  GrowableArray<BasicType>* fieldtypes =
    new (arena) GrowableArray<BasicType>(arena, 8, 0, T_VOID);
  GrowableArray<int>* aflags =
    new (arena) GrowableArray<int>(arena, 8, 0, 0);
  GrowableArray<int>* fieldoffsets =
    new (arena) GrowableArray<int>(arena, 8, 0, 0);

  int pos = 0;

  fill_in_header_fields(fieldtypes, fieldoffsets, pos);
  _header_count = pos;
  fill_in_instance_fields(fieldtypes, fieldoffsets, aflags, pos, klass);

#if 0
  // [RGV] instance size is in word's but pos is number
  // of fields.
  int fill_to = my_klass->instance_size();
  if (fieldtypes->length() < fill_to)
    fields->at_put_grow(fill_to-1, T_VOID, T_VOID);
  if (aflags->length() < fill_to)
    aflags->at_put_grow(fill_to-1, 0, 0);
#endif

  _fieldtypes = fieldtypes;
  _access_flags = aflags;
  _fieldoffsets = fieldoffsets;
}

// ------------------------------------------------------------------
// ciFieldLayout::fill_in_header_fields
//
// Set up the header fields in this ciFieldLayout.
void ciFieldLayout::fill_in_header_fields(GrowableArray<BasicType>* fieldtypes,
					  GrowableArray<int>* fieldoffsets, 
 					  int& pos) {
  assert(pos*wordSize == oopDesc::mark_offset_in_bytes(),
	 "mark offset failure");
  fieldtypes->append(T_OBJECT);
  fieldoffsets->append(pos++ * wordSize );
  assert(pos*wordSize == oopDesc::klass_offset_in_bytes(),
	 "klass offset failure");
  fieldtypes->append(T_OBJECT);
  fieldoffsets->append(pos++ * wordSize );

  uint header_size_in_words = (uint)instanceOopDesc::header_size();
  for (; (uint)pos < header_size_in_words; pos++) {
    fieldtypes->append(T_VOID);
    fieldoffsets->append(pos * wordSize );
  }
}

// ------------------------------------------------------------------
// ciFieldLayout::fill_in_instance_fields
//
// Set up the instance fields in this ciFieldLayout.
void ciFieldLayout::fill_in_instance_fields(GrowableArray<BasicType>* fieldtypes,
					    GrowableArray<int>* fieldoffsets, 
					    GrowableArray<int>* aflags,
					    int& pos,
					    klassOop current) {
  Klass* k = current->klass_part();
  assert(k->oop_is_instance(), "must be instanceKlass");
  instanceKlass* ik = (instanceKlass*)k;
  klassOop super = k->super();
  if (super) {
    fill_in_instance_fields(fieldtypes, fieldoffsets, aflags, pos, super);
  }

  // Fill in this klass's instance fields.
  typeArrayOop field_list = ik->fields();
  constantPoolOop cpool = ik->constants();
  uint num_fields = field_list->length();

  for (uint i = 0; i < num_fields; i += instanceKlass::next_offset) {
    AccessFlags flags;
    flags.set_flags(field_list->short_at(i + instanceKlass::access_flags_offset));
   
    if (!flags.is_static()) {
      // This is an instance field.  Add it to our list.
      int field_offset = ik->offset_from_fields( i );
      // Add the type to our list.
      symbolOop type_sym = cpool->symbol_at(field_list->short_at(i+
                           instanceKlass::signature_index_offset));
      BasicType field_type; 
      field_type = type2field[FieldType::basic_type(type_sym)];
      fieldtypes->at_put_grow(pos, field_type, T_VOID);
      aflags->at_put_grow(pos, flags.as_int(), 0);
      // The field offset we set here includes the header_size
      fieldoffsets->at_put_grow(pos, field_offset, 0);
      pos++;
    }
  }
}

// ------------------------------------------------------------------
// ciFieldLayout::print_layout
void ciFieldLayout::print_layout() {
  for (ciFieldLayoutStream ls(this); !ls.is_done(); ls.next()) {
    tty->print("%c", basictype_to_char(ls.type()));
  }  
}

// ------------------------------------------------------------------
// ciFieldLayout::print
void ciFieldLayout::print() {
  tty->print("<ciLayout layout=");
  print_layout();
  tty->print(" address=0x%x>", (address)this);
}


