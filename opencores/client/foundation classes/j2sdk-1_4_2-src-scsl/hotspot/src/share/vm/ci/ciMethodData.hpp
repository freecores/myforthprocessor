#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciMethodData.hpp	1.15 03/01/23 11:57:31 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ciBitData;
class ciCounterData;
class ciJumpData;
class ciVirtualCallData;
class ciRetData;
class ciBranchData;
class ciArrayData;
class ciMultiBranchData;

typedef ProfileData ciProfileData;

class ciBitData : public BitData {
public:
  ciBitData(DataLayout* layout) : BitData(layout) {};
};

class ciCounterData : public CounterData {
public:
  ciCounterData(DataLayout* layout) : CounterData(layout) {};
};

class ciJumpData : public JumpData {
public:
  ciJumpData(DataLayout* layout) : JumpData(layout) {};
};

class ciVirtualCallData : public VirtualCallData {
public:
  ciVirtualCallData(DataLayout* layout) : VirtualCallData(layout) {};

  void set_receiver(uint row, ciKlass* recv) {
    assert((uint)row < row_limit(), "oob");
    set_intptr_at(receiver0_offset + row * virtual_call_row_cell_count, 
		  (intptr_t) recv);
  }

  ciKlass* receiver(uint row) {
    assert((uint)row < row_limit(), "oob");
    ciObject* recv = (ciObject*)intptr_at(receiver0_offset + row * virtual_call_row_cell_count);
    assert(recv == NULL || recv->is_klass(), "wrong type");
    return (ciKlass*)recv;
  }

  // Copy & translate from oop based VirtualCallData
  void translate_from(ProfileData* data);
#ifndef PRODUCT
  void ciVirtualCallData::print_data_on(outputStream* st);
#endif
};

class ciRetData : public RetData {
public:
  ciRetData(DataLayout* layout) : RetData(layout) {};
};

class ciBranchData : public BranchData {
public:
  ciBranchData(DataLayout* layout) : BranchData(layout) {};
};

class ciArrayData : public ArrayData {
public:
  ciArrayData(DataLayout* layout) : ArrayData(layout) {};
};

class ciMultiBranchData : public MultiBranchData {
public:
  ciMultiBranchData(DataLayout* layout) : MultiBranchData(layout) {};
};

// ciMethodData
//
// This class represents a methodDataOop in the HotSpot virtual
// machine.

class ciMethodData : public ciObject {
  CI_PACKAGE_ACCESS

private:
  // Unchanging copy of whole-method flags
  int _flags;

  // Count of null pointer exceptions thrown
  int _null_ptr_count;

  // Size in bytes
  int _data_size;

  // Data entries
  intptr_t * _data;

  // Cached hint for data_before()
  int _hint_di;

  // Is data attached?
  bool _empty;

  ciMethodData(methodDataHandle h_md);
  ciMethodData();

  // Accessors
  int data_size() { return _data_size; }
  intptr_t * data() { return _data; }

  methodDataOop get_methodDataOop() const {
    if (handle() == NULL) return NULL;
    methodDataOop mdo = (methodDataOop)get_oop();
    assert(mdo != NULL, "illegal use of unloaded method data");
    return mdo;
  }

  const char* type_string()                      { return "ciMethodData"; }

  void print_impl();

  DataLayout* data_layout_at(int data_index) {
    assert(data_index % sizeof(intptr_t) == 0, "unaligned");
    return (DataLayout*) (((address)_data) + data_index);
  }

  bool out_of_bounds(int data_index) {
    return data_index >= data_size();
  }

  // hint accessors
  int      hint_di() const  { return _hint_di; }
  void set_hint_di(int di)  { 
    assert(!out_of_bounds(di), "hint_di out of bounds");
    _hint_di = di; 
  }
  ciProfileData* data_before(int bci) {
    // avoid SEGV on this edge case
    if (data_size() == 0)
      return NULL;
    int hint = hint_di();
    if (data_layout_at(hint)->bci() <= bci)
      return data_at(hint);
    return first_data();
  }


  // What is the index of the first data entry?
  int first_di() { return 0; }

public:
  bool is_method_data()  { return true; }
  bool is_empty() { return _empty; }

  void load_data();

  bool get_range_check_deopt() const { return _flags & methodDataOopDesc::MDO_RangeCheckDeopt; }
  bool get_array_check_deopt() const { return _flags & methodDataOopDesc::MDO_ArrayCheckDeoptMany; }
  bool get_null_ptr_count() const { return _null_ptr_count; }

  // Convert a dp (data pointer) to a di (data index).
  int dp_to_di(address dp) {
    return dp - ((address)_data);
  }

  // Get the data at an arbitrary (sort of) data index.
  ciProfileData* data_at(int data_index);

  // Walk through the data in order.
  ciProfileData* first_data() { return data_at(first_di()); }
  ciProfileData* next_data(ciProfileData* current);
  bool is_valid(ciProfileData* current) { return current != NULL; }

  // Get the data at an arbitrary bci, or NULL if there is none.
  ciProfileData* bci_to_data(int bci);

#ifndef PRODUCT
  // printing support for method data
  void print();
  void print_data_on(outputStream* st);
#endif
};
