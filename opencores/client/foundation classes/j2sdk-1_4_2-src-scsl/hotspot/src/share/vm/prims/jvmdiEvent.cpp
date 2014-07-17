#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdiEvent.cpp	1.18 03/01/23 12:20:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdiEvent.cpp.incl"


//
// JvmdiEvent
//

JvmdiEvent::JvmdiEvent(int kind)
: _kind(kind) {
}

void JvmdiEvent::write_kind(outputStream& out) {
  write_kind(out,get_kind());
}

void JvmdiEvent::write_kind(outputStream& out, int kind) {
#ifdef ASSERT
 switch (kind) {
  case JVMDI_EVENT_SINGLE_STEP:        out.print("single step"); break;
  case JVMDI_EVENT_BREAKPOINT:         out.print("breakpoint"); break;
  case JVMDI_EVENT_FRAME_POP:          out.print("frame pop"); break;
  case JVMDI_EVENT_EXCEPTION:          out.print("exception"); break;
  case JVMDI_EVENT_USER_DEFINED:       out.print("user"); break;
  case JVMDI_EVENT_THREAD_START:       out.print("thread start"); break;
  case JVMDI_EVENT_THREAD_END:         out.print("thread end"); break;
  case JVMDI_EVENT_CLASS_PREPARE:      out.print("class prepare"); break;
  case JVMDI_EVENT_CLASS_UNLOAD:       out.print("class unload"); break;
  case JVMDI_EVENT_CLASS_LOAD:         out.print("class load"); break;
  case JVMDI_EVENT_FIELD_ACCESS:       out.print("field access"); break;
  case JVMDI_EVENT_FIELD_MODIFICATION: out.print("field modification"); break;
  case JVMDI_EVENT_EXCEPTION_CATCH:    out.print("exception catch"); break;
  case JVMDI_EVENT_METHOD_ENTRY:       out.print("method entry"); break;
  case JVMDI_EVENT_METHOD_EXIT:        out.print("method exit"); break;
  case JVMDI_EVENT_VM_INIT:            out.print("vm init"); break;
  case JVMDI_EVENT_VM_DEATH:           out.print("vm death"); break;

  default: 
    assert(kind <= JVMDI_MAX_EVENT_TYPE_VAL, "kind <= JVMDI_MAX_EVENT_TYPE_VAL");
    assert(false, "Unknown JvmdiEvent kind");
  }
#endif
}

void JvmdiEvent::write_sep(outputStream& out) {
#ifdef ASSERT
 out.print(",");
#endif
}
void JvmdiEvent::write_thread(outputStream& out, const Handle thread) {
#ifdef ASSERT
 const char *thread_name = JvmdiInternal::safe_get_thread_name(thread()); 
 out.print(thread_name);
#endif
}
void JvmdiEvent::write_klass(outputStream& out, const KlassHandle klass) {
#ifdef ASSERT
  const char *klass_name = klass->external_name();
  out.print(klass_name);
#endif
}
void JvmdiEvent::write_method(outputStream& out, const methodHandle method, const address location) {
#ifdef ASSERT
  char *method_name = method->name()->as_C_string();
  out.print(method_name);
#endif
}

void JvmdiEvent::write_object(outputStream& out, const Handle object) {
#ifdef ASSERT
 const char *object_name = instanceKlass::cast(object->klass())->name()->as_C_string();
 out.print(object_name);
#endif
}

void JvmdiEvent::write_field(outputStream& out, const jfieldID field) {
#ifdef ASSERT
  out.print("XXX - need to add support for field here");
#endif
}

void JvmdiEvent::write_signature_type(outputStream& out, const char sig_type) {
#ifdef ASSERT
  out.print("XXX - need to add support for signature type here");
#endif
}

void JvmdiEvent::write_value(outputStream& out, const jvalue *value) {
#ifdef ASSERT
  out.print("XXX - need to add support for jvalue here");
#endif
}

