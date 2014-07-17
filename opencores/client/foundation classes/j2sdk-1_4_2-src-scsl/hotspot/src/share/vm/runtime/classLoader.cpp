#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)classLoader.cpp	1.149 03/01/23 12:21:38 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_classLoader.cpp.incl"
#include <sys/stat.h>


// Entry points in zip.dll for loading zip/jar file entries

typedef void * * (JNICALL *ZipOpen_t)(const char *name, char **pmsg);
typedef void * * (JNICALL *FindEntry_t)(jzfile *zip, const char *name, jint *sizeP, jint *nameLen);
typedef jboolean (JNICALL *ReadEntry_t)(jzfile *zip, jzentry *entry, unsigned char *buf, char *namebuf);
typedef jboolean (JNICALL *ReadMappedEntry_t)(jzfile *zip, jzentry *entry, unsigned char **buf, char *namebuf);
typedef void * * (JNICALL *GetNextEntry_t)(jzfile *zip, jint n);

static ZipOpen_t         ZipOpen            = NULL;
static FindEntry_t       FindEntry          = NULL;
static ReadEntry_t       ReadEntry          = NULL;
static ReadMappedEntry_t ReadMappedEntry    = NULL;
static GetNextEntry_t    GetNextEntry       = NULL;
static canonicalize_fn_t CanonicalizeEntry  = NULL;


// Globals

elapsedTimer    ClassLoader::_accumulated_time;
PerfCounter*    ClassLoader::_perf_accumulated_time = NULL;
PerfCounter*    ClassLoader::_perf_classes_loaded = NULL;
PerfCounter*    ClassLoader::_perf_classes_unloaded = NULL;
PerfCounter*    ClassLoader::_perf_classbytes_loaded = NULL;
PerfCounter*    ClassLoader::_perf_classbytes_unloaded = NULL;
ClassPathEntry* ClassLoader::_first_entry         = NULL;
PackageInfo*    ClassLoader::_package_hash_table[ClassLoader::package_hash_table_size];
int             ClassLoader::_nof_package_entries = 0;


ClassPathEntry::ClassPathEntry() {
  set_next(NULL);
}


ClassPathDirEntry::ClassPathDirEntry(char* dir) : ClassPathEntry() {
  _dir = dir;
}


ClassFileStream* ClassPathDirEntry::open_stream(const char* name) {
  // construct full path name
  char path[JVM_MAXPATHLEN];
  if (jio_snprintf(path, sizeof(path), "%s%s%s", _dir, os::file_separator(), name) == -1) {
    return NULL;
  }
  // check if file exists
  struct stat st;
  if (os::stat(path, &st) == 0) {
    // found file, open it
    int file_handle = hpi::open(path, 0, 0);
    if (file_handle != -1) {
      // read contents into resource array
      u1* buffer = NEW_RESOURCE_ARRAY(u1, st.st_size);
      size_t num_read = hpi::read(file_handle, (char*) buffer, st.st_size);
      // close file
      hpi::close(file_handle);
      // construct ClassFileStream
      if (num_read == (size_t)st.st_size) {
        return new ClassFileStream(buffer, st.st_size, _dir);    // Resource allocated
      }
    }
  }
  return NULL;
}


ClassPathZipEntry::ClassPathZipEntry(jzfile* zip, char* zip_name) : ClassPathEntry() {
  _zip = zip;
  _zip_name = zip_name;
}


ClassFileStream* ClassPathZipEntry::open_stream(const char* name) {
  // enable call to C land
  JavaThread* thread = JavaThread::current();
  StateSaver nsv(thread);
  ThreadToNativeFromVM ttn(thread);
  HandleMark hm(thread);
  // check whether zip archive contains name
  jint filesize, name_len;
  jzentry* entry = (*FindEntry)(_zip, name, &filesize, &name_len);
  if (entry == NULL) return NULL;
  u1* buffer;
  char* filename = NEW_RESOURCE_ARRAY(char, name_len + 1);

  // file found, get pointer to class in mmaped jar file.
  if (ReadMappedEntry == NULL ||
      !(*ReadMappedEntry)(_zip, entry, &buffer, filename)) {
      // mmaped access not available, perhaps due to compression,
      // read contents into resource array
      buffer     = NEW_RESOURCE_ARRAY(u1, filesize);
      if (!(*ReadEntry)(_zip, entry, buffer, filename)) return NULL;
  }
  // return result
  return new ClassFileStream(buffer, filesize, _zip_name);    // Resource allocated
}


