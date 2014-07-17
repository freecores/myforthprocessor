#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os.hpp	1.154 03/04/04 11:04:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// os defines the interface to operating system; this includes traditional
// OS services (time, I/O) as well as other functionality with system-
// dependent code.

typedef void (*dll_func)(...);

class Thread;
class JavaThread;
class Event;
class DLL;
class FileHandle;

// %%%%% Moved ThreadState, START_FN, OSThread to new osThread.hpp. -- Rose

// Platform-independent error return values from OS functions
enum OSReturn {
  OS_OK         =  0,        // Operation was successful
  OS_ERR        = -1,        // Operation failed 
  OS_INTRPT     = -2,        // Operation was interrupted
  OS_TIMEOUT    = -3,        // Operation timed out
  OS_NOMEM      = -5,        // Operation failed for lack of memory
  OS_NORESOURCE = -6         // Operation failed for lack of nonmemory resource
};

enum ThreadPriority {        // JLS 20.20.1-3
  NoPriority       = -1,     // Initial non-priority value
  MinPriority      =  1,     // Minimum priority
  NormPriority     =  5,     // Normal (non-daemon) priority
  NearMaxPriority  =  9,     // High priority, used for VMThread
  MaxPriority      = 10      // Highest priority, used for WatcherThread
                             // and SuspendCheckerThread, ensures that
                             // VMThread doesn't starve profiler
};


// HotSpot integration note:

// Struct containing system dependent properties. 
// Matches struct props_md_t in interpreter.h 1.213 99/05/19 from JDK1.3 beta H build

typedef struct {
    char *sysclasspath;
    char *library_path;
    char *dll_dir;
    char *java_home;
    char *ext_dirs;
    char *endorsed_dirs;
} props_md_t;


// Typedef for structured exception handling support 
typedef void (*java_call_t)(JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread);

class os: AllStatic {
 private:
  static OSThread*          _starting_thread;
  static props_md_t*        _system_properties;
 public:

  static void init(void);			// Called before command line parsing
  static jint init_2(void);                    // Called after command line parsing
  static void exit(void);

  // File names are case-insensitive on windows only
  // Override me as needed
  static inline int    file_name_strcmp(const char* s1, const char* s2);

  static bool getenv(const char* name, char* buffer, int len);

  static jlong  javaTimeMillis();
  
  // Returns the elapsed time in seconds since the vm started.
  static double elapsedTime();
  
  // Interface to the win32 performance counter
  static jlong elapsed_counter();
  static jlong elapsed_frequency();

  // Interface for detecting multiprocessor system
  static bool is_MP();
  static julong physical_memory();
  static julong allocatable_physical_memory(julong size);

  // Returns the number of CPUs this process is currently allowed to run on.
  // Note that on some OSes this can change dynamically.
  static int active_processor_count();

  // Bind processes to processors.
  //     This is a two step procedure:
  //     first you generate a distribution of processes to processors,
  //     then you bind processes according to that distribution.
  // Compute a distribution for number of processes to processors.
  //    Stores the processor id's into the distribution array argument. 
  //    Returns true if it worked, false if it didn't.
  static bool distribute_processes(uint length, uint* distribution);
  // Binds the current process to a processor.
  //    Returns true if it worked, false if it didn't.
  static bool bind_to_processor(uint processor_id);

  // Interface for stack banging (predetect possible stack overflow for
  // exception processing)  There are guard pages, and above that shadow
  // pages for stack overflow checking.
  static inline bool uses_stack_guard_pages();
  static inline bool allocate_stack_guard_pages();
  static inline void bang_stack_shadow_pages();
  static bool stack_shadow_pages_available(Thread *thread);

  // OS interface to Virtual Memory
  static int   vm_page_size();
  static char* reserve_memory(size_t bytes);
  static bool   commit_memory(char* addr, size_t bytes);
  static bool   uncommit_memory(char* addr, size_t bytes);
  static bool   release_memory(char* addr, size_t bytes);
  static bool   protect_memory(char* addr, size_t bytes);
  static bool   guard_memory(char* addr, size_t bytes);
  static bool   unguard_memory(char* addr, size_t bytes);
  static char*  non_memory_address_word();
  static char*  reserve_memory_special(size_t size);
  static bool   set_mpss_range(char* addr, size_t size, size_t align);

