#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)instanceKlass.hpp	1.151 03/01/23 12:12:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An instanceKlass is the VM level representation of a Java class. 
// It contains all information needed for at class at execution runtime.

//  instanceKlass layout:
//    [header                     ] klassOop
//    [klass pointer              ] klassOop
//    [C++ vtbl pointer           ] Klass
//    [subtype cache              ] Klass
//    [instance size              ] Klass
//    [java mirror                ] Klass
//    [super                      ] Klass
//    [access_flags               ] Klass 
//    [name                       ] Klass
//    [first subklass             ] Klass
//    [next sibling               ] Klass
//    [array klasses              ]
//    [methods                    ]
//    [local interfaces           ]
//    [transitive interfaces      ]
//    [number of implementors     ]
//    [implementor                ] set if exactly one implementor
//    [fields                     ]
//    [constants                  ]
//    [class loader               ]
//    [protection domain          ]
//    [signers                    ]
//    [source file name           ]
//    [inner classes              ]
//    [static field size          ]
//    [nonstatic field size       ]
//    [static oop fields size     ]
//    [nonstatic oop maps size    ]
//    [has finalize method        ]
//    [deoptimization mark bit    ]
//    [initialization state       ]
//    [initializing thread        ]
//    [Java vtable length         ]
//    [oop map cache (stack maps) ]
//    [EMBEDDED Java vtable             ] size in words = vtable_len
//    [EMBEDDED static oop fields       ] size in words = static_oop_fields_size
//    [         static non-oop fields   ] size in words = static_field_size - static_oop_fields_size
//    [EMBEDDED nonstatic oop-map blocks] size in words = nonstatic_oop_map_size
//
//    The embedded nonstatic oop-map blocks are short pairs (offset, length) indicating
//    where oops are located in instances of this klass.


// forward declaration for class -- see below for definition
class SuperTypeClosure;
class OopMapBlock;
class JNIid;
class BreakpointInfo;
class fieldDescriptor;
class nmethodBucket;
class CMSIsAliveClosure;
class CMSKeepAliveClosure;

class instanceKlass: public Klass {
  friend class VMStructs;
 public:
  // See "The Java Virtual Machine Specification" section 2.16.2-5 for a detailed description 
  // of the class loading & initialization procedure, and the use of the states.
  enum ClassState {
    unparsable_by_gc = 0,               // object is not yet parsable by gc. Value of _init_state at object allocation.
    allocated,                          // allocated (but not yet linked)
    loaded,                             // loaded and inserted in class hierarchy (but not linked yet)
    linked,                             // successfully linked/verified (but not initialized yet)
    being_initialized,                  // currently running class initializer
    fully_initialized,                  // initialized (successfull final state)
    initialization_error                // error happened during initialization
  };

 protected:
  klassOop        _array_klasses;        // Array classes holding elements of this class
  objArrayOop     _methods;              // Method array
  typeArrayOop    _method_ordering;      // Int array containing the original order of method in the class file (for JVMDI)
  objArrayOop     _local_interfaces;     // Interface (klassOops) this class declares locally to implements
  objArrayOop     _transitive_interfaces;// Interface (klassOops) this class implements transitively
  int             _nof_implementors;     // No of implementors of this interface (if it is an interface)
  klassOop        _implementor;          // implementor of this interface (only valid if exactly one implementor of interface)  
  typeArrayOop    _fields;               // Instance and static variable information, 5-tuples of shorts [access, name index, sig index, initval index, offset]
  constantPoolOop _constants;            // Constant pool for this class
  oop             _class_loader;         // Class loader used to load this class, NULL if VM loader used
  oop             _protection_domain;    // Protection domain
  objArrayOop     _signers;              // Class signers
  symbolOop       _source_file_name;     // Name of source file containing this klass, NULL if not specified
  symbolOop       _source_debug_extension;// the source debug extension for this klass, NULL if not specified
  typeArrayOop    _inner_classes;        // inner_classes attribute
  int             _nonstatic_field_size; // number of non-static fields in this klass (including inherited fields)
  int             _static_field_size;    // number of static fields (oop and non-oop) in this klass
  int             _static_oop_field_size;// number of static oop fields in this klass
  int             _nonstatic_oop_map_size;// number of nonstatic oop-map blocks allocated at end of this klass
  bool            _is_marked_dependent;  // used for marking during flushing and deoptimization
  ClassState      _init_state;           // state of class
  Thread*         _init_thread;          // Pointer to current thread doing initialization (to handle recusive initialization)
  int             _vtable_len;           // length of Java vtable (in words)
  int             _itable_len;           // length of Java itable (in words)
  ReferenceType   _reference_type;       // reference type
  OopMapCache*    volatile _oop_map_cache;   // OopMapCache for all methods in the klass (allocated lazily)
  JNIid*          _jni_ids;              // First JNI identifier for static field/methods in this klass
  nmethodBucket*  _dependencies;         // list of dependent nmethods
  nmethod*        _osr_nmethods_head;    // Head of list of on-stack replacement nmethods for this class
  BreakpointInfo* _breakpoints;          // bpt lists, managed by methodOop
  klassOop        _previous_version;     // Previous version of a class which has been redefined

