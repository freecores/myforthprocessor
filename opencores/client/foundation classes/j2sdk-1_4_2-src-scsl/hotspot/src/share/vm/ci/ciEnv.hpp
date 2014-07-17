#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciEnv.hpp	1.42 03/02/28 09:18:05 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciEnv
//
// This class is the top level broker for requests from the compiler
// to the VM.
class ciEnv : StackObj {
  CI_PACKAGE_ACCESS_TO

  friend class CompileBroker;

private:
  JNIEnv*          _jni_env;
  Arena*           _arena;       // Alias for _ciEnv_arena except in init_shared_objects()
  Arena            _ciEnv_arena;
  int              _system_dictionary_modification_counter;
  ciObjectFactory* _factory;
  int              _invalidated_index;
  bool             _success;
  bool             _break_at_compile;
  bool             _create_method_data;
  int              _num_inlined_bytecodes;
  CompileLog*      _log;
  bool             _bailed_out;  // Set if certain out-of-memory errors occur during compile
#ifndef PRODUCT
  int              _fire_out_of_memory_count;
  static int       _fire_out_of_memory_count_delay;
#endif

  char* _name_buffer;
  int   _name_buffer_len;

  // Distinguished instances of certain ciObjects..
  static ciObject*              _null_object_instance;
  static ciMethodKlass*         _method_klass_instance;
  static ciSymbolKlass*         _symbol_klass_instance;
  static ciKlassKlass*          _klass_klass_instance;
  static ciInstanceKlassKlass*  _instance_klass_klass_instance;
  static ciTypeArrayKlassKlass* _type_array_klass_klass_instance;
  static ciObjArrayKlassKlass*  _obj_array_klass_klass_instance;

  static ciInstanceKlass* _ArrayStoreException;
  static ciInstanceKlass* _Class;
  static ciInstanceKlass* _ClassCastException;
  static ciInstanceKlass* _Object;
  static ciInstanceKlass* _Throwable;
  static ciInstanceKlass* _Thread;
  static ciInstanceKlass* _OutOfMemoryError;
  static ciInstanceKlass* _String;

  static ciSymbol*        _unloaded_cisymbol;
  static ciInstanceKlass* _unloaded_ciinstance_klass;
  static ciObjArrayKlass* _unloaded_ciobjarrayklass;

  ciInstance* _NullPointerException_instance;
  ciInstance* _ArithmeticException_instance;

  // Look up a klass by name from a particular class loader (the accessor's).
  // If require_local, result must be defined in that class loader, or NULL.
  // If !require_local, a result from remote class loader may be reported,
  // if sufficient class loader constraints exist such that initiating
  // a class loading request from the given loader is bound to return
  // the class defined in the remote loader (or throw an error).
  //
  // Return an unloaded klass if !require_local and no class at all is found.
  //
  // The CI treats a klass as loaded if it is consistently defined in
  // another loader, even if it hasn't yet been loaded in all loaders
  // that could potentially see it via delegation.
  ciKlass* get_klass_by_name(ciKlass* accessing_klass,
                             ciSymbol* klass_name,
                             bool require_local);

  // Constant pool access.
  ciKlass*   get_klass_by_index(ciInstanceKlass* loading_klass,
                                int klass_index,
                                bool& is_accessible);
  ciConstant get_constant_by_index(ciInstanceKlass* loading_klass,
                                   int constant_index);
  bool       is_unresolved_string(ciInstanceKlass* loading_klass,
                                   int constant_index) const;
  ciField*   get_field_by_index(ciInstanceKlass* loading_klass,
                                int field_index);
  ciMethod*  get_method_by_index(ciInstanceKlass* loading_klass,
                                 int method_index, Bytecodes::Code bc);

