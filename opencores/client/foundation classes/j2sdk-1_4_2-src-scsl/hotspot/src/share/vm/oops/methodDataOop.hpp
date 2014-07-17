#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodDataOop.hpp	1.34 03/01/23 12:13:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class BytecodeStream;

// DataLayout
//
// Overlay for generic profiling data.
class DataLayout VALUE_OBJ_CLASS_SPEC {
private:
  // Every data layout begins with a header.  This header
  // contains a tag, which is used to indicate the size/layout
  // of the data, 8 bits of flags, which can be used in any way,
  // and a bci, which is used to tie this piece of data to a
  // specific bci in the bytecodes.
  union {
    intptr_t _header;
    struct {
      u1 _tag;
      u1 _flags;
      u2 _bci;
    } _struct;
  };

  // The data layout has an arbitrary number of cells, each sized
  // to accomodate a pointer or an integer.
  intptr_t _cells[1];

  // Some types of data layouts need a length field.
  static bool needs_array_len(u1 tag);

public:
  enum {
    counter_increment = 1
  };

  enum {
    cell_size = sizeof(intptr_t)
  };

  // Tag values
  enum {
    no_tag,
    bit_data_tag,
    counter_data_tag,
    jump_data_tag,
    virtual_call_data_tag,
    ret_data_tag,
    branch_data_tag,
    multi_branch_data_tag
  };

  enum {
    first_flag = 0
  };

  // Size computation
  static int header_size_in_bytes() {
    return cell_size;
  }

  static int compute_size_in_bytes(int cell_count) {
    return header_size_in_bytes() + cell_count * cell_size;
  }

  // Initialization
  void initialize(u1 tag, u2 bci, int cell_count);

  // Accessors
  u1 tag() {
    return _struct._tag;
  }
  u1 flags() {
    return _struct._flags;
  }
  u2 bci() {
    return _struct._bci;
  }
  
  void set_cell_at(int index, intptr_t value) {
    _cells[index] = value;
  }
  intptr_t cell_at(int index) {
    return _cells[index];
  }
  oop* adr_oop_at(int index) {
    return (oop*)&(_cells[index]);
  }

  void set_flag_at(int flag_number) {
    _struct._flags |= (0x1 << flag_number);
  }
  bool flag_at(int flag_number) {
    return (_struct._flags & (0x1 << flag_number)) != 0;
  }
  
  // Low-level support for code generation.
  static ByteSize header_offset() {
    return byte_offset_of(DataLayout, _header);
  }
  static ByteSize tag_offset() {
    return byte_offset_of(DataLayout, _struct._tag);
  }
  static ByteSize flags_offset() {
    return byte_offset_of(DataLayout, _struct._flags);
  }
  static ByteSize bci_offset() {
    return byte_offset_of(DataLayout, _struct._bci);
  }
  static ByteSize cell_offset(int index) {
    return byte_offset_of(DataLayout, _cells[index]);
  }
  static intptr_t flag_number_to_constant(int bitpos) {
    assert(0 <= bitpos && bitpos < 8, "oob");
    intptr_t temp = (intptr_t)0;
    DataLayout* temp_data = (DataLayout*)&temp;
    temp_data->_struct._flags = 0x1 << bitpos;
    return temp;
  }
};


// ProfileData class hierarchy
class ProfileData;
class   BitData;
class   CounterData;
class     VirtualCallData;
class     RetData;
class   JumpData;
class     BranchData;
class   ArrayData;
class     MultiBranchData;


// ProfileData
//
// A ProfileData object is created to refer to a section of profiling
// data in a structured way.
class ProfileData : public ResourceObj {
private:
#ifndef PRODUCT
  enum {
    tab_width_one = 16,
    tab_width_two = 36
  };
#endif // !PRODUCT

  // This is a pointer to a section of profiling data.
  DataLayout* _data;

protected:
  DataLayout* data() { return _data; }

  enum {
    cell_size = DataLayout::cell_size
  };

public:
  // How many cells are in this?
  virtual int cell_count() {
    ShouldNotReachHere();
    return -1;
  }

