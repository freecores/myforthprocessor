#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodOop.hpp	1.170 03/02/03 13:51:14 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A methodOop represents a Java method. 
//
// Memory layout (each line represents a word). Note that most applications load thousands of methods,
// so keeping the size of this structure small has a big impact on footprint.
//
// We put all oops and method_size first for better gc cache locality.
//
// The actual bytecodes are inlined after the end of the methodOopDesc struct.
//
// There are bits in the access_flags telling whether inlined tables are present.
// Note that accessing the line number and local variable tables is not performance critical at all.
// Accessing the checked exceptions table is used by reflection, so we put that last to make access
// to it fast.
//
// The line number table is compressed and inlined following the byte codes. It is found as the first 
// byte following the byte codes. The checked exceptions table and the local variable table are inlined 
// after the line number table, and indexed from the end of the method. We do not compress the checked
// exceptions table since the average length is less than 2, and do not bother to compress the local 
// variable table either since it is mostly absent.
//
// Note that native_function and signature_handler has to be at fixed offsets (required by the interpreter)
//
// |------------------------------------------------------|
// | header                                               |
// | klass                                                |
// |------------------------------------------------------|
// | constants                                            |
// | exception_table                                      |
// | method_size             | max_stack                  |
// | max_locals              | size_of_parameters         |
// | name index              | signature index            |
// | access_flags                                         |
// | code_size               | signature info             |
// | vtable_index                                         |
// | invocation_counter                                   |
// | code                                                 |
// | interpreter_entry                                    |
// | from_compiled_code_entry_point                       |
// |------------------------------------------------------|
// | byte codes              | native_function            |  // native_function and signature_handler are used for
// |                         | signature_handler          |  // native methods only, and they do not contain byte codes
// |------------------------------------------------------|
// | compressed linenumber table                          |  // access flags bit tells whether table is present
// | (see class CompressedLineNumberReadStream)           |  // indexed from start of methodOop
// | (note that length is unknown until decompressed)     |  // elements not necessarily sorted!
// |------------------------------------------------------|
// | localvariable table elements + length (length last)  |  // access flags bit tells whether table is present
// | (length is u2, elements are 5-tuples of u2)          |  // indexed from end of methodOop
// | (see class LocalVariableTableElement)                |
// |------------------------------------------------------|
// | checked exceptions elements + length (length last)   |  // access flags bit tells whether table is present
// | (length is u2, elements are u2)                      |  // indexed from end of methodOop
// | (see class CheckedExceptionElement)                  |
// |------------------------------------------------------|


// Utitily class decribing elements in checked exceptions table inlined in methodOop.
class CheckedExceptionElement VALUE_OBJ_CLASS_SPEC {
 public:
  u2 class_cp_index;
};


// Utitily class decribing elements in local variable table inlined in methodOop.
class LocalVariableTableElement VALUE_OBJ_CLASS_SPEC {
 public:
  u2 start_bci;
  u2 length;
  u2 name_cp_index;
  u2 descriptor_cp_index;
  u2 slot;
};


