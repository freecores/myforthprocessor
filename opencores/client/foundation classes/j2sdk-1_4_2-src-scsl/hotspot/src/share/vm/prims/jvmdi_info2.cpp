#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdi_info2.cpp	1.23 03/01/23 12:20:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdi_info2.cpp.incl"


//
// utilities: JNI objects -> oops
//

klassOop JvmdiInternal::get_klassOop(jclass clazz) {
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  if (mirror == NULL || !mirror->is_a(SystemDictionary::class_klass())) {
    return NULL;
  }
  return java_lang_Class::as_klassOop(mirror);
}

methodOop JvmdiInternal::get_methodOop(jmethodID method, klassOop k) {
  if (method == NULL || k == NULL) {
    return NULL;
  }
  methodOop m = ((JNIid *)method)->method();
  if (m == NULL || !m->is_method()) {
    return NULL;
  }

  // Check if method m is defined in instanceKlass k or its supers (classes
  // or interfaces)
  if (k->klass_part()->oop_is_instance()) {
    instanceKlass *ik = instanceKlass::cast(k);
    if (ik->lookup_method(m->name(), m->signature()) == NULL &&
        ik->lookup_method_in_all_interfaces(m->name(), m->signature()) == NULL) {
      return NULL;
    }
  }

  // check if method m is defined in objArrayKlass k
  else if (k->klass_part()->oop_is_objArray()) {
    objArrayKlass *oak = objArrayKlass::cast(k);
    if (oak->lookup_method(m->name(), m->signature()) == NULL) {
      return NULL;
    }
  }

  // check if method m is defined in typeArrayKlass k
  else if (k->klass_part()->oop_is_typeArray()) {
    typeArrayKlass *tak = typeArrayKlass::cast(k);
    if (tak->lookup_method(m->name(), m->signature()) == NULL) {
      return NULL;
    }
  }

  // catch all for other remaining Klass subtypes
  else {
    return NULL;
  }

  // m is really a method and it is implemented by k or its supers
  return m;
}

oop JvmdiInternal::get_Oop(jobject obj) {
  return (oop) JNIHandles::resolve(obj);
}


//
// utilities: oops -> JNI objects
//

jmethodID JvmdiInternal::get_jni_method_non_null(methodOop method) {
  assert(method != NULL, "m != NULL");
  return (jmethodID) method->jni_id();
}

jclass JvmdiInternal::get_jni_class_non_null(klassOop k) {
  assert(k != NULL, "k != NULL");
  return (jclass)JNIHandles::make_global(Klass::cast(k)->java_mirror());
}

jobject JvmdiInternal::get_jni_object(oop obj) {
  return (jobject) JNIHandles::make_global(obj);
}

 
//
// Method Information
//