  // embedded Java vtable follows here
  // embedded Java itables follows here
  // embedded static fields follows here
  // embedded nonstatic oop-map blocks follows here

  friend class instanceKlassKlass;
  friend class SystemDictionary;

 public:
  // field sizes
  int nonstatic_field_size() const         { return _nonstatic_field_size; }
  void set_nonstatic_field_size(int size)  { _nonstatic_field_size = size; }
  
  int static_field_size() const            { return _static_field_size; }
  void set_static_field_size(int size)     { _static_field_size = size; }
  
  int static_oop_field_size() const        { return _static_oop_field_size; }
  void set_static_oop_field_size(int size) { _static_oop_field_size = size; }
  
  // Java vtable
  int  vtable_length() const               { return _vtable_len; }
  void set_vtable_length(int len)          { _vtable_len = len; }

  // Java itable
  int  itable_length() const               { return _itable_len; }
  void set_itable_length(int len)          { _itable_len = len; }
  
  // array klasses
  klassOop array_klasses() const           { return _array_klasses; }
  void set_array_klasses(klassOop k)       { oop_store_without_check((oop*) &_array_klasses, (oop) k); }

  // methods
  objArrayOop methods() const              { return _methods; }
  void set_methods(objArrayOop a)          { oop_store_without_check((oop*) &_methods, (oop) a); }

  // method ordering
  typeArrayOop method_ordering() const     { return _method_ordering; }
  void set_method_ordering(typeArrayOop m) { oop_store_without_check((oop*) &_method_ordering, (oop) m); }

  // interfaces
  objArrayOop local_interfaces() const          { return _local_interfaces; }
  void set_local_interfaces(objArrayOop a)      { oop_store_without_check((oop*) &_local_interfaces, (oop) a); }
  objArrayOop transitive_interfaces() const     { return _transitive_interfaces; }
  void set_transitive_interfaces(objArrayOop a) { oop_store_without_check((oop*) &_transitive_interfaces, (oop) a); }

  // fields
  // Field info extracted from the class file and stored
  // as an array of 6 shorts 
  enum FieldOffset {
    access_flags_offset    = 0,
    name_index_offset      = 1,
    signature_index_offset = 2,
    initval_index_offset   = 3,
    low_offset             = 4,
    high_offset            = 5,
    next_offset            = 6
  };

  typeArrayOop fields() const              { return _fields; }
  int offset_from_fields( int index ) const {
    return build_int_from_shorts( fields()->ushort_at(index + low_offset),
                                  fields()->ushort_at(index + high_offset) );
  }
 
  void set_fields(typeArrayOop f)          { oop_store_without_check((oop*) &_fields, (oop) f); }

  // inner classes
  typeArrayOop inner_classes() const       { return _inner_classes; }
  void set_inner_classes(typeArrayOop f)   { oop_store_without_check((oop*) &_inner_classes, (oop) f); }

  enum InnerClassAttributeOffset {
    // From http://mirror.eng/products/jdk/1.1/docs/guide/innerclasses/spec/innerclasses.doc10.html#18814
    inner_class_inner_class_info_offset = 0,
    inner_class_outer_class_info_offset = 1,
    inner_class_inner_name_offset = 2,
    inner_class_access_flags_offset = 3,
    inner_class_next_offset = 4
  };

  // package
  bool is_same_class_package(klassOop class2);
  bool is_same_class_package(oop classloader2, symbolOop classname2);
  static bool is_same_class_package(oop class_loader1, symbolOop class_name1, oop class_loader2, symbolOop class_name2);
  