void ClassLoader::setup_bootstrap_search_path() {
  assert(_first_entry == NULL, "should not setup bootstrap class search path twice");
  char* sys_class_path = os::strdup(Arguments::sysclasspath());
  if (TraceClassLoading && Verbose) {
    tty->print_cr("[Bootstrap loader class path=%s]", sys_class_path);
  }

  ClassPathEntry* last_entry = NULL;
  int len = (int)strlen(sys_class_path);
  int end = 0;

  // Iterate over class path entries
  for (int start = 0; start < len; start = end) {
    while (sys_class_path[end] && sys_class_path[end] != os::path_separator()[0]) {
      end++;
    }
    char* path = NEW_C_HEAP_ARRAY(char, end-start+1);
    strncpy(path, &sys_class_path[start], end-start);
    path[end-start] = '\0';
    // path now contains an entry
    struct stat st;
    if (os::stat(path, &st) == 0) {      
      // File or directory found
      ClassPathEntry* new_entry = NULL;
      if ((st.st_mode & S_IFREG) == S_IFREG) {
        // Regular file, should be a zip file
        // Canonicalized filename
        char canonical_path[JVM_MAXPATHLEN];
        assert(!is_init_completed(), "sanity check. Just to make sure the exception code below will work");
        if (!get_canonical_path(path, canonical_path, JVM_MAXPATHLEN)) {
          // This matches the classic VM
          EXCEPTION_MARK;
          THROW_MSG(vmSymbols::java_io_IOException(), "Bad pathname");          
	}
        char* error_msg = NULL;
        jzfile* zip;
        {
          // enable call to C land
          JavaThread* thread = JavaThread::current();
          ThreadToNativeFromVM ttn(thread);
          HandleMark hm(thread);
          zip = (*ZipOpen)(canonical_path, &error_msg);
        }
        if (zip != NULL && error_msg == NULL) {
          new_entry = new ClassPathZipEntry(zip, path);
          if (TraceClassLoading) {
            tty->print_cr("[Opened %s]", path);
          }
        }
      } else {
        // Directory
        new_entry = new ClassPathDirEntry(path);
        if (TraceClassLoading) {
          tty->print_cr("[Path %s]", path);
        }
      }      

      // Add new entry to linked list
      if (new_entry != NULL) {
        if (last_entry == NULL) {
          _first_entry = last_entry = new_entry;
        } else {
          last_entry->set_next(new_entry);
          last_entry = new_entry;
        }
      }
    }
    while (sys_class_path[end] == os::path_separator()[0]) {
      end++;
    }
  }
}


void ClassLoader::load_zip_library() {
  assert(ZipOpen == NULL, "should not load zip library twice");
  // First make sure native library is loaded
  os::native_java_library();
  // Load zip library
  char path[JVM_MAXPATHLEN];
  char ebuf[1024];
  hpi::dll_build_name(path, sizeof(path), os::get_system_properties()->dll_dir, "zip");
  void* handle = hpi::dll_load(path, ebuf, sizeof ebuf);
  if (handle == NULL) {
    vm_exit_during_initialization("Unable to load ZIP library", path);
  }
  // Lookup zip entry points
  ZipOpen      = CAST_TO_FN_PTR(ZipOpen_t, hpi::dll_lookup(handle, "ZIP_Open"));
  FindEntry    = CAST_TO_FN_PTR(FindEntry_t, hpi::dll_lookup(handle, "ZIP_FindEntry"));
  ReadEntry    = CAST_TO_FN_PTR(ReadEntry_t, hpi::dll_lookup(handle, "ZIP_ReadEntry"));
  ReadMappedEntry = CAST_TO_FN_PTR(ReadMappedEntry_t, hpi::dll_lookup(handle, "ZIP_ReadMappedEntry"));
#ifndef PRODUCT
  GetNextEntry = CAST_TO_FN_PTR(GetNextEntry_t, hpi::dll_lookup(handle, "ZIP_GetNextEntry"));
#endif
  if (ZipOpen == NULL || FindEntry == NULL || ReadEntry == NULL) {
    vm_exit_during_initialization("Corrupted ZIP library", path);
  }

  // Lookup canonicalize entry in libjava.dll  
  void *javalib_handle = os::native_java_library();
  CanonicalizeEntry = CAST_TO_FN_PTR(canonicalize_fn_t, hpi::dll_lookup(javalib_handle, "Canonicalize"));
  // This lookup only works on 1.3. Do not check for non-null here
}


