#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodKlass.cpp	1.94 03/01/23 12:13:33 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_methodKlass.cpp.incl"

klassOop methodKlass::create_klass(TRAPS) {
  methodKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}


int methodKlass::oop_size(oop obj) const {
  assert(obj->is_method(), "must be method oop");
  return methodOop(obj)->object_size();
}


bool methodKlass::oop_is_parsable(oop obj) const {
  assert(obj->is_method(), "must be method oop");
  return methodOop(obj)->object_is_parsable();
}


methodOop methodKlass::allocate(int byte_code_size, AccessFlags access_flags, int compressed_line_number_size, int localvariable_table_length, int checked_exceptions_length, TRAPS) {
  int size = methodOopDesc::object_size(byte_code_size, access_flags.is_native(), compressed_line_number_size, localvariable_table_length, checked_exceptions_length);
  KlassHandle h_k(THREAD, as_klassOop());
  methodOop m = (methodOop)CollectedHeap::permanent_obj_allocate(h_k, size, CHECK_0);
  assert(!m->is_parsable(), "not expecting parsability yet.");
  m->set_access_flags(access_flags);
  m->set_method_size(size);
  m->set_code_size(byte_code_size);
  m->set_parameter_info(0);
  m->set_name_index(0);
  m->set_signature_index(0);
  m->set_fingerprint(CONST64(0));
#ifdef CC_INTERP
  m->set_result_index(T_VOID);
#endif
  m->set_constants(NULL);
  m->set_max_stack(0);
  m->set_max_locals(0);
#ifdef COMPILER2
  m->set_method_data(NULL);
  m->set_interpreter_throwout_count(0);
#endif
  m->set_vtable_index(-1);  
  m->init_code();
  if (access_flags.is_native()) {
    m->clear_native_function();
    m->set_signature_handler(NULL);
  }
  m->set_interpreter_entry(NULL);
  m->set_exception_table(NULL);
  m->set_inlined_tables_length(checked_exceptions_length, compressed_line_number_size, localvariable_table_length); // updates access flags as well
  NOT_CORE(NOT_PRODUCT(m->set_compiled_invocation_count(0);))
  COMPILER2_ONLY(m->set_interpreter_invocation_count(0);)
  NOT_CORE(m->invocation_counter()->reset();)
  NOT_CORE(m->backedge_counter()->reset();)
  m->clear_number_of_breakpoints();
  m->alloc_bci_histogram();
  assert(m->is_parsable(), "must be parsable here.");
  assert(m->size() == size, "wrong size for object");
  return m;
}


void methodKlass::oop_follow_contents(oop obj) {
  assert (obj->is_method(), "object must be method");
  methodOop m = methodOop(obj);
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::methodKlassObj never moves.
  MarkSweep::mark_and_push(m->adr_exception_table());
  MarkSweep::mark_and_push(m->adr_constants());
#ifdef COMPILER2
  if (m->method_data() != NULL) {
    MarkSweep::mark_and_push(m->adr_method_data());
  }
#endif // COMPILER2
}


int methodKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert (obj->is_method(), "object must be method");
  methodOop m = methodOop(obj);
  // Get size before changing pointers. 
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::methodKlassObj never moves
  blk->do_oop(m->adr_exception_table());
  blk->do_oop(m->adr_constants());
#ifdef COMPILER2
  if (m->method_data() != NULL) {
    blk->do_oop(m->adr_method_data());
  }
#endif // COMPILER2
  return size;
}


int methodKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert (obj->is_method(), "object must be method");
  methodOop m = methodOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::methodKlassObj never moves.
  oop* adr;
  adr = m->adr_exception_table();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = m->adr_constants();
  if (mr.contains(adr)) blk->do_oop(adr);
#ifdef COMPILER2
  if (m->method_data() != NULL) {
    adr = m->adr_method_data();
    if (mr.contains(adr)) blk->do_oop(adr);
  }
#endif // COMPILER2
  return size;
}


int methodKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_method(), "should be method");
  methodOop m = methodOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();  
  // Performance tweak: We skip iterating over the klass pointer since we 
  // know that Universe::methodKlassObj never moves.
  MarkSweep::adjust_pointer(m->adr_exception_table());
  MarkSweep::adjust_pointer(m->adr_constants());
#ifdef COMPILER2
  if (m->method_data() != NULL) {
    MarkSweep::adjust_pointer(m->adr_method_data());
  }
#endif // COMPILER2
  return size;
}

void methodKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_method(), "should be method");
}

#ifndef PRODUCT

// Printing

