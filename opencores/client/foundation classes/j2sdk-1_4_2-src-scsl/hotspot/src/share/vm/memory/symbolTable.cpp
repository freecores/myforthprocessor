#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)symbolTable.cpp	1.54 03/01/23 12:10:18 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_symbolTable.cpp.incl"

// --------------------------------------------------------------------------
// Hash function

// We originally used hashpjw, but hash P(31) gives just as good results
// and is slighly faster. We would like a hash function that looks at every
// character, since package names have large common prefixes, and also because
// hash_or_fail does error checking while iterating.

// hash P(31) from Kernighan & Ritchie

static int hash(const char* s, int len) {
  unsigned h = 0;
  while (len-- > 0) {
    h = 31*h + (unsigned) *s;
    s++;
  }
  h %= SymbolTable::symbol_table_size;
  assert(h>=0 && h < SymbolTable::symbol_table_size, "Illegal hash value");
  return h;
}

// --------------------------------------------------------------------------
// Symbol table entry

symbolOop symbolTableBucket::lookup(const char* name, int len) {
  for (symbolOop symbol = entry(); symbol; symbol = symbol->next()) {
    if (symbol->equals(name, len)) {
      return symbol;
    }
  }
  return NULL;
}

// --------------------------------------------------------------------------
// symbolTable

symbolTableBucket* SymbolTable::_buckets;

void SymbolTable::initialize() {  
  _buckets = NEW_C_HEAP_ARRAY(symbolTableBucket, symbol_table_size);
  // Called on startup, no locking needed
  for (int index = 0; index < symbol_table_size; index++) {
    _buckets[index].clear();
  }
}


// We take care not to be blocking while holding the SymbolTable_lock. Otherwise, the
// system might deadlock, since the symboltable is used during compilation (VM_thread)
// The lock free synchronization is simplified by the fact that we do not delete entries
// in the symbol table during normal execution (only during safepoints).
symbolOop SymbolTable::lookup(const char* name, int len, TRAPS) {  
  int hashValue = hash(name, len);
  symbolTableBucket* bucket = bucketFor(hashValue);

  symbolOop s = bucket->lookup(name, len);

  // Found
  if (s != NULL) return s;
  
  // Otherwise, add to symbol to table
  return basic_add((u1*)name, len, hashValue, CHECK_0);
}

//%%TEMPORARY FUNCTION, needed for preloading classes in signatures when compiling. 
symbolOop SymbolTable::probe(char* name, int len) {
  MutexLocker ml(SymbolTable_lock);

  int hashValue = hash(name, len);
  symbolTableBucket* bucket = bucketFor(hashValue);

  return bucket->lookup(name, len);
}

#ifdef ASSERT
class StableMemoryChecker : public StackObj {
  enum { _bufsize = wordSize*4 };

  address _region;
  jint    _size;
  u1      _save_buf[_bufsize];

  int sample(u1* save_buf) {
    if (_size <= _bufsize) {
      memcpy(save_buf, _region, _size);
      return _size;
    } else {
      // copy head and tail
      memcpy(&save_buf[0],          _region,                      _bufsize/2);
      memcpy(&save_buf[_bufsize/2], _region + _size - _bufsize/2, _bufsize/2);
      return (_bufsize/2)*2;
    }
  }

 public:
  StableMemoryChecker(const void* region, jint size) {
    _region = (address) region;
    _size   = size;
    sample(_save_buf);
  }

  bool verify() {
    u1 check_buf[sizeof(_save_buf)];
    int check_size = sample(check_buf);
    return (0 == memcmp(_save_buf, check_buf, check_size));
  }

  void set_region(const void* region) { _region = (address) region; }
};
#endif