unsigned int ClassLoader::hash(const char *s, int n) {
  unsigned int val = 0;
  while (--n >= 0) {
    val = *s++ + 31 * val;
  }
  return val % package_hash_table_size;
}


class PackageInfo: public CHeapObj {
 public:
  char *pkgname;       // Package name
  char *filename;      // Directory or JAR file loaded from
  PackageInfo *next;   // Next entry in hash table
};


PackageInfo* ClassLoader::lookup_package(const char *pkgname) {
  const char *cp = strrchr(pkgname, '/');
  if (cp != NULL) {
    // Package prefix found
    PackageInfo *pp;
    int n = cp - pkgname + 1;
    for (pp = _package_hash_table[hash(pkgname,n)]; pp != NULL; pp = pp->next) {
      if (strncmp(pkgname, pp->pkgname, n) == 0) {
        return pp;
      }
    }
  }
  return NULL;
}


bool ClassLoader::add_package(const char *pkgname, const char *filename) {
  assert(pkgname != NULL && filename != NULL, "just checking");
  // First check for previously loaded entry
  PackageInfo* pp = lookup_package(pkgname);
  if (pp != NULL) {
    // Existing entry found, check source of package
    if (strcmp(pp->filename, filename) != 0) {
      // Source for package changed, replace existing entry
      char* new_filename = os::strdup(filename);
      if (new_filename == NULL) return false;
      FreeHeap(pp->filename);
      pp->filename = new_filename;
    }
    return true;
  }

  const char *cp = strrchr(pkgname, '/');
  if (cp != NULL) {
    // Package prefix found
    int n = cp - pkgname + 1;

    // Allocate a new entry and fill it
    if ((pp = new PackageInfo()) == NULL) {
      return false;
    }

    if ((pp->pkgname = NEW_C_HEAP_ARRAY(char, n + 1)) == NULL) {
      FreeHeap(pp);
      return false;
    }
    memcpy(pp->pkgname, pkgname, n);
    pp->pkgname[n] = '\0';

    if ((pp->filename = os::strdup(filename)) == NULL) {
      FreeHeap(pp->pkgname);
      FreeHeap(pp);
      return false;
    }
    
    // Insert into hash table
    int i = hash(pkgname,n);
    pp->next = _package_hash_table[i];
    _package_hash_table[i] = pp;
    _nof_package_entries++;
  }
  return true;
}


oop ClassLoader::get_system_package(const char* name, TRAPS) {
  PackageInfo* pp;
  {
    MutexLocker ml(PackageTable_lock, THREAD);
    pp = lookup_package(name);
  }
  if (pp == NULL) {
    return NULL;
  } else {
    Handle p = java_lang_String::create_from_str(pp->filename, THREAD);
    return p();
  }
}


objArrayOop ClassLoader::get_system_packages(TRAPS) {
  ResourceMark rm(THREAD);
  int nof_entries;
  char** packages;
  {
    MutexLocker ml(PackageTable_lock, THREAD);
    // Allocate resource char* array containing package names
    nof_entries = _nof_package_entries;
    if ((packages = NEW_RESOURCE_ARRAY(char*, nof_entries)) == NULL) {
      return NULL;
    }
    int n = 0;
    for (int i = 0; i < package_hash_table_size; i++) {
      for (PackageInfo *pp = _package_hash_table[i]; pp != NULL; pp = pp->next) {
        packages[n++] = pp->pkgname;
      }
    }
    assert(n == nof_entries, "just checking");
  }
  // Allocate objArray and fill with java.lang.String
  objArrayOop r = oopFactory::new_objArray(SystemDictionary::string_klass(), nof_entries, CHECK_0);
  objArrayHandle result(THREAD, r);
  for (int i = 0; i < nof_entries; i++) {
    Handle str = java_lang_String::create_from_str(packages[i], CHECK_0);
    result->obj_at_put(i, str());
  }

  return result();
}