jobject JvmdiEvent::get_local_jni_object(JavaThread *current_thread, oop object) {
  if (object == NULL) { return NULL; }
  return (jobject) JNIHandles::make_local(current_thread, object);
}

jobject JvmdiEvent::get_local_jni_object_non_null(JavaThread *current_thread, oop object) {
  assert(object != NULL, "object != NULL");
  return (jobject) JNIHandles::make_local(current_thread, object);
}

jthread JvmdiEvent::get_local_jni_thread(JavaThread *current_thread, oop thread) {
  assert(!JvmdiInternal::is_jvmdi_initialized() || thread != NULL, "bad thread");
  return (jthread) JNIHandles::make_local(current_thread, thread);
}

jthread JvmdiEvent::get_local_jni_thread_non_null(JavaThread *current_thread, oop thread) {
  assert(thread != NULL, "thread != NULL");
  return get_local_jni_thread(current_thread,thread);
}

jclass JvmdiEvent::get_local_jni_class(JavaThread* current_thread, klassOop k) {
  if (k == NULL) {
    return NULL;
  } else {
    return get_local_jni_class_non_null(current_thread, k);
  }
}

jclass JvmdiEvent::get_local_jni_class_non_null(JavaThread* current_thread, klassOop k) {
  assert(k != NULL, "k != NULL");
  return (jclass) JNIHandles::make_local(current_thread, Klass::cast(k)->java_mirror() );
}

jmethodID JvmdiEvent::get_local_jni_method(JavaThread *current_thread, methodOop m) {
  if (m == NULL) {
    return NULL;
  } else {
    return get_local_jni_method_non_null(current_thread, m);
  }
}

jmethodID JvmdiEvent::get_local_jni_method_non_null(JavaThread *current_thread, methodOop m) {
  assert(m != NULL, "m != NULL");
  return (jmethodID) m->jni_id();
}


//
// JvmdiEventProxy
//
JvmdiEventProxy::JvmdiEventProxy(int kind)
: JvmdiEvent(kind) {
  assert(kind == JVMDI_EVENT_CLASS_UNLOAD, "Illegal JvmdiEventProxy kind");
}


//
// JvmdiEventSingleStep
//

JvmdiEventSingleStep::JvmdiEventSingleStep(Handle thread, KlassHandle klass, methodHandle method, address location) 
: JvmdiEvent(JVMDI_EVENT_SINGLE_STEP)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
{}

void JvmdiEventSingleStep::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location());  
}

void JvmdiEventSingleStep::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.single_step.thread   = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.single_step.clazz    = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.single_step.method   = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.single_step.location = get_location() - get_method()->code_base();
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventSingleStep::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.single_step.thread);
  JNIHandles::destroy_local(event.u.single_step.clazz);
}

//
// JvmdiEventClass
//

JvmdiEventClass::JvmdiEventClass(int kind, Handle thread, KlassHandle klass)
: JvmdiEvent(kind) 
, _thread (thread)
, _klass(klass) {
  assert(kind == JVMDI_EVENT_CLASS_LOAD    ||
	 kind == JVMDI_EVENT_CLASS_PREPARE, 
	 "Illegal JvmdiEventClass kind");
}

void JvmdiEventClass::write(outputStream& out) {  
  write_kind(out); 
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
}

void JvmdiEventClass::to_event(JVMDI_Event& event) { 
  event.kind = get_kind(); 

  JavaThread *current_thread = JavaThread::current();
  // from Universe::genesis we may get a Class Event whose threadobj
  // is not initialized (= NULL)
  event.u.class_event.thread   = get_local_jni_thread(current_thread, get_thread()());
  event.u.class_event.clazz    = get_local_jni_class_non_null (current_thread, get_klass()());
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventClass::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.class_event.thread);
  JNIHandles::destroy_local(event.u.class_event.clazz);
}


//
// JvmdiEventClassUnload
//