symbolOop SymbolTable::basic_add(u1 *name, int len, int hashValue, TRAPS) {  
  assert(!Universe::heap()->is_in_reserved(name) || GC_locker::is_active(), "proposed name of symbol must be stable");

  symbolKlass* sk  = (symbolKlass*) Universe::symbolKlassObj()->klass_part();
  symbolOop s_oop = sk->allocate_symbol(name, len, CHECK_0);
  symbolHandle sym (THREAD, s_oop);

  // Allocation must be done before grapping the SymbolTable_lock lock
  MutexLocker ml(SymbolTable_lock, THREAD);

  assert(sym->equals((char*)name, len), "symbol must be properly initialized");
  // Since look-up was done lock-free, we need to check if another thread beat us in the race to insert the symbol.  
  symbolTableBucket* bucket = bucketFor(hashValue);

  symbolOop test = bucket->lookup((char*)name, len); // calls lookup(u1*, int)
  if (test != NULL) {
    // Entry already added
    return test;
  }  

  sym->set_next(bucket->entry());
  bucket->set_entry(sym());

  return sym();
}


// GC support

void SymbolTable::unlink() {
  // Readers of the symbol table are unlocked, so we should only be
  // removing entries at a safepoint.
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint")
  for (symbolTableBucket* bucket = firstBucket(); bucket <= lastBucket(); bucket++) {
    for (symbolOop* p = bucket->entry_addr(); *p != NULL;) {
      symbolOop symbol = *p;
      if (symbol->is_gc_marked()) {
        p = symbol->next_addr();
      } else {
        *p = symbol->next();
      }
    }
  }
}

void SymbolTable::unlink_for_cms(CMSIsAliveClosure* isAlive) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint")
  for (symbolTableBucket* bucket = firstBucket();
       bucket <= lastBucket(); bucket++) {
    for (symbolOop* p = bucket->entry_addr(); *p != NULL;) {
      symbolOop symbol = *p;
      if (isAlive->do_object_b(symbol)) {
        p = symbol->next_addr();
      } else {
        // remove otherwise unreachable symbol
        *p = symbol->next();
      }
    }
  }
}



void SymbolTable::oops_do(OopClosure* f) {
  for (symbolTableBucket* bucket = firstBucket(); bucket <= lastBucket(); bucket++) {
    f->do_oop((oop*) bucket->entry_addr());
  }
}


#ifndef PRODUCT

void SymbolTable::verify() {
  for (int index = 0; index < symbol_table_size; index++) {
    _buckets[index].verify(index);
  }
}

#endif


//-----------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT


void symbolTableBucket::verify(int index) {
  for (symbolOop s = entry(); s != NULL; s = s->next()) {
    guarantee(s != NULL, "symbol is NULL");
    s->verify();
    guarantee(s->is_perm(), "symbol not in permspace");
    int h = hash((char*)s->bytes(), s->utf8_length());
    guarantee(h % SymbolTable::symbol_table_size == index, "wrong index in symbol table");
  }
}


void SymbolTable::print() {  
  ResourceMark rm;

  for (int i = 0; i < symbol_table_size; i++) {
    symbolOop entry = _buckets[i].entry();
    while(entry != NULL) {
      tty->print("%d : ", i);
      entry->print();
      tty->cr();
      entry = entry->next();
    }
  }    
}


void SymbolTable::print_histogram() {
  MutexLocker ml(SymbolTable_lock);
  const int results_length = 100;
  int results[results_length];
  int i,j;
  
  // initialize results to zero
  for (j = 0; j < results_length; j++) {
    results[j] = 0;
  }

  int total = 0;
  int max_symbols = 0;
  int out_of_range = 0;
  for (i = 0; i < symbol_table_size; i++) {
    symbolOop curr = _buckets[i].entry();
    if (curr == NULL) {
      continue;
    }
    int counter = curr->utf8_length();
    total += counter;
    if (counter < results_length) {
      results[counter]++;
    } else {
      out_of_range++;
    }
    max_symbols = MAX2(max_symbols, counter);
  }
  tty->print_cr("Symbol Table:");
  tty->print_cr("%8s %5d", "Total  ", total);
  tty->print_cr("%8s %5d", "Maximum", max_symbols);
  tty->print_cr("%8s %3.2f", "Average",
	  ((float) total / (float) symbol_table_size));
  tty->print_cr("%s", "Histogram:");
  tty->print_cr(" %s %29s", "Length", "Number chains that length");
  for (i = 0; i < results_length; i++) {
    if (results[i] > 0) {
      tty->print_cr("%6d %10d", i, results[i]);
    }
  }
  int line_length = 70;    
  tty->print_cr("%s %30s", " Length", "Number chains that length");
  for (i = 0; i < results_length; i++) {
    if (results[i] > 0) {
      tty->print("%4d", i);
      for (j = 0; (j < results[i]) && (j < line_length);  j++) {
        tty->print("%1s", "*");
      }
      if (j == line_length) {
        tty->print("%1s", "+");
      }
      tty->cr();
    }
  }  
  tty->print_cr(" %s %d: %d\n", "Number chains longer than",
	            results_length, out_of_range);
}