class methodOopDesc : public oopDesc {
 friend class methodKlass;
 friend class VMStructs;
 private:
  constantPoolOop   _constants;                  // Constant pool
  typeArrayOop      _exception_table;            // The exception handler table. 4-tuples of ints [start_pc, end_pc, handler_pc, catch_type index]
                                                 // For methods with no exceptions the table is pointing to Universe::the_empty_int_array
  u2                _method_size;                // size of this object
  u2                _max_stack;                  // Maximum number of entries on the expression stack
  u2                _max_locals;                 // Number of local variables used by this method
  u2                _size_of_parameters;         // size of the parameter block (receiver + arguments) in words
  uint64_t          _fingerprint;                // Bit vector of signature
                                                 // 0  = Not initialized yet
                                                 // -1 = Too many args to fit, 
                                                 //      must parse the slow way
  u2                _name_index;                 // Method name (index in constant pool)
  u2                _signature_index;            // Method signature (index in constant pool)
#ifdef COMPILER2
  u1                _tier1_compile_done;         // Set by C2 if a tier 1 compile has been done
  u2                _interpreter_throwout_count; // Count of times method was exited via exception while interpreting
#endif
  AccessFlags       _access_flags;               // Access flags
#ifdef CC_INTERP
  int               _result_index;               // C++ interpreter needs for converting results to/from stack
#endif
  u2                _code_size;                  // Size of Java bytecodes allocated immediately after methodOop
  u2                _parameter_info;             // Platform specific information about parameters - for C1 parameter passing
  int               _vtable_index;               // vtable index of this method (-1 for static methods)
                                                 // note: can have vtables with >2**16 elements (because of inheritance)
#ifndef CORE
  InvocationCounter _invocation_counter;         // Incremented before each activation of the method - used to trigger frequency-based optimizations
  InvocationCounter _backedge_counter;           // Incremented before each backedge taken - used to trigger frequencey-based optimizations
#ifdef COMPILER2
  methodDataOop     _method_data;
  int               _interpreter_invocation_count; // Count of times invoked
#endif // COMPILER2
#ifndef PRODUCT
  int               _compiled_invocation_count;  // Number of nmethod invocations so far (for perf. debugging)
#endif
#endif
  nmethod*          _code;                       // Points to the corresponding piece of native code
  address           _interpreter_entry;          // entry point for interpretation (used by vtables)
#ifndef CORE
  address           _from_compiled_code_entry_point; // address to dispatch to (either nmethod or interpreter entry)
#endif
  u2                _number_of_breakpoints;      // fullspeed debugging support
  NOT_PRODUCT(int*  _bci_histogram;)             // array of counts, one per bci

 public:
  // accessors for instance variables

  // access flag
  AccessFlags access_flags() const               { return _access_flags;  }
  void set_access_flags(AccessFlags flags)       { _access_flags = flags; }

  // name
  symbolOop name() const                         { return _constants->symbol_at(_name_index); }
  int name_index()                               { return _name_index; }
  void set_name_index(int index)                 { _name_index = index; }

  // signature
  symbolOop signature() const                    { return _constants->symbol_at(_signature_index); }
  int signature_index()                          { return _signature_index; }
  void set_signature_index(int index)            { _signature_index = index; }
#ifdef CC_INTERP
  void set_result_index(BasicType type);
  int  result_index()                            { return _result_index; }
#endif

  uint64_t fingerprint() const                   { return _fingerprint; }
  uint64_t set_fingerprint( uint64_t fingerprint ) { return _fingerprint = fingerprint; }

  // Helper routine: get klass name + "." + method name + signature as
  // C string in newly-allocated resource array, for the purpose of
  // providing more useful NoSuchMethodErrors
  char* name_and_sig_as_C_string();
  // Static routine in the situations we don't have a methodOop
  static char* name_and_sig_as_C_string(Klass* klass, symbolOop method_name, symbolOop signature);

  // JVMDI breakpoints
  Bytecodes::Code orig_bytecode_at(int bci);
  void        set_orig_bytecode_at(int bci, Bytecodes::Code code);
  void set_breakpoint(int bci);
  void clear_breakpoint(int bci);
  void clear_all_breakpoints();
  // Tracking number of breakpoints, for fullspeed debugging.
  // Only mutated by VM thread.
  u2   number_of_breakpoints() const             { return _number_of_breakpoints; }
  void incr_number_of_breakpoints()              { ++_number_of_breakpoints; }
  void decr_number_of_breakpoints()              { --_number_of_breakpoints; }
  // Initialization only
  void clear_number_of_breakpoints()             { _number_of_breakpoints = 0; }

  // code size
  int code_size() const                          { return _code_size; }
  void set_code_size(int size) {
    assert(max_method_code_size < (1 << 16), "u2 is too small to hold method code size in general");
    assert(0 <= size && size <= max_method_code_size, "invalid code size");
    _code_size = size;
  }

  // method size
  int method_size() const                        { return _method_size; }
  void set_method_size(int size) {
    assert(0 <= size && size < (1 << 16), "invalid method size");
    _method_size = size;
  }

  // constant pool for klassOop holding this method
  constantPoolOop constants() const              { return _constants; }
  void set_constants(constantPoolOop c)          { _constants = c; }

