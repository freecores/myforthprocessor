#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)globals.cpp	1.30 03/01/23 12:22:32 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_globals.cpp.incl"


RUNTIME_FLAGS(MATERIALIZE_DEVELOPER_FLAG, MATERIALIZE_PD_DEVELOPER_FLAG, MATERIALIZE_PRODUCT_FLAG, MATERIALIZE_PD_PRODUCT_FLAG, MATERIALIZE_DIAGNOSTIC_FLAG)

bool Flag::is_unlocker() const {
  return strcmp(name, "UnlockDiagnosticVMOptions") == 0;
}

bool Flag::is_unlocked() const {
  if (strcmp(kind, "{diagnostic}") == 0) {
    return UnlockDiagnosticVMOptions;
  } else {
    return true;
  }
}

void Flag::print_on(outputStream* st) {
  st->print("%5s %-35s %c= ", type, name, (was_set ? ':' : ' '));
  if (is_bool())  st->print("%-8s", get_bool() ? "true" : "false");
  if (is_intx())  st->print("%-16ld", get_intx());
  if (is_uintx()) st->print("%-16ul", get_uintx());
  if (is_ccstr()) {
    const char* cp = get_ccstr();
    const char* eol;
    while ((eol = strchr(cp, '\n')) != NULL) {
      int llen = eol - cp;
      char buffer[4096];
      strncpy(buffer, cp, llen);
      buffer[llen] = '\0';
      st->print("%s", buffer);
      st->cr();
      cp = eol+1;
      st->print("%5s %-35s+= ", "", name);
    }
    st->print("%-16s", cp);
  }
  st->print(" %s", kind);
  st->cr();
}

#define RUNTIME_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{product}" },
#define RUNTIME_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{pd product}" },
#define RUNTIME_DIAGNOSTIC_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{diagnostic}" },
#ifdef PRODUCT
  #define RUNTIME_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define RUNTIME_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define RUNTIME_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "" },
  #define RUNTIME_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{pd}" }, 
#endif

#define C1_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C1 product}" },
#define C1_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C1 pd product}" },
#ifdef PRODUCT
  #define C1_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define C1_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define C1_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C1}" },
  #define C1_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C1 pd}" }, 
#endif


#define C2_PRODUCT_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C2 product}" },
#define C2_PD_PRODUCT_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C2 pd product}" },
#ifdef PRODUCT
  #define C2_DEVELOP_FLAG_STRUCT(type, name, value, doc) /* flag is constant */ 
  #define C2_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     /* flag is constant */ 
#else
  #define C2_DEVELOP_FLAG_STRUCT(type, name, value, doc) { #type, XSTR(name), &name, "{C2}" },
  #define C2_PD_DEVELOP_FLAG_STRUCT(type, name, doc)     { #type, XSTR(name), &name, "{C2 pd}" }, 
#endif


static Flag flagTable[] = {
 RUNTIME_FLAGS(RUNTIME_DEVELOP_FLAG_STRUCT, RUNTIME_PD_DEVELOP_FLAG_STRUCT, RUNTIME_PRODUCT_FLAG_STRUCT, RUNTIME_PD_PRODUCT_FLAG_STRUCT, RUNTIME_DIAGNOSTIC_FLAG_STRUCT)
#ifdef COMPILER1
 C1_FLAGS(C1_DEVELOP_FLAG_STRUCT, C1_PD_DEVELOP_FLAG_STRUCT, C1_PRODUCT_FLAG_STRUCT, C1_PD_PRODUCT_FLAG_STRUCT)
#endif
#ifdef COMPILER2
 C2_FLAGS(C2_DEVELOP_FLAG_STRUCT, C2_PD_DEVELOP_FLAG_STRUCT, C2_PRODUCT_FLAG_STRUCT, C2_PD_PRODUCT_FLAG_STRUCT)
#endif
 {0, NULL, NULL}
};

Flag* Flag::flags = flagTable;
size_t Flag::numFlags = (sizeof(flagTable) / sizeof(Flag));