instanceKlassHandle ClassLoader::load_classfile(symbolHandle h_name, TRAPS) {
  VTuneClassLoadMarker clm;
  ResourceMark rm(THREAD);
  EventMark m("loading class %x", h_name());
  ThreadProfilerMark tpm(ThreadProfilerMark::classLoaderRegion);

  stringStream st;
  // st.print() uses too much stack space while handling a StackOverflowError
  // st.print("%s.class", h_name->as_utf8());
  st.print_raw(h_name->as_utf8());
  st.print_raw(".class");
  char* name = st.as_string();

  // Lookup stream for parsing .class file
  ClassFileStream* stream = NULL;
  {
    TraceTime timer(NULL, accumulated_time(), TraceClassLoadingTime);
    PerfTraceTime vmtimer(perf_accumulated_time());
    ClassPathEntry* e = _first_entry;
    while (e != NULL && stream == NULL) {
      stream = e->open_stream(name);
      e = e->next();
    }
  }

  instanceKlassHandle h(THREAD, klassOop(NULL));
  if (stream != NULL) {

    // class file found, parse it
    ClassFileParser parser(stream);
    Handle class_loader;
    Handle protection_domain;
    symbolHandle parsed_name;
    instanceKlassHandle result = parser.parseClassFile(h_name, 
                                                       class_loader, 
                                                       protection_domain, 
                                                       parsed_name,
                                                       CHECK_(h));

    // add to package table
    if (add_package(name, stream->source())) {
      h = result;
    }
  }

  return h;
}

void ClassLoader::notify_class_unloaded(instanceKlass* k) {


  if (UsePerfData) {
    size_t size = compute_class_size(k);
    _perf_classbytes_unloaded->inc(size);
    _perf_classes_unloaded->inc();
  }

  // Clean up C heap
  k->release_C_heap_structures();
  if (TraceClassUnloading) {
    ResourceMark rm;
    tty->print_cr("[Unloading class %s]", k->external_name());
  }
}

void ClassLoader::notify_class_loaded(instanceKlass* k) {

  if (!UsePerfData) return;

  size_t size = compute_class_size(k);
  _perf_classbytes_loaded->inc(size);
  _perf_classes_loaded->inc();

}

size_t ClassLoader::compute_class_size(instanceKlass* k) {
  // lifted from ClassStatistics.do_class(klassOop k)

  size_t class_size = 0;

  class_size += k->as_klassOop()->size();

  if (k->oop_is_instance()) {
    class_size += k->methods()->size();
    class_size += k->constants()->size();
    class_size += k->local_interfaces()->size();
    class_size += k->transitive_interfaces()->size();
    // We do not have to count implementors, since we only store one!
    class_size += k->fields()->size();
  }
  return class_size * oopSize;
}

// Production Performance counter name prefix
#define PERF_CL       PERF_RUNTIME ".cl"

void ClassLoader::initialize() {
  // initialize package hash table
  for (int i = 0; i < package_hash_table_size; i++) {
    _package_hash_table[i] = NULL;
  }

  if (UsePerfData) {

    EXCEPTION_MARK;

    // Production Performance counters
    _perf_accumulated_time =
        PerfDataManager::create_counter(PERF_CL ".time",
                                        PerfData::U_Ticks, CHECK);

    _perf_classes_loaded =
        PerfDataManager::create_counter(PERF_CL ".classes.loaded",
                                        PerfData::U_Events, CHECK);

    _perf_classes_unloaded =
        PerfDataManager::create_counter(PERF_CL ".classes.unloaded",
                                        PerfData::U_Events, CHECK);

    _perf_classbytes_loaded =
        PerfDataManager::create_counter(PERF_CL ".bytes.loaded",
                                        PerfData::U_Bytes, CHECK);

    _perf_classbytes_unloaded =
        PerfDataManager::create_counter(PERF_CL ".bytes.unloaded",
                                        PerfData::U_Bytes, CHECK);
  }

  // lookup zip library entry points
  load_zip_library();
  // initialize search path
  setup_bootstrap_search_path();
}