  // initialization state  
  bool is_loaded() const                   { return _init_state >= loaded; }
  bool is_linked() const                   { return _init_state >= linked; }
  bool is_initialized() const              { return _init_state == fully_initialized; }
  bool is_not_initialized() const          { return _init_state <  being_initialized; }
  bool is_being_initialized() const        { return _init_state == being_initialized; }
  bool is_in_error_state() const           { return _init_state == initialization_error; }
  bool is_reentrant_initialization(Thread *thread)  { return thread == _init_thread; }
  int  get_init_state()                    { return _init_state; } // Useful for debugging

  // marking
  bool is_marked_dependent() const         { return _is_marked_dependent; }
  void set_is_marked_dependent(bool value) { _is_marked_dependent = value; }

  // initialization (virtuals from Klass)
  bool should_be_initialized() const;  // means that initialize should be called
  void initialize(TRAPS);
  void link_class(TRAPS);
  methodOop class_initializer();
  
  // set the class to initialized if no static initializer is present
  void eager_initialize(Thread *thread);

  // reference type
  ReferenceType reference_type() const     { return _reference_type; }
  void set_reference_type(ReferenceType t) { _reference_type = t; }

  // find local field, returns true if found
  bool find_local_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const;
  // find field in direct superinterfaces, returns the interface in which the field is defined
  klassOop find_interface_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const;
  // find field according to JVM spec 5.4.3.2, returns the klass in which the field is defined
  klassOop find_field(symbolOop name, symbolOop sig, fieldDescriptor* fd) const;
  // find instance or static fields according to JVM spec 5.4.3.2, returns the klass in which the field is defined
  klassOop jni_find_field(symbolOop name, symbolOop sig, bool is_static, fieldDescriptor* fd) const;
  
  // find a non-static or static field given its offset within the class.
  bool contains_field_offset(int offset) { 
      return ((offset/wordSize) >= instanceOopDesc::header_size() && 
             (offset/wordSize)-instanceOopDesc::header_size() < nonstatic_field_size()); 
  }

  bool jni_find_local_field_from_offset(int offset, bool is_static, fieldDescriptor* fd) const;
  bool jni_find_field_from_offset(int offset, bool is_static, fieldDescriptor* fd) const;
  
  // find a local method (returns NULL if not found)
  methodOop find_method(symbolOop name, symbolOop signature) const;
  static methodOop find_method(objArrayOop methods, symbolOop name, symbolOop signature);

  // lookup operation (returns NULL if not found)
  methodOop uncached_lookup_method(symbolOop name, symbolOop signature) const;

  // lookup a method in all the interfaces that this class implements
  // (returns NULL if not found)
  methodOop lookup_method_in_all_interfaces(symbolOop name, symbolOop signature) const;
  
  // Return the index in the method array for 'method'
  // -1 is returned if the method is not found (and an exception is thrown)
  int method_index_for(methodOop method, TRAPS) const;

  // constant pool
  constantPoolOop constants() const        { return _constants; }
  void set_constants(constantPoolOop c)    { oop_store_without_check((oop*) &_constants, (oop) c); }
 
  // class loader
  oop class_loader() const                 { return _class_loader; }
  void set_class_loader(oop l)             { oop_store((oop*) &_class_loader, l); }
 
  // protection domain
  oop protection_domain()                  { return _protection_domain; }
  void set_protection_domain(oop pd)       { oop_store((oop*) &_protection_domain, pd); }

  // signers
  objArrayOop signers() const              { return _signers; }
  void set_signers(objArrayOop s)          { oop_store((oop*) &_signers, oop(s)); }
 
  // source file name
  symbolOop source_file_name() const       { return _source_file_name; }
  void set_source_file_name(symbolOop n)   { oop_store_without_check((oop*) &_source_file_name, (oop) n); }

  // source debug extension
  symbolOop source_debug_extension() const    { return _source_debug_extension; }
  void set_source_debug_extension(symbolOop n){ oop_store_without_check((oop*) &_source_debug_extension, (oop) n); }

  // nonstatic oop-map blocks
  int nonstatic_oop_map_size() const        { return _nonstatic_oop_map_size; }
  void set_nonstatic_oop_map_size(int size) { _nonstatic_oop_map_size = size; }

  // previous version of redefined class
  bool has_previous_version() const                   { return _previous_version != NULL; }
  klassOop previous_version() const                   { return _previous_version; }
  void set_previous_version(instanceKlassHandle ikh)  { _previous_version = ikh(); }  // convert to oop and assign
  void init_previous_version()                        { _previous_version = NULL; }  