#endif // PRODUCT

static int hash_string(jchar* s, int len) {
  unsigned h = 0;
  while (len-- > 0) {
    h = 31*h + (unsigned) *s;
    s++;
  }
  h %= StringTable::string_table_size;
  assert(h>=0 && h < StringTable::string_table_size, "Illegal hash value");
  return h;
}

inline void stringTableEntry::oops_do(OopClosure* f) {
  f->do_oop((oop*) &_literal_string);
}

stringTableBucket StringTable::buckets[StringTable::string_table_size];
stringTableEntry* StringTable::free_list        = NULL;
stringTableEntry* StringTable::first_free_entry = NULL;
stringTableEntry* StringTable::end_block        = NULL;

void StringTable::initialize() {
  // Called on startup, no locking needed
  for (int index = 0; index < string_table_size; index++) {
    buckets[index].clear();
  }
}

void StringTable::unlink() {
  // Readers of the string table are unlocked, so we should only be
  // removing entries at a safepoint.
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint")
  for (stringTableBucket* bucket = firstBucket(); bucket <= lastBucket(); bucket++) {
    for (stringTableEntry** p = bucket->entry_addr(); *p != NULL;) {
      stringTableEntry* entry = *p;
      assert(entry->literal_string() != NULL, "just checking");
      if (entry->literal_string()->is_gc_marked()) {
	// Is this one of calls those necessary only for verification? (DLD)
        entry->oops_do(&MarkSweep::follow_root_closure);
        p = entry->next_addr();
      } else {
        *p = entry->next();
        entry->set_next(free_list);
        free_list = entry;
      }
    }
  }
}

void StringTable::unlink_for_cms(CMSIsAliveClosure* isAlive) {
  assert(SafepointSynchronize::is_at_safepoint(), "must be at safepoint")
  for (stringTableBucket* bucket = firstBucket();
       bucket <= lastBucket(); bucket++) {
    for (stringTableEntry** p = bucket->entry_addr(); *p != NULL;) {
      stringTableEntry* entry = *p;
      assert(entry->literal_string() != NULL, "just checking");
      if (isAlive->do_object_b(entry->literal_string())) {
        p = entry->next_addr();
      } else {
        *p = entry->next();
        entry->set_next(free_list);
        free_list = entry;
      }
    }
  }
}


void StringTable::oops_do(OopClosure* f) {
  for (stringTableBucket* bucket = firstBucket(); bucket <= lastBucket(); bucket++) {
    stringTableEntry* l_next = NULL;
    for (stringTableEntry* l = bucket->entry(); l != NULL; l = l_next) {
      // compute next here
      l_next = l->next();
      assert(l->literal_string() != NULL, "just checking");
      l->oops_do(f);
    }
  }
}

oop stringTableBucket::lookup(jchar* name, int len) {
  for (stringTableEntry* l = entry(); l; l = l->next()) {
    if (java_lang_String::equals(l->literal_string(), name, len)) {
      return l->literal_string();
    }
  }
  return NULL;
}