JvmdiEventClassUnload::JvmdiEventClassUnload(Handle thread, KlassHandle klass)
: JvmdiEventProxy(JVMDI_EVENT_CLASS_UNLOAD)
, _thread(thread)
, _klass(klass) {
}

void JvmdiEventClassUnload::write(outputStream& out) {  
  write_kind(out); 
  write_sep(out);
  write_thread(out, get_thread());
  write_sep(out);
  write_klass(out, get_klass());
}

void JvmdiEventClassUnload::to_event(JVMDI_Event& event, JavaThread *thread) { 
  // VMThread is proxying for the specified JavaThread
  assert(Thread::current()->is_VM_thread(), "wrong thread");
  event.kind = get_kind(); 

  // from Universe::genesis we may get a Class Event whose threadobj
  // is not initialized (= NULL)
  event.u.class_event.thread = get_local_jni_thread(thread, get_thread()());
  event.u.class_event.clazz  = get_local_jni_class_non_null(thread, get_klass()());
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventClassUnload::destroy_local_JNI_handles(JVMDI_Event& event) { 
  JNIHandles::destroy_local(event.u.class_event.thread);
  JNIHandles::destroy_local(event.u.class_event.clazz);
}
 


//
// JvmdiEventBreakpoint
//

JvmdiEventBreakpoint::JvmdiEventBreakpoint(Handle thread, KlassHandle klass, methodHandle method, address location) 
: JvmdiEvent(JVMDI_EVENT_BREAKPOINT)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
{}

void JvmdiEventBreakpoint::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location());  
}

void JvmdiEventBreakpoint::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.single_step.thread   = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.single_step.clazz    = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.single_step.method   = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.single_step.location = get_location() - get_method()->code_base();
}


// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventBreakpoint::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.single_step.thread);
  JNIHandles::destroy_local(event.u.single_step.clazz);
  //JNIHandles::destroy_local(event.u.single_step.method);
}


//
// JvmdiEventFieldAccess
//

JvmdiEventFieldAccess::JvmdiEventFieldAccess(Handle thread,
  KlassHandle klass, methodHandle method, address location,
  KlassHandle field_klass, Handle object, jfieldID field)
: JvmdiEvent(JVMDI_EVENT_FIELD_ACCESS)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
, _field_klass(field_klass)
, _object(object)
, _field(field)
{}

void JvmdiEventFieldAccess::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location()); 
  write_sep(out);
  write_klass(out, get_field_klass());
  write_sep(out);
  write_object(out, get_object());
  write_sep(out);
  write_field(out, get_field());
}


void JvmdiEventFieldAccess::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.field_access.thread      = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.field_access.clazz       = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.field_access.method      = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.field_access.location    = get_location() - get_method()->code_base();
  event.u.field_access.field_clazz = get_local_jni_class_non_null (current_thread, get_field_klass()());
  event.u.field_access.object      = get_local_jni_object (current_thread, get_object()());
  event.u.field_access.field       = get_field();
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventFieldAccess::destroy_local_JNI_handles(JVMDI_Event& event) {

  JNIHandles::destroy_local(event.u.field_access.thread);
  JNIHandles::destroy_local(event.u.field_access.clazz);
  JNIHandles::destroy_local(event.u.field_access.object);
  JNIHandles::destroy_local(event.u.field_access.field_clazz);

}

//
// JvmdiEventFieldModification
//

JvmdiEventFieldModification::JvmdiEventFieldModification(Handle thread,
  KlassHandle klass, methodHandle method, address location,
  KlassHandle field_klass, Handle object, jfieldID field,
  char signature_type, jvalue *new_value)
: JvmdiEvent(JVMDI_EVENT_FIELD_MODIFICATION)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
, _field_klass(field_klass)
, _object(object)
, _field(field)
, _signature_type(signature_type)
, _new_value(new_value)
{}