  // Return the size of this data.
  int size_in_bytes() {
    return DataLayout::compute_size_in_bytes(cell_count());
  }

protected:
  // Low-level accessors for underlying data
  void set_intptr_at(int index, intptr_t value) {
    assert(0 <= index && index < cell_count(), "oob");
    data()->set_cell_at(index, value);
  }
  intptr_t intptr_at(int index) {
    assert(0 <= index && index < cell_count(), "oob");
    return data()->cell_at(index);
  }
  void set_uint_at(int index, uint value) {
    set_intptr_at(index, (intptr_t) value);
  }
  uint uint_at(int index) {
    return (uint)intptr_at(index);
  }
  void set_int_at(int index, int value) {
    set_intptr_at(index, (intptr_t) value);
  }
  int int_at(int index) {
    return (int)intptr_at(index);
  }
  int int_at_unchecked(int index) {
    return (int)data()->cell_at(index);
  }
  void set_oop_at(int index, oop value) {
    set_intptr_at(index, (intptr_t) value);
  }
  oop oop_at(int index) {
    return (oop)intptr_at(index);
  }
  oop* adr_oop_at(int index) {
    assert(0 <= index && index < cell_count(), "oob");
    return data()->adr_oop_at(index);
  }

  void set_flag_at(int flag_number) {
    data()->set_flag_at(flag_number);
  }
  bool flag_at(int flag_number) {
    return data()->flag_at(flag_number);
  }

  // Low-level accessor for code generation
  static ByteSize cell_offset(int index) {
    return DataLayout::cell_offset(index);
  }
  static int flag_number_to_constant(int bitpos) {
    return DataLayout::flag_number_to_constant(bitpos);
  }

  ProfileData(DataLayout* data) {
    _data = data;
  }

public:
  // Constructor for invalid ProfileData.
  ProfileData();

  u2 bci() {
    return data()->bci();
  }

  address dp() {
    return (address)_data;
  }

  // Type checking
  virtual bool is_BitData()         { return false; }
  virtual bool is_CounterData()     { return false; }
  virtual bool is_JumpData()        { return false; }
  virtual bool is_VirtualCallData() { return false; }
  virtual bool is_RetData()         { return false; }
  virtual bool is_BranchData()      { return false; }
  virtual bool is_ArrayData()       { return false; }
  virtual bool is_MultiBranchData() { return false; }

  BitData* as_BitData() {
    assert(is_BitData(), "wrong type");
    return is_BitData()         ? (BitData*)        this : NULL;
  }
  CounterData* as_CounterData() {
    assert(is_CounterData(), "wrong type");
    return is_CounterData()     ? (CounterData*)    this : NULL;
  }
  JumpData* as_JumpData() {
    assert(is_JumpData(), "wrong type");
    return is_JumpData()        ? (JumpData*)       this : NULL;
  }
  VirtualCallData* as_VirtualCallData() {
    assert(is_VirtualCallData(), "wrong type");
    return is_VirtualCallData() ? (VirtualCallData*)this : NULL;
  }
  RetData* as_RetData() {
    assert(is_RetData(), "wrong type");
    return is_RetData()         ? (RetData*)        this : NULL;
  }
  BranchData* as_BranchData() {
    assert(is_BranchData(), "wrong type");
    return is_BranchData()      ? (BranchData*)     this : NULL;
  }
  ArrayData* as_ArrayData() {
    assert(is_ArrayData(), "wrong type");
    return is_ArrayData()       ? (ArrayData*)      this : NULL;        
  }
  MultiBranchData* as_MultiBranchData() {
    assert(is_MultiBranchData(), "wrong type");
    return is_MultiBranchData() ? (MultiBranchData*)this : NULL;
  }


  // Subclass specific initialization
  virtual void post_initialize(BytecodeStream* stream, methodDataOop mdo) {}

  // GC support
  virtual void follow_contents() {}
  virtual void oop_iterate(OopClosure* blk) {}
  virtual void oop_iterate_m(OopClosure* blk, MemRegion mr) {}
  virtual void adjust_pointers() {}