  // threads

  enum ThreadType {
    vm_thread,
    cms_thread,
    pgc_thread,
    java_thread,
    compiler_thread,
    watcher_thread,
    suspend_checker_thread,
    gc_thread
  };

  static bool create_thread(Thread* thread,
                            ThreadType thr_type,
                            size_t stack_size = 0);
  static bool create_main_thread(Thread* thread);
  static bool create_attached_thread(Thread* thread);
  static void pd_start_thread(Thread* thread);
  static void start_thread(Thread* thread);

  static void initialize_thread();
  static void free_thread(OSThread* osthread);
  static void exit_thread();

  // thread id on Linux/64bit is 64bit, on Windows and Solaris, it's 32bit
  static intx current_thread_id();
  static int current_process_id();
  static int sleep(Thread* thread, jlong ms, bool interruptable);
  static void infinite_sleep(); // never returns, use with CAUTION
  static void yield();        // Yields to all threads with same priority
  static void yield_all(int attempts = 0); // Yields to all other threads including lower priority
  static void loop_breaker(int attempts);  // called from within tight loops to possibly influence time-sharing
  static OSReturn set_priority(Thread* thread, ThreadPriority priority);
  static OSReturn get_priority(Thread* thread, ThreadPriority& priority);
  static bool boost_priority  (Thread* thread, int max_java_priority);
  static bool deflate_priority(Thread* thread, int min_java_priority);

  static void interrupt(Thread* thread);
  static bool is_interrupted(Thread* thread, bool clear_interrupted);

  static int pd_suspend_thread(Thread* thread, bool _fence);
  static int pd_resume_thread(Thread* thread);
  static int pd_self_suspend_thread(Thread* thread);
  
  static ExtendedPC fetch_top_frame(Thread* thread, intptr_t** sp, intptr_t** fp);
  NOT_CORE(static bool set_thread_pc_and_resume(JavaThread* thread, ExtendedPC old_addr, ExtendedPC new_addr);)
  static ExtendedPC get_thread_pc(Thread *thread);
  static void breakpoint();

  static address current_stack_pointer();
  static address current_stack_base();
  static size_t current_stack_size();

  static int message_box(const char* title, const char* message);
  static char* do_you_want_to_debug(const char* message);

  // os::exit() is merged with vm_exit()
  // static void exit(int num);

  // Terminate with an error.  Default is to generate a core file on platforms
  // that support such things.
  static void abort(bool dump_core = true);

  // Reading directories.
  static DIR*           opendir(const char* dirname);
  static int            readdir_buf_size(const char *path);
  static struct dirent* readdir(DIR* dirp, dirent* dbuf);
  static int            closedir(DIR* dirp);
  static char*          asctime(const struct tm *tm, char *buf, int buflen);
  static struct tm*     localtime(const time_t *clock, struct tm *res);
  
  // Fatal error handling

  // install/deinstall a simple signal handler during fatal error handling to print
  // out some basic information in case the fatal error handler crashes.
  static void set_exception_handler_during_fatal_error(int sig, int is_install);

  // generate fatal error report
  static void report_fatal_error(outputStream *st, address pc, int sig);

  // fatal error handler
  static void handle_unexpected_exception(Thread * thread, int sig, address pc, void * extra_info);

  // print heap before abort 
  static void print_vm_state_before_abort(outputStream *st); 

  static void handle_recursive_fatal_error(int sig);

  // Dynamic library extension
  static const char*    dll_file_extension();

  static const char*    get_temp_directory();

  // Map PC to symbol
  // Return true if addr is mapped to a function, also being returned is
  // the function name (symname) and offset from its base address.
  // Return false if the name of the function that contains addr cannot be
  // determined. However, if addr can be mapped to a nearest function, both 
  // the function name and offset will still be returned in symname and 
  // offset. Otherwise, symname will be set to NULL and offset be -1 upon 
  // return.
  // Note: When mapping PC to symbol, the operating system only searches for
  // runtime structures, which may only contain global symbols. Therefore,
  // it's not unusual that symname has the wrong function name, even though
  // this procedure returns true. Treat the result as a guess.
  static bool dll_address_to_function_name(address addr, const char** symname,
                                           int* offset);
  static const char* dll_address_to_function_name(address addr) {
     const char * name;
     if (dll_address_to_function_name(addr, &name, NULL)) return name;
     else return NULL;
  }

