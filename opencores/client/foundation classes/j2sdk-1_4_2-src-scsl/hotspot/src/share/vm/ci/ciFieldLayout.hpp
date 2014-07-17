#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciFieldLayout.hpp	1.8 03/01/23 11:56:51 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciFieldLayout
//
// This class represents the layout of the fields in an instance klass.
class ciFieldLayout : public ResourceObj {
  friend class ciInstanceKlass;

private:
  ciFieldLayout(ciInstanceKlass* k);

  GrowableArray<BasicType>* _fieldtypes;
  GrowableArray<int>*       _fieldoffsets;
  GrowableArray<int>*       _access_flags;
  int                       _header_count;

  void fill_in_header_fields(GrowableArray<BasicType>* fieldtypes,
			     GrowableArray<int>* fieldoffsets,
			     int& pos);
  void fill_in_instance_fields(GrowableArray<BasicType>* fieldstypes, 
			       GrowableArray<int>* fieldoffsets,
			       GrowableArray<int>* flags,
			       int& pos, klassOop k);

public:
  BasicType type_at(int index) { return _fieldtypes->at(index); }

  int offset_at(int index)     { return _fieldoffsets->at(index); }

  ciFlags flags_at(int index)  {
    ciFlags flags;
    flags._flags = _access_flags->at(index);
    return flags;
  }
		
  int header_count() { return _header_count; }
  int count() { return _fieldtypes->length(); }

  void print_layout();
  void print();

};
