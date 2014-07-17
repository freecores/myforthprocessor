#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdiEvent.hpp	1.24 03/01/23 12:20:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// wraps JVMDI_Event struct, provides debugging output
//

// Todo: Add appropriate ifdefs

class JvmdiEvent : public ResourceObj {
protected:
  const int _kind;
  JvmdiEvent(int kind);

  static jobject   get_local_jni_object         (JavaThread *current_thread, oop object);
  static jobject   get_local_jni_object_non_null(JavaThread *current_thread, oop object);
  static jthread   get_local_jni_thread         (JavaThread *current_thread, oop thread);
  static jthread   get_local_jni_thread_non_null(JavaThread *current_thread, oop thread);
  static jclass    get_local_jni_class          (JavaThread *current_thread, klassOop k);
  static jclass    get_local_jni_class_non_null (JavaThread *current_thread, klassOop k);
  static jmethodID get_local_jni_method         (JavaThread *current_thread, methodOop m);
  static jmethodID get_local_jni_method_non_null(JavaThread *current_thread, methodOop m);


  void write_sep(outputStream& out);  
  void write_kind(outputStream& out);  
  void write_thread(outputStream& out, const Handle thread);
  void write_klass(outputStream& out, const KlassHandle klass);
  void write_method(outputStream& out, const methodHandle method, const address location = NULL);
  void write_object(outputStream& out, const Handle object);
  void write_field(outputStream& out, const jfieldID field);
  void write_signature_type(outputStream& out, const char sig_type);
  void write_value(outputStream& out, const jvalue *value);


public:
  static void write_kind(outputStream& out, int kind);

  int get_kind() { return _kind; }
  virtual void write(outputStream& out)     =0; 
  virtual void to_event(JVMDI_Event& event) =0; 
  virtual void destroy_local_JNI_handles(JVMDI_Event& event) =0; 
};


class JvmdiEventProxy : public JvmdiEvent {

public:
  JvmdiEventProxy(int kind);

  void write(outputStream& out)                                 = 0;
  // proxy events must specify JavaThread with to_event()
  void to_event(JVMDI_Event& event) { Unimplemented(); }
  virtual void to_event(JVMDI_Event& event, JavaThread *thread) = 0;
  virtual void destroy_local_JNI_handles(JVMDI_Event& event)    = 0;
};


class JvmdiEventSingleStep : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;

public:
  JvmdiEventSingleStep(Handle thread, KlassHandle klass, methodHandle method, address location);

  const Handle         get_thread() const   { return _thread;   }
  const KlassHandle    get_klass() const    { return _klass;    }
  const methodHandle   get_method() const   { return _method;   }
  const address        get_location() const { return _location; }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventClass : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;

