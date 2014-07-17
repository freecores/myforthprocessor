#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdi_hotswap.cpp	1.38 03/02/28 10:27:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdi_hotswap.cpp.incl"


static objArrayOop _old_methods, _new_methods;  
static klassOop _evolving_koop;
static constantPoolOop _old_constants;

class VM_RedefineClasses: public VM_Operation {
 private:
  jint _class_count;
  JVMDI_class_definition *_class_defs;
  instanceKlassHandle *_k_h_new;
  jvmdiError _res;

 public:
  VM_RedefineClasses(jint class_count, JVMDI_class_definition *class_defs) {
    _class_count = class_count;
    _class_defs = class_defs;
    _res = JVMDI_ERROR_NONE;
  }

  bool doit_prologue() {
    // We first load new class versions in the prologue, because somewhere down the
    // call chain it is required that the current thread is a Java thread.
    Thread *THREAD = Thread::current();
    if ((_res = load_new_class_versions(THREAD)) != JVMDI_ERROR_NONE) {
     // Free os::malloc allocated memory in load_new_class_version.
      os::free(_k_h_new);
      return false;
    }
    return true;
  }

  void doit() {
    Thread *thread = Thread::current();
    for (int i = 0; i < _class_count; i++) {
      redefine_single_class(_class_defs[i].clazz, _k_h_new[i], thread);
    }
    // Disable any dependent concurrent compilations
    SystemDictionary::notice_modification();
  }

  void doit_epilogue() {
    // Free os::malloc allocated memory.
    // The memory allocated in redefine will be free'ed in next VM operation.
    os::free(_k_h_new);
  }