void JvmdiEventFieldModification::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location()); 
  write_sep(out);
  write_klass(out, get_field_klass());
  write_sep(out);
  write_object(out, get_object());
  write_sep(out);
  write_field(out, get_field());
  write_sep(out);
  write_signature_type(out, get_signature_type());
  write_sep(out);
  write_value(out, get_new_value());
}


void JvmdiEventFieldModification::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.field_modification.thread         = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.field_modification.clazz          = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.field_modification.method         = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.field_modification.location       = get_location() - get_method()->code_base();
  event.u.field_modification.field_clazz    = get_local_jni_class_non_null (current_thread, get_field_klass()());
  event.u.field_modification.object         = get_local_jni_object (current_thread, get_object()());
  event.u.field_modification.field          = get_field();
  event.u.field_modification.signature_type = get_signature_type();
  assert(get_new_value() != NULL, "new_value must be non-NULL");
  event.u.field_modification.new_value      = *get_new_value();
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventFieldModification::destroy_local_JNI_handles(JVMDI_Event& event) {

  JNIHandles::destroy_local(event.u.field_modification.thread);
  JNIHandles::destroy_local(event.u.field_modification.clazz);
  JNIHandles::destroy_local(event.u.field_modification.field_clazz);
  JNIHandles::destroy_local(event.u.field_modification.object);
}

//
// JvmdiEventFrame
//

JvmdiEventFrame::JvmdiEventFrame(int kind, Handle thread, KlassHandle klass, methodHandle method, jframeID frame_id) 
: JvmdiEvent(kind)
, _thread(thread)
, _klass(klass)
, _method(method)
, _frame_id(frame_id) {
  assert(kind == JVMDI_EVENT_FRAME_POP    ||
	 kind == JVMDI_EVENT_METHOD_ENTRY ||
	 kind == JVMDI_EVENT_METHOD_EXIT,
	 "Illegal JvmdiEventFrame kind");
}

void JvmdiEventFrame::write(outputStream& out) { 
  write_kind(out);
  write_sep(out);
  write_thread(out, get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out, get_method());
}

void JvmdiEventFrame::to_event(JVMDI_Event& event) { 
  JavaThread *current_thread = JavaThread::current();
  
  event.kind = get_kind();
  event.u.frame.thread   = get_local_jni_thread(current_thread, get_thread()());
  event.u.frame.clazz    = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.frame.method   = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.frame.frame    = get_jframeID();
}
  
// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventFrame::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.frame.thread);
  JNIHandles::destroy_local(event.u.frame.clazz);
}

//
// JvmdiEventException
//

JvmdiEventException::JvmdiEventException(Handle thread, 
					 KlassHandle klass, methodHandle method, address location,
					 Handle exception,
					 KlassHandle catch_klass, methodHandle catch_method, address catch_location) 
: JvmdiEvent(JVMDI_EVENT_EXCEPTION)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
, _exception(exception)
, _catch_klass(catch_klass)
, _catch_method(catch_method)
, _catch_location(catch_location)
{}

void JvmdiEventException::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location()); 
  write_sep(out);
  write_object(out, get_exception());
  write_sep(out);
  write_klass(out, get_catch_klass());
  write_sep(out);
  write_method(out,get_catch_method(), get_catch_location()); 
}

void JvmdiEventException::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.exception.thread         = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.exception.clazz          = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.exception.method         = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.exception.location       = get_location() - get_method()->code_base();
  event.u.exception.exception      = get_local_jni_object_non_null(current_thread, get_exception()());
  // catch_clazz and catch_method can be NULL
  event.u.exception.catch_clazz    = get_local_jni_class(current_thread, get_catch_klass()());
  event.u.exception.catch_method   = get_local_jni_method(current_thread, get_catch_method()());
  if (get_catch_method().is_null()) {
    event.u.exception.catch_location = (jlocation)0;
  } else {
    event.u.exception.catch_location = get_catch_location() - get_catch_method()->code_base();
  }
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventException::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.exception.thread);
  JNIHandles::destroy_local(event.u.exception.clazz);
  JNIHandles::destroy_local(event.u.exception.exception);
  JNIHandles::destroy_local(event.u.exception.catch_clazz);
}