  // Implementation methods for loading and constant pool access.
  ciKlass* get_klass_by_name_impl(ciKlass* accessing_klass,
                                  ciSymbol* klass_name,
                                  bool require_local);
  ciKlass*   get_klass_by_index_impl(ciInstanceKlass* loading_klass,
                                     int klass_index,
                                     bool& is_accessible);
  ciConstant get_constant_by_index_impl(instanceKlass* loading_klass,
					int constant_index);
  bool       is_unresolved_string_impl (instanceKlass* loading_klass,
					int constant_index) const;
  ciField*   get_field_by_index_impl(ciInstanceKlass* loading_klass,
				     int field_index);
  ciMethod*  get_method_by_index_impl(ciInstanceKlass* loading_klass,
				      int method_index, Bytecodes::Code bc);

  // Helper methods
  bool       check_klass_accessibility(ciKlass* accessing_klass,
				      klassOop resolved_klassOop);
  methodOop  lookup_method(instanceKlass*  accessor,
			   instanceKlass*  holder,
			   symbolOop       name,
			   symbolOop       sig,
			   Bytecodes::Code bc);

  // Get a ciObject from the object factory.  Ensures uniqueness
  // of ciObjects.
  ciObject* get_object(oop o) {
    if (o == NULL) {
      return _null_object_instance;
    } else {
      return _factory->get(o);
    }
  }

  ciMethod* get_method_from_handle(jobject method);

  // Get a ciMethod representing either an unfound method or
  // a method with an unloaded holder.  Ensures uniqueness of
  // the result.
  ciMethod* get_unloaded_method(ciInstanceKlass* holder,
                                ciSymbol*        name,
                                ciSymbol*        signature) {
    return _factory->get_unloaded_method(holder, name, signature);
  }

  // Get a ciKlass representing an unloaded klass.
  // Ensures uniqueness of the result.
  ciKlass* get_unloaded_klass(ciKlass* accessing_klass,
                              ciSymbol* name) {
    return _factory->get_unloaded_klass(accessing_klass, name, true);
  }

  // See if we already have an unloaded klass for the given name
  // or return NULL if not.
  ciKlass *check_get_unloaded_klass(ciKlass* accessing_klass, ciSymbol* name) {
    return _factory->get_unloaded_klass(accessing_klass, name, false);
  }

  // Get a ciReturnAddress corresponding to the given bci.
  // Ensures uniqueness of the result.
  ciReturnAddress* get_return_address(int bci) {
    return _factory->get_return_address(bci);
  }

  // Get a ciMethodData representing the methodData for a method
  // with none.
  ciMethodData* get_empty_methodData() {
    return _factory->get_empty_methodData();
  }

  // General utility : get a buffer of some required length.
  // Used in symbol creation.
  char* name_buffer(int req_len);

  // Is this thread currently in the VM state?
  static bool is_in_vm();

  // Helper routines for determining the validity of a compilation
  // with respect to concurrent class loading.
  bool must_invalidate_compile_internal(DebugInformationRecorder* recorder);
  static int call_has_multiple_targets(instanceKlass* current,
                                symbolHandle   method_name,
                                symbolHandle   method_sig,
                                bool&          found);
  static bool is_dependence_violated(klassOop k, methodOop m);

public:
  enum {
    not_adapter = -1,
    i2c = 0,
    c2i = 1
  };

  ciEnv(JNIEnv* jni_env, int system_dictionary_modification_counter, bool break_at_compile, bool create_method_data);
  ~ciEnv();

  int  invalidated_index() { return _invalidated_index; }
  bool is_invalidated()    { return _invalidated_index != 0; }

  bool is_success() { return _success; }

  bool break_at_compile() { return _break_at_compile; }

  bool create_method_data() { return _create_method_data; }

  uint compile_id();

  // Register the result of a compilation.
  void register_method(ciMethod*                 target,
                       int                       entry_bci,
                       int                       iep_offset,
                       int                       ep_offset,
                       int                       vep_offset,
                       int                       code_offset,
                       int                       osr_offset,
                       DebugInformationRecorder* recorder,
                       CodeBuffer*               code_buffer,
                       int                       frame_words,
                       OopMapSet*                oop_map_set,
                       ExceptionHandlerTable*    handler_table,
                       ImplicitNullCheckTable*   inc_table,
                       ExceptionRangeTable*      exception_range_table,
                       bool                      has_debug_info = true,
                       bool                      has_unsafe_access = false);

#ifdef COMPILER2
  // Register compiled code for an i2c adapter.
  void register_i2c_adapter(ciMethod*   target,
                            OopMapSet*  oop_map_set,
                            CodeBuffer* code_buffer,
                            int         frame_words);