  const char* name() const                       { return "VM_RedefineClasses"; }
  bool allow_nested_vm_operations() const        { return true; }
  jvmdiError check_error()                       { return _res; }


private:
  jvmdiError compare_class_versions(instanceKlassHandle k_h_old, instanceKlassHandle k_h_new) {
    int i;

    // Check whether essential class modifiers are the same. The rest can probably differ, e.g.
    // why not allow substitute class to be synthetic, if it satisfies other conditions.
    AccessFlags old_flags = k_h_old->access_flags();
    AccessFlags new_flags = k_h_new->access_flags();
    if (old_flags.is_public() != new_flags.is_public() ||
	old_flags.is_final() != new_flags.is_final() ||
	old_flags.is_interface() != new_flags.is_interface() ||
	old_flags.is_abstract() != new_flags.is_abstract()) {
      return JVMDI_ERROR_CLASS_MODIFIERS_CHANGE_NOT_IMPLEMENTED;
    }

    // Check superclasses, or rather their names, since superclasses themselves can be
    // requested to replace
    if (Klass::cast(k_h_old->super())->name() != Klass::cast(k_h_new->super())->name()) {
      return JVMDI_ERROR_HIERARCHY_CHANGE_NOT_IMPLEMENTED;
    }

    // Check if the number, names and order of directly implemented interfaces are the same.
    // I think in principle we should just check if the sets of names of directly implemented
    // interfaces are the same, i.e. the order of declaration (which, however, if changed in the
    // .java file, also changes in .class file) should not matter. However, comparing sets is
    // technically a bit more difficult, and, more importantly, I am not sure at present that the
    // order of interfaces does not matter on the implementation level, i.e. that the VM does not
    // rely on it somewhere.
    objArrayOop k_interfaces = k_h_old->local_interfaces();
    objArrayOop k_new_interfaces = k_h_new->local_interfaces();
    int n_intfs = k_interfaces->length();
    if (n_intfs != k_new_interfaces->length()) {
      return JVMDI_ERROR_HIERARCHY_CHANGE_NOT_IMPLEMENTED;
    }
    for (i = 0; i < n_intfs; i++) {
      if (Klass::cast((klassOop) k_interfaces->obj_at(i))->name() !=
	  Klass::cast((klassOop) k_new_interfaces->obj_at(i))->name()) {
        return JVMDI_ERROR_HIERARCHY_CHANGE_NOT_IMPLEMENTED;
      }
    }

    // Check if the number, names, types and order of fields declared in these classes
    // are the same.
    typeArrayOop k_old_fields = k_h_old->fields();
    typeArrayOop k_new_fields = k_h_new->fields();
    int n_fields = k_old_fields->length();
    if (n_fields != k_new_fields->length()) {
      return JVMDI_ERROR_SCHEMA_CHANGE_NOT_IMPLEMENTED;
    }

    for (i = 0; i < n_fields; i += instanceKlass::next_offset) {
      // access
      if (k_old_fields->ushort_at(i + instanceKlass::access_flags_offset) !=
        k_new_fields->ushort_at(i + instanceKlass::access_flags_offset)) {
        return JVMDI_ERROR_SCHEMA_CHANGE_NOT_IMPLEMENTED;
      }
      // offset
      if (k_old_fields->short_at(i + instanceKlass::low_offset) != 
	  k_new_fields->short_at(i + instanceKlass::low_offset) ||
	  k_old_fields->short_at(i + instanceKlass::high_offset) != 
	  k_new_fields->short_at(i + instanceKlass::high_offset)) {
        return JVMDI_ERROR_SCHEMA_CHANGE_NOT_IMPLEMENTED;
      }
      // name and signature
      jshort name_index = k_old_fields->short_at(i + instanceKlass::name_index_offset);
      jshort sig_index = k_old_fields->short_at(i +instanceKlass::signature_index_offset);
      symbolOop name_sym1 = k_h_old->constants()->symbol_at(name_index);
      symbolOop sig_sym1 = k_h_old->constants()->symbol_at(sig_index);
      name_index = k_new_fields->short_at(i + instanceKlass::name_index_offset);
      sig_index = k_new_fields->short_at(i + instanceKlass::signature_index_offset);
      symbolOop name_sym2 = k_h_new->constants()->symbol_at(name_index);
      symbolOop sig_sym2 = k_h_new->constants()->symbol_at(sig_index);
      if (name_sym1 != name_sym2 || sig_sym1 != sig_sym2) {
        return JVMDI_ERROR_SCHEMA_CHANGE_NOT_IMPLEMENTED;
      }
    }

    // Check if the number, names, signatures and order of methods declared in these classes
    // are the same.
    objArrayOop k_methods = k_h_old->methods();
    objArrayOop k_new_methods = k_h_new->methods();
    int n_methods = k_methods->length();
    if (n_methods < k_new_methods->length()) return JVMDI_ERROR_ADD_METHOD_NOT_IMPLEMENTED;
    else if (n_methods > k_new_methods->length()) return JVMDI_ERROR_DELETE_METHOD_NOT_IMPLEMENTED;

    for (i = 0; i < n_methods; i++) {
      methodOop k_method = (methodOop) k_methods->obj_at(i);
      methodOop k_new_method = (methodOop) k_new_methods->obj_at(i);
      if (k_method->name() != k_new_method->name()) return JVMDI_ERROR_DELETE_METHOD_NOT_IMPLEMENTED;
      if (k_method->signature() != k_new_method->signature()) {
        return JVMDI_ERROR_DELETE_METHOD_NOT_IMPLEMENTED;
      }
      old_flags = k_method->access_flags();
      new_flags = k_new_method->access_flags();
      // It's probably safer to not compare the values of access_flags directly, since
      // some bits in them encode some implementation-specific information, e.g.
      // something about inlined tables. This may be different in the new version,
      // but should not affect method changeability.
      if (old_flags.is_public() != new_flags.is_public() ||
	  old_flags.is_protected() != new_flags.is_protected() ||
	  old_flags.is_private() != new_flags.is_private() ||
	  old_flags.is_static() != new_flags.is_static() ||
	  old_flags.is_final() != new_flags.is_final() ||
	  old_flags.is_synchronized() != new_flags.is_synchronized() ||
          old_flags.is_strict() != new_flags.is_strict() ||
	  old_flags.is_interface() != new_flags.is_interface() ||
	  old_flags.is_abstract() != new_flags.is_abstract()) {
        return JVMDI_ERROR_METHOD_MODIFIERS_CHANGE_NOT_IMPLEMENTED;
      }      
    }

    return JVMDI_ERROR_NONE;
  }


