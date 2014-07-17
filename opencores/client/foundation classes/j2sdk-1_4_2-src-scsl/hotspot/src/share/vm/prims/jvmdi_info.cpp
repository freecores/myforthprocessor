#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdi_info.cpp	1.82 03/01/23 12:20:26 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdi_info.cpp.incl"

 
//
// Class Information
//

JVMDI_ENTER(jvmdiError, 
            GetClassSignature, (jclass clazz, 
                                char **sigPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(sigPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    ResourceMark rm(THREAD);
    char* result = NULL;
    if (java_lang_Class::is_primitive(mirror)) {
      result = (char*) JvmdiInternal::jvmdiMalloc(2);
      result[0] = type2char(java_lang_Class::primitive_type(mirror));
      result[1] = '\0';
    } else {
      klassOop k = java_lang_Class::as_klassOop(mirror);
      NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
      JVMDI_TRACE_ARG2("JvmdiInternal::GetClassSignature of ", Klass::cast(k)->external_name());
      const char* class_sig = Klass::cast(k)->signature_name();
      result = (char *) JvmdiInternal::jvmdiMalloc(strlen(class_sig)+1);
      strcpy(result, class_sig);
    }
    *sigPtr = result;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassSignature: *sigPtr", *sigPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassStatus, (jclass clazz, 
                             jint *statusPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(statusPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    jint result = 0;
    if (!java_lang_Class::is_primitive(mirror)) {
      klassOop k = java_lang_Class::as_klassOop(mirror);
      NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
      result = Klass::cast(k)->class_status();
    }
    *statusPtr = result;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassStatus: *status", *statusPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetSourceFileName, (jclass clazz, 
                                char **sourceNamePtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(sourceNamePtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    if (java_lang_Class::is_primitive(mirror)) {
      JVMDI_RETURN(JVMDI_ERROR_ABSENT_INFORMATION);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
    if (!Klass::cast(k)->oop_is_instance()) {
      JVMDI_RETURN(JVMDI_ERROR_ABSENT_INFORMATION);
    }
    symbolOop sfnOop = instanceKlass::cast(k)->source_file_name();
    NULL_CHECK(sfnOop, JVMDI_ERROR_ABSENT_INFORMATION);

    {
      ResourceMark rm(THREAD);
      const char* sfncp = (const char*) sfnOop->as_C_string();
      *sourceNamePtr = (char *) JvmdiInternal::jvmdiMalloc(strlen(sfncp)+1);
      strcpy(*sourceNamePtr, sfncp);
    }

    JVMDI_TRACE_ARG2("JvmdiInternal::GetSourceFileName: *sourceNamePtr", *sourceNamePtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetSourceDebugExtension, (jclass clazz, 
                                char **sourceDebugExtensionPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(sourceDebugExtensionPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    if (java_lang_Class::is_primitive(mirror)) {
      JVMDI_RETURN(JVMDI_ERROR_ABSENT_INFORMATION);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
    if (!Klass::cast(k)->oop_is_instance()) {
      JVMDI_RETURN(JVMDI_ERROR_ABSENT_INFORMATION);
    }
    symbolOop sdeOop = instanceKlass::cast(k)->source_debug_extension();
    NULL_CHECK(sdeOop, JVMDI_ERROR_ABSENT_INFORMATION);

    {
      ResourceMark rm(THREAD);
      const char* sdecp = (const char*) sdeOop->as_C_string();
      *sourceDebugExtensionPtr = (char *) JvmdiInternal::jvmdiMalloc(strlen(sdecp)+1);
      strcpy(*sourceDebugExtensionPtr, sdecp);
    }

    JVMDI_TRACE_ARG2("JvmdiInternal::GetSourceDebugExtension: *sourceDebugExtensionPtr", 
                     *sourceDebugExtensionPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassModifiers, (jclass clazz, 
                                jint *modifiersPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(modifiersPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  
  { 
    jint result = 0;
    if (!java_lang_Class::is_primitive(mirror)) {
      klassOop k = java_lang_Class::as_klassOop(mirror);
      NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
      assert((Klass::cast(k)->oop_is_instance() || Klass::cast(k)->oop_is_array()), "should be an instance or an array klass");
      result = Klass::cast(k)->compute_modifier_flags(JVMDI_CHECK);

      // Reset the deleted  ACC_SUPER bit ( deleted in compute_modifier_flags()).
      if(Klass::cast(k)->is_super()) {
	result |= JVM_ACC_SUPER;
      }

    } else {
      result = (JVM_ACC_ABSTRACT | JVM_ACC_FINAL | JVM_ACC_PUBLIC);
    }
    *modifiersPtr = result;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassModifiers: *modifiersPtr", *modifiersPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassMethods, (jclass clazz, 
                              jint* methodCountPtr, 
                              jmethodID** methodsPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(methodCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(methodsPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  
  { 
    if (java_lang_Class::is_primitive(mirror)) {
      *methodCountPtr = 0;
      *methodsPtr = (jmethodID*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jmethodID));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

    // Return CLASS_NOT_PREPARED error as per JVMDI spec.
    if(!(Klass::cast(k)->class_status() & JVMDI_CLASS_STATUS_PREPARED ))
	JVMDI_RETURN( JVMDI_ERROR_CLASS_NOT_PREPARED );

    if (!Klass::cast(k)->oop_is_instance()) {
      *methodCountPtr = 0;
      *methodsPtr = (jmethodID*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jmethodID));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    instanceKlassHandle instanceK_h (thread, k);
    // Allocate the result and fill it in
    int result_length = instanceK_h->methods()->length();
    jmethodID* result_list = (jmethodID*)JvmdiInternal::jvmdiMalloc(result_length * sizeof(jmethodID));
    int index;
    // Use the original method ordering indices stored in the class, so we can emit
    // jmethodIDs in the order they appeared in the class file
    for (index = 0; index < result_length; index++) {
      methodOop m = methodOop(instanceK_h->methods()->obj_at(index));
      int original_index = instanceK_h->method_ordering()->int_at(index);
      assert(original_index >= 0 && original_index < result_length, "invalid original method index");
      jmethodID id = (jmethodID) m->jni_id();
      result_list[original_index] = id;
    }
    // Fill in return value.
    *methodCountPtr = result_length;
    *methodsPtr = result_list;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassMethods: *methodCountPtr", *methodCountPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassFields, (jclass clazz, 
                             jint *fieldCountPtr, 
                             jfieldID **fieldsPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(fieldCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(fieldsPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  
  { 
    if (java_lang_Class::is_primitive(mirror)) {
      *fieldCountPtr = 0;
      *fieldsPtr = (jfieldID*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jfieldID));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  
    // Return CLASS_NOT_PREPARED error as per JVMDI spec. 
    if(!(Klass::cast(k)->class_status() & JVMDI_CLASS_STATUS_PREPARED ))
	JVMDI_RETURN( JVMDI_ERROR_CLASS_NOT_PREPARED );

    if (!Klass::cast(k)->oop_is_instance()) {
      *fieldCountPtr = 0;
      *fieldsPtr = (jfieldID*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jfieldID));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }

    // We want to filter out java.lang.Throwable.backtrace (see 4446677).
    // It contains some methodOops that aren't quite real Objects.  Calling certain
    // methods on these pseudo Objects can cause segvs.  
    // This code assumes that offsets are never negative.
    //
    int backtraceOffset = k == SystemDictionary::throwable_klass()? 
                                java_lang_Throwable::get_backtrace_offset(): -1;

    instanceKlassHandle instanceK_h (thread, k);
    int result_count = 0;
    // First, count the fields.  Ignore java.lang.Throwable.backtrace.
    for (FieldStream count_st(instanceK_h, true, true); !count_st.eos(); count_st.next()) {
      if (count_st.offset() != backtraceOffset) {
        result_count += 1;
      }
    }
    // Allocate the result and fill it in
    jfieldID* result_list = (jfieldID*) JvmdiInternal::jvmdiMalloc(result_count * sizeof(jfieldID));
    // jvmdi seems to want the reverse order of what FieldStream hands out.
    int id_index = (result_count - 1);


    for (FieldStream src_st(instanceK_h, true, true); !src_st.eos(); src_st.next()) {
      int thisOffset = src_st.offset();
      if (thisOffset != backtraceOffset) {
        result_list[id_index--] = jfieldIDWorkaround::to_jfieldID(
                                             instanceK_h, thisOffset,
                                             src_st.access_flags().is_static());
      }
    }
    assert(id_index == -1, "just checking");
    // Fill in the results
    *fieldCountPtr = result_count;
    *fieldsPtr = result_list;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassFields: *fieldCountPtr", *fieldCountPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetImplementedInterfaces, (jclass clazz, 
                                       jint *interfaceCountPtr, 
                                       jclass **interfacesPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(interfaceCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(interfacesPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  
  {
    if (java_lang_Class::is_primitive(mirror)) {
      *interfaceCountPtr = 0;
      *interfacesPtr = (jclass*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jclass));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

    // Return CLASS_NOT_PREPARED error as per JVMDI spec. 
    if(!(Klass::cast(k)->class_status() & JVMDI_CLASS_STATUS_PREPARED ))
	JVMDI_RETURN( JVMDI_ERROR_CLASS_NOT_PREPARED );

    if (!Klass::cast(k)->oop_is_instance()) {
      *interfaceCountPtr = 0;
      *interfacesPtr = (jclass*) JvmdiInternal::jvmdiMalloc(0 * sizeof(jclass));
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    objArrayHandle interface_list (thread, instanceKlass::cast(k)->local_interfaces());
    const int result_length = (interface_list.is_null() ? 0 : interface_list->length());
    jclass* result_list = (jclass*) JvmdiInternal::jvmdiMalloc(result_length * sizeof(jclass));
    for (int i_index = 0; i_index < result_length; i_index += 1) {
      oop oop_at = interface_list->obj_at(i_index);
      assert(oop_at->is_klass(), "interfaces must be klassOops");
      klassOop klassOop_at = klassOop(oop_at);      // ???: is there a better way?
      assert(Klass::cast(klassOop_at)->is_interface(), "interfaces must be interfaces");
      oop mirror_at = Klass::cast(klassOop_at)->java_mirror();
      Handle handle_at = Handle(thread, mirror_at);
      result_list[i_index] = (jclass) JNIHandles::make_global(handle_at);
    }
    *interfaceCountPtr = result_length;
    *interfacesPtr = result_list;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetImplementedInterfaces: *interfaceCountPtr", *interfaceCountPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            IsInterface, (jclass clazz, 
                          jboolean *isInterfacePtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(isInterfacePtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    bool result = false;
    if (!java_lang_Class::is_primitive(mirror)) {
      klassOop k = java_lang_Class::as_klassOop(mirror);
      if (k != NULL && Klass::cast(k)->is_interface()) {
        result = true;
      }
    }
    *isInterfacePtr = result; 
    JVMDI_TRACE_ARG2("JvmdiInternal::IsInterface: *isInterfacePtr", *isInterfacePtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            IsArrayClass, (jclass clazz, 
                           jboolean *isArrayClassPtr))
  //Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(isArrayClassPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  
  {
    bool result = false;
    if (!java_lang_Class::is_primitive(mirror)) {
      klassOop k = java_lang_Class::as_klassOop(mirror);
      if (k != NULL && Klass::cast(k)->oop_is_array()) {
        result = true;
      }
    }
    *isArrayClassPtr = result;
    JVMDI_TRACE_ARG2("JvmdiInternal::IsArrayClass: *isArrayClassPtr", *isArrayClassPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassLoader, (jclass clazz, 
                             jobject *classloaderPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(classloaderPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }

  {
    if (java_lang_Class::is_primitive(mirror)) {
      *classloaderPtr = (jclass) JNIHandles::make_global(NULL);
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    klassOop k = java_lang_Class::as_klassOop(mirror);
    NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
    if (Klass::cast(k)->oop_is_instance()) {
      oop result_oop = instanceKlass::cast(k)->class_loader();
      if (result_oop == NULL) {
        *classloaderPtr = (jclass) JNIHandles::make_global(NULL);
        JVMDI_RETURN(JVMDI_ERROR_NONE);
      }
      Handle result_handle = Handle(thread, result_oop);
      jclass result_jnihandle = (jclass) JNIHandles::make_global(result_handle);
      *classloaderPtr = result_jnihandle;
    } else if (Klass::cast(k)->oop_is_objArray()) {
      klassOop bk = objArrayKlass::cast(k)->bottom_klass();
      oop result_oop = Klass::cast(bk)->oop_is_instance() ? instanceKlass::cast(bk)->class_loader() : NULL;
      if (result_oop == NULL) {
        *classloaderPtr = (jclass) JNIHandles::make_global(NULL);
        JVMDI_RETURN(JVMDI_ERROR_NONE);
      }
      Handle result_handle = Handle(thread, result_oop);
      jclass result_jnihandle = (jclass) JNIHandles::make_global(result_handle);
      *classloaderPtr = result_jnihandle;
    } else {
      *classloaderPtr = (jclass) JNIHandles::make_global(NULL);
    }
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


//
// Object Information
//

JVMDI_ENTER_NO_TRACE(jvmdiError,
            GetObjectHashCode, (jobject object, 
                                jint *hashCodePtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(object);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_OBJECT);
  NULL_CHECK(hashCodePtr, JVMDI_ERROR_NULL_POINTER);

  {
    jint result = (jint) mirror->identity_hash();
    *hashCodePtr = result;
    //JVMDI_TRACE_ARG2("JvmdiInternal::GetObjectHashCode: *hashCodePtr", *hashCodePtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


// The GetMonitorInfo method name caused problems with Win64 port.
JVMDI_ENTER(jvmdiError,
            hsGetMonitorInfo, (jobject object, 
                             JVMDI_monitor_info *infoPtr))
  HandleMark hm;
  Handle hobj;

  // Check arguments
  {
    oop mirror = JNIHandles::resolve_external_guard(object);
    NULL_CHECK(mirror, JVMDI_ERROR_INVALID_OBJECT);
    NULL_CHECK(infoPtr, JVMDI_ERROR_NULL_POINTER);

    hobj = Handle(mirror);
  }

  JavaThread *jt = NULL;
  ObjectMonitor *mon = NULL;
  JVMDI_monitor_info ret = {
    NULL, 0, 0, NULL
  };

  // first derive the object's owner and entry_count (if any)
  {
    address owner = NULL;
    {
      markOop mark = hobj()->mark();

      if (!mark->has_monitor()) {
        // this object has a lightweight monitor

        if (mark->has_locker()) {
          owner = (address)mark->locker(); // save the address of the Lock word
        }
        // implied else: no owner
      } else {
        // this object has a heavyweight monitor
        mon = mark->monitor();

        // The owner field of a heavyweight monitor may be NULL for no
        // owner, a JavaThread * or it may still be the address of the
        // Lock word in a JavaThread's stack. A monitor can be inflated
        // by a non-owning JavaThread, but only the owning JavaThread
        // can change the owner field from the Lock word to the
        // JavaThread * and it may not have done that yet.
        owner = (address)mon->owner();
      }
    }

    if (owner != NULL) {
      // This monitor is owned so we have to find the owning JavaThread.
      // Since owning_thread_from_monitor_owner() grabs a lock, GC can
      // move our object at this point. However, our owner value is safe
      // since it is either the Lock word on a stack or a JavaThread *.
      jt = Threads::owning_thread_from_monitor_owner(owner,
        true /* need locking */);
      assert(jt != NULL, "sanity check");
      if (jt != NULL) {  // robustness
        // The monitor's owner either has to be the current thread or
        // it has to be suspended. Either condition will prevent both
        // contending and waiting threads from modifying the state of
        // the monitor.
        if (!JvmdiInternal::is_thread_fully_suspended(jt, true)) {
          JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
        }
        HandleMark hm;
        Handle     th(jt->threadObj());
        ret.owner = (jthread)JNIHandles::make_global(th);
      }
      // implied else: no owner
    }

    if (jt != NULL) {  // monitor is owned
      if ((address)jt == owner) {
        // the owner field is the JavaThread *
        assert(mon != NULL,
          "must have heavyweight monitor with JavaThread * owner");
        ret.entry_count = mon->recursions() + 1;
      } else {
        // The owner field is the Lock word on the JavaThread's stack
        // so the recursions field is not valid. We have to count the
        // number of recursive monitor entries the hard way. We pass
        // a handle to survive any GCs along the way.
        ResourceMark rm;
        ret.entry_count = jt->count_lock_entries(hobj);
      }
    }
    // implied else: entry_count == 0
  }

  // now derive the rest of the fields
  if (mon != NULL) {
    // this object has a heavyweight monitor

    int nWant = mon->contentions(); // # of threads contending for monitor
    int nWait = mon->waiters();     // # of threads in Object.wait()
    ret.waiter_count = nWant + nWait;

    if (ret.waiter_count > 0) {
      // we have contending and/or waiting threads
      HandleMark hm;
      ret.waiters = (jthread *)JvmdiInternal::jvmdiMalloc(ret.waiter_count
        * sizeof(jthread *));
      if (nWant > 0) {
        // we have contending threads
        ResourceMark rm;
        GrowableArray<JavaThread*>* wantList = Threads::get_pending_threads(
          nWant, (address)mon, true /* need locking */);
        if (wantList->length() < nWant) {
          // robustness: the pending list has gotten smaller
          nWant = wantList->length();
        }
        for (int i = 0; i < nWant; i++) {
          JavaThread *wjt = wantList->at(i);
          // If the monitor has no owner, then a non-suspended contending
          // thread could potentially change the state of the monitor by
          // entering it. The JVM/DI spec doesn't allow this.
          if (jt == NULL &&
          !JvmdiInternal::is_thread_fully_suspended(wjt, true)) {
            if (ret.owner != NULL) {
              JNIHandles::destroy_global(ret.owner);
            }
            for (int j = 0; j < i; j++) {
              JNIHandles::destroy_global(ret.waiters[j]);
            }
            JvmdiInternal::jvmdiFree((jbyte *)ret.waiters);
            JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
          }
          Handle th(wjt->threadObj());
          ret.waiters[i] = (jthread)JNIHandles::make_global(th);
        }
      }
      if (nWait > 0) {
        // we have threads in Object.wait()
        int offset = nWant;  // add after any contending threads
        ObjectWaiter *waiter = mon->first_waiter();
        for (int i = 0; i < nWait; i++) {
          if (waiter == NULL) {
            // robustness: the waiting list has gotten smaller
            nWait = i;
            break;
          }
          Thread *t = mon->thread_of_waiter(waiter);
          if (t->is_Java_thread()) {
            JavaThread *wjt = (JavaThread *)t;
            // If the thread was found on the ObjectWaiter list, then
            // it has not been notified. This thread can't change the
            // state of the monitor so it doesn't need to be suspended.
            Handle th(wjt->threadObj());
            ret.waiters[offset + i] = (jthread)JNIHandles::make_global(th);
          }
          waiter = mon->next_waiter(waiter);
        }
      }

      if (nWant + nWait < ret.waiter_count) {
        // robustness: nWant or nWait got smaller so NULL out extra memory
        for (int i = nWant + nWait; i < ret.waiter_count; i++) {
          ret.waiters[i] = NULL;
        }
        ret.waiter_count = nWant + nWait;  // adjust the returned count smaller
        if (ret.waiter_count == 0) {
          JvmdiInternal::jvmdiFree((jbyte *)ret.waiters);
          ret.waiters = NULL;
        }
      }
    }
  }
  // implied else:
  // This is tricky. ObjectMonitor::wait() causes a lightweight
  // monitor to be inflated so we wouldn't be in this implied else
  // if any thread was waiting on this monitor. So we go with the
  // default waiter_count == 0 and empty waiters array.

  // we don't update return parameter unless everything worked
  *infoPtr = ret;

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


//
// Field Information
//

bool JvmdiInternal::get_field_descriptor(klassOop k, jfieldID field, fieldDescriptor* fd) {
  bool found = false;
  if (jfieldIDWorkaround::is_static_jfieldID(field)) {
    JNIid* id = jfieldIDWorkaround::from_static_jfieldID(field);
    int offset = id->offset();
    klassOop holder = id->holder();
    found = instanceKlass::cast(holder)->jni_find_local_field_from_offset(offset, true, fd);
  } else {
    // Non-static field. The fieldID is really the offset of the field within the object.
    int offset = jfieldIDWorkaround::from_instance_jfieldID(k, field);
    found = instanceKlass::cast(k)->jni_find_field_from_offset(offset, false, fd);
  }
  return found;
}


JVMDI_ENTER(jvmdiError,
            GetFieldName, (jclass clazz, 
                           jfieldID field, 
                           char **namePtr, 
                           char **signaturePtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(field, JVMDI_ERROR_INVALID_FIELDID);
  NULL_CHECK(namePtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(signaturePtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  if (java_lang_Class::is_primitive(mirror)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  klassOop k = java_lang_Class::as_klassOop(mirror);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

  {
    ResourceMark rm(THREAD);
    fieldDescriptor fd;
    bool found = JvmdiInternal::get_field_descriptor(k, field, &fd);
    if (!found) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_FIELDID);
    }
    const char* fieldName = fd.name()->as_C_string();
    const char* fieldSignature = fd.signature()->as_C_string();
    char* resultName = (char*) JvmdiInternal::jvmdiMalloc(strlen(fieldName) + 1);
    char* resultSignature = (char*) JvmdiInternal::jvmdiMalloc(strlen(fieldSignature) + 1);
    strcpy(resultName, fieldName);
    strcpy(resultSignature, fieldSignature);
    *namePtr = resultName;
    *signaturePtr = resultSignature;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetFieldName: *namePtr", *namePtr);
    JVMDI_TRACE_ARG2("JvmdiInternal::GetFieldName: *signaturePtr", *signaturePtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetFieldDeclaringClass, (jclass clazz, 
                                     jfieldID field, 
                                     jclass *declaringClassPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(field, JVMDI_ERROR_INVALID_FIELDID);
  NULL_CHECK(declaringClassPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  if (java_lang_Class::is_primitive(mirror)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  klassOop k = java_lang_Class::as_klassOop(mirror);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

  {
    fieldDescriptor fd;
    bool found = JvmdiInternal::get_field_descriptor(k, field, &fd);
    if (!found) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_FIELDID);
    }
    *declaringClassPtr = JvmdiInternal::get_jni_class_non_null(fd.field_holder());
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetFieldModifiers, (jclass clazz, 
                                jfieldID field, 
                                jint *modifiersPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(field, JVMDI_ERROR_INVALID_FIELDID);
  NULL_CHECK(modifiersPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  if (java_lang_Class::is_primitive(mirror)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  klassOop k = java_lang_Class::as_klassOop(mirror);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

  {
    fieldDescriptor fd;
    bool found = JvmdiInternal::get_field_descriptor(k, field, &fd);
    if (!found) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_FIELDID);
    }
    AccessFlags resultFlags = fd.access_flags();
    jint result = resultFlags.as_int();
    *modifiersPtr = result;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetFieldModifiers: *modifiersPtr", *modifiersPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            IsFieldSynthetic, (jclass clazz, 
                               jfieldID field, 
                               jboolean *isSyntheticPtr))
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  NULL_CHECK(mirror, JVMDI_ERROR_INVALID_CLASS);
  NULL_CHECK(field, JVMDI_ERROR_INVALID_FIELDID);
  NULL_CHECK(isSyntheticPtr, JVMDI_ERROR_NULL_POINTER);
  if (mirror->klass() != SystemDictionary::class_klass()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  if (java_lang_Class::is_primitive(mirror)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
  }
  klassOop k = java_lang_Class::as_klassOop(mirror);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);

  {
    fieldDescriptor fd;
    bool found = JvmdiInternal::get_field_descriptor(k, field, &fd);
    if (!found) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_FIELDID);
    }
    *isSyntheticPtr = fd.is_synthetic();
    JVMDI_TRACE_ARG2("JvmdiInternal::isFieldSynthetic: *isSyntheticPtr", *isSyntheticPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


//
// Miscellaneous Functions
//

// The closure for GetLoadedClasses and GetClassLoaderClasses
class JvmdiGetLoadedClassesClosure : public StackObj {
  // Since the SystemDictionary::classes_do callback 
  // doesn't pass a closureData pointer, 
  // we use a thread-local slot to hold a pointer to 
  // a stack allocated instance of this structure.
 private:
  jobject _initiatingLoader;
  int     _count;
  Handle* _list;
  int     _index;

 private:
  // Getting and setting the thread local pointer
  static JvmdiGetLoadedClassesClosure* get_this() {
    JvmdiGetLoadedClassesClosure* result = NULL;
    JavaThread* thread = JavaThread::current();
    result = thread->get_jvmdi_get_loaded_classes_closure();
    return result;
  }
  static void set_this(JvmdiGetLoadedClassesClosure* that) {
    JavaThread* thread = JavaThread::current();
    thread->set_jvmdi_get_loaded_classes_closure(that);
  }

 public:
  // Constructor/Destructor
  JvmdiGetLoadedClassesClosure() {
    JvmdiGetLoadedClassesClosure* that = get_this();
    assert(that == NULL, "JvmdiGetLoadedClassesClosure in use");
    _initiatingLoader = NULL;
    _count = 0;
    _list = NULL;
    _index = 0;
    set_this(this);
  }

  JvmdiGetLoadedClassesClosure(jobject initiatingLoader) {
    JvmdiGetLoadedClassesClosure* that = get_this();
    assert(that == NULL, "JvmdiGetLoadedClassesClosure in use");
    _initiatingLoader = initiatingLoader;
    _count = 0;
    _list = NULL;
    _index = 0;
    set_this(this);
  }

  ~JvmdiGetLoadedClassesClosure() {
    JvmdiGetLoadedClassesClosure* that = get_this();
    assert(that != NULL, "JvmdiGetLoadedClassesClosure not found");
    set_this(NULL);
    _initiatingLoader = NULL;
    _count = 0;
    if (_list != NULL) {
      FreeHeap(_list);
      _list = NULL;
    }
    _index = 0;
  }

  // Accessors.
  jobject get_initiatingLoader() {
    return _initiatingLoader;
  }

  int get_count() {
    return _count;
  }

  void set_count(int value) {
    _count = value;
  }

  Handle* get_list() {
    return _list;
  }

  void set_list(Handle* value) {
    _list = value;
  }

  int get_index() {
    return _index;
  }

  void set_index(int value) {
    _index = value;
  }

  Handle get_element(int index) {
    Handle result = Handle();
    if ((_list != NULL) && (index < _count)) {
      result = _list[index];
    }
    return result;
  }

  void set_element(int index, Handle value) {
    if ((_list != NULL) && (index < _count)) {
      _list[index] = value;
    }
  }

  // Other predicates
  bool available() {
    return (_list != NULL);
  }

#ifdef ASSERT
  // For debugging.
  void check(int limit) {
    for (int i = 0; i < limit; i += 1) {
      assert(Universe::heap()->is_in(get_element(i)()), "check fails");
    }
  }
#endif

  // Public methods that get called within the scope of the closure
  void allocate() {
    _list = NEW_C_HEAP_ARRAY(Handle, _count);
    assert(_list != NULL, "Out of memory");
    if (_list == NULL) {
      _count = 0;
    }
  }

  void extract(jclass* result) {
    for (int index = 0; index < _count; index += 1) {
      result[index] = (jclass) JNIHandles::make_global(get_element(index)());
    }
  }

  // Finally, the static methods that are the callbacks
  static void increment(klassOop k) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (that->get_initiatingLoader() == NULL) {
      for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
        that->set_count(that->get_count() + 1);
      }
    } else if (k != NULL) {
      // if initiating loader not null, just include the instance with 1 dimension
      that->set_count(that->get_count() + 1);
    }
  }

  static void increment_with_loader(klassOop k, oop loader) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
      for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
        that->set_count(that->get_count() + 1);
      }
    }
  }

  static void prim_array_increment_with_loader(klassOop array, oop loader) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
      that->set_count(that->get_count() + 1);
    }
  }

  static void add(klassOop k) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (that->get_initiatingLoader() == NULL) {
        for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
          oop mirror = Klass::cast(l)->java_mirror();
          that->set_element(that->get_index(), mirror);
          that->set_index(that->get_index() + 1);
        }
      } else if (k != NULL) {
        // if initiating loader not null, just include the instance with 1 dimension
        oop mirror = Klass::cast(k)->java_mirror();
        that->set_element(that->get_index(), mirror);
        that->set_index(that->get_index() + 1);
      }        
    }
  }

  static void add_with_loader(klassOop k, oop loader) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
        for (klassOop l = k; l != NULL; l = Klass::cast(l)->array_klass_or_null()) {
          oop mirror = Klass::cast(l)->java_mirror();
          that->set_element(that->get_index(), mirror);
          that->set_index(that->get_index() + 1);
        }
      }
    }
  }

  static void prim_array_add_with_loader(klassOop array, oop loader) {
    JvmdiGetLoadedClassesClosure* that = JvmdiGetLoadedClassesClosure::get_this();
    if (that->available()) {
      if (loader == JNIHandles::resolve(that->get_initiatingLoader())) {
        oop mirror = Klass::cast(array)->java_mirror();
        that->set_element(that->get_index(), mirror);
        that->set_index(that->get_index() + 1);
      }
    }
  }

};


JVMDI_ENTER(jvmdiError,
            GetLoadedClasses, (jint *classCountPtr, 
                               jclass **classesPtr))
  // Check arguments
  NULL_CHECK(classCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(classesPtr, JVMDI_ERROR_NULL_POINTER);
  
  {
    // Since SystemDictionary::classes_do only takes a function pointer 
    // and doesn't call back with a closure data pointer, 
    // we can only pass static methods.
    JvmdiGetLoadedClassesClosure closure;
    {
      // For consistency of the loaded classes, grab the SystemDictionary lock
      MutexLocker sd_mutex(SystemDictionary_lock);
      // First, count the classes
      SystemDictionary::classes_do(&JvmdiGetLoadedClassesClosure::increment);
      Universe::basic_type_classes_do(&JvmdiGetLoadedClassesClosure::increment);
      // Next, fill in the classes
      closure.allocate();
      SystemDictionary::classes_do(&JvmdiGetLoadedClassesClosure::add);
      Universe::basic_type_classes_do(&JvmdiGetLoadedClassesClosure::add);
      // Drop the SystemDictionary_lock, so the results could be wrong from here,
      // but we still have a snapshot.
    }
    // Post results
    jclass* result_list = (jclass*)JvmdiInternal::jvmdiMalloc(closure.get_count() * sizeof(jclass));
    closure.extract(result_list);
    *classCountPtr = closure.get_count();
    *classesPtr = result_list;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetLoadedClasses: *classCountPtr", *classCountPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetClassLoaderClasses, (jobject initiatingLoader, 
                                    jint *classesCountPtr, 
                                    jclass **classesPtr))
  // Check arguments
  // if (jobject == NULL) {
  //   we want classes loaded by the system class loader
  // }
  NULL_CHECK(classesCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(classesPtr, JVMDI_ERROR_NULL_POINTER);

  {
    // Since SystemDictionary::classes_do only takes a function pointer 
    // and doesn't call back with a closure data pointer, 
    // we can only pass static methods.
    JvmdiGetLoadedClassesClosure closure(initiatingLoader);
    {
      // For consistency of the loaded classes, grab the SystemDictionary lock
      MutexLocker sd_mutex(SystemDictionary_lock);
      // First, count the classes
      // a. SystemDictionary::classes_do doesn't include arrays of primitive types (any dimensions)
      SystemDictionary::classes_do(&JvmdiGetLoadedClassesClosure::increment_with_loader);
      // b. Count arrays of primitive types
      SystemDictionary::prim_array_classes_do(&JvmdiGetLoadedClassesClosure::prim_array_increment_with_loader);
      // c. single-dimensional array of primitive types created by newarray instruction
      //    do not require resolution; thus they are not added in system dictionary.
      //    For simplicity, add single-dimensional array of all primitive types.
      Universe::basic_type_classes_do(&JvmdiGetLoadedClassesClosure::increment);
      // Next, fill in the classes
      closure.allocate();
      SystemDictionary::classes_do(&JvmdiGetLoadedClassesClosure::add_with_loader);
      SystemDictionary::prim_array_classes_do(&JvmdiGetLoadedClassesClosure::prim_array_add_with_loader);
      Universe::basic_type_classes_do(&JvmdiGetLoadedClassesClosure::add);
      // Drop the SystemDictionary_lock, so the results could be wrong from here,
      // but we still have a snapshot.
    }
    // Post results
    jclass* result_list = (jclass*)JvmdiInternal::jvmdiMalloc(closure.get_count() * sizeof(jclass));
    closure.extract(result_list);
    *classesCountPtr = closure.get_count();
    *classesPtr = result_list;
    JVMDI_TRACE_ARG2("JvmdiInternal::GetClassLoaderClasses: *classesCountPtr", *classesCountPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetVersionNumber, (jint *versionPtr))
  NULL_CHECK(versionPtr, JVMDI_ERROR_NULL_POINTER);

  {
    if (UseSuspendResumeThreadLists) {
      *versionPtr = JVMDI_VERSION_1_3;
    } else {
      *versionPtr = JVMDI_VERSION_1_2;
    }
    JVMDI_TRACE_ARG2("JvmdiInternal::GetVersionNumber: *versionPtr", *versionPtr);
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


JVMDI_ENTER(jvmdiError,
            GetCapabilities, (JVMDI_capabilities *capabilitiesPtr))
  // Check argument
  NULL_CHECK(capabilitiesPtr, JVMDI_ERROR_NULL_POINTER);
  {
    JVMDI_capabilities result;
    memset(&result, 0, sizeof(result)); // unused values need to be false

    result.can_watch_field_modification      = JNI_TRUE;
    result.can_watch_field_access            = JNI_TRUE;
    result.can_get_bytecodes                 = JNI_TRUE;
    result.can_get_synthetic_attribute       = JNI_TRUE;
    result.can_get_owned_monitor_info        = JNI_TRUE;
    result.can_get_current_contended_monitor = JNI_TRUE;
    result.can_get_monitor_info              = JNI_TRUE;
    result.can_get_heap_info                 = JNI_FALSE;
    result.can_get_operand_stack             = JNI_FALSE;
    result.can_set_operand_stack             = JNI_FALSE;
#ifdef HOTSWAP
    result.can_pop_frame                     = JNI_TRUE;
#else
    result.can_pop_frame                     = JNI_FALSE;
#endif HOTSWAP
    result.can_get_class_definition          = JNI_FALSE;
#ifdef HOTSWAP
    result.can_redefine_classes              = JNI_TRUE;
#else
    result.can_redefine_classes              = JNI_FALSE;
#endif HOTSWAP
    result.can_add_method                    = JNI_FALSE;
    result.can_unrestrictedly_redefine_classes = JNI_FALSE;
    if (UseSuspendResumeThreadLists) {
      result.can_suspend_resume_thread_lists = JNI_TRUE;
    } else {
      result.can_suspend_resume_thread_lists = JNI_FALSE;
    }
    *capabilitiesPtr = result;
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
JVMDI_END


void JvmdiInternal::set_info_hooks(JVMDI_Interface_1 *jvmdi_interface) {
  jvmdi_interface->GetClassSignature = GetClassSignature;
  jvmdi_interface->GetClassStatus = GetClassStatus;
  jvmdi_interface->GetSourceFileName = GetSourceFileName;
  jvmdi_interface->GetClassModifiers = GetClassModifiers;
  jvmdi_interface->GetClassMethods = GetClassMethods;
  jvmdi_interface->GetClassFields = GetClassFields;
  jvmdi_interface->GetImplementedInterfaces = GetImplementedInterfaces;
  jvmdi_interface->IsInterface = IsInterface;
  jvmdi_interface->IsArrayClass = IsArrayClass;
  jvmdi_interface->GetClassLoader = GetClassLoader;
  jvmdi_interface->GetObjectHashCode = GetObjectHashCode;
  jvmdi_interface->GetMonitorInfo = hsGetMonitorInfo;
  jvmdi_interface->GetFieldName = GetFieldName;
  jvmdi_interface->GetFieldDeclaringClass = GetFieldDeclaringClass;
  jvmdi_interface->GetFieldModifiers = GetFieldModifiers;
  jvmdi_interface->IsFieldSynthetic = IsFieldSynthetic;
  jvmdi_interface->GetLoadedClasses = GetLoadedClasses;
  jvmdi_interface->GetClassLoaderClasses = GetClassLoaderClasses;
  jvmdi_interface->GetVersionNumber = GetVersionNumber;
  jvmdi_interface->GetCapabilities = GetCapabilities;
  jvmdi_interface->GetSourceDebugExtension = GetSourceDebugExtension;
}