  // max stack
  int  max_stack() const                         { return _max_stack; }
  void set_max_stack(int size)                   { _max_stack = size; }

  // max locals
  int  max_locals() const                        { return _max_locals; }
  void set_max_locals(int size)                  { _max_locals = size; }

#ifdef COMPILER2
  bool tier1_compile_done()                      { return _tier1_compile_done != 0;}
  void set_tier1_compile_done()                  { _tier1_compile_done = 1;}

  // Count of times method was exited via exception while interpreting
  void interpreter_throwout_increment() { 
    if (_interpreter_throwout_count < 65534) {
      _interpreter_throwout_count++;
    }
  }   

  int  interpreter_throwout_count() const        { return _interpreter_throwout_count; }
  void set_interpreter_throwout_count(int count) { _interpreter_throwout_count = count; }
#endif // COMPILER2


  // entry point when called from compiled code
#ifndef CORE
  address from_compiled_code_entry_point() const { return _from_compiled_code_entry_point; }
  void set_from_compiled_code_entry_point(address a) { _from_compiled_code_entry_point = a; }
#endif // CORE

  // size of parameters
  int  size_of_parameters() const                { return _size_of_parameters; }

  // exception handler table
  typeArrayOop exception_table() const           { return _exception_table; }
  void set_exception_table(typeArrayOop e)       { oop_store_without_check((oop*) &_exception_table, (oop) e); }
  bool has_exception_handler() const             { return exception_table() != NULL && exception_table()->length() > 0; }

  // Finds the first entry point bci of an exception handler for an
  // exception of klass ex_klass thrown at throw_bci. A value of NULL
  // for ex_klass indicates that the exception klass is not known; in
  // this case it matches any constraint class. Returns -1 if the
  // exception cannot be handled in this method. The handler
  // constraint classes are loaded if necessary if load_classes is
  // true only. Note that this may throw an exception if loading of
  // the constraint classes caused an IllegalAccessError (bugid
  // 4307310). If an exception is thrown, returns the bci of the
  // exception handler which caused the exception to be thrown, which
  // is needed for proper retries. See, for example,
  // InterpreterRuntime::exception_handler_for_exception.
  int fast_exception_handler_bci_for(KlassHandle ex_klass, int throw_bci, bool load_classes, TRAPS);

#ifndef CORE
  // invocation counter
  InvocationCounter* invocation_counter()        { return &_invocation_counter; }
  InvocationCounter* backedge_counter()          { return &_backedge_counter; }
  int invocation_count() const                   { return _invocation_counter.count(); }
  int backedge_count() const                     { return _backedge_counter.count(); }
  bool was_executed_more_than(int n) const;
  bool was_never_executed() const                { return !was_executed_more_than(0); }

  // method data access
  methodDataOop method_data() const              { 
    COMPILER1_ONLY(return NULL;)
    COMPILER2_ONLY(return _method_data;)
  }
  void set_method_data(methodDataOop data)       { 
    COMPILER1_ONLY(ShouldNotReachHere();) 
    COMPILER2_ONLY(_method_data = data;) 
  }
  static void build_interpreter_method_data(methodHandle method, TRAPS);

#ifdef COMPILER2
  int interpreter_invocation_count() const       { return _interpreter_invocation_count; }
  void set_interpreter_invocation_count(int count) { _interpreter_invocation_count = count; }
  int increment_interpreter_invocation_count() { return ++_interpreter_invocation_count; }
#endif // COMPILER2

#ifndef PRODUCT
  int  compiled_invocation_count() const         { return _compiled_invocation_count; }
  void set_compiled_invocation_count(int count)  { _compiled_invocation_count = count; }
#endif // not PRODUCT
#endif // not CORE

  void alloc_bci_histogram()                     PRODUCT_RETURN;
  int* bci_histogram() const                     PRODUCT_RETURN0 NOT_PRODUCT({ return _bci_histogram; });
  static ByteSize bci_histogram_offset()         PRODUCT_RETURN0 NOT_PRODUCT({ return byte_offset_of(methodOopDesc, _bci_histogram); });