  // Map pc to a dynamic library, returns the full path the library file
  static const char* dll_address_to_library_name(address addr);

  // Maps PC to source file name and line number. Returns -1 if failed.
  static int dll_address_to_line_number(address addr, char ** filename);

  // Find out whether the pc is in the static code for jvm.dll/libjvm.so.
  static bool dll_address_is_in_vm(address addr);

  // Dump information about all open dynamic libraries; currently it prints out a
  // list of library names and their base addresses. Output format is slightly
  // different on different platforms.
  static void dump_dll_info(outputStream *st);

  // returns a string to describe the exception/signal; returns NULL if no information
  // is available.
  static const char * os::exception_name(int exception_code);

  // Returns native Java library, loads if necessary
  static void*    native_java_library();

  // Fills in path to jvm.dll/libjvm.so (this info used to find hpi).
  static void     jvm_path(char *buf, jint buflen);

  // JNI names
  static void     print_jni_name_prefix_on(outputStream* st, int args_size);
  static void     print_jni_name_suffix_on(outputStream* st, int args_size);

  // File conventions
  static inline const char* file_separator();
  static inline const char* line_separator();
  static inline const char* path_separator();

  // Properties
  static props_md_t* get_system_properties();

  // IO operations, non-JVM_ version.
  static int stat(const char* path, struct stat* sbuf);

  // Thread Local Storage
  static int   allocate_thread_local_storage();
  static void  thread_local_storage_at_put(int index, void* value);
  static void* thread_local_storage_at(int index);
  static void  free_thread_local_storage(int index);

  // General allocation (must be MT-safe)
  static void* malloc  (size_t size);
  static void* realloc (void *memblock, size_t size);
  static void  free    (void *memblock);
  static bool  check_heap(bool force = false);      // verify C heap integrity
  static char* strdup(const char *);  // Like strdup
#ifndef PRODUCT
  static int  num_mallocs;            // # of calls to malloc/realloc
  static size_t  alloc_bytes;         // # of bytes allocated
  static int  num_frees;              // # of calls to free
#endif

  // Printing 64 bit integers
  static inline const char* jlong_format_specifier();
  static inline const char* julong_format_specifier();

  // Support for signals (see JVM_RaiseSignal, JVM_RegisterSignal)
  static void  signal_init();
  static void  signal_init_pd();
  static void  signal_notify(int signal_number);
  static void* signal(int signal_number, void* handler);
  static void  signal_raise(int signal_number);
  static int   signal_wait();
  static int   signal_lookup();
  static void* user_handler();
  static void  terminate_signal_thread();
  static int   sigexitnum_pd();

  // random number generation
  static long random();                    // return 32bit pseudorandom number
  static void init_random(long initval);   // initialize random sequence

  // Structured OS Exception support
  static void os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread);

  // jvmpi
  static bool thread_is_running(JavaThread* tp);
  static jlong thread_cpu_time();

  // Hook for os specific jvm options that we don't want to abort on seeing
  static bool obsolete_option(JavaVMOption *option);

  // Platform dependent stuff
  #include "incls/_os_pd.hpp.incl"

  // debugging support (mostly used by debug.cpp)
  static bool find(address pc) PRODUCT_RETURN0; // OS specific function to make sense out of an address
  
  static bool dont_yield();                     // when true, JVM_Yield() is nop
  static void print_statistics();

  // Thread priority helpers (implemented in OS-specific part)
  static OSReturn set_native_priority(Thread* thread, int native_prio);
  static OSReturn get_native_priority(Thread* thread, int* priority_ptr);
  static int java_to_os_priority[MaxPriority + 1];
  // Hint to the underlying OS that a task switch would not be good.
  // Void return because it's a hint and can fail.
  static void hint_no_preempt();

 protected:
  static long _rand_seed;                   // seed for random number generator

  static bool set_boot_path(props_md_t *sprops, char fileSep, char pathSep);
};