JVMDI_ENTER(jvmdiError,
            GetMethodName, (jclass clazz, 
                          jmethodID method, 
                          char **namePtr, 
                          char **signaturePtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(namePtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(signaturePtr, JVMDI_ERROR_NULL_POINTER);
  
  {  ResourceMark rm(THREAD); // get the utf8 name and signature
    
     const char* utf8_name = (const char *) m->name()->as_utf8();
     *namePtr = (char *) JvmdiInternal::jvmdiMalloc(strlen(utf8_name)+1);
     strcpy(*namePtr, utf8_name);

     const char* utf8_signature = (const char *) m->signature()->as_utf8();
     *signaturePtr = (char *) JvmdiInternal::jvmdiMalloc(strlen(utf8_signature) + 1);
     strcpy(*signaturePtr, utf8_signature);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
            GetMethodDeclaringClass, (jclass clazz, 
                                      jmethodID method, 
                                      jclass *declaringClassPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(declaringClassPtr, JVMDI_ERROR_NULL_POINTER);

  // get declaring class
  (*declaringClassPtr) = JvmdiInternal::get_jni_class_non_null(m->method_holder());

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
            GetMethodModifiers, (jclass clazz, 
                               jmethodID method, 
                               jint *modifiersPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(modifiersPtr, JVMDI_ERROR_NULL_POINTER);

  // get method modifiers
  (*modifiersPtr) = m->access_flags().as_int() & JVM_ACC_WRITTEN_FLAGS;
  
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, 
            GetMaxStack, (jclass clazz, 
                        jmethodID method, 
                        jint *maxPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(maxPtr, JVMDI_ERROR_NULL_POINTER);

  // get max stack
  (*maxPtr) = m->max_stack();

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
            GetMaxLocals, (jclass clazz, 
                         jmethodID method, 
                         jint *maxPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(maxPtr, JVMDI_ERROR_NULL_POINTER);

  // get max stack
  (*maxPtr) = m->max_locals();

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, 
            GetArgumentsSize, (jclass clazz, 
                               jmethodID method, 
                               jint *sizePtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(sizePtr, JVMDI_ERROR_NULL_POINTER);

  // get size of arguments
  (*sizePtr) = m->size_of_parameters();

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
	    GetLineNumberTable, (jclass clazz, 
				 jmethodID method, 
				 jint *entryCountPtr, 
				 JVMDI_line_number_entry **tablePtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(entryCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(tablePtr, JVMDI_ERROR_NULL_POINTER);

  // The line number table is compressed so we don't know how big it is until decompressed.
  // Decompression is really fast so we just do it twice.

  // Compute size of table
  jint num_entries = 0;
  if (m->has_linenumber_table()) {
    CompressedLineNumberReadStream stream(m->compressed_linenumber_table());
    while (stream.read_pair()) {
      num_entries++;
    }
  }
  JVMDI_line_number_entry *jvmdi_table = 
	    (JVMDI_line_number_entry *)JvmdiInternal::jvmdiMalloc(num_entries * (sizeof(JVMDI_line_number_entry)));
 
  // Fill jvmdi table
  if (num_entries > 0) {
    int index = 0;
    CompressedLineNumberReadStream stream(m->compressed_linenumber_table());
    while (stream.read_pair()) {
      jvmdi_table[index].start_location = (jlocation) stream.bci();
      jvmdi_table[index].line_number = (jint) stream.line();
      index++;
    }
    assert(index == num_entries, "sanity check");
  }
  
  // Set up results  
  (*entryCountPtr) = num_entries;
  (*tablePtr) = jvmdi_table;

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END  

JVMDI_ENTER(jvmdiError, 
            GetMethodLocation, (jclass clazz, 
                                jmethodID method,
                                jlocation *startLocationPtr, 
                                jlocation *endLocationPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(startLocationPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(endLocationPtr, JVMDI_ERROR_NULL_POINTER);

  // get start and end location
  (*endLocationPtr) = (jlocation) (m->code_size() - 1);
  if (m->code_size() == 0) {
    // there is no code so there is no start location
    (*startLocationPtr) = (jlocation)(-1);
  } else {
    (*startLocationPtr) = (jlocation)(0);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, 
	    GetLocalVariableTable, (jclass clazz, 
				    jmethodID method,
				    jint *entryCountPtr, 
				    JVMDI_local_variable_entry **tablePtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(entryCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(tablePtr, JVMDI_ERROR_NULL_POINTER);

  // does the klass have any local variable information?
  instanceKlass* ik = instanceKlass::cast(k);
  if (!ik->access_flags().has_localvariable_table()) {
    JVMDI_RETURN(JVMDI_ERROR_ABSENT_INFORMATION);
  }

  constantPoolOop constants = m->constants();
  NULL_CHECK(constants, JVMDI_ERROR_ABSENT_INFORMATION);
  
  // in the vm localvariable table representation, 5 consecutive elements in the table
  // represent a 5-tuple of shorts [start_pc, length, name_index, descriptor_index, index]
  jint num_entries = m->localvariable_table_length();
  JVMDI_local_variable_entry *jvmdi_table = (JVMDI_local_variable_entry *)
    JvmdiInternal::jvmdiMalloc(num_entries * (sizeof(JVMDI_local_variable_entry)));
  
  if (num_entries > 0) {
    LocalVariableTableElement* table = m->localvariable_table_start();
    for (int i = 0; i < num_entries; i++) {
      // get the 5 tuple information from the vm table
      jlocation start_location = (jlocation) table[i].start_bci;
      jint length = (jint) table[i].length;
      int name_index = (int) table[i].name_cp_index;
      int signature_index = (int) table[i].descriptor_cp_index;
      jint slot = (jint) table[i].slot;

      // get utf8 name and signature
      char *name_buf = NULL;
      char *sig_buf = NULL;
      { 
        ResourceMark rm(THREAD);
    
        const char *utf8_name = (const char *) constants->symbol_at(name_index)->as_utf8();
        name_buf = (char *) JvmdiInternal::jvmdiMalloc(strlen(utf8_name)+1);
        strcpy(name_buf, utf8_name);
      
        const char *utf8_signature = (const char *) constants->symbol_at(signature_index)->as_utf8();
        sig_buf = (char *) JvmdiInternal::jvmdiMalloc(strlen(utf8_signature)+1);
        strcpy(sig_buf, utf8_signature);
      }

      // fill in the jvmdi table
      jvmdi_table[i].start_location = start_location;
      jvmdi_table[i].length = length;
      jvmdi_table[i].name = name_buf;
      jvmdi_table[i].signature = sig_buf;
      jvmdi_table[i].slot = slot;
    }
  }
  
  // set results
  (*entryCountPtr) = num_entries;
  (*tablePtr) = jvmdi_table;
  
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END  

JVMDI_ENTER(jvmdiError,
	    GetExceptionHandlerTable, (jclass clazz, 
                                       jmethodID method,
                                       jint *entryCountPtr, 
                                       JVMDI_exception_handler_entry **tablePtr))
  JVMDI_RETURN(JVMDI_ERROR_NOT_IMPLEMENTED);
JVMDI_END

JVMDI_ENTER(jvmdiError, GetThrownExceptions, (jclass clazz, 
                                              jmethodID method,
                                              jint *exceptionCountPtr, 
                                              jclass **exceptionsPtr))
  JVMDI_RETURN(JVMDI_ERROR_NOT_IMPLEMENTED);
JVMDI_END

JVMDI_ENTER(jvmdiError,
            GetBytecodes, (jclass clazz, 
                           jmethodID method,
                           jint *bytecodeCountPtr, 
                           jbyte **bytecodesPtr))
  // check for NULL arguments
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  NULL_CHECK(bytecodeCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(bytecodesPtr, JVMDI_ERROR_NULL_POINTER);

  // get byte codes
  jint size = (jint)m->code_size();
  (*bytecodeCountPtr) = size;
  (*bytecodesPtr) = (jbyte *) JvmdiInternal::jvmdiMalloc(size);
  memcpy((*bytecodesPtr), (jbyte *)m->code_base(), size);
  
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
	    IsMethodNative, (jclass clazz, 
                             jmethodID method, 
                             jboolean *isNativePtr))
  // check for NULL arguments
  NULL_CHECK(isNativePtr, JVMDI_ERROR_NULL_POINTER);
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);

  (*isNativePtr) = m->is_native();

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError,
	    JNICALL IsMethodSynthetic, (jclass clazz, 
                                        jmethodID method, 
                                        jboolean *isSyntheticPtr))
  // check for NULL arguments
  NULL_CHECK(isSyntheticPtr, JVMDI_ERROR_NULL_POINTER);
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);

  (*isSyntheticPtr) = m->is_synthetic();


  JVMDI_RETURN(JVMDI_ERROR_NONE); 
  //JVMDI_RETURN(JVMDI_ERROR_NOT_IMPLEMENTED);
JVMDI_END


void JvmdiInternal::set_info2_hooks(JVMDI_Interface_1 *jvmdi_interface) {
  jvmdi_interface->GetMethodName = GetMethodName;
  jvmdi_interface->GetMethodDeclaringClass = GetMethodDeclaringClass;
  jvmdi_interface->GetMethodModifiers = GetMethodModifiers;
  jvmdi_interface->GetMaxStack = GetMaxStack;
  jvmdi_interface->GetMaxLocals = GetMaxLocals;
  jvmdi_interface->GetArgumentsSize = GetArgumentsSize;
  jvmdi_interface->GetLineNumberTable = GetLineNumberTable;
  jvmdi_interface->GetMethodLocation = GetMethodLocation;
  jvmdi_interface->GetLocalVariableTable = GetLocalVariableTable;
  jvmdi_interface->GetExceptionHandlerTable = GetExceptionHandlerTable;
  jvmdi_interface->GetThrownExceptions = GetThrownExceptions;
  jvmdi_interface->GetBytecodes = GetBytecodes;
  jvmdi_interface->IsMethodNative = IsMethodNative;
  jvmdi_interface->IsMethodSynthetic = IsMethodSynthetic;
}