  jvmdiError load_new_class_versions(TRAPS) {
    // For consistency allocate memory using os::malloc wrapper.
    _k_h_new = (instanceKlassHandle *) os::malloc(sizeof(instanceKlassHandle) * _class_count);

    ResourceMark rm(THREAD);

    for (int i = 0; i < _class_count; i++) {
      oop mirror = JNIHandles::resolve_non_null(_class_defs[i].clazz);
      klassOop k_oop = java_lang_Class::as_klassOop(mirror);
      instanceKlassHandle k_h = instanceKlassHandle(THREAD, k_oop);
      symbolHandle k_name = symbolHandle(THREAD, k_h->name());

      ClassFileStream st((u1*) _class_defs[i].class_bytes, _class_defs[i].class_byte_count, NULL);

      // Parse the stream.
      Handle k_loader_h(THREAD, k_h->class_loader());
      Handle protection_domain;
      klassOop k = SystemDictionary::parse_stream(k_name, 
                                                  k_loader_h, 
                                                  protection_domain,
                                                  &st,
                                                  THREAD);
                                     
      instanceKlassHandle k_h_new (THREAD, k);

      if (HAS_PENDING_EXCEPTION) {
        if (PENDING_EXCEPTION->klass()->klass_part()->name() ==
          vmSymbols::java_lang_UnsupportedClassVersionError()) {
          CLEAR_PENDING_EXCEPTION;
          return JVMDI_ERROR_UNSUPPORTED_VERSION;

        } else if (PENDING_EXCEPTION->klass()->klass_part()->name() == 
          vmSymbols::java_lang_ClassFormatError()) {
          CLEAR_PENDING_EXCEPTION;
          return JVMDI_ERROR_INVALID_CLASS_FORMAT;
        } else if (PENDING_EXCEPTION->klass()->klass_part()->name() ==
          vmSymbols::java_lang_ClassCircularityError()) {
          CLEAR_PENDING_EXCEPTION;
          return JVMDI_ERROR_CIRCULAR_CLASS_DEFINITION;
        } else if (PENDING_EXCEPTION->klass()->klass_part()->name() ==
          vmSymbols::java_lang_NoClassDefFoundError()) {
          // The message will be "XXX (wrong name: YYY)"
          CLEAR_PENDING_EXCEPTION;
          return JVMDI_ERROR_NAMES_DONT_MATCH;
        } else {  // Just in case more exceptions can be thrown..
          return JVMDI_ERROR_FAILS_VERIFICATION;
        }
      }

      // See instanceKlass::link_klass_impl()
      { ObjectLocker ol(k_h_new, THREAD);
        Verifier::verify_byte_codes(k_h_new, THREAD);

        if (HAS_PENDING_EXCEPTION) {
          CLEAR_PENDING_EXCEPTION;
          return JVMDI_ERROR_FAILS_VERIFICATION;
        }
        Rewriter::rewrite(k_h_new, THREAD); // No exception can happen here
      }

      jvmdiError res = compare_class_versions(k_h, k_h_new);
      if (res != JVMDI_ERROR_NONE) return res;
  
      _k_h_new[i] = k_h_new;
    }
  
    return JVMDI_ERROR_NONE;
  }


  // Field names and signatures are referenced via constantpool indexes. In the new class
  // version, the layout of the constantpool can be different, so these indexes should be
  // patched.
  void patch_indexes_for_fields(instanceKlassHandle k_h, instanceKlassHandle k_h_new) {
    typeArrayOop k_fields = k_h->fields();
    typeArrayOop k_new_fields = k_h_new->fields();
    int n_fields = k_fields->length();

    for (int i = 0; i < n_fields; i += instanceKlass::next_offset) {
      // name and signature
      k_fields->short_at_put(
        i + instanceKlass::name_index_offset,
        k_new_fields->short_at(i + instanceKlass::name_index_offset)
        );
      k_fields->short_at_put(
        i + instanceKlass::signature_index_offset,
        k_new_fields->short_at(i + instanceKlass::signature_index_offset)
        );
    }
  }

  // Unevolving classes may point to methods of the evolving class directly
  // from their constantpool caches and vtables/itables. Fix this. 
  static void adjust_cpool_cache_and_vtable(klassOop k_oop, oop loader) {
    Klass *k = k_oop->klass_part();
    if (k->oop_is_instance()) {
      instanceKlass *ik = (instanceKlass *) k;
      bool previous_version; 

      if (java_core_class_name(ik->name()->as_utf8())) return;

      // By this time we have already replaced the constantpool pointer in the evolving
      // class itself. However, we need to fix the entries in the old constantpool, which
      // is still referenced by active old methods of this class.
      constantPoolCacheOop cp_cache = (k_oop == _evolving_koop) ? 
                _old_constants->cache() : ik->constants()->cache();
      
      do {

        if (cp_cache != NULL) {
          cp_cache->adjust_method_entries(_old_methods, _new_methods);
        }

        // Fix vtable (this is needed for the evolving class itself and its subclasses).
        if (ik->vtable_length() > 0 && ik->is_subclass_of(_evolving_koop)) {
          ik->vtable()->adjust_entries(_old_methods, _new_methods);         
        }
        // Fix itable, if it exists.
        if (ik->itable_length() > 0) {
          ik->itable()->adjust_method_entries(_old_methods, _new_methods);
        }

        // Previous version methods could be still on stack so adjust entries
        // in all previous versions.
        if (ik->has_previous_version()) {
          ik = (instanceKlass *)(ik->previous_version())->klass_part();
          cp_cache = ik->constants()->cache();
          previous_version = true;
        } else {
          previous_version = false;
        }
      } while (previous_version);
    }
  }