  // Register compiled code for a c2i adapter.
  void register_c2i_adapter(ciMethod*   target,
                            OopMapSet*  oop_map_set,
                            CodeBuffer* code_buffer,
                            int         first_block_size,
                            int         frame_words);
#endif // COMPILER2

  // JVMPI
  void post_compiled_method_load_event(nmethod *nm);
  void build_jvmpi_line_number_mapping(nmethod *nm, compiled_method_t *cm);

  // Access to certain well known ciObjects.
  ciInstanceKlass* ArrayStoreException_klass() {
    return _ArrayStoreException;
  }
  ciInstanceKlass* Class_klass() {
    return _Class;
  }
  ciInstanceKlass* ClassCastException_klass() {
    return _ClassCastException;
  }
  ciInstanceKlass* Object_klass() {
    return _Object;
  }
  ciInstanceKlass* Throwable_klass() {
    return _Throwable;
  }
  ciInstanceKlass* Thread_klass() {
    return _Thread;
  }
  ciInstanceKlass* OutOfMemoryError_klass() {
    return _OutOfMemoryError;
  }
  ciInstanceKlass* String_klass() {
    return _String;
  }
  ciInstance* NullPointerException_instance() {
    assert(_NullPointerException_instance != NULL, "initialization problem");
    return _NullPointerException_instance;
  }
  ciInstance* ArithmeticException_instance() {
    assert(_ArithmeticException_instance != NULL, "initialization problem");
    return _ArithmeticException_instance;
  }
  static ciSymbol* unloaded_cisymbol() {
    return _unloaded_cisymbol;
  }
  static ciObjArrayKlass* unloaded_ciobjarrayklass() {
    return _unloaded_ciobjarrayklass;
  }
  static ciInstanceKlass* unloaded_ciinstance_klass() {
    return _unloaded_ciinstance_klass;
  }

  ciKlass*  find_system_klass(ciSymbol* klass_name);
  // Note:  To find a class from its name string, use ciSymbol::make,
  // but consider adding to vmSymbols.hpp instead.

  // Use this to make a holder for non-perm compile time constants.
  // The resulting array is guaranteed to satisfy "has_encoding".
  ciArray*  make_array(GrowableArray<ciObject*>* objects);

  // Return the machine-level offset of o, which must be an element of a.
  // This may be used to form constant-loading expressions in lieu of simpler encodings.
  int       array_element_offset_in_bytes(ciArray* a, ciObject* o);

  // Access to the compile-lifetime allocation arena.
  Arena*    arena() { return _arena; }

  // What is the current compilation environment?
  static ciEnv* current() { return ((CompilerThread*)JavaThread::current())->env(); }

  // Overload with current thread argument
  static ciEnv* current(Thread *thread) { return ((CompilerThread*)thread)->env(); }

  static void set_data(void *data) {
    ((CompilerThread*)JavaThread::current())->set_data(data);
  }

  static void* data() {
    return ((CompilerThread*)JavaThread::current())->data();
  }

  // Notice that a method has been inlined in the current compile;
  // used only for statistics.
  void notice_inlined_method(ciMethod* method);

  // Total number of bytecodes in inlined methods in this compile
  int num_inlined_bytecodes() const;

  // Output stream for logging compilation info.
  CompileLog* log() { return _log; }
  void set_log(CompileLog* log) { _log = log; }

  // Check for changes to the system dictionary during compilation
  bool must_invalidate_compile(ciMethod* method, DebugInformationRecorder* recorder);

  // Check for out-of-memory errors that occurred during compilation
  // which may cause incorrect results to be propagated out of the CI
  void set_bailout();
  bool bailed_out(); 

#ifndef PRODUCT
  // Stress testing of the CI's PermGen out-of-memory handling
  void maybe_fire_out_of_memory(TRAPS);
  bool should_fire_out_of_memory();
#endif
};