int ClassLoader::compute_Object_vtable() {
  // hardwired for JDK1.2 -- would need to duplicate class file parsing
  // code to determine actual value from file
  int JDK_1_2_Object_vtable_size = RespectFinal ? 5 : 11;
  return JDK_1_2_Object_vtable_size * vtableEntry::size();
}


void classLoader_init() {
  ClassLoader::initialize();
}


bool ClassLoader::get_canonical_path(char* orig, char* out, int len) {
  assert(orig != NULL && out != NULL && len > 0, "bad arguments");        
  if (CanonicalizeEntry != NULL) {
    JNIEnv* env = JavaThread::current()->jni_environment();
    if ((CanonicalizeEntry)(env, hpi::native_path(orig), out, len) < 0) {    
      return false;  
    }    
  } else {
    // On JDK 1.2.2 the Canonicalize does not exist, so just do nothing
    strncpy(out, orig, len);
    out[len - 1] = '\0';    
  }
  return true;
}




#ifndef PRODUCT

// CompileTheWorld
//
// Iterates over all class path entries and forces compilation of all methods
// in all classes found. Currently, only zip/jar archives are searched.
// 
// The classes are loaded by the Java level bootstrap class loader, and the
// initializer is called. If DelayCompilationDuringStartup is true (default),
// the interpreter will run the initialization code. Note that forcing 
// initialization in this way could potentially lead to initialization order
// problems, in which case we could just force the initialization bit to be set.


// We need to iterate over the contents of a zip/jar file, so we replicate the
// jzcell and jzfile definitions from zip_util.h but rename jzfile to real_jzfile,
// since jzfile already has a void* definition.
//
// Note that this is only used in debug mode.
//
// HotSpot integration note:
// Matches zip_util.h 1.14 99/06/01 from jdk1.3 beta H build


// JDK 1.3 version
typedef struct real_jzentry13 { 	/* Zip file entry */
    char *name;	  	  	/* entry name */
    jint time;            	/* modification time */
    jint size;	  	  	/* size of uncompressed data */
    jint csize;  	  	/* size of compressed data (zero if uncompressed) */
    jint crc;		  	/* crc of uncompressed data */
    char *comment;	  	/* optional zip file comment */
    jbyte *extra;	  	/* optional extra data */
    jint pos;	  	  	/* position of LOC header (if negative) or data */
} real_jzentry13;

typedef struct real_jzfile13 {  /* Zip file */
    char *name;	  	        /* zip file name */
    jint refs;		        /* number of active references */
    jint fd;		        /* open file descriptor */
    void *lock;		        /* read lock */
    char *comment; 	        /* zip file comment */
    char *msg;		        /* zip error message */
    void *entries;          	/* array of hash cells */
    jint total;	  	        /* total number of entries */
    unsigned short *table;      /* Hash chain heads: indexes into entries */
    jint tablelen;	        /* number of hash eads */
    real_jzfile13 *next;        /* next zip file in search list */
    jzentry *cache;             /* we cache the most recently freed jzentry */
    /* Information on metadata names in META-INF directory */
    char **metanames;           /* array of meta names (may have null names) */
    jint metacount;	        /* number of slots in metanames array */
    /* If there are any per-entry comments, they are in the comments array */
    char **comments;
} real_jzfile13;

// JDK 1.2 version
typedef struct real_jzentry12 {  /* Zip file entry */
    char *name;                  /* entry name */
    jint time;                   /* modification time */
    jint size;                   /* size of uncompressed data */
    jint csize;                  /* size of compressed data (zero if uncompressed) */
    jint crc;                    /* crc of uncompressed data */
    char *comment;               /* optional zip file comment */
    jbyte *extra;                /* optional extra data */
    jint pos;                    /* position of LOC header (if negative) or data */
    struct real_jzentry12 *next; /* next entry in hash table */
} real_jzentry12;