  // allocation
  DEFINE_ALLOCATE_PERMANENT(instanceKlass);
  instanceOop allocate_instance(TRAPS);
  instanceOop allocate_permanent_instance(TRAPS);

  // additional member function to return a handle
  instanceHandle allocate_instance_handle(TRAPS)      { return instanceHandle(THREAD, allocate_instance(THREAD)); }

  objArrayOop allocate_objArray(int n, int length, TRAPS);
  // Helper function
  static instanceOop register_finalizer(instanceOop i, TRAPS);

  // Check whether reflection/jni/jvm code is allowed to instantiate this class;
  // if not, throw either an Error or an Exception.
  virtual void check_valid_for_instantiation(bool throwError, TRAPS);

  // initialization
  void call_class_initializer(TRAPS);
  void set_initialization_state_and_notify(ClassState state, TRAPS);

  // OopMapCache support
  OopMapCache* oop_map_cache()               { return _oop_map_cache; }
  void set_oop_map_cache(OopMapCache *cache) { _oop_map_cache = cache; }
  void mask_for(methodHandle method, int bci, InterpreterOopMap* entry);

  // JNI identifier support (for static fields/methods)
  JNIid* jni_ids()                           { return _jni_ids; }
  void set_jni_ids(JNIid* ids)               { _jni_ids = ids; }
  JNIid* jni_id_for(int offset);

#ifndef CORE
  // maintenance of deoptimization dependencies
  int mark_dependent_nmethods(klassOop dependee);
  void add_dependent_nmethod(nmethod* nm);
  void remove_dependent_nmethod(nmethod* nm);
#endif // !CORE

  // On-stack replacement support
  nmethod* osr_nmethods_head() const         { return _osr_nmethods_head; };
  void set_osr_nmethods_head(nmethod* h)     { _osr_nmethods_head = h; };
  void add_osr_nmethod(nmethod* n)           CORE_RETURN;  
  void remove_osr_nmethod(nmethod* n)        CORE_RETURN;  
  nmethod* lookup_osr_nmethod(const methodOop m, int bci) const CORE_RETURN0;

  // Breakpoint support (see methods on methodOop for details)
  BreakpointInfo* breakpoints() const       { return _breakpoints; };
  void set_breakpoints(BreakpointInfo* bps) { _breakpoints = bps; };

  // support for stub routines
  static int init_state_offset_in_bytes()    { return (intptr_t)&((instanceKlass*)NULL)->_init_state; }
  static int init_thread_offset_in_bytes()    { return (intptr_t)&((instanceKlass*)NULL)->_init_thread; }

  // subclass/subinterface checks
  bool implements_interface(klassOop k) const;

  // Access to implementors of an interface. We only store the count of implementors, and in case, there
  // is only one implementor, we store the implementor.
  klassOop implementor() const        { return _implementor; }  // NULL if nof_implementors() != 1
  int  nof_implementors() const       { return _nof_implementors; }
  void add_implementor(klassOop k);  // k is a new class that implements this interface
  void init_implementor();           // initialize

  // link this class into the implementors list of every interface it implements
  void process_interfaces(Thread *thread);

  // virtual operations from Klass
  bool is_leaf_class() const               { return _subklass == NULL; }
  objArrayOop compute_secondary_supers(int num_extra_slots, TRAPS);
  bool compute_is_subtype_of(klassOop k);
  bool can_be_primary_super_slow() const;
  klassOop java_super() const              { return super(); }
  int oop_size(oop obj)  const             { return size_helper(); }
  int klass_oop_size() const               { return object_size(); }
  bool oop_is_instance() const             { return true; }

  // Iterators
  void do_local_static_fields(void f(fieldDescriptor*, oop), oop obj);
  void do_local_static_fields(void f(fieldDescriptor*, TRAPS), TRAPS);
  void do_nonstatic_fields(void f(fieldDescriptor*, oop), oop obj); // including inherited fields
  void methods_do(void f(methodOop method));
  void array_klasses_do(void f(klassOop k));
  void with_array_klasses_do(void f(klassOop k));
  bool super_types_do(SuperTypeClosure* blk);

