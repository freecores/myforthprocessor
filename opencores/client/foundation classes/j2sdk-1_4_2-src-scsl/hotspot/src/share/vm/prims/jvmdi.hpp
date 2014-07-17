#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdi.hpp	1.28 03/01/23 12:20:10 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class contains the JVMDI implementation for hotspot.
//
// We need to pick up the last set of JVMDI revisions!
//
//


class JvmdiEventEnabled;
class JvmdiFramePops;
class JvmdiBreakpoints;


class jvmdi : public AllStatic {
 private:
  static bool        _enabled;
  static bool        _bytecode_stepping;
  static int         _field_access_count;
  static int         _field_modification_count;
  static bool        _method_entry_on;
  static bool        _method_exit_on;

  static int         _notice_safepoints_depth;

  static jframeID get_current_frame_id(JavaThread *thread, frame current_frame);

 public:  
  // Initialize/destroy our state
  static void initialize();
  static void destroy();

  // State so VM knows whether it should post events or not.
  static void enable();
  static void disable();
  inline static bool enabled()                                    { return _enabled;  }

  // Are we bytecode stepping?
  inline static bool is_bytecode_stepping()                       { return _bytecode_stepping; }

  // turn bytecode stepping (via safepoints) on and off
  static void set_bytecode_stepping(bool on);


  static JVMDI_Interface_1* GetInterface_1(JavaVM *interfaces_vm);
  
  // is the event_type valid?
  static bool is_valid_event_type(jint event_type);  

  // field watch operations
  static void set_dynamic_enable_watchpoints(int event_type, bool enabled);

  // field access management methods
  static void dec_field_access_count();
  static address  get_field_access_count_addr();
  static void inc_field_access_count();

  // field modification management methods
  static void dec_field_modification_count();
  static address  get_field_modification_count_addr();
  static void inc_field_modification_count();

  // return addr of method entry/exit flags (used by the interpreter generator)
  static inline address get_method_entry_on_addr() {
    return (address)(&_method_entry_on);
  }
  static inline void set_method_entry_on(bool value) {
    _method_entry_on = value;
  }
  static inline intptr_t get_method_exit_on_addr() {
    return (intptr_t)(&_method_exit_on);
  }
  static inline void set_method_exit_on(bool value) {
    _method_exit_on = value;
  }

  static inline void notice_safepoints() {
    _notice_safepoints_depth++;
  }

  static inline int notice_safepoints_depth() {
    return _notice_safepoints_depth;
  }

  static inline bool ignore_safepoints() {
    assert(_notice_safepoints_depth > 0, "depth > 0");
    _notice_safepoints_depth--;
    return (_notice_safepoints_depth == 0);
  }


  // Allow native methods to walk the stack.
  // In order to support stack walking operations when inside a native
  // method (which is required by the spec as well as by some tests and
  // the jdbx debugger), we make a thread walkable while inside the
  // native method's context. Note that we have to be careful to make it
  // "unwalkable" upon re-entering Java code.

  // These routines are called by the interpreter and by compiled native
  // wrappers upon entry to and exit from a native method
  static void set_cur_thread_in_native_code();
  static void clear_cur_thread_in_native_code();
  static bool is_cur_thread_in_native_code();

  // single stepping management methods
  static void at_single_stepping_point(JavaThread *thread, methodOop method, address location);
  static void expose_single_stepping(JavaThread *thread);
  static bool hide_single_stepping(JavaThread *thread);

  // Methods that notify the debugger that something interesting has happened in the VM.
  static void post_vm_initialized_event  (); 
  static void post_vm_death_event        ();
  
  static void post_single_step_event     (JavaThread *thread, methodOop method, address location);
  static void post_breakpoint_event      (JavaThread *thread, methodOop method, address location);
  
  static void post_exception_throw_event (JavaThread *thread, methodOop method, address location, oop exception);
  static void notice_unwind_due_to_exception (JavaThread *thread, methodOop method, address location, oop exception, bool in_handler_frame);

  static oop jni_GetField_probe          (JavaThread *thread, jobject jobj,
    oop obj, klassOop klass, jfieldID fieldID, bool is_static);
  static oop jni_GetField_probe_nh       (JavaThread *thread, jobject jobj,
    oop obj, klassOop klass, jfieldID fieldID, bool is_static);
  static void post_field_access_by_jni   (JavaThread *thread, oop obj,
    klassOop klass, jfieldID fieldID, bool is_static);
  static void post_field_access_event    (JavaThread *thread, methodOop method,
    address location, KlassHandle field_klass, Handle object, jfieldID field);
  static oop jni_SetField_probe          (JavaThread *thread, jobject jobj,
    oop obj, klassOop klass, jfieldID fieldID, bool is_static, char sig_type,
    jvalue *value);
  static oop jni_SetField_probe_nh       (JavaThread *thread, jobject jobj,
    oop obj, klassOop klass, jfieldID fieldID, bool is_static, char sig_type,
    jvalue *value);
  static void post_field_modification_by_jni(JavaThread *thread, oop obj,
    klassOop klass, jfieldID fieldID, bool is_static, char sig_type,
    jvalue *value);
  static void post_field_modification_event(JavaThread *thread, methodOop method,
    address location, KlassHandle field_klass, Handle object, jfieldID field,
    char sig_type, jvalue *value);

  static void post_method_entry_event    (JavaThread *thread, methodOop method, frame current_frame);
  static void post_method_exit_event     (JavaThread *thread, methodOop method, frame current_frame);

  static void post_class_load_event      (JavaThread *thread, klassOop klass);
  static void post_class_unload_event    (klassOop klass);
  static void post_class_prepare_event   (JavaThread *thread, klassOop klass);
  
  static void post_thread_start_event    (JavaThread *thread);
  static void post_thread_end_event      (JavaThread *thread);
  
  static void initialize_thread          (JavaThread *thread);
  static void cleanup_thread             (JavaThread* thread);  

  static void oops_do(OopClosure* f);
  static void gc_epilogue();
};



