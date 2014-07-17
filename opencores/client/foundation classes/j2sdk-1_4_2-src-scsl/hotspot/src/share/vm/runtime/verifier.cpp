#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)verifier.cpp	1.50 03/01/23 12:26:27 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_verifier.cpp.incl"

extern "C" {
  typedef jboolean (*verify_byte_codes_fn_t)(JNIEnv *, jclass, char *, jint);
}

// Access to external entry for VerifyClassFormat
static check_format_fn_t _verify_class_format_fn = NULL;

static check_format_fn_t verify_class_format_fn() {
  if (_verify_class_format_fn == NULL) {
    void *lib_handle = os::native_java_library();
    _verify_class_format_fn = CAST_TO_FN_PTR(check_format_fn_t, hpi::dll_lookup(lib_handle, "VerifyClassFormat"));
  }
  return _verify_class_format_fn;
}

// Access to external entry for VerifyClassCodes

static verify_byte_codes_fn_t _verify_byte_codes_fn = NULL;

static verify_byte_codes_fn_t verify_byte_codes_fn() {
  if (_verify_byte_codes_fn == NULL) {
    void *lib_handle = os::native_java_library();
    _verify_byte_codes_fn = CAST_TO_FN_PTR(verify_byte_codes_fn_t, hpi::dll_lookup(lib_handle, "VerifyClassCodes"));
  }
  return _verify_byte_codes_fn;
}

bool Verifier::should_verify_for(oop class_loader) {
  return class_loader == NULL ? BytecodeVerificationLocal : BytecodeVerificationRemote;
}

bool Verifier::relax_verify_for(oop loader) {
  bool trusted = java_lang_ClassLoader::is_trusted_loader(loader);
  bool need_verify = (BytecodeVerificationLocal && BytecodeVerificationRemote) ||               // verifyAll
                     (!BytecodeVerificationLocal && BytecodeVerificationRemote && !trusted);    // verifyRemote
  return !need_verify;
}

void Verifier::verify_class_format(symbolHandle class_name, Handle class_loader, ClassFileStream* st, TRAPS) {
  // Figure out whether we can skip format check (matching classic VM behavior)
  if (!should_verify_for(class_loader())) return;

  ResourceMark rm;
  char* name = class_name.is_null() ? NULL : class_name->as_utf8();

  check_format_fn_t func = verify_class_format_fn();

  if (func == 0) {
    THROW_MSG(vmSymbols::java_lang_VerifyError(), "Could not link verifier");
  }

  // Call the external verifier
  char message[256];
  message[0] = 0;
  class_size_info csi;
  int result;
  {
    JavaThread* thread = (JavaThread*) THREAD;
    HandleMark hm(thread);

    jboolean relax_checking = relax_verify_for(class_loader());
    if (thread->has_last_Java_frame()) { 
      StateSaver nsv(thread); // we must have post Java state set while in native
      ThreadToNativeFromVM ttn(thread);
      // ThreadToNativeFromVM takes care of changing thread_state, so safepoint code knows that
      // we have left the VM

      result = (*func)(name, st->buffer(), st->length(), &csi, message, sizeof(message), false, relax_checking);
    } else {
      ThreadToNativeFromVM ttn(thread);

      result = (*func)(name, st->buffer(), st->length(), &csi, message, sizeof(message), false, relax_checking);
    }
  }

  //  0: good
  if (result == 0) return;

  // -1: out of memory
  if (result == -1) {
    THROW_OOP(Universe::out_of_memory_error_instance());
  }
  // -2: class format error
  if (result == -2) {
    THROW_MSG(vmSymbols::java_lang_ClassFormatError(), message);
  }
  // -3: unsupported version error
  if (result == -3) {
    THROW_MSG(vmSymbols::java_lang_UnsupportedClassVersionError(), message);
  }
  // -4: bad name
  if (result == -4) {
    THROW_MSG(vmSymbols::java_lang_NoClassDefFoundError(), message);
  }
  // Undocumented result value
  ShouldNotReachHere();
}

void Verifier::verify_byte_codes(instanceKlassHandle klass, TRAPS) {
  if (!should_verify_for(klass->class_loader())) return;

  JavaThread *thread = JavaThread::current();

  // return if the class is a bootstraping class
  symbolOop name = klass->name();  
  if (thread == NULL || // We can't verify classes in VM thread
      // We need to skip the following three for bootstraping
      name == vmSymbols::java_lang_Class() ||
      name == vmSymbols::java_lang_String() ||
      name == vmSymbols::java_lang_Throwable()) {
    return;
  }  
  
  // As of the fix for 4486457 we disable verification for all of the
  // dynamically-generated bytecodes associated with the 1.4
  // reflection implementation, not just those associated with
  // sun/reflect/SerializationConstructorAccessor.
  // NOTE: this is called too early in the bootstrapping process to be
  // guarded by Universe::is_gte_jdk14x_version()/UseNewReflection.
  klassOop refl_magic_klass =
    SystemDictionary::reflect_magic_klass();
  if (refl_magic_klass != NULL) {
    if (klass->is_subtype_of(refl_magic_klass)) {
      if (!VerifyReflectionBytecodes) {
        return;
      }
    }
  }

  JNIEnv *env = thread->jni_environment();
  
  verify_byte_codes_fn_t func = verify_byte_codes_fn();

  if (func == 0) {
    THROW_MSG(vmSymbols::java_lang_VerifyError(), "Could not link verifier");
  }

  ResourceMark rm;
  if (TraceClassInitialization) {
    tty->print_cr("Start class verification for: %s", klass->external_name());
  }
  
  char message[256];
  message[0] = 0;

  jclass cls = (jclass) JNIHandles::make_local(env, klass->java_mirror());
  bool result;

  {
    HandleMark hm(thread);
    if (thread->has_last_Java_frame()) { 
      StateSaver nsv(thread); // we must have post Java state set while in native
      ThreadToNativeFromVM ttn(thread);
      // ThreadToNativeFromVM takes care of changing thread_state, so safepoint code knows that
      // we have left the VM

      result = (*func)(env, cls, message, sizeof(message));
    } else {
      ThreadToNativeFromVM ttn(thread);

      result = (*func)(env, cls, message, sizeof(message));
    }
  }
 
  if (TraceClassInitialization) {
    tty->print_cr("Verification result is %d for %s", result, klass->external_name());
    tty->print_cr(" exception pending %d ", thread->pending_exception());
  }

  if (!result && !thread->pending_exception()) {    
    THROW_MSG(vmSymbols::java_lang_VerifyError(), message);  
  }
}