//
// JvmdiEventExceptionCatch
//

JvmdiEventExceptionCatch::JvmdiEventExceptionCatch(Handle thread, KlassHandle klass, 
						   methodHandle method, address location, Handle exception) 
: JvmdiEvent(JVMDI_EVENT_EXCEPTION_CATCH)
, _thread (thread)
, _klass(klass)
, _method(method)
, _location(location)
, _exception(exception)
{}

void JvmdiEventExceptionCatch::write(outputStream& out)     { 
  write_kind(out);
  write_sep(out);
  write_thread(out,get_thread());
  write_sep(out);
  write_klass(out, get_klass());
  write_sep(out);
  write_method(out,get_method(), get_location()); 
  write_sep(out);
  write_object(out, get_exception());
}

void JvmdiEventExceptionCatch::to_event(JVMDI_Event& event) {
  JavaThread *current_thread = JavaThread::current();

  event.kind = get_kind();
  event.u.exception_catch.thread    = get_local_jni_thread_non_null(current_thread, get_thread()());
  event.u.exception_catch.clazz     = get_local_jni_class_non_null (current_thread, get_klass()());
  event.u.exception_catch.method     = get_local_jni_method_non_null(current_thread, get_method()());
  event.u.exception_catch.location  = get_location() - get_method()->code_base();
  event.u.exception_catch.exception = get_local_jni_object_non_null(current_thread, get_exception()());
  
}

// Delete JNIHandle references allocated in to_event() method.
void JvmdiEventExceptionCatch::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.exception_catch.thread);
  JNIHandles::destroy_local(event.u.exception_catch.clazz);
  JNIHandles::destroy_local(event.u.exception_catch.exception);
}

//
// JvmdiEventUser
//

JvmdiEventUser::JvmdiEventUser() 
: JvmdiEvent(JVMDI_EVENT_USER_DEFINED) {
}
void JvmdiEventUser::write(outputStream& out) { Unimplemented(); }
void JvmdiEventUser::to_event(JVMDI_Event& event) { Unimplemented(); }
void JvmdiEventUser::destroy_local_JNI_handles(JVMDI_Event& event) { Unimplemented(); }

//
// JvmdiEventThreadChange
//

JvmdiEventThreadChange::JvmdiEventThreadChange(int kind, Handle thread) 
: JvmdiEvent(kind)
, _thread (thread) {
  assert(kind == JVMDI_EVENT_THREAD_END ||
	 kind == JVMDI_EVENT_THREAD_START,
	 "Illegal JvmdiEventThreadChange kind");
}

void JvmdiEventThreadChange::write(outputStream& out) {
  write_kind(out);
  write_sep(out);
  write_thread(out, get_thread());
}

void JvmdiEventThreadChange::to_event(JVMDI_Event& event) { 
  event.kind = get_kind();
  
  JavaThread *current_thread = JavaThread::current();
  event.u.thread_change.thread = get_local_jni_thread_non_null(current_thread, get_thread()());
}

// Delete JNIHandle references allocated in to_event() method.
void  JvmdiEventThreadChange::destroy_local_JNI_handles(JVMDI_Event& event) {
  JNIHandles::destroy_local(event.u.thread_change.thread);
}


//
// JvmdiEventVm
//

JvmdiEventVm::JvmdiEventVm(int kind)
: JvmdiEvent(kind) {
  assert(kind == JVMDI_EVENT_VM_INIT ||
	 kind == JVMDI_EVENT_VM_DEATH,
	 "Illegal JvmdiEventVm kind")   
}

void JvmdiEventVm::write(outputStream& out)     {   
  write_kind(out); 
}
void JvmdiEventVm::to_event(JVMDI_Event& event) { 
  event.kind = get_kind(); 
}

void  JvmdiEventVm::destroy_local_JNI_handles(JVMDI_Event& event) {
	;
}