  // nmethod/verified compiler entry
#ifndef CORE
  static address verified_code_entry(methodHandle m);
  bool check_code() const;	// Not inline to avoid circular ref
  nmethod* code() const                          { assert( check_code(), "" ); return _code; }
#else
  nmethod* code() const                          { return _code; }
#endif // not CORE
  void set_code(nmethod* code);
  void init_code();
  static void link_method(methodHandle method);
  void update_compiled_code_entry_point(bool lazy);

  // vtable index
  int  vtable_index() const                      { return _vtable_index; }
  void set_vtable_index(int index)               { _vtable_index = index; }
  void set_has_other_vtable_index1()             { _access_flags.set_has_other_vtable_index1(); }
  void set_has_other_vtable_index2()             { _access_flags.set_has_other_vtable_index2(); }
  bool has_other_vtable_index1() const           { return access_flags().has_other_vtable_index1(); }
  bool has_other_vtable_index2() const           { return access_flags().has_other_vtable_index2(); }

  // interpreter entry
  address interpreter_entry() const              { return _interpreter_entry; }
  void set_interpreter_entry(address entry)      { _interpreter_entry = entry; }

  // parameter handler support
  int parameter_info() const                     { return _parameter_info; }
  void set_parameter_info(int info) {
    assert(0 <= info && info < (1 << 16), "invalid parameter info");
    _parameter_info = info;
  }

  // native function (used for native methods only)
  address native_function() const                { return *(native_function_addr()); }
  void set_native_function(address function);    // Must specify a real function (not NULL).
                                                 // Use clear_native_function() to unregister.
  bool has_native_function() const;
  void clear_native_function();

  // signature handler (used for native methods only)
  address signature_handler() const              { return *(signature_handler_addr()); }
  void set_signature_handler(address handler);

  // Interpreter oopmap support
  void mask_for(int bci, InterpreterOopMap* mask);

#ifndef CORE
#ifndef PRODUCT
  // operations on invocation counter
  void print_invocation_count() const;
#endif
#endif

  // byte codes
  address code_base() const                      { return (address) (this+1); }
  address code_end() const                       { return code_base() + code_size(); }
  bool    contains(address bcp) const            { return code_base() <= bcp && bcp < code_end(); }

  void print_codes() const                       PRODUCT_RETURN; // prints byte codes
  void print_codes(int from, int to) const       PRODUCT_RETURN;

  // checked exceptions
  int checked_exceptions_length() const;
  CheckedExceptionElement* checked_exceptions_start() const;    // not preserved by gc

  // localvariable table
  bool has_localvariable_table() const           { return access_flags().has_localvariable_table(); }
  int localvariable_table_length() const;
  LocalVariableTableElement* localvariable_table_start() const; // not preserved by gc

  // linenumber table
  // note that length is unknown until decompression, see class CompressedLineNumberReadStream
  bool has_linenumber_table() const              { return access_flags().has_linenumber_table(); }
  u_char* compressed_linenumber_table() const;                  // not preserved by gc

  // method holder (the klassOop holding this method)
  klassOop method_holder() const                 { return _constants->pool_holder(); }

  void compute_size_of_parameters(Thread *thread); // word size of parameters (receiver if any + arguments)
  symbolOop klass_name() const;                  // returns the name of the method holder
  BasicType result_type() const;                 // type of the method result
  bool is_returning_oop() const                  { BasicType r = result_type(); return (r == T_OBJECT || r == T_ARRAY); }
  bool is_returning_fp() const                   { BasicType r = result_type(); return (r == T_FLOAT || r == T_DOUBLE); }

  // Checked exceptions thrown by this method (resolved to mirrors)
  objArrayHandle resolved_checked_exceptions(TRAPS) { return resolved_checked_exceptions_impl(this, THREAD); }

  // Access flags
  bool is_public() const                         { return access_flags().is_public();      }
  bool is_private() const                        { return access_flags().is_private();     }
  bool is_protected() const                      { return access_flags().is_protected();   }
  bool is_package_private() const                { return !is_public() && !is_private() && !is_protected(); }
  bool is_static() const                         { return access_flags().is_static();      }
  bool is_final() const                          { return access_flags().is_final();       }
  bool is_synchronized() const                   { return access_flags().is_synchronized();}
  bool is_native() const                         { return access_flags().is_native();      }
  bool is_abstract() const                       { return access_flags().is_abstract();    }
  bool is_strict() const                         { return access_flags().is_strict();      }
  bool is_synthetic() const                      { return access_flags().is_synthetic();   }
  
