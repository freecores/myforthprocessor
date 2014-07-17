#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vframeArray.hpp	1.67 03/01/23 12:26:41 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A vframeArray is an array used for storing off stack Java method activations
// during deoptimization,

class LocalsClosure;
class ExpressionStackClosure;
class MonitorStackClosure;
class MonitorArrayElement;

// The MonitorArray represents the collection of live monitors for a deoptimized vframe. 
// After a compiled frame has been deoptimized a MonitorArray will refer the 
// BasicLocks in the old compiled frame. This is needed since we could be inside
// a runtime routine referring the BasicLock (example: ObjectSynchronizer::enter)
// When unpacking happens, the BasicLocks are first migrated to an off stack structure
// and then migrated back to interpreter frames.
//
// It is the responsibility of the owner thread to traverse the oops in the MonitorArray
//

class MonitorArray : public CHeapObj {
 private:
  int                  _length;        // Number of monitors int the array
  MonitorArrayElement* _monitors;      // Refers the elements
  MonitorChunk*        _monitor_chunk; // Tells whether the locks are located on stack
  JavaThread*          _owner;
  MonitorArray*        _next;
 public:
  // constructor
  MonitorArray(int length, JavaThread* owner);
  ~MonitorArray();

  // returns an element
  MonitorArrayElement* at(int index);
  void at_put(int index, oop owner, BasicLock* lock);
  // returns the length
  int length() const { return _length; }
  // returns the monitor_chunk if allocated
  MonitorChunk* monitor_chunk() const { return _monitor_chunk; }
  // tells whether the lock are on the stack
  bool has_locks_on_stack() const { return monitor_chunk() == NULL; }
  // migrate off the stack
  void migrate_locks_off_stack(JavaThread* owner);
  // conversion
  GrowableArray<MonitorInfo*>* as_monitor_list();  
  // gc
  void oops_do(OopClosure* f);
  // traverse monitors
  void monitors_do(void f(oop, BasicLock*));


  // Link information
  MonitorArray* next() const { return _next; }
  void set_next(MonitorArray* next) { _next = next; }
};

class vframeArray: public CHeapObj {
 private:
  JavaThread*                  _owner_thread;
  vframeArray*                 _next;
  CodeBlob*                    _old_adapter;       // I2C/OSR adapter, if being removed
  frame                        _original;          // the original frame of the deoptee
  frame                        _caller;            // caller of root frame in vframeArray
  // This frame is the sender (ignoring adapters) of the frame being deoptimized
  // When an I2C or OSR adapter is being removed is identical to _adapter_caller
  // it'd be nice to remove _adapter_caller and use sender with an appropriate flag.
  frame                        _sender;
#ifdef COMPILER2
  // Either a C2I adapter will be added or an existing I2C adapter will be removed.
  C2IAdapter*                  _new_adapter;           // C2I adapter, if required
  bool                         _adjust_adapter_caller; // Does interp-frame need more space for non-parameter_locals
#endif
  frame                        _adapter_caller;    // caller of I2C or OSR adapter to be removed

  Deoptimization::UnrollBlock* _unroll_block;
  int                          _frame_size;

 public:
  int                          _length; // number of elements in the array

  // Returns the address of the first element.
  intptr_t* base() const              { return (intptr_t*) (((intptr_t) this) + sizeof(vframeArray)); }

  // Tells whether index is within bounds.
  bool is_within_bounds(int index) const        { return 0 <= index && index < length(); }

  // Accessores for instance variable
  int length() const                            { return _length;   }
  void set_length(int length)                   { _length = length; }

  intptr_t* base_at_addr(int index) const {
    assert(is_within_bounds(index), "index out of bounds");
    return &((intptr_t*) base())[index];
  }
  intptr_t* int_at_addr(int index)    const { return       base_at_addr(index); }
  oop*      obj_at_addr(int index)    const { return (oop*)base_at_addr(index); }

 public:
  static vframeArray* allocate(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk,
                               RegisterMap* reg_map, frame sender, frame caller, frame self, frame adapter_caller);

  intptr_t int_at(int index) const                 { return *int_at_addr(index);     }
  void     int_at_put(int index, intptr_t value)   { *int_at_addr(index) = value;    }

  oop    obj_at(int index) const                   { return *obj_at_addr(index);           }
  void   obj_at_put(int index, oop value)          { *obj_at_addr(index) = value; }

  void validate_index(int index) const;
  int  locals_index_for(int index, int offset) const;
  int  expression_stack_index_for(int index, int offset) const;

  int locals_size_for(int index) const;
  int expression_stack_size_for(int index) const;

  int fill_in_vframe(int index, compiledVFrame* vf);
  int fill_in_locals(int index, compiledVFrame* vf);
  int fill_in_expression_stack(int index, compiledVFrame* vf);

  MonitorArray* register_monitors(compiledVFrame* vf);

 public:

  // Moves the monitors on the stack to an off stack representation
  // (called when computing new stack pointer [before frame is mangled).
  void migrate_monitors_off_stack();