  // CI translation: ProfileData can represent both MethodDataOop data 
  // as well as CIMethodData data. This function is provided for translating 
  // an oop in a ProfileData to the ci equivalent. Generally speaking, 
  // most ProfileData don't require any translation, so we provide the null 
  // translation here, and the required translators are in the ci subclasses.
  virtual void translate_from(ProfileData* data) {}

  // Support for code generation.
  static ByteSize header_offset() {
    return DataLayout::header_offset();
  }

  virtual void print_data_on(outputStream* st) {
    ShouldNotReachHere();
  }

#ifndef PRODUCT
  void print_shared(outputStream* st, const char* name);
  void tab(outputStream* st);
#endif
};

// BitData
//
// A BitData corresponds to a one-bit flag.  This is used to indicate
// a heroic optimization has failed.
class BitData : public ProfileData {
protected:
  enum { heroic_opt_failure_seen = DataLayout::first_flag,
         null_seen };
  enum { bit_cell_count = 0 };  // no additional data fields needed. 
public:
  BitData(DataLayout* layout) : ProfileData(layout) {
    assert(layout->tag() == DataLayout::bit_data_tag, "wrong type");
  }

  virtual bool is_BitData() { return true; }

  static int static_cell_count() {
    return bit_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  // Accessor
  bool was_heroic_opt_failure_seen() { return flag_at(heroic_opt_failure_seen); }
  void set_heroic_opt_failure_seen()    { set_flag_at(heroic_opt_failure_seen); }
  bool was_null_seen()               { return flag_at(              null_seen); }
  void set_null_seen()                  { set_flag_at(              null_seen); }
  
  // Code generation support
  static intptr_t heroic_flag_constant() {
    return flag_number_to_constant(heroic_opt_failure_seen);
  }
  static intptr_t null_flag_constant() {
    return flag_number_to_constant(null_seen);
  }
  static ByteSize bit_data_size() {
    return cell_offset(bit_cell_count);
  }
  
#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// CounterData
//
// A CounterData corresponds to a simple counter.
class CounterData : public ProfileData {
protected:
  enum { math_pow_failure_seen = DataLayout::first_flag };
  enum {
    count_off,
    counter_cell_count
  };
public:
  CounterData(DataLayout* layout) : ProfileData(layout) {}

  virtual bool is_CounterData() { return true; }

  static int static_cell_count() {
    return counter_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  // Direct accessor
  uint count() {
    return uint_at(count_off);
  }
  bool was_math_pow_failure_seen() { return flag_at(math_pow_failure_seen); }
  void set_math_pow_failure_seen()    { set_flag_at(math_pow_failure_seen); }
  
  // Code generation support
  static ByteSize count_offset() {
    return cell_offset(count_off);
  }
  static ByteSize counter_data_size() {
    return cell_offset(counter_cell_count);
  }
  
#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};
  
// JumpData
//
// A JumpData is used to access profiling information for a direct
// branch.  It is a counter, used for counting the number of branches,
// plus a data displacement, used for realigning the data pointer to
// the corresponding target bci.
class JumpData : public ProfileData {
protected:
  enum {
    taken_off_set,
    displacement_off_set,
    jump_cell_count
  };

  void set_displacement(int displacement) {
    set_int_at(displacement_off_set, displacement);
  }
  
public:
  JumpData(DataLayout* layout) : ProfileData(layout) {
    assert(layout->tag() == DataLayout::jump_data_tag ||
      layout->tag() == DataLayout::branch_data_tag, "wrong type");
  }

  virtual bool is_JumpData() { return true; }

  static int static_cell_count() {
    return jump_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  // Direct accessor
  uint taken() {
    return uint_at(taken_off_set);
  }
  
  int displacement() {
    return int_at(displacement_off_set);
  }
  
  // Code generation support
  static ByteSize taken_offset() {
    return cell_offset(taken_off_set);
  }

  static ByteSize displacement_offset() {
    return cell_offset(displacement_off_set);
  }

  // Specific initialization.
  void post_initialize(BytecodeStream* stream, methodDataOop mdo);
  
#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// VirtualCallData
//
// A VirtualCallData is used to access profiling information about a
// virtual call.  It consists of a counter which counts the total times
// that the call is reached, and a series of (klassOop, count) pairs
// which are used to store a type profile for the virtual call.
class VirtualCallData : public CounterData {
protected:
  enum {
    receiver0_offset = counter_cell_count,
    count0_offset,
    virtual_call_row_cell_count = (count0_offset + 1) - receiver0_offset
  };
public:
  VirtualCallData(DataLayout* layout) : CounterData(layout) {
    assert(layout->tag() == DataLayout::virtual_call_data_tag, "wrong type");
  }

  virtual bool is_VirtualCallData() { return true; }

  static int static_cell_count() {
    return counter_cell_count + (uint) TypeProfileWidth * virtual_call_row_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  // Direct accessors
  static uint row_limit() {
    return TypeProfileWidth;
  }
  klassOop receiver(uint row) {
    assert((uint)row < row_limit(), "oob");
    oop recv = oop_at(receiver0_offset + row * virtual_call_row_cell_count);
    assert(recv == NULL || recv->is_klass(), "wrong type");
    return (klassOop)recv;
  }
  uint receiver_count(uint row) {
    assert((uint)row < row_limit(), "oob");
    return uint_at(count0_offset + row * virtual_call_row_cell_count);
  }
  
  // Code generation support
  static ByteSize receiver_offset(uint row) {
    return cell_offset(receiver0_offset + row * virtual_call_row_cell_count);
  }
  static ByteSize receiver_count_offset(uint row) {
    return cell_offset(count0_offset + row * virtual_call_row_cell_count);
  }
  static ByteSize virtual_call_data_size() {
    return cell_offset(static_cell_count());
  }
  
  // GC support
  virtual void follow_contents();
  virtual void oop_iterate(OopClosure* blk);
  virtual void oop_iterate_m(OopClosure* blk, MemRegion mr);
  virtual void adjust_pointers();

  oop* adr_receiver(uint row) {
    return adr_oop_at(receiver0_offset + row * virtual_call_row_cell_count);
  }

#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// RetData
//
// A RetData is used to access profiling information for a ret bytecode.
// It is composed of a count of the number of times that the ret has
// been executed, followed by a series of triples of the form
// (bci, count, di) which count the number of times that some bci was the
// target of the ret and cache a corresponding data displacement.
class RetData : public CounterData {
protected:
  enum {
    bci0_offset = counter_cell_count,
    count0_offset,
    displacement0_offset,
    ret_row_cell_count = (displacement0_offset + 1) - bci0_offset
  };

  void set_bci(uint row, int bci) {
    assert((uint)row < row_limit(), "oob");
    set_int_at(bci0_offset + row * ret_row_cell_count, bci);
  }
  void set_bci_count(uint row, uint count) {
    assert((uint)row < row_limit(), "oob");
    set_uint_at(count0_offset + row * ret_row_cell_count, count);
  }
  void set_bci_displacement(uint row, int disp) {
    set_int_at(displacement0_offset + row * ret_row_cell_count, disp);
  }

public:
  RetData(DataLayout* layout) : CounterData(layout) {
    assert(layout->tag() == DataLayout::ret_data_tag, "wrong type");
  }

  virtual bool is_RetData() { return true; }

  enum {
    no_bci = -1 // value of bci when bci1/2 are not in use.
  };

  static int static_cell_count() {
    return counter_cell_count + (uint) TypeProfileWidth * ret_row_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  static uint row_limit() {
    return TypeProfileWidth;
  }

  // Direct accessors
  int bci(uint row) {
    return int_at(bci0_offset + row * ret_row_cell_count);
  }
  uint bci_count(uint row) {
    return uint_at(count0_offset + row * ret_row_cell_count);
  }
  int bci_displacement(uint row) {
    return int_at(displacement0_offset + row * ret_row_cell_count);
  }
  
  // Interpreter Runtime support
  address fixup_ret(int return_bci, methodDataHandle mdo);

  // Code generation support
  static ByteSize bci_offset(uint row) {
    return cell_offset(bci0_offset + row * ret_row_cell_count);
  }
  static ByteSize bci_count_offset(uint row) {
    return cell_offset(count0_offset + row * ret_row_cell_count);
  }
  static ByteSize bci_displacement_offset(uint row) {
    return cell_offset(displacement0_offset + row * ret_row_cell_count);
  }

  // Specific initialization.
  void post_initialize(BytecodeStream* stream, methodDataOop mdo);
  
#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// BranchData
//
// A BranchData is used to access profiling data for a two-way branch.
// It consists of taken and not_taken counts as well as a data displacement
// for the taken case.
class BranchData : public JumpData {
protected:
  enum {
    not_taken_off_set = jump_cell_count,
    branch_cell_count
  };

  void set_displacement(int displacement) {
    set_int_at(displacement_off_set, displacement);
  }
  
public:
  BranchData(DataLayout* layout) : JumpData(layout) {
    assert(layout->tag() == DataLayout::branch_data_tag, "wrong type");
  }

  virtual bool is_BranchData() { return true; }

  static int static_cell_count() {
    return branch_cell_count;
  }
  
  virtual int cell_count() {
    return static_cell_count();
  }
  
  // Direct accessor
  uint not_taken() {
    return uint_at(not_taken_off_set);
  }
  
  // Code generation support
  static ByteSize not_taken_offset() {
    return cell_offset(not_taken_off_set);
  }
  static ByteSize branch_data_size() {
    return cell_offset(branch_cell_count);
  }
  
  // Specific initialization.
  void post_initialize(BytecodeStream* stream, methodDataOop mdo);
  
#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// ArrayData
//
// A ArrayData is a base class for accessing profiling data which does
// not have a statically known size.  It consists of an array length
// and an array start.
class ArrayData : public ProfileData {
protected:
  friend class DataLayout;

  enum {
    array_len_off_set,
    array_start_off_set
  };

  uint array_uint_at(int index) {
    int aindex = index + array_start_off_set;
    return uint_at(aindex);
  }
  int array_int_at(int index) {
    int aindex = index + array_start_off_set;
    return int_at(aindex);
  }
  oop array_oop_at(int index) {
    int aindex = index + array_start_off_set;
    return oop_at(aindex);
  }
  void array_set_int_at(int index, int value) {
    int aindex = index + array_start_off_set;
    set_int_at(aindex, value);
  }
  
  // Code generation support for subclasses.
  static ByteSize array_element_offset(int index) {
    return cell_offset(array_start_off_set + index);
  }

public:
  ArrayData(DataLayout* layout) : ProfileData(layout) {}

  virtual bool is_ArrayData() { return true; }

  static int static_cell_count() {
    return -1;
  }

  int array_len() {
    return int_at_unchecked(array_len_off_set);
  }
  
  virtual int cell_count() {
    return array_len() + 1;
  }

  // Code generation support
  static ByteSize array_len_offset() {
    return cell_offset(array_len_off_set);
  }
  static ByteSize array_start_offset() {
    return cell_offset(array_start_off_set);
  }
};

// MultiBranchData
//
// A MultiBranchData is used to access profiling information for
// a multi-way branch (*switch bytecodes).  It consists of a series
// of (count, displacement) pairs, which count the number of times each
// case was taken and specify the data displacment for each branch target.
class MultiBranchData : public ArrayData {
protected:
  enum {
    default_count_off_set,
    default_disaplacement_off_set,
    case_array_start
  };
  enum {
    relative_count_off_set,
    relative_displacement_off_set,
    per_case_cell_count
  };

  void set_default_displacement(int displacement) {
    array_set_int_at(default_disaplacement_off_set, displacement);
  }
  void set_displacement_at(int index, int displacement) {
    array_set_int_at(case_array_start +
                     index * per_case_cell_count +
                     relative_displacement_off_set,
                     displacement);
  }

public:
  MultiBranchData(DataLayout* layout) : ArrayData(layout) {
    assert(layout->tag() == DataLayout::multi_branch_data_tag, "wrong type");
  }

  virtual bool is_MultiBranchData() { return true; }

  static int compute_cell_count(BytecodeStream* stream);

  int number_of_cases() {
    int alen = array_len() - 2; // get rid of default case here.
    assert(alen % per_case_cell_count == 0, "must be even");
    return (alen / per_case_cell_count);
  }

  uint default_count() {
    return array_uint_at(default_count_off_set);
  }
  int default_displacement() {
    return array_int_at(default_disaplacement_off_set);
  }

  uint count_at(int index) {
    return array_uint_at(case_array_start +
			 index * per_case_cell_count +
			 relative_count_off_set);
  }
  int displacement_at(int index) {
    return array_int_at(case_array_start +
			index * per_case_cell_count +
			relative_displacement_off_set);
  }

  // Code generation support
  static ByteSize default_count_offset() {
    return array_element_offset(default_count_off_set);
  }
  static ByteSize default_displacement_offset() {
    return array_element_offset(default_disaplacement_off_set);
  }
  static ByteSize case_count_offset(int index) {
    return case_array_offset() + 
           (per_case_size() * index) +
           relative_count_offset();
  }
  static ByteSize case_array_offset() {
    return array_element_offset(case_array_start);
  }
  static ByteSize per_case_size() {
    return in_ByteSize(per_case_cell_count) * cell_size;
  }
  static ByteSize relative_count_offset() {
    return in_ByteSize(relative_count_off_set) * cell_size;
  }
  static ByteSize relative_displacement_offset() {
    return in_ByteSize(relative_displacement_off_set) * cell_size;
  }

  // Specific initialization.
  void post_initialize(BytecodeStream* stream, methodDataOop mdo);

#ifndef PRODUCT
  void print_data_on(outputStream* st);
#endif
};

// methodDataOop
//
// A methodDataOop holds information which has been collected about
// a method.  Its layout looks like this:
//
// -----------------------------
// | header                    |
// | klass                     |
// -----------------------------
// | method                    |
// | size of the methodDataOop |
// -----------------------------
// | Data entries...           |
// |   (variable size)         |
// |			       |
// .			       .
// .			       .
// .			       .
// |   			       |
// -----------------------------
//
// The data entry area is a heterogeneous array of ProfileData. Each 
// ProfileData in the array corresponds to a specific bytecode in the 
// method.  The entries in the array are sorted by the corresponding 
// bytecode.
//
// During interpretation, if profiling in enabled, the interpreter
// maintains a method data pointer (mdp), which points at the entry
// in the array corresponding to the current bci.  In the course of 
// intepretation, when a bytecode is encountered that has ProfileData 
// associated with it, the entry pointed to by mdp is updated, then the
// mdp is adjusted to point to the next appropriate ProfileData.  If mdp
// is NULL to begin with, the interpreter assumes that the current method
// is not (yet) being profiled.
//
// In methodDataOop parlance, "dp" is a "data pointer", the actual address
// of a ProfileData element.  A "di" is a "data index", the offset in bytes
// from the base of the data entry array.  A "displacement" is the byte offset
// in certain ProfileData objects that indicate the amount the mdp must be 
// adjusted in the event of a change in control flow.
// 

class methodDataOopDesc : public oopDesc {
  friend class VMStructs;
private:
  friend class ProfileData;

  // Back pointer to the methodOop
  methodOop _method;
  
  // Size of this oop in bytes
  int _size;

  // Cached hint for bci_to_dp and bci_to_data
  int _hint_di;

  // Whole-method sticky bits and flags
public:
  enum { 
    MDO_IsParsableByGC     =  nth_bit(0),
    MDO_RangeCheckDeopt     = nth_bit(1),
    MDO_ArrayCheckDeoptOnce = nth_bit(2),
    MDO_ArrayCheckDeoptMany = nth_bit(3)
  }; // Public flag values
private:
  int _flags;
  int _null_ptr_count;

  // Beginning of the data entries
  intptr_t _data[1];

  // Helper for size computation
  static int compute_data_size(BytecodeStream* stream);
  static int bytecode_cell_count(Bytecodes::Code code);
  enum { no_profile_data = -1, variable_cell_count = -2 };

  // Helper for initialization
  DataLayout* data_layout_at(int data_index) {
    assert(data_index % sizeof(intptr_t) == 0, "unaligned");
    return (DataLayout*) (((address)_data) + data_index);
  }

  // Initialize an individual data segment.  Returns the size of
  // the segment in bytes.
  int initialize_data(BytecodeStream* stream, int data_index);

  // Helper for data_at
  DataLayout* limit_data_position() {
    return (DataLayout*)((address)this + _size);
  }
  bool out_of_bounds(int data_index) {
    return data_index >= data_size();
  }

  // Give each of the data entries a chance to perform specific
  // data initialization.
  void post_initialize(BytecodeStream* stream);

  // hint accessors
  int      hint_di() const  { return _hint_di; }
  void set_hint_di(int di)  { 
    assert(!out_of_bounds(di), "hint_di out of bounds");
    _hint_di = di; 
  }
  ProfileData* data_before(int bci) {
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
  static int header_size() {
    return sizeof(methodDataOopDesc)/wordSize;
  }

  // Compute the size of a methodDataOop before it is created.
  static int compute_allocation_size_in_bytes(methodOop method);
  static int compute_allocation_size_in_words(methodOop method);

  // Determine if a given bytecode can have profile information.
  static bool bytecode_has_profile(Bytecodes::Code code) {
    return bytecode_cell_count(code) != no_profile_data;
  }

  // Perform initialization of a new methodDataOop
  void initialize(methodOop method);

  // My size
  int object_size_in_bytes() { return _size; }
  int object_size() {
    return align_object_size(align_size_up(_size, BytesPerWord)/BytesPerWord);
  }

  // Location and size of data area
  address data_base() const {
    return (address) _data;
  }
  int data_size() {
    return object_size_in_bytes() - in_bytes(data_offset());
  }
   
  // Accessors
  methodOop method() { return _method; }

  bool get_range_check_deopt() const { return _flags & MDO_RangeCheckDeopt; }
  void set_range_check_deopt() { _flags |= MDO_RangeCheckDeopt; }

  bool get_array_check_deopt() const { return _flags & MDO_ArrayCheckDeoptMany; }
  void set_array_check_deopt() { // Count: 0, 1, many
    _flags |= (_flags & MDO_ArrayCheckDeoptOnce) 
      ? MDO_ArrayCheckDeoptMany
      : MDO_ArrayCheckDeoptOnce; 
  }

  int get_null_ptr_count() const { return _null_ptr_count; }

  void incr_null_ptr_count() { _null_ptr_count++; }

  // Export all flags so ciMethodData can get a consistent snapshot
  int get_flags() const { return _flags; }

  // Get the data at an arbitrary (sort of) data index.
  ProfileData* data_at(int data_index);

  // Walk through the data in order.
  ProfileData* first_data() { return data_at(first_di()); }
  ProfileData* next_data(ProfileData* current);
  bool is_valid(ProfileData* current) { return current != NULL; }

  // Convert a dp (data pointer) to a di (data index).
  int dp_to_di(address dp) {
    return dp - ((address)_data);
  }

  address di_to_dp(int di) {
    return (address)data_layout_at(di);
  }

  // bci to di/dp conversion.
  address bci_to_dp(int bci);
  int bci_to_di(int bci) {
    return dp_to_di(bci_to_dp(bci));
  }

  // Get the data at an arbitrary bci, or NULL if there is none.
  ProfileData* bci_to_data(int bci);

  // Support for code generation
  static ByteSize data_offset() {
    return byte_offset_of(methodDataOopDesc, _data[0]); 
  }

  // GC support
  oop* adr_method() const { return (oop*)&_method; }
  bool object_is_parsable() const { return mask_bits_are_true(_flags, MDO_IsParsableByGC); }
  void set_object_is_parsable()   { _flags |= MDO_IsParsableByGC; }

#ifndef PRODUCT
  // printing support for method data
  void print_data_on(outputStream* st);

  // verification
  void verify_data_on(outputStream* st);
#endif
};