typedef struct real_jzfile12 {  /* Zip file */
    char *name;                 /* zip file name */
    jint refs;                  /* number of active references */
    jint fd;                    /* open file descriptor */
    void *lock;                 /* read lock */
    char *comment;              /* zip file comment */
    char *msg;                  /* zip error message */
    real_jzentry12 *entries;    /* array of zip entries */
    jint total;                 /* total number of entries */
    real_jzentry12 **table;     /* hash table of entries */
    jint tablelen;              /* number of buckets */
    jzfile *next;               /* next zip file in search list */
} real_jzfile12;


void ClassPathDirEntry::compile_the_world(Handle loader, TRAPS) {
  // For now we only compile all methods in all classes in zip/jar files
  tty->print_cr("CompileTheWorld : Skipped classes in %s", _dir);
  tty->cr();
}


bool ClassPathDirEntry::is_rt_jar() {
  return false;
}

void ClassPathZipEntry::compile_the_world(Handle loader, TRAPS) {
  if (Universe::is_jdk12x_version()) {
    compile_the_world12(loader, THREAD);
  } else {
    compile_the_world13(loader, THREAD);
  }
  if (HAS_PENDING_EXCEPTION) {
    if (PENDING_EXCEPTION->is_a(SystemDictionary::OutOfMemoryError_klass())) {
      CLEAR_PENDING_EXCEPTION;
      tty->print_cr("\nCompileTheWorld : Ran out of memory\n");
      size_t used = Universe::heap()->permanent_used();
      size_t capacity = Universe::heap()->permanent_capacity();
      tty->print_cr("Permanent generation used %dK of %dK", used/K, capacity/K);
      tty->print_cr("Increase size by setting e.g. -XX:MaxPermSize=%dK\n", capacity*2/K);
    } else {
      tty->print_cr("\nCompileTheWorld : Unexpected exception occurred\n");
    }
  }
}

// Version that works for JDK 1.3.x
void ClassPathZipEntry::compile_the_world13(Handle loader, TRAPS) {
  real_jzfile13* zip = (real_jzfile13*) _zip;
  tty->print_cr("CompileTheWorld : Compiling all classes in %s", zip->name);
  tty->cr();
  // Iterate over all entries in zip file
  for (int n = 0; ; n++) {
    real_jzentry13 * ze = (real_jzentry13 *)((*GetNextEntry)(_zip, n));
    if (ze == NULL) break;
    ClassLoader::compile_the_world_in(ze->name, loader, CHECK);
  }
}


// Version that works for JDK 1.2.x
void ClassPathZipEntry::compile_the_world12(Handle loader, TRAPS) {
  real_jzfile12* zip = (real_jzfile12*) _zip;
  tty->print_cr("CompileTheWorld : Compiling all classes in %s", zip->name);
  tty->cr();
  // Iterate over all entries in zip file
  for (int n = 0; ; n++) {
    real_jzentry12 * ze = (real_jzentry12 *)((*GetNextEntry)(_zip, n));
    if (ze == NULL) break;
    ClassLoader::compile_the_world_in(ze->name, loader, CHECK);
  }
}

bool ClassPathZipEntry::is_rt_jar() {
  if (Universe::is_jdk12x_version()) {
    return is_rt_jar12();
  } else {
    return is_rt_jar13();
  }
}

// JDK 1.3 version
bool ClassPathZipEntry::is_rt_jar13() {
  real_jzfile13* zip = (real_jzfile13*) _zip;
  int len = (int)strlen(zip->name);
  // Check whether zip name ends in "rt.jar"
  // This will match other archives named rt.jar as well, but this is
  // only used for debugging.
  return (len >= 6) && (strcasecmp(zip->name + len - 6, "rt.jar") == 0);
}

// JDK 1.2 version
bool ClassPathZipEntry::is_rt_jar12() {
  real_jzfile12* zip = (real_jzfile12*) _zip;
  int len = (int)strlen(zip->name);
  // Check whether zip name ends in "rt.jar"
  // This will match other archives named rt.jar as well, but this is
  // only used for debugging.
  return (len >= 6) && (strcasecmp(zip->name + len - 6, "rt.jar") == 0);
}