  void check_methods_and_mark_as_old() {
    for (int i = 0; i < _old_methods->length(); i++) {
      methodOop old_method = (methodOop) _old_methods->obj_at(i);
      old_method->set_old_version();
      if (jvmdi::enabled()) {
          methodOop new_method = (methodOop) _new_methods->obj_at(i);
          if (!MethodComparator::methods_EMCP(old_method, new_method)) {
              // Mark non-EMCP methods as such
              old_method->set_non_emcp_with_new_version();
          }
      }
    }
  }

  void flush_method_JNIid_cache() {
    for (int i = 0; i < _old_methods->length(); i++) {
      methodOop mop = (methodOop)_old_methods->obj_at(i);
      JNIid *mid = mop->jni_id();
      if (mid != NULL) {
        // don't need to call JNIid::lock() since we are at a safepoint
        mid->set_resolved_method((methodOop)NULL, (klassOop)NULL);
      }
    }
  }


  // Install the redefinition of a class --
  // The original instanceKlass object (k_h) always represents the latest 
  // version of the respective class. However, during class redefinition we swap
  // or replace much of its content with that of the instanceKlass object created
  // from the bytes of the redefine (k_h_new). Specifically, k_h points to the new
  // constantpool and methods objects, which we take from k_h_new. k_h_new, in turn,
  // assumes the role of the previous class version, with the old constantpool and
  // methods (taken from k_h) attached to it. k_h links to k_h_new to create a 
  // linked list of class versions. 
  void redefine_single_class(jclass j_clazz, instanceKlassHandle k_h_new, TRAPS) {
    oop mirror = JNIHandles::resolve_non_null(j_clazz);
    klassOop k_oop = java_lang_Class::as_klassOop(mirror);
    instanceKlassHandle k_h = instanceKlassHandle(THREAD, k_oop);

    if (jvmdi::enabled()) {
      // Remove all breakpoints in methods of this class
      JvmdiBreakpoints& jvmdi_breakpoints = JvmdiCurrentBreakpoints::get_jvmdi_breakpoints();
      jvmdi_breakpoints.clearall_in_class_at_safepoint(k_oop); 
    }

    // Deoptimize all compiled code that depends on this class
    NOT_CORE(Universe::flush_evol_dependents_on(k_h));

    _old_methods = k_h->methods();
    _new_methods = k_h_new->methods();
    _evolving_koop = k_oop;
    _old_constants = k_h->constants();

    // flush the cached JNIid fields for _old_methods
    flush_method_JNIid_cache();

    // Patch the indexes into the constantpool from the array of fields of the evolving
    // class. This is required, because the layout of the new constantpool can be different,
    // so old indexes corresponding to field names and signatures can become invalid.
    patch_indexes_for_fields(k_h, k_h_new);

    // Make new constantpool object (and methodOops via it) point to the original class object
    k_h_new->constants()->set_pool_holder(k_h());

    // Replace methods and constantpool
    k_h->set_methods(_new_methods);
    k_h_new->set_methods(_old_methods);     // To prevent potential GCing of the old methods, 
                                            // and to be able to undo operation easily.

    constantPoolOop old_constants = k_h->constants();
    k_h->set_constants(k_h_new->constants());
    k_h_new->set_constants(old_constants);  // See the previous comment.

    check_methods_and_mark_as_old();

    // Initialize the vtable and interface table after
    // methods have been rewritten
    { ResourceMark rm(THREAD);
      k_h->vtable()->initialize_vtable(THREAD); // No exception can happen here
      k_h->itable()->initialize_itable();
    }

    // Copy the "source file name" attribute from new class version
    k_h->set_source_file_name(k_h_new->source_file_name());

    // Copy the "source debug extension" attribute from new class version
    k_h->set_source_debug_extension(k_h_new->source_debug_extension());

    // Use of javac -g could be different in the old and the new
    if (k_h_new->access_flags().has_localvariable_table() !=
	k_h->access_flags().has_localvariable_table()) {

	AccessFlags flags = k_h->access_flags();
	if (k_h_new->access_flags().has_localvariable_table()) {
	    flags.set_has_localvariable_table();
	} else {
	    flags.clear_has_localvariable_table();
	}
	k_h->set_access_flags(flags);
    }


    // Maintain a linked list of versions of this class. 
    // List is in ascending age order. Current version (k_h) is the head.
    if (k_h->has_previous_version()) {
        k_h_new->set_previous_version(k_h->previous_version());
    }
    k_h->set_previous_version(k_h_new);

    // Adjust constantpool caches and vtables for all classes
    // that reference methods of the evolved class.
    SystemDictionary::classes_do(adjust_cpool_cache_and_vtable);
  }
  
};