  // returns true if contains only return operation
  bool is_empty_method() const;

  // returns true if this is a vanilla constructor
  bool is_vanilla_constructor() const;

   // checks method and its method holder
  bool is_final_method() const;
  bool is_strict_method() const;

  // returns true if the method has any backward branches.
  bool has_loops() { 
    return access_flags().loops_flag_init() ? access_flags().has_loops() : compute_has_loops_flag(); 
  };

  bool compute_has_loops_flag();
  
  bool has_jsrs() { 
    return access_flags().has_jsrs();
  };
  void set_has_jsrs() {
    _access_flags.set_has_jsrs();
  }

  // returns true if the method has any monitors.
  bool has_monitors() const                      { return is_synchronized() || access_flags().has_monitor_bytecodes(); } 
  bool has_monitor_bytecodes() const             { return access_flags().has_monitor_bytecodes(); }

  void set_has_monitor_bytecodes()               { _access_flags.set_has_monitor_bytecodes(); }
  
  // monitor matching. This returns a conservative estimate of whether the monitorenter/monitorexit bytecodes
  // propererly nest in the method. It might return false, even though they actually nest properly, since the info.
  // has not been computed yet.
  bool guaranteed_monitor_matching() const       { return access_flags().is_monitor_matching(); }
  void set_guaranteed_monitor_matching()         { _access_flags.set_monitor_matching(); }

  // returns true if the method is an accessor function (setter/getter).
  bool is_accessor() const;

  // returns true if the method is an initializer (<init> or <clinit>).
  bool is_initializer() const;

  // compiled code support
  bool has_compiled_code() const                 { return code() != NULL; }

  // sizing
  static int object_size(int code_size, bool is_native, int compressed_line_number_size, int local_variable_table_length, int checked_exceptions_length);
  static int header_size()                       { return sizeof(methodOopDesc)/HeapWordSize; }
  int object_size() const                        { return method_size(); }

  bool object_is_parsable() const                { return method_size() > 0; }

  // interpreter support
  static ByteSize constants_offset()             { return byte_offset_of(methodOopDesc, _constants         ); }
  static ByteSize exception_table_offset()       { return byte_offset_of(methodOopDesc, _exception_table   ); }
  static ByteSize access_flags_offset()          { return byte_offset_of(methodOopDesc, _access_flags      ); }
  static ByteSize name_index_offset()            { return byte_offset_of(methodOopDesc, _name_index        ); }
  static ByteSize signature_index_offset()       { return byte_offset_of(methodOopDesc, _signature_index   ); }
#ifdef CC_INTERP
  static ByteSize result_index_offset()          { return byte_offset_of(methodOopDesc, _result_index ); }
#endif /* CC_INTERP */
  static ByteSize codes_offset()                 { return in_ByteSize(sizeof(methodOopDesc));                 }
  static ByteSize size_of_locals_offset()        { return byte_offset_of(methodOopDesc, _max_locals        ); }
  static ByteSize size_of_parameters_offset()    { return byte_offset_of(methodOopDesc, _size_of_parameters); }
#ifndef CORE
  static ByteSize from_compiled_code_entry_point_offset() { return byte_offset_of(methodOopDesc, _from_compiled_code_entry_point); }  
  static ByteSize invocation_counter_offset()    { return byte_offset_of(methodOopDesc, _invocation_counter); }
  static ByteSize backedge_counter_offset()      { return byte_offset_of(methodOopDesc, _backedge_counter); }
  static ByteSize method_data_offset()           { 
    COMPILER1_ONLY(ShouldNotReachHere(); return in_ByteSize(0);)
    COMPILER2_ONLY(return byte_offset_of(methodOopDesc, _method_data);)
  }
#ifdef COMPILER2
  static ByteSize interpreter_invocation_counter_offset() { return byte_offset_of(methodOopDesc, _interpreter_invocation_count); }
#endif // COMPILER2
#ifndef PRODUCT
  static ByteSize compiled_invocation_counter_offset() { return byte_offset_of(methodOopDesc, _compiled_invocation_count); }
#endif // not PRODUCT
#endif // not CORE
  static ByteSize compiled_code_offset()         { return byte_offset_of(methodOopDesc, _code              ); }
  static ByteSize native_function_offset()       { return in_ByteSize(sizeof(methodOopDesc));                 }
  static ByteSize interpreter_entry_offset()     { return byte_offset_of(methodOopDesc, _interpreter_entry ); }
  static ByteSize parameter_info_offset()        { return byte_offset_of(methodOopDesc, _parameter_info    ); }
  static ByteSize signature_handler_offset()     { return in_ByteSize(sizeof(methodOopDesc) + wordSize);      }
  static ByteSize max_stack_offset()             { return byte_offset_of(methodOopDesc, _max_stack         ); } 

#ifdef COMPILER2
  // for code generation
  static int method_data_offset_in_bytes()       { return (intptr_t)&(((methodOop) oop(NULL))->_method_data); }
  static int interpreter_invocation_counter_offset_in_bytes()       
                                                 { return (intptr_t)&(((methodOop) oop(NULL))->_interpreter_invocation_count); }
#endif // COMPILER2

