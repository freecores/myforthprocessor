#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)symbolTable.hpp	1.31 03/01/23 12:10:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The symbol table holds all symbolOops and corresponding interned strings.
// symbolOops and literal strings should be canonicalized.
//
// The interned strings are created lazily.
//
// It is implemented as an open hash table with a fixed number of buckets.
//
// %note:
//  - symbolTableEntrys are allocated in blocks to reduce the space overhead.

class CMSIsAliveClosure;

class symbolTableBucket: public CHeapObj {
  friend class VMStructs;
 private:
  // Instance variable
  symbolOop _entry;
 public:
  // Accessing
  void clear()                { _entry = NULL; }
  symbolOop entry() const     { return _entry; }
  void set_entry(symbolOop l) { _entry = l; }  
  symbolOop* entry_addr()     { return &_entry; }

  // Lookup name in bucket
  symbolOop lookup(const char* name, int len);

  // Debugging
  void verify(int index)  PRODUCT_RETURN;
  int  length()           PRODUCT_RETURN0;
};

class SymbolTable: public AllStatic {
  friend class VMStructs;
 public:
  // Table size
  enum {
    symbol_table_size = 20011
  };
  // Initialization
  static void initialize();
  // Symbol lookup
  static symbolOop lookup(const char* name, int len, TRAPS);

  //%%TEMPORARY FUNCTION, needed for preloading classes in signatures when compiling. 
  // Returns the symbol is already present in symbol table, otherwise NULL
  // NO ALLOCATION IS GUARANTEED!
  static symbolOop probe(char* name, int len);

  // GC support
  //   Delete pointers to otherwise-unreachable symbols.
  static void unlink(); 
  static void unlink_for_cms(CMSIsAliveClosure* cl); 

  // Invoke "f->do_oop" on the locations of all oops in the table.
  static void oops_do(OopClosure* f);

  // Histogram
  static void print_histogram()     PRODUCT_RETURN;
  // Debugging
  static void verify()              PRODUCT_RETURN;
  static void print()               PRODUCT_RETURN;

 private:
  // Instance variables
  static symbolTableBucket* _buckets;

  // Adding elements    
  static symbolOop basic_add(u1* name, int len, int hashValue, TRAPS);

  // Bucket handling
  static symbolTableBucket* bucketFor(int hashValue) {
    assert(hashValue >= 0 && hashValue < symbol_table_size, "illegal hash value");
    return &_buckets[hashValue]; 
  }
  static symbolTableBucket* firstBucket() { return &_buckets[0]; }
  static symbolTableBucket* lastBucket()  { return &_buckets[symbol_table_size-1]; }
};

class stringTableEntry: public CHeapObj {
 private:
  oop _literal_string;        // corresponding java.lang.String instance
  stringTableEntry* _next;    // link to next element in bucket
 public:
  stringTableEntry* next() const        { return _next; }
  void set_next(stringTableEntry* next) { _next = next; }

  stringTableEntry** next_addr()        { return &_next; }

  // Literal string
  oop literal_string() { return _literal_string; }
  void set_literal_string(oop s) { _literal_string = s; }

  void oops_do(OopClosure* f);

  // Debugging
  void verify(int index)          PRODUCT_RETURN;   
};


class stringTableBucket: public CHeapObj {
 private:
  // Instance variable
  stringTableEntry* _entry;
 public:
  // Accessing
  void clear()                        { _entry = NULL; }
  stringTableEntry* entry() const     { return _entry; }
  void set_entry(stringTableEntry* l) { _entry = l; }  

  stringTableEntry** entry_addr()     { return &_entry; }

  // Lookup name in bucket
  oop lookup(jchar* str, int len);

  // Debugging
  void verify(int index)  PRODUCT_RETURN;
};


class StringTable: public AllStatic {
 private:
  static oop intern(Handle string_or_null, jchar* chars, int length, TRAPS);
  static oop basic_add(Handle string_or_null, jchar* name, int len, int hashValue, TRAPS);

 public:
  // Table size
  enum {
    string_table_size = 1009
  };
  // Initialization
  static void initialize();

  // Probing
  static oop lookup(symbolOop symbol);

  // Interning
  static oop intern(symbolOop symbol, TRAPS);
  static oop intern(oop string, TRAPS);
  static oop intern(char *utf8_string, TRAPS);

  // GC support
  //   Delete pointers to otherwise-unreachable symbols.
  static void unlink(); 
  static void unlink_for_cms(CMSIsAliveClosure* cl); 
  
  // Invoke "f->do_oop" on the locations of all oops in the table.
  static void oops_do(OopClosure* f);

  // Debugging
  static void verify()              PRODUCT_RETURN;
  static void print()               PRODUCT_RETURN;

 private:
  // Instance variables
  static stringTableBucket buckets[string_table_size];
  static stringTableEntry* free_list;
  static stringTableEntry* first_free_entry;
  static stringTableEntry* end_block;

  // Bucket handling
  static stringTableBucket* bucketFor(int hashValue) {
    assert(hashValue>=0 && hashValue<string_table_size, "illegal hash value");
    return &buckets[hashValue]; 
  }
  static stringTableBucket* firstBucket() { return &buckets[0]; }
  static stringTableBucket* lastBucket()  { return &buckets[string_table_size-1]; }
};