void methodKlass::oop_print_on(oop obj, outputStream* st) {
  ResourceMark rm;
  assert(obj->is_method(), "must be method");
  Klass::oop_print_on(obj, st);
  methodOop m = methodOop(obj);
  st->print   (" - method holder:     ");    m->method_holder()->print_value_on(st); st->cr();
  st->print   (" - constants:         ");    m->constants()->print_value_on(st); st->cr();
  st->print   (" - access:            0x%x  ", m->access_flags().as_int()); m->access_flags().print_on(st); st->cr();
  st->print   (" - name:              ");    m->name()->print_value_on(st); st->cr();
  st->print   (" - signature:         ");    m->signature()->print_value_on(st); st->cr();
  st->print_cr(" - max stack:         %d",   m->max_stack());
  st->print_cr(" - max locals:        %d",   m->max_locals());
  st->print_cr(" - size of params:    %d",   m->size_of_parameters());
  st->print_cr(" - method size:       %d",   m->method_size());
  st->print_cr(" - vtable index:      %d",   m->vtable_index());
  st->print   (" - exceptions:        ");    m->exception_table()->print_value_on(st); st->cr();
  st->print_cr(" - code size:         %d",   m->code_size());
  st->print_cr(" - code start:        " INTPTR_FORMAT, m->code_base());
  st->print_cr(" - code end (excl):   " INTPTR_FORMAT, m->code_base() + m->code_size());
#ifdef COMPILER2
  if (m->method_data() != NULL) {
    st->print_cr(" - method data:       " INTPTR_FORMAT, m->method_data());
  }
#endif // COMPILER2
  st->print_cr(" - checked ex length: %d",   m->checked_exceptions_length());
  if (m->checked_exceptions_length() > 0) {
    CheckedExceptionElement* table = m->checked_exceptions_start();
    st->print_cr(" - checked ex start:  " INTPTR_FORMAT, table);
    if (Verbose) {
      for (int i = 0; i < m->checked_exceptions_length(); i++) {
        st->print_cr("   - throws %s", m->constants()->printable_name_at(table[i].class_cp_index));
      }
    }
  }
  if (m->has_linenumber_table()) {
    u_char* table = m->compressed_linenumber_table();
    st->print_cr(" - linenumber start:  " INTPTR_FORMAT, table);
    if (Verbose) {
      CompressedLineNumberReadStream stream(table);
      while (stream.read_pair()) {
        st->print_cr("   - line %d: %d", stream.line(), stream.bci());
      }
    }
  }
  st->print_cr(" - localvar length:   %d",   m->localvariable_table_length());
  if (m->localvariable_table_length() > 0) {
    LocalVariableTableElement* table = m->localvariable_table_start();
    st->print_cr(" - localvar start:    " INTPTR_FORMAT, table);
    if (Verbose) {
      for (int i = 0; i < m->localvariable_table_length(); i++) {
        int bci = table[i].start_bci;
        int len = table[i].length;
        const char* name = m->constants()->printable_name_at(table[i].name_cp_index);
        const char* desc = m->constants()->printable_name_at(table[i].descriptor_cp_index);
        int slot = table[i].slot;
        st->print_cr("   - %s %s bci=%d len=%d slot=%d", desc, name, bci, len, slot);
      }
    }
  }
#ifndef CORE
  if (m->code() != NULL) {
    st->print   (" - compiled code: ");
    m->code()->print_value_on(st);
    st->cr();
  }
#endif
}


void methodKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_method(), "must be method");
  Klass::oop_print_value_on(obj, st);
  methodOop m = methodOop(obj);
  st->print(" ");
  m->name()->print_value_on(st);
  st->print(" ");
  m->signature()->print_value_on(st);
  st->print(" in ");
  m->method_holder()->print_value_on(st);
  if (WizardMode) st->print("[%d,%d]", m->size_of_parameters(), m->max_locals());
  if (WizardMode && m->code() != NULL) st->print(" ((nmethod*)%p)", m->code());
}


const char* methodKlass::internal_name() const {
  return "{method}";
}


// Verification

void methodKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_method(), "object must be method");
  methodOop m = methodOop(obj);
  guarantee(m->is_perm(),                                  "should be in permspace");
  guarantee(m->name()->is_perm(),                          "should be in permspace");
  guarantee(m->name()->is_symbol(),                        "should be symbol");
  guarantee(m->signature()->is_perm(),                     "should be in permspace");
  guarantee(m->signature()->is_symbol(),                   "should be symbol");
  guarantee(m->constants()->is_perm(),                     "should be in permspace");
  guarantee(m->constants()->is_constantPool(),             "should be constant pool");
  guarantee(m->exception_table()->is_perm(),               "should be in permspace");
  guarantee(m->exception_table()->is_typeArray(),          "should be type array");
#ifdef COMPILER2
  methodDataOop method_data = m->method_data();
  guarantee(method_data == NULL || method_data->is_perm(), "should be in permspace");
  guarantee(method_data == NULL || method_data->is_methodData(), "should be method data");
#endif // COMPILER2
  address m_end = (address)((oop*) m + m->size());
  address compressed_table_start = m->inlined_table_start();
  guarantee(compressed_table_start <= m_end, "invalid method layout");
  address compressed_table_end = compressed_table_start;
  // Verify line number table
  if (m->access_flags().has_linenumber_table()) {
    CompressedLineNumberReadStream stream(m->compressed_linenumber_table());
    while (stream.read_pair()) {
      guarantee(stream.bci() >= 0 && stream.bci() <= m->code_size(), "invalid bci in line number table");
    }
    compressed_table_end += stream.position();
  }
  guarantee(compressed_table_end <= m_end, "invalid method layout");
  // Verify checked exceptions and local variable tables
  if (m->access_flags().has_checked_exceptions()) {
    u2* addr = m->checked_exceptions_length_addr();
    guarantee(*addr > 0 && (address) addr >= compressed_table_end && (address) addr < m_end, "invalid method layout");
  }
  if (m->access_flags().has_localvariable_table()) {
    u2* addr = m->localvariable_table_length_addr();
    guarantee(*addr > 0 && (address) addr >= compressed_table_end && (address) addr < m_end, "invalid method layout");
  }
  // Check compressed_table_end relative to uncompressed_table_start
  u2* uncompressed_table_start;
  if (m->access_flags().has_localvariable_table()) {
    uncompressed_table_start = (u2*) m->localvariable_table_start();
  } else {
    if (m->access_flags().has_checked_exceptions()) {
      uncompressed_table_start = (u2*) m->checked_exceptions_start();
    } else {
      uncompressed_table_start = (u2*) m_end;
    }
  }
  int gap = (intptr_t) uncompressed_table_start - (intptr_t) compressed_table_end;
  int max_gap = align_object_size(1)*BytesPerWord;
  guarantee(gap >= 0 && gap < max_gap, "invalid method layout");
}

#endif // PRODUCT