  enum {
    // offsets for the prologue
    first_register_entry_offset     = 0, // Register serialization 
    last_register_entry_offset      = 0 + (RegisterMap::reg_count * 2), // Currently assuming the all entries are wordsize! 
    prologue_size                   = last_register_entry_offset,

    // offsets for vframe attributes
    sender_index_offset          = 0,
    method_offset                = 1,
    bci_offset                   = 2,
    monitor_list_offset          = 3,
    locals_size_offset           = 4,
    expression_stack_size_offset = 5,
    vframe_fixed_size            = 6
  };

  static int first_index() { return prologue_size; }
  javaVFrame* vframe_at(int index);

  // Fills the array with vframe information in chunk
  void fill_in(JavaThread* thread, int frame_size, GrowableArray<compiledVFrame*>* chunk, const RegisterMap *reg_map, bool needs_adapter);

  // Returns the owner of this vframeArray
  JavaThread* owner_thread() const           { return _owner_thread; }

  // Accessors for next
  vframeArray* next() const                  { return _next; }
  void set_next(vframeArray* value)          { _next = value; }

  // Accessors for sp
  intptr_t* sp() const                       { return _original.sp(); }

  intptr_t* unextended_sp() const            { return _original.unextended_sp(); }

  address original_pc() const                { return _original.pc(); }

  frame original() const                     { return _original; }
  void set_original(frame original)          { _original = original; }

  frame caller() const                       { return _caller; }
  void set_caller(frame caller)              { _caller = caller; }

  frame sender() const                       { return _sender; }
  void set_sender(frame sender)              { _sender = sender; }

  void set_old_adapter(CodeBlob* adapter)    { _old_adapter = adapter; }
  CodeBlob* old_adapter(void) const          { return _old_adapter; }

#ifdef COMPILER2
  C2IAdapter* new_adapter() const            { return _new_adapter; }
  void set_new_adapter(C2IAdapter* adapter)  { _new_adapter = adapter; }
  bool        adjust_adapter_caller() const  { return _adjust_adapter_caller; }
  void set_adjust_adapter_caller(bool adjust){ _adjust_adapter_caller = adjust; }
#endif
  // Like to remove these guys
  frame adapter_caller() const               { return _adapter_caller; }
  void set_adapter_caller(frame caller)      { _adapter_caller = caller; }

  // Accessors for unroll block
  Deoptimization::UnrollBlock* unroll_block() const         { return _unroll_block; }
  void set_unroll_block(Deoptimization::UnrollBlock* block) { _unroll_block = block; }

  // Returns the size of the frame that got deoptimized
  int frame_size() const { return _frame_size; }

  // Machine dependent code for extending the caller_frame for interpreted callee's locals
  COMPILER1_ONLY(int extend_caller_frame(int callee_parameters, int callee_locals);)

  // Machine dependent code for extending the adapter_caller for interpreted callee's locals
  COMPILER2_ONLY(int i2c_frame_adjust(int callee_parameters, int callee_locals);)

  // Accessors for static part of an activation
  int          sender_index_for(int index) const;
  methodOop    method_for(int index)       const;
  int          bci_for(int index)          const;
  void         set_bci_for(int index, int rawbci);
  MonitorArray* monitor_array_for(int index) const;

  // Locals
  bool     locals_is_obj_for(int index, int offset)             const;
  oop      locals_obj_for(int index, int offset)                const;
  void     set_locals_obj_for(int index, int offset, oop value);
  intptr_t locals_int_for(int index, int offset)                const;
  void     set_locals_int_for(int index, int offset, intptr_t value);
  oop*     locals_obj_addr_for(int index, int offset)           const;

  // Expression stack
  bool     expression_stack_is_obj_for(int index, int offset)   const;
  oop      expression_stack_obj_for(int index, int offset)      const;
  intptr_t expression_stack_int_for(int index, int offset)      const;
  oop*     expression_stack_obj_addr_for(int index, int offset) const;

  // Unpack the array on the stack passed in stack interval
  void unpack_to_stack(frame &unpack_frame, int exec_mode);

  // Deallocates monitor chunks allocated during deoptimization.
  // This should be called when the array is not used anymore.
  void deallocate_monitor_chunks();

  // Sizing
  static int object_size(int length)  { return align_object_size(header_size() + length); }
  static int header_size()            { return sizeof(vframeArray)/HeapWordSize; }
  int object_size() const             { return object_size(length()); }

  // Computes the array length needed for the list of vframes 
  static int array_length_for(GrowableArray<compiledVFrame*>* chunk);
  static int vframe_store_size(compiledVFrame* vf);
  friend class vframeArrayKlass;
  friend class IterateVFrameClosure;

#ifndef PRODUCT
  // Comparing
  bool structural_compare(JavaThread* thread, GrowableArray<compiledVFrame*>* chunk);
#endif
  
  // Accessor for register map
  address register_location(int i) const;

  // Stack-crawl support
  void update_register_map(RegisterMap *map);

  // GC support
  void oops_do(OopClosure* f);

  void print_on_2(outputStream* st) PRODUCT_RETURN;
  void print_value_on(outputStream* st) const PRODUCT_RETURN;
};