  // Static methods that are used to implement member methods where an exposed this pointer
  // is needed due to possible GCs
  static objArrayHandle resolved_checked_exceptions_impl(methodOop this_oop, TRAPS);

  // Returns the byte code index from the byte code pointer
  int     bci_from(address bcp) const;
  address bcp_from(int     bci) const;

  // Returns the line number for a bci if debugging information for the method is prowided,
  // -1 is returned otherwise.
  int line_number_from_bci(int bci) const;

  // Reflection support
  bool is_overridden_in(klassOop k) const;
#ifdef HOTSWAP
  // Dynamic class reloading (evolution) support
  bool is_old_version() const                       { return access_flags().is_old_version(); }
  void set_old_version()                            { _access_flags.set_is_old_version(); }
  bool is_non_emcp_with_new_version() const         { return access_flags().is_non_emcp_with_new_version(); }
  void set_non_emcp_with_new_version()              { _access_flags.set_is_non_emcp_with_new_version(); }
#endif HOTSWAP

  // Rewriting support
  static methodHandle clone_with_new_data(methodHandle m, u_char* new_code, int new_code_length, 
                                          u_char* new_compressed_linenumber_table, int new_compressed_linenumber_size, TRAPS);

  // JNI identifier
  JNIid* jni_id();

  // Lookup JNIid for this method.  Return NULL if not found.
  JNIid* find_jni_id_or_null();

  // (Note:  Here is the only reason this file requires vmSymbols.hpp.)
#define VM_INTRINSIC_ENUM(id, klass, name, sig)  id,
  enum IntrinsicId {
    _none = 0,                      // not an intrinsic (default answer)
    VM_INTRINSICS_DO(VM_INTRINSIC_ENUM)
    _vm_intrinsics_terminating_enum
  };
#undef VM_INTRINSIC_ENUM

  // Support for inlining of intrinsic methods
  IntrinsicId intrinsic_id() const;              // returns zero if not an intrinsic

  // On-stack replacement support   
  nmethod* lookup_osr_nmethod_for(int bci)       { return instanceKlass::cast(method_holder())->lookup_osr_nmethod(this, bci); }

  // Inline cache support
  void cleanup_inline_caches();

  // Find if klass for method is loaded
  bool is_klass_loaded_by_klass_index(int klass_index) const;
  bool is_klass_loaded(int refinfo_index, bool must_be_resolved = false) const;

  // Indicates whether compilation failed earlier for this method, or
  // whether it is not compilable for another reason like having a
  // breakpoint set in it.
  bool is_not_compilable() const;
  void set_not_compilable();

  bool is_not_osr_compilable() const             { return is_not_compilable() || access_flags().is_not_osr_compilable(); }
  void set_not_osr_compilable()                  { _access_flags.set_not_osr_compilable(); }