JVMDI_ENTER(jvmdiError,
	    IsMethodObsolete, (jclass clazz, 
                             jmethodID method, 
                             jboolean *isObsoletePtr))
  (*isObsoletePtr) = method == OBSOLETE_METHOD_ID;

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError,
	    RedefineClasses, 
	    (jint classCount,
	     JVMDI_class_definition *classDefs))
  // check for NULL arguments
  if (classCount == 0) {
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }
  NULL_CHECK(classDefs, JVMDI_ERROR_NULL_POINTER);
  for (int i = 0; i < classCount; i++) {
    if (classDefs[i].clazz == NULL) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS);
    }
    if (classDefs[i].class_byte_count == 0) {
      JVMDI_RETURN(JVMDI_ERROR_INVALID_CLASS_FORMAT);
    }
    if (classDefs[i].class_bytes == NULL) {
      JVMDI_RETURN(JVMDI_ERROR_NULL_POINTER);
    }
  }
  VM_RedefineClasses op(classCount, classDefs);
  VMThread::execute(&op);

  JVMDI_RETURN(op.check_error());
JVMDI_END


JVMDI_ENTER(jvmdiError,
	    PopFrame, (jthread jni_thread))
  NULL_CHECK(jni_thread, JVMDI_ERROR_NULL_POINTER);
  JavaThread* java_thread = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);

  Thread *cur_thread = Thread::current();

  // Check if java_thread is fully suspended
  if (!JvmdiInternal::is_thread_fully_suspended(java_thread, true /* wait for suspend completion */)) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }

  // Check to see if a PopFrame was already in progress
  if (java_thread->popframe_condition() != JavaThread::popframe_inactive) {
    // Probably possible for JVMDI clients to trigger this, but the
    // JPDA backend shouldn't allow this to happen
    JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
  }

  {
    // Workaround bug 
    //    4812902: popFrame hangs if the method is waiting at a synchronize 
    // Catch this condition and return an error to avoid hanging.
    OSThread* osThread = java_thread->osthread();
    if (osThread->get_state() == MONITOR_WAIT) {
      JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
    }
  }

  {
    ResourceMark rm(THREAD);

    // Check if there are more than one Java frame in this thread, that the top two frames
    // are Java (not native) frames, and that there is no intervening VM frame
    int frame_count = 0;
    bool is_interpreted[2];
    intptr_t *frame_sp[2];
    for (vframeStream vfs(java_thread); !vfs.at_end(); vfs.next()) {
      methodHandle mh(cur_thread, vfs.method());
      if (mh->is_native()) JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
      if ((frame_count == 0) && vfs.is_entry_frame()) JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
      is_interpreted[frame_count] = vfs.is_interpreted_frame();
      frame_sp[frame_count] = vfs.frame_id();
      if (++frame_count > 1) break;
    }
    if (frame_count < 2) JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
    
#ifndef CORE
    // If any of the top 2 frames is a compiled one, need to deoptimize it
    for (int i = 0; i < 2; i++) {
      if (!is_interpreted[i]) {
        VM_DeoptimizeFrame op(java_thread, frame_sp[i]);
        VMThread::execute(&op);
      }
    }
#endif /* !CORE */
    
    // Update the thread state to reflect that the top frame is popped
    // so that cur_stack_depth is maintained properly and all frameIDs
    // are invalidated.
    // The current frame will be popped later when the suspended thread
    // is resumed and right before returning from VM to Java.
    // (see call_VM_base() in assembler_<cpu>.cpp).
    
    // It's fine to update the thread state here because no JVMDI events 
    // shall be posted for this PopFrame.
    
    java_thread->jvmdi_thread_state()->pop_top_frame();
    java_thread->set_popframe_condition(JavaThread::popframe_pending_bit);
    // Set pending step flag for this popframe and it is cleared when next
    // step event is posted.
    java_thread->jvmdi_thread_state()->set_pending_step_for_popframe();
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


void JvmdiInternal::set_hotswap_hooks(JVMDI_Interface_1 *jvmdi_interface) {
  jvmdi_interface->RedefineClasses = RedefineClasses;
  jvmdi_interface->PopFrame = PopFrame;
  jvmdi_interface->IsMethodObsolete = IsMethodObsolete;
}