inline bool str_equal(const char* s, char* q, size_t len) {
  // s is null terminated, q is not!
  if (strlen(s) != (unsigned int) len) return false;
  return strncmp(s, q, len) == 0;
}

static Flag* find_flag(char* name, size_t length) {
  for (Flag* current = &flagTable[0]; current->name; current++) {
    if (str_equal(current->name, name, length)) {
      if (!(current->is_unlocked() || current->is_unlocker())) {
	// disable use of diagnostic flags until they are unlocked
	return NULL;
      }
      return current;
    }
  }
  return NULL;
}



bool DebugFlags::boolAt(char* name, size_t len, bool* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_bool()) return NULL;   
  *value = result->get_bool();
  return true;
}

bool DebugFlags::boolAtPut(char* name, size_t len, bool* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_bool()) return NULL;   
  bool old_value = result->get_bool();
  result->set_bool(*value); 
  *value = old_value;
  result->was_set = true;
  return true;
}

bool DebugFlags::intxAt(char* name, size_t len, intx* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_intx()) return NULL;   
  *value = result->get_intx();
  return true;
}

bool DebugFlags::intxAtPut(char* name, size_t len, intx* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return NULL;
  if (!result->is_intx()) return NULL;   
  intx old_value = result->get_intx();
  result->set_intx(*value); 
  *value = old_value;
  result->was_set = true;
  return true;
}

bool DebugFlags::uintxAt(char* name, size_t len, uintx* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_uintx()) return false;
  *value = result->get_uintx();
  return true;
}

bool DebugFlags::uintxAtPut(char* name, size_t len, uintx* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_uintx()) return false;
  uintx old_value = result->get_uintx();
  result->set_uintx(*value);
  *value = old_value;
  result->was_set = true;
  return true;
}

bool DebugFlags::ccstrAt(char* name, size_t len, ccstr* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_ccstr()) return false;
  *value = result->get_ccstr();
  return true;
}

// Contract:  Flag will make private copy of the incoming value.
// Outgoing value is always malloc-ed, and caller MUST call free.
bool DebugFlags::ccstrAtPut(char* name, size_t len, ccstr* value) {
  Flag* result = find_flag(name, len);
  if (result == NULL) return false;
  if (!result->is_ccstr()) return false;
  ccstr old_value = result->get_ccstr();
  char* new_value = NEW_C_HEAP_ARRAY(char, strlen(*value)+1);
  strcpy(new_value, *value);
  result->set_ccstr(new_value);
  if (!result->was_set) {
    // Prior value is NOT heap allocated, but was a literal constant.
    char* old_value_to_free = NEW_C_HEAP_ARRAY(char, strlen(old_value)+1);
    strcpy(old_value_to_free, old_value);
    old_value = old_value_to_free;
  }
  *value = old_value;
  result->was_set = true;
  return true;
}


#ifndef PRODUCT


void DebugFlags::verify() {
  assert(Arguments::check_vm_args_consistency(), "Some flag settings conflict");
}

extern "C" int compare_flags(const void* void_a, const void* void_b) {
  return strcmp((*((Flag**) void_a))->name, (*((Flag**) void_b))->name);
}

void DebugFlags::printFlags() {
  // Print the flags sorted by name
  // note: this method is called before the thread structure is in place
  //       which means resource allocation cannot be used.

  // Compute size
  int length= 0;
  while (flagTable[length].name != NULL) length++;

  // Sort
  Flag** array = NEW_C_HEAP_ARRAY(Flag*, length);
  for (int index = 0; index < length; index++) {
    array[index] = &flagTable[index];
  }
  qsort(array, length, sizeof(Flag*), compare_flags);

  // Print
  tty->print_cr("[Global flags]");
  for (int i = 0; i < length; i++) {
    if (array[i]->is_unlocked()) {
      array[i]->print_on(tty);
    }
  }
  FREE_C_HEAP_ARRAY(Flag*, array);
}

#endif