  // Casting from klassOop
  static instanceKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_instance(), "cast to instanceKlass");
    return (instanceKlass*) k->klass_part(); 
  }

  // Sizing (in words)
  static int header_size()            { return align_object_offset(oopDesc::header_size() + sizeof(instanceKlass)/HeapWordSize); }
  int object_size() const             { return object_size(align_object_offset(vtable_length()) + align_object_offset(itable_length()) + static_field_size() + nonstatic_oop_map_size()); }
  static int vtable_start_offset()    { return header_size(); }
  static int vtable_length_offset()   { return oopDesc::header_size() + intptr_t(&((instanceKlass*)NULL)->_vtable_len) / HeapWordSize; }
  static int object_size(int extra)   { return align_object_size(header_size() + extra); }
  
  intptr_t* start_of_vtable() const        { return ((intptr_t*)as_klassOop()) + vtable_start_offset(); }
  intptr_t* start_of_itable() const        { return start_of_vtable() + align_object_offset(vtable_length()); }  
  int  itable_offset_in_words() const { return start_of_itable() - (intptr_t*)as_klassOop(); }

  oop* start_of_static_fields() const { return (oop*)(start_of_itable() + align_object_offset(itable_length())); }
  intptr_t* end_of_itable() const          { return start_of_itable() + itable_length(); }
  oop* end_of_static_fields() const   { return start_of_static_fields() + static_field_size(); }
  int offset_of_static_fields() const { return (intptr_t)start_of_static_fields() - (intptr_t)as_klassOop(); }

  OopMapBlock* start_of_nonstatic_oop_maps() const { return (OopMapBlock*) (start_of_static_fields() + static_field_size()); }

  // Allocation profiling support
  juint alloc_size() const            { return _alloc_count * _size_helper; }
  void set_alloc_size(juint n)        {}

  // Java vtable/itable
  klassVtable* vtable() const;        // return new klassVtable wrapper
  klassItable* itable() const;        // return new klassItable wrapper

  // Garbage collection
  void oop_follow_contents(oop obj);
  void follow_static_fields();
  void adjust_static_fields();
  int  oop_adjust_pointers(oop obj);
  bool object_is_parsable() const { return _init_state != unparsable_by_gc; }
       // Value of _init_state must be zero (unparsable_by_gc) when klass field is set.

  void follow_weak_klass_links();
  void follow_weak_klass_links_for_CMS(
    CMSIsAliveClosure* is_alive, CMSKeepAliveClosure* keep_alive);
  void release_C_heap_structures();

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  void copy_static_fields(PSPromotionManager* pm);

  // Naming
  char* signature_name() const;

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk) {
    return oop_oop_iterate_v(obj, blk);
  }

  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
    return oop_oop_iterate_v_m(obj, blk, mr);
  }