oop StringTable::basic_add(Handle string_or_null, jchar* name, int len, int hashValue, TRAPS) {  
  debug_only(StableMemoryChecker smc(name, len * sizeof(name[0])));
  assert(!Universe::heap()->is_in_reserved(name) || GC_locker::is_active(), "proposed name of symbol must be stable");

  Handle string;
  // try to reuse the string if possible
  if (!string_or_null.is_null() && string_or_null()->is_perm()) {
    string = string_or_null;
  } else {
    string = java_lang_String::create_tenured_from_unicode(name, len, CHECK_0);
  }

  // Allocation must be done before grapping the SymbolTable_lock lock
  MutexLocker ml(StringTable_lock, THREAD);

  assert(java_lang_String::equals(string(), name, len), "string must be properly initialized");
  // Since look-up was done lock-free, we need to check if another thread beat us in the race to insert the symbol.  
  stringTableBucket* bucket = bucketFor(hashValue);

  oop test = bucket->lookup(name, len); // calls lookup(u1*, int)
  if (test != NULL) {
    // Entry already added
    return test;
  }  

  stringTableEntry* entry;
  
  if (free_list) {
    entry = free_list;
    free_list = free_list->next();
  } else {
    const int block_size = 500;
    if (first_free_entry == end_block) {
      first_free_entry = NEW_C_HEAP_ARRAY(stringTableEntry, block_size);
      end_block = first_free_entry + block_size;
    }
    entry = first_free_entry++;
  }

  entry->set_literal_string(string());   // clears literal string field
  entry->set_next(bucket->entry());
  bucket->set_entry(entry);

  return string();
}


oop StringTable::lookup(symbolOop symbol) {
  ResourceMark rm;
  int length;
  jchar* chars = symbol->as_unicode(length);
  int hashValue = hash_string(chars, length);
  stringTableBucket* bucket = bucketFor(hashValue);
  return bucket->lookup(chars, length);
}


oop StringTable::intern(Handle string_or_null, jchar* name, int len, TRAPS)
{
  int hashValue = hash_string(name, len);
  stringTableBucket* bucket = bucketFor(hashValue);

  oop string = bucket->lookup(name, len);

  // Found
  if (string != NULL) return string;
  
  // Otherwise, add to symbol to table
  return basic_add(string_or_null, name, len, hashValue, CHECK_0);  
}

oop StringTable::intern(symbolOop symbol, TRAPS)
{
  if (symbol == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length;
  jchar* chars = symbol->as_unicode(length);
  Handle string;
  oop result = intern(string, chars, length, CHECK_0);
  return result;
}

oop StringTable::intern(oop string, TRAPS)
{
  if (string == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length;
  Handle h_string (THREAD, string);
  jchar* chars = java_lang_String::as_unicode_string(string, length);
  oop result = intern(h_string, chars, length, CHECK_0);
  return result;
}

oop StringTable::intern(char* utf8_string, TRAPS)
{
  if (utf8_string == NULL) return NULL;
  ResourceMark rm(THREAD);
  int length = UTF8::unicode_length(utf8_string);
  jchar* chars = NEW_RESOURCE_ARRAY(jchar, length);
  UTF8::convert_to_unicode(utf8_string, chars, length);
  Handle string;
  oop result = intern(string, chars, length, CHECK_0);
  return result;
}

//-----------------------------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void stringTableBucket::verify(int index) {
  for (stringTableEntry* l = entry(); l != NULL; l = l->next()) {
    l->verify(index);
  }
}

void stringTableEntry::verify(int index) {
  guarantee(literal_string() != NULL, "interned string is NULL");
  guarantee(literal_string()->is_perm(), "interned string not in permspace");
  int length;
  jchar* chars = java_lang_String::as_unicode_string(literal_string(), length);
  int h = hash_string(chars, length);
  guarantee(h % StringTable::string_table_size == index, "wrong index in string table");
}


void StringTable::print() {  
  ResourceMark rm;

  for (int i = 0; i < string_table_size; i++) {
    stringTableEntry* entry = buckets[i].entry();
    while(entry != NULL) {
      tty->print("%d : ", i);
      entry->literal_string()->print();
      tty->cr();
      entry = entry->next();
    }
  }    
}

void StringTable::verify() {
  for (int index = 0; index < string_table_size; index++) {
    buckets[index].verify(index);
  }
}

#endif // PRODUCT