public:
  JvmdiEventClass(int kind, Handle thread, KlassHandle klass);

  const Handle         get_thread() const   { return _thread;   }
  const KlassHandle    get_klass() const    { return _klass;    }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventClassUnload : public JvmdiEventProxy {
private:
  const Handle         _thread;
  const KlassHandle    _klass;

public:
  JvmdiEventClassUnload(Handle thread, KlassHandle klass);

  const Handle         get_thread() const   { return _thread;   }
  const KlassHandle    get_klass() const    { return _klass;    }

  void write(outputStream& out);
  // Proxy events must specify JavaThread with to_event().
  // JvmdiEventProxy defines a non-virtual version of this method that
  // is Unimplemented(), but the Solaris compilers still complain so
  // we add this definition here too.
  void to_event(JVMDI_Event& event) { Unimplemented(); }
  void to_event(JVMDI_Event& event, JavaThread *thread);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventBreakpoint : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;

public:
  JvmdiEventBreakpoint(Handle thread, KlassHandle klass, methodHandle method, address location);

  const Handle         get_thread() const   { return _thread;   }
  const KlassHandle    get_klass() const    { return _klass;    }
  const methodHandle   get_method() const   { return _method;   }
  const address        get_location() const { return _location; }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventFieldAccess : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;
  const KlassHandle    _field_klass;
  const Handle         _object;
  const jfieldID       _field;

public:
  JvmdiEventFieldAccess(Handle thread, KlassHandle klass, methodHandle method,
    address location, KlassHandle field_klass, Handle object, jfieldID field);

  const Handle       get_thread()         const { return _thread;         }
  const KlassHandle  get_klass()          const { return _klass;          }
  const methodHandle get_method()         const { return _method;         }
  const address      get_location()       const { return _location;       }
  const KlassHandle  get_field_klass()    const { return _field_klass;    }
  const Handle       get_object()         const { return _object;         }
  const jfieldID     get_field()          const { return _field;          }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventFieldModification : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;
  const KlassHandle    _field_klass;
  const Handle         _object;
  const jfieldID       _field;
  const char           _signature_type;
  const jvalue *       _new_value;

public:
  JvmdiEventFieldModification(Handle thread, KlassHandle klass,
    methodHandle method, address location, KlassHandle field_klass,
    Handle object, jfieldID field, char signature_type, jvalue *new_value);

  const Handle       get_thread()         const { return _thread;         }
  const KlassHandle  get_klass()          const { return _klass;          }
  const methodHandle get_method()         const { return _method;         }
  const address      get_location()       const { return _location;       }
  const KlassHandle  get_field_klass()    const { return _field_klass;    }
  const Handle       get_object()         const { return _object;         }
  const jfieldID     get_field()          const { return _field;          }
  const char         get_signature_type() const { return _signature_type; }
  const jvalue *     get_new_value()      const { return _new_value;      }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventFrame : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const jframeID       _frame_id;
public:
  JvmdiEventFrame(int kind, Handle thread, KlassHandle klass, methodHandle method, jframeID frame_id);
  
  const Handle         get_thread() const   { return _thread;   }
  const KlassHandle    get_klass() const    { return _klass;    }
  const methodHandle   get_method() const   { return _method;   }
  const jframeID       get_jframeID() const { return _frame_id; }

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};


class JvmdiEventException : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;
  const Handle         _exception;
  const KlassHandle    _catch_klass;
  const methodHandle   _catch_method;
  const address        _catch_location;

public:
  JvmdiEventException(Handle thread, KlassHandle klass, methodHandle method, address location, Handle exception,
		      KlassHandle catch_klass, methodHandle catch_method, address catch_location);

  const Handle         get_thread()            const { return _thread;         }
  const KlassHandle    get_klass()             const { return _klass;          }
  const methodHandle   get_method()            const { return _method;         }
  const address        get_location()          const { return _location;       }
  const Handle         get_exception()         const { return _exception;      }
  const KlassHandle    get_catch_klass()       const { return _catch_klass;    }
  const methodHandle   get_catch_method()      const { return _catch_method;   }
  const address        get_catch_location()    const { return _catch_location; }
  
  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};

class JvmdiEventExceptionCatch : public JvmdiEvent {
private:
  const Handle         _thread;
  const KlassHandle    _klass;
  const methodHandle   _method;
  const address        _location;
  const Handle         _exception;

public:
  JvmdiEventExceptionCatch(Handle thread, KlassHandle klass, methodHandle method, address location, Handle exception);

  const Handle         get_thread()    const { return _thread;    }
  const KlassHandle    get_klass()     const { return _klass;     }
  const methodHandle   get_method()    const { return _method;    }
  const address        get_location()  const { return _location;  }
  const Handle         get_exception() const { return _exception; }
  

  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};

class JvmdiEventUser : public JvmdiEvent {
public:
  JvmdiEventUser();
  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};

class JvmdiEventThreadChange : public JvmdiEvent {
private:
  const Handle        _thread;
public:
  JvmdiEventThreadChange(int kind, Handle);
  
  const Handle        get_thread() const   { return _thread;   } 
 
  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};

class JvmdiEventVm : public JvmdiEvent {
public:
  JvmdiEventVm(int kind);
  void write(outputStream& out);
  void to_event(JVMDI_Event& event);
  void destroy_local_JNI_handles(JVMDI_Event& event);
};