#define InstanceKlass_OOP_OOP_ITERATE_DECL(OopClosureType, nv_suffix)   \
  int  oop_oop_iterate##nv_suffix(oop obj, OopClosureType* blk);        \
  int  oop_oop_iterate##nv_suffix##_m(oop obj, OopClosureType* blk,     \
                                      MemRegion mr);

  ALL_OOP_OOP_ITERATE_CLOSURES_1(InstanceKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_2(InstanceKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_3(InstanceKlass_OOP_OOP_ITERATE_DECL)

  void iterate_static_fields(OopClosure* closure);
  void iterate_static_fields(OopClosure* closure, MemRegion mr);


private:
  // initialization state  
#ifdef ASSERT
  void set_init_state(ClassState state);  
#else
  void set_init_state(ClassState state) { _init_state = state; }
#endif
  void set_init_thread(Thread *thread)  { _init_thread = thread; }

  // Offsets for memory management
  oop* adr_array_klasses() const     { return (oop*)&this->_array_klasses;}
  oop* adr_methods() const           { return (oop*)&this->_methods;}
  oop* adr_method_ordering() const   { return (oop*)&this->_method_ordering;}
  oop* adr_local_interfaces() const  { return (oop*)&this->_local_interfaces;}
  oop* adr_transitive_interfaces() const  { return (oop*)&this->_transitive_interfaces;}
  oop* adr_fields() const            { return (oop*)&this->_fields;}
  oop* adr_constants() const         { return (oop*)&this->_constants;}
  oop* adr_class_loader() const      { return (oop*)&this->_class_loader;}
  oop* adr_protection_domain() const { return (oop*)&this->_protection_domain;}
  oop* adr_signers() const           { return (oop*)&this->_signers;}
  oop* adr_source_file_name() const  { return (oop*)&this->_source_file_name;}
  oop* adr_source_debug_extension() const { return (oop*)&this->_source_debug_extension;}
  oop* adr_inner_classes() const     { return (oop*)&this->_inner_classes;}
  oop* adr_implementor() const       { return (oop*)&this->_implementor;}
  oop* adr_previous_version() const  { return (oop*)&this->_previous_version;}

  // Static methods that are used to implement member methods where an exposed this pointer
  // is needed due to possible GCs
  static void link_class_impl                           (instanceKlassHandle this_oop, TRAPS);
  static void verify_code                               (instanceKlassHandle this_oop, TRAPS);
  static void add_loader_constraints                    (instanceKlassHandle this_oop, TRAPS);
  static void initialize_impl                           (instanceKlassHandle this_oop, TRAPS);
  static void eager_initialize_impl                     (instanceKlassHandle this_oop);
  static void set_initialization_state_and_notify_impl  (instanceKlassHandle this_oop, ClassState state, TRAPS);
  static void call_class_initializer_impl               (instanceKlassHandle this_oop, TRAPS);
  static klassOop array_klass_impl                      (instanceKlassHandle this_oop, bool or_null, int n, TRAPS);
  static void do_local_static_fields_impl               (instanceKlassHandle this_oop, void f(fieldDescriptor* fd, TRAPS), TRAPS);              
  static JNIid* jni_id_for_impl                         (instanceKlassHandle this_oop, int offset);

  // Returns the array class for the n'th dimension
  klassOop array_klass_impl(bool or_null, int n, TRAPS);
  
  // Returns the array class with this class as element type
  klassOop array_klass_impl(bool or_null, TRAPS);

public:
  // jvm support
  jint compute_modifier_flags(TRAPS) const;

public:
  // jvmdi support
  jint class_status() const;

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on      (oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;

  // Verification
  void oop_verify_on(oop obj, outputStream* st);
  static void verify_class_klass_nonstatic_oop_maps(klassOop k) PRODUCT_RETURN;
#endif
};


// ValueObjs embedded in klass. Describes where oops are located in instances of this klass.

class OopMapBlock VALUE_OBJ_CLASS_SPEC {
 private:
  jushort _offset;    // Offset of first oop in oop-map block
  jushort _length;    // Length of oop-map block
 public:
  // Accessors  
  jushort offset() const          { return _offset; }
  void set_offset(jushort offset) { _offset = offset; }

  jushort length() const          { return _length; }
  void set_length(jushort length) { _length = length; }
};



class JNIid: public CHeapObj {
  friend class VMStructs;
 private:
  klassOop           _holder;
  JNIid*             _next;
  int                _offset;
  volatile jint      _resolved_lock;     // Lock for updating _resolved fields
  volatile methodOop _resolved_method;   // Non NULL if method is resolved
  volatile klassOop  _resolved_receiver; // Receiver at time of resolve
#ifdef ASSERT
  bool               _is_static_field_id;
  bool               _is_method_id;
#endif
 public:
  // Accessors
  klassOop holder() const         { return _holder; }
  int offset() const              { return _offset; }
  JNIid* next()                   { return _next; }
  // Constructor
  JNIid(klassOop holder, int offset, JNIid* next);
  // Identifier lookup
  JNIid* find(int offset);
  // Method lookup
  methodOop method();

  // Optimization for avoiding the overhead of resolve


  inline void JNIid::lock() {
    while(atomic::exchange(1, (jint *)&_resolved_lock) != 0) os::yield();
  }
  inline void JNIid::unlock() { 
    _resolved_lock = 0; 
  } 
  methodOop resolved_method() const { return _resolved_method; }        
  klassOop  resolved_receiver() const { return _resolved_receiver; }
  void set_resolved_method( methodOop m, klassOop r ) { _resolved_receiver = r;
                                                        _resolved_method = m; } 
  // Garbage collection support
  void oops_do(OopClosure* f);
  static void deallocate(JNIid* id);
  // Debugging
#ifdef ASSERT
  bool is_static_field_id() const { return _is_static_field_id; }
  void set_is_static_field_id()   { _is_static_field_id = true; }
  bool is_method_id() const       { return _is_method_id; }
  void set_is_method_id()         { _is_method_id = true; }
#endif
  void verify(klassOop holder) PRODUCT_RETURN;
};


// If breakpoints are more numerous than just JVMDI breakpoints,
// consider compressing this data structure.
// It is currently a simple linked list defined in methodOop.cpp.

class BreakpointInfo;

