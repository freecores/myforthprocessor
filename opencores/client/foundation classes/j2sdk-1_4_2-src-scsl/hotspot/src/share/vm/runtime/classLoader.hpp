#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)classLoader.hpp	1.44 03/01/23 12:21:41 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The VM class loader.


// Class path entry (directory or zip file)

class ClassPathEntry: public CHeapObj {
 private:
  ClassPathEntry* _next;
 public:
  // Next entry in class path
  ClassPathEntry* next()              { return _next; }
  void set_next(ClassPathEntry* next) { _next = next; }
  // Constructor
  ClassPathEntry();
  // Attempt to locate file_name through this class path entry.
  // Returns a class file parsing stream if successfull.
  virtual ClassFileStream* open_stream(const char* name) = 0;
  // Debugging
  NOT_PRODUCT(virtual void compile_the_world(Handle loader, TRAPS) = 0;)
  NOT_PRODUCT(virtual bool is_rt_jar() = 0;)
};


class ClassPathDirEntry: public ClassPathEntry {
 private:
  char* _dir;           // Name of directory
 public:
  ClassPathDirEntry(char* dir);
  ClassFileStream* open_stream(const char* name);
  // Debugging
  NOT_PRODUCT(void compile_the_world(Handle loader, TRAPS);)
  NOT_PRODUCT(bool is_rt_jar();)
};


// Type definitions for zip file and zip file entry
typedef void* jzfile;
typedef void* jzentry;


class ClassPathZipEntry: public ClassPathEntry {
 private:
  jzfile* _zip;        // The zip archive
  char*   _zip_name;   // Name of zip archive
 public:
  ClassPathZipEntry(jzfile* zip, char* zip_name);
  ClassFileStream* open_stream(const char* name);
  // Debugging
  NOT_PRODUCT(void compile_the_world(Handle loader, TRAPS);)
  NOT_PRODUCT(void compile_the_world12(Handle loader, TRAPS);) // JDK 1.2 version
  NOT_PRODUCT(void compile_the_world13(Handle loader, TRAPS);) // JDK 1.3 version
  NOT_PRODUCT(bool is_rt_jar();)
  NOT_PRODUCT(bool is_rt_jar12();)
  NOT_PRODUCT(bool is_rt_jar13();)
};


class PackageInfo;

class ClassLoader: AllStatic {
 public:
  enum SomeConstants {
    package_hash_table_size = 31  // Number of buckets
  };
 private:
  // Active count
  // Accumulated time
  static elapsedTimer  _accumulated_time;

  // Performance counters
  static PerfCounter* _perf_accumulated_time;
  static PerfCounter* _perf_classes_loaded;
  static PerfCounter* _perf_classes_unloaded;
  static PerfCounter* _perf_classbytes_loaded;
  static PerfCounter* _perf_classbytes_unloaded;

  // First entry in linked list of ClassPathEntry instances
  static ClassPathEntry* _first_entry;
  // Hash table used to keep track of loaded packages
  static PackageInfo* _package_hash_table[package_hash_table_size];
  // Number of elements in package table
  static int _nof_package_entries;

  // Hash function
  static unsigned int hash(const char *s, int n);
  // Returns the package file name corresponding to the specified package 
  // or class name, or null if not found.
  static PackageInfo* lookup_package(const char *pkgname);
  // Adds a new package entry for the specified class or package name and
  // corresponding directory or jar file name.
  static bool add_package(const char *pkgname, const char *filename);

  // Initialization
  static void setup_bootstrap_search_path();
  static void load_zip_library();

  // Canonicalizes path names, so strcmp will work properly. This is mainly
  // to avoid confusing the zip library
  static bool get_canonical_path(char* orig, char* out, int len);
 public:
  // Timing
  static elapsedTimer* accumulated_time() { return &_accumulated_time; }
  static PerfCounter* perf_accumulated_time() { return _perf_accumulated_time; }
  static void notify_class_unloaded(instanceKlass* k);
  static void notify_class_loaded(instanceKlass* k);
  static size_t compute_class_size(instanceKlass* k);

  // Load individual .class file
  static instanceKlassHandle load_classfile(symbolHandle h_name, TRAPS);  

  // If the specified package has been loaded by the system, then returns
  // the name of the directory or ZIP file that the package was loaded from.
  // Returns null if the package was not loaded.
  // Note: The specified name can either be the name of a class or package.
  // If a package name is specified, then it must be "/"-separator and also
  // end with a trailing "/".
  static oop get_system_package(const char* name, TRAPS);

  // Returns an array of Java strings representing all of the currently
  // loaded system packages.
  // Note: The package names returned are "/"-separated and end with a
  // trailing "/".
  static objArrayOop get_system_packages(TRAPS);

  // Initialization
  static void initialize();
  static int compute_Object_vtable();

  // Debugging
  // Force compilation of all methods in all classes in bootstrap class path (stress test)
#ifndef PRODUCT
 private:
  static int _compile_the_world_counter;
 public:
  static void compile_the_world();
  static void compile_the_world_in(char* name, Handle loader, TRAPS);
  static int  compile_the_world_counter() { return _compile_the_world_counter; }
#endif //PRODUCT
};