  // Background compilation support
  bool queued_for_compilation() const            { return access_flags().queued_for_compilation();    }
  void set_queued_for_compilation()              { _access_flags.set_queued_for_compilation(); }
  void clear_queued_for_compilation()            { _access_flags.clear_queued_for_compilation(); }

  static methodOop method_from_bcp(address bcp);

#ifndef CORE
  // Resolve all classes in signature, return 'true' if successful
  static bool load_signature_classes(methodHandle m, TRAPS);

  // Return if true if not all classes references in signature, including return type, has been loaded
  static bool has_unloaded_classes_in_signature(methodHandle m, TRAPS);
#endif // CORE

  // Printing
  void print_short_name(outputStream* st)        /*PRODUCT_RETURN*/; // prints as klassname::methodname; Exposed so field engineers can debug VM
  void print_name(outputStream* st)              PRODUCT_RETURN; // prints as "virtual void foo(int)"

 private:
  // size of parameters
  void set_size_of_parameters(int size)          { _size_of_parameters = size; }

  // Inlined tables
  void set_inlined_tables_length(int checked_exceptions_len, int compressed_line_number_size, int localvariable_table_len);
  u2* checked_exceptions_length_addr() const;
  u2* localvariable_table_length_addr() const;

  // Inlined elements
  address* native_function_addr() const          { assert(is_native(), "must be native"); return (address*) (this+1); }
  address* signature_handler_addr() const        { return native_function_addr() + 1; }

  // Last short in methodOop
  u2* last_u2_element() const                    { return (u2*)method_end() - 1; }
  // First byte after methodOop
  address method_end() const                     { return (address)((oop*)this + method_size()); }
  // First position for inlined tables (for native methods add word for native_function and signature_handler)
  address inlined_table_start() const            { return code_end() + (is_native() ? 2*sizeof(address*) : 0); }

  // Garbage collection support
  oop*  adr_exception_table() const              { return (oop*)&_exception_table; }
  oop*  adr_constants() const                    { return (oop*)&_constants;       }
#ifdef COMPILER2
  oop*  adr_method_data() const                  { return (oop*)&_method_data;     }
#endif // COMPILER2
};


// Utility class for compressing line number tables

class CompressedLineNumberWriteStream: public CompressedWriteStream {
 private:
  int _bci;
  int _line;
 public:
  // Constructor
  CompressedLineNumberWriteStream(int initial_size);
  // Write (bci, line number) pair to stream
  void write_pair(int bci, int line);
  // Write end-of-stream marker
  void write_terminator()                        { write_byte(0); }
};


// Utility class for decompressing line number tables

class CompressedLineNumberReadStream: public CompressedReadStream {
 private:
  int _bci;
  int _line;
 public:
  // Constructor
  CompressedLineNumberReadStream(u_char* buffer);
  // Read (bci, line number) pair from stream. Returns false at end-of-stream.
  bool read_pair();
  // Accessing bci and line number (after calling read_pair)
  int bci() const                               { return _bci; }
  int line() const                              { return _line; }
};


/// Fast Breakpoints.

// If this structure gets more complicated (because bpts get numerous),
// move it into its own header.

// There is presently no provision for concurrent access
// to breakpoint lists, which is only OK for JVMDI because
// breakpoints are written only at safepoints, and are read
// concurrently only outside of safepoints.

class BreakpointInfo : public CHeapObj {
  friend class VMStructs;
 private:
  Bytecodes::Code  _orig_bytecode;
  int              _bci;
  u2               _name_index;       // of method
  u2               _signature_index;  // of method
  BreakpointInfo*  _next;             // simple storage allocation

 public:
  BreakpointInfo(methodOop m, int bci);

  // accessors
  Bytecodes::Code orig_bytecode()                     { return _orig_bytecode; }
  void        set_orig_bytecode(Bytecodes::Code code) { _orig_bytecode = code; }

  BreakpointInfo*          next() const               { return _next; }
  void                 set_next(BreakpointInfo* n)    { _next = n; }

  // helps for searchers
  bool match(methodOop m, int bci) {
    return bci == _bci && match(m);
  }

  bool match(methodOop m) {
    return _name_index == m->name_index() &&
      _signature_index == m->signature_index();
  }

  void set(methodOop method);
  void clear(methodOop method);
};