void ClassLoader::compile_the_world() {
  EXCEPTION_MARK;
  HandleMark hm(THREAD);
  ResourceMark rm(THREAD);
  // Make sure we don't run with background compilation
  BackgroundCompilation = false;
  // Find bootstrap loader
  Handle system_class_loader (THREAD, SystemDictionary::java_system_loader());
  // Wait for compiler to be initialized
  NOT_CORE(while (!CompileBroker::compiler()->is_initialized()) os::yield();)
  // Iterate over all bootstrap class path entries
  ClassPathEntry* e = _first_entry;
  while (e != NULL) {
    // We stop at rt.jar, unless it is the first bootstrap path entry
    if (e->is_rt_jar() && e != _first_entry) break;
    e->compile_the_world(system_class_loader, CATCH);
    e = e->next();
  }
  tty->print_cr("CompileTheWorld : Done");
  {
    // Print statistics as if before normal exit:
    extern void print_statistics();
    print_statistics();
  }
  vm_exit(0);
}

int ClassLoader::_compile_the_world_counter = 0;

void ClassLoader::compile_the_world_in(char* name, Handle loader, TRAPS) {
#ifndef CORE
  int len = (int)strlen(name);
  if (len > 6 && strcmp(".class", name + len - 6) == 0) {
    // We have a .class file
    char buffer[2048];
    strncpy(buffer, name, len - 6);
    buffer[len-6] = 0;
    // If the file has a period after removing .class, it's not really a
    // valid class file.  The class loader will check everything else.
    if (strchr(buffer, '.') == NULL) {
      _compile_the_world_counter++;
      if (_compile_the_world_counter >= CompileTheWorldStartAt && _compile_the_world_counter <= CompileTheWorldStopAt) {
        // Construct name without extension
        symbolHandle sym = oopFactory::new_symbol_handle(buffer, CHECK);
        // Use loader to load and initialize class
        klassOop ik = SystemDictionary::resolve_or_null(sym, loader, Handle(), THREAD);
        instanceKlassHandle k (THREAD, ik);
        if (k.not_null() && !HAS_PENDING_EXCEPTION) {
          k->initialize(THREAD);
        }
        bool exception_occurred = HAS_PENDING_EXCEPTION;
        CLEAR_PENDING_EXCEPTION;
        if (k.is_null() || (exception_occurred && !CompileTheWorldIgnoreInitErrors)) {
          // If something went wrong (e.g. ExceptionInInitializerError) we skip this class
          tty->print_cr("CompileTheWorld (%d) : Skipping %s", _compile_the_world_counter, buffer);
        } else {
          tty->print_cr("CompileTheWorld (%d) : %s", _compile_the_world_counter, buffer);
          // Preload all classes to get around uncommon traps
          if (CompileTheWorldPreloadClasses) {
            constantPoolKlass::preload_and_initialize_all_classes(k->constants(), THREAD);
            if (HAS_PENDING_EXCEPTION) {
              // If something went wrong in preloading we just ignore it
              CLEAR_PENDING_EXCEPTION;
              tty->print_cr("Preloading failed for (%d) %s", _compile_the_world_counter, buffer);
            }
          }
          // Iterate over all methods in class
          for (int n = 0; n < k->methods()->length(); n++) {
            methodHandle m (THREAD, methodOop(k->methods()->obj_at(n)));
            if (CompilationPolicy::canBeCompiled(m)) {
              // Force compilation           
              CompileBroker::compile_method(m, InvocationEntryBci,
                                            methodHandle(), 0, "CTW", THREAD);
              if (HAS_PENDING_EXCEPTION) {
                CLEAR_PENDING_EXCEPTION;
                tty->print_cr("CompileTheWorld (%d) : Skipping method: %s", _compile_the_world_counter, m->name()->as_C_string());
              }
  	    if (TieredCompilation) {
  	      // Clobber the first compile and force second tier compilation
  	      m->set_code(NULL);
  	      CompileBroker::compile_method(m, InvocationEntryBci,
                                            methodHandle(), 0, "CTW", THREAD);
  	      if (HAS_PENDING_EXCEPTION) {
  		CLEAR_PENDING_EXCEPTION;
  		tty->print_cr("CompileTheWorld (%d) : Skipping method: %s", _compile_the_world_counter, m->name()->as_C_string());
  	      }
  	    }
            }
          }
        }
      }
    }
  }
#endif //CORE
}

#endif //PRODUCT
