#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os.cpp	1.122 03/01/23 12:24:03 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_os.cpp.incl"

# include <signal.h>


OSThread*   os::_starting_thread    = NULL;
props_md_t* os::_system_properties  = NULL;
long        os::_rand_seed          = 1;
#ifndef PRODUCT
int os::num_mallocs = 0;            // # of calls to malloc/realloc
size_t os::alloc_bytes = 0;         // # of bytes allocated
int os::num_frees = 0;              // # of calls to free
#endif


OSReturn os::set_priority(Thread* thread, ThreadPriority p) {
#ifdef ASSERT
  if (!(!thread->is_Java_thread() ||
         Thread::current() == thread  ||
         Threads_lock->owned_by_self()
#ifndef CORE
         || thread->is_Compiler_thread()
#endif
	)) {
    assert(false, "possibility of dangling Thread pointer");
  }
#endif

#if 0
  // return OK is we're not using thread priorities
  if (!UseThreadPriorities) return OS_OK;
#endif

  if (p >= MinPriority && p <= MaxPriority) {
    int priority = java_to_os_priority[p];
    return set_native_priority(thread, priority);
  } else {
    assert(false, "Should not happen");
    return OS_ERR;
  }
}


OSReturn os::get_priority(Thread* thread, ThreadPriority& priority) {
  int p;
  int os_prio;
  OSReturn ret = get_native_priority(thread, &os_prio);
  if (ret != OS_OK) return ret;

  for (p = MaxPriority; p > MinPriority && java_to_os_priority[p] > os_prio; p--) ;

  // In the presence of BackgroundCompilation and StarvationMonitor, the VM thread may 
  // have OS priority that does not map exactly to any legal Java priority level 
  // (VMThread priority is incremented/decremented in OS priority units - there
  // is more of these than in Java, for instance 128 in Solaris Threads). Kind of a 
  // special case - return the closest Java priority that is no greater than the VM 
  // thread OS priority.
  //
  // !!!!! StarvationMonitor has been removed from the system.  The above comment
  // is out of date.

#if 0
  assert((java_to_os_priority[p] == os_prio), 
         "priority not found (may happen only if thread uses native OS calls to manipulate priority");
#endif
  priority = (ThreadPriority)p;
  return OS_OK;
}


// --------------------- sun.misc.Signal (optional) ---------------------


// SIGBREAK is sent by the keyboard to query the VM state
#ifndef SIGBREAK
#define SIGBREAK SIGQUIT
#endif

// sigexitnum_pd is a platform-specific special signal used for terminating the Signal thread.


static void signal_thread_entry(JavaThread* thread, TRAPS) {
  os::set_priority(thread, MaxPriority);  
  while (true) {
    int sig;
    {
      ThreadWaitTransition twt(thread);
      sig = os::signal_wait();
    }
    if (sig == os::sigexitnum_pd()) {
       // Terminate the signal thread
       return;
    }

    switch (sig) {
      case SIGBREAK: {
        // Print stack traces
        VM_PrintThreads op;
        VMThread::execute(&op);
        VM_FindDeadlocks op1;
        VMThread::execute(&op1);
        if (PrintClassHistogram) {
          VM_GC_HeapInspection op1;
          VMThread::execute(&op1);
        }
        jvmpi::post_dump_event();
        break;
      }
      default: {        
        // Dispatch the signal to java
        HandleMark hm(THREAD);
        klassOop k = SystemDictionary::resolve_or_null(vmSymbolHandles::sun_misc_Signal(), THREAD);
        KlassHandle klass (THREAD, k);
        if (klass.not_null()) {
          JavaValue result(T_VOID);
          JavaCallArguments args;
          args.push_int(sig);
          JavaCalls::call_static(
            &result,
            klass, 
            vmSymbolHandles::dispatch_name(), 
            vmSymbolHandles::int_void_signature(),
            &args,
            THREAD
          );
        }
        CLEAR_PENDING_EXCEPTION;
      }
    }
  }
}


void os::signal_init() {
  if (!ReduceSignalUsage) {
    // Setup JavaThread for processing signals
    EXCEPTION_MARK;
    klassOop k = SystemDictionary::resolve_or_fail(vmSymbolHandles::java_lang_Thread(), true, CHECK);
    instanceKlassHandle klass (THREAD, k);
    instanceHandle thread_oop = klass->allocate_instance_handle(CHECK);
    Handle string = java_lang_String::create_from_str("Signal Dispatcher", CHECK);    

    // Initialize thread_oop to put it into the system threadGroup    
    Handle thread_group (THREAD, Universe::system_thread_group());
    JavaValue result(T_VOID);
    JavaCalls::call_special(&result, thread_oop, 
                           klass, 
                           vmSymbolHandles::object_initializer_name(), 
                           vmSymbolHandles::threadgroup_string_void_signature(), 
                           thread_group, 
                           string, 
                           CHECK);  
    
    os::signal_init_pd();
    { MutexLocker mu(Threads_lock);
      JavaThread* signal_thread = new JavaThread(&signal_thread_entry);

      // At this point it may be possible that no osthread was created for the
      // JavaThread due to lack of memory. We would have to throw an exception
      // in that case. However, since this must work and we don not allow
      // exceptions anyway, just print an error and abort.
      if (signal_thread->osthread() == NULL) {
         tty->print_cr("Fatal: Unable to create signal thread (may be due to resource limit)");
         os::abort();
      }
     
      java_lang_Thread::set_thread(thread_oop(), signal_thread);      
      java_lang_Thread::set_priority(thread_oop(), MaxPriority);
      java_lang_Thread::set_daemon(thread_oop());

      signal_thread->set_threadObj(thread_oop());
      Threads::add(signal_thread);  
      Thread::start(signal_thread);
    }
    // Handle ^BREAK
    os::signal(SIGBREAK, os::user_handler());
  }
}


void os::terminate_signal_thread() {
  signal_notify(sigexitnum_pd());
}


// --------------------- loading libraries ---------------------

typedef jint (JNICALL *JNI_OnLoad_t)(JavaVM *, void *);
extern struct JavaVM_ main_vm;

static void* _native_java_library = NULL;

void* os::native_java_library() {
  if (_native_java_library == NULL) {
    char buffer[JVM_MAXPATHLEN];
    char ebuf[1024];

    // Try to load verify dll first. In 1.3 java dll depends on it and is not always
    // able to find it when the loading executable is outside the JDK. 
    // In order to keep working with 1.2 we ignore any loading errors.
    hpi::dll_build_name(buffer, sizeof(buffer), os::get_system_properties()->dll_dir, "verify");
    hpi::dll_load(buffer, ebuf, sizeof(ebuf));

    // Load java dll
    hpi::dll_build_name(buffer, sizeof(buffer), os::get_system_properties()->dll_dir, "java");
    _native_java_library = hpi::dll_load(buffer, ebuf, sizeof(ebuf));
    if (_native_java_library == NULL) {
      vm_exit_during_initialization("Unable to load native library", ebuf);
    }
    // The JNI_OnLoad handling is normally done by method load in java.lang.ClassLoader$NativeLibrary,
    // but the VM loads the base library explicitly so we have to check for JNI_OnLoad as well
    const char *onLoadSymbols[] = JNI_ONLOAD_SYMBOLS;
    JNI_OnLoad_t JNI_OnLoad = CAST_TO_FN_PTR(JNI_OnLoad_t, hpi::dll_lookup(_native_java_library, onLoadSymbols[0]));
    if (JNI_OnLoad != NULL) {
      JavaThread* thread = JavaThread::current();
      StateSaver nsv(thread); // we must have post Java state set while in native
      ThreadToNativeFromVM ttn(thread);
      HandleMark hm(thread);
      jint ver = (*JNI_OnLoad)(&main_vm, NULL);
      if (!Threads::is_supported_jni_version_including_1_1(ver)) {
        vm_exit_during_initialization("Unsupported JNI version");
      }
    }
  }
  return _native_java_library;
}

// --------------------- heap allocation utilities ---------------------

char *os::strdup(const char *str) {
  size_t size = strlen(str);
  char *dup_str = (char *)malloc(size + 1);
  if (dup_str == NULL) return NULL;
  strcpy(dup_str, str);
  return dup_str;
}


//// A version of malloc which supports pointer debugging.
#ifdef ASSERT
size_t largest_seen_so_far = 0;
#endif

#ifdef ASSERT
#define space_before             (MallocCushion + sizeof(double))
#define space_after              MallocCushion
#define size_addr_from_base(p)   (size_t*)(p + space_before - sizeof(size_t))
#define size_addr_from_obj(p)    ((size_t*)p - 1)
// MallocCushion: size of extra cushion allocated around objects with +UseMallocOnly
// NB: cannot be debug variable, because these aren't set from the command line until
// *after* the first few allocs already happened
#define MallocCushion            16 
#else
#define space_before             0
#define space_after              0
#define size_addr_from_base(p)   should not use w/o ASSERT
#define size_addr_from_obj(p)    should not use w/o ASSERT
#define MallocCushion            0 
#endif
#define paranoid                 0  /* only set to 1 if you suspect checking code has bug */

#ifdef ASSERT
inline size_t get_size(void* obj) {
  size_t size = *size_addr_from_obj(obj);
  if (size < 0 || size > largest_seen_so_far) 
    fatal2("free: size field of object #%p was overwritten (%lu)", obj, size);
  return size;
}

u_char* find_cushion_backwards(u_char* start) {
  u_char* p = start; 
  while (p[ 0] != badResourceValue || p[-1] != badResourceValue ||
         p[-2] != badResourceValue || p[-3] != badResourceValue) p--;
  // ok, we have four consecutive marker bytes; find start
  u_char* q = p - 4;
  while (*q == badResourceValue) q--;
  return q + 1;
}

u_char* find_cushion_forwards(u_char* start) {
  u_char* p = start; 
  while (p[0] != badResourceValue || p[1] != badResourceValue ||
         p[2] != badResourceValue || p[3] != badResourceValue) p++;
  // ok, we have four consecutive marker bytes; find end of cushion
  u_char* q = p + 4;
  while (*q == badResourceValue) q++;
  return q - MallocCushion;
}

void print_neighbor_blocks(void* ptr) {
  // find block allocated before ptr (not entirely crash-proof)
  if (MallocCushion < 4) {
    tty->print_cr("### cannot find previous block (MallocCushion < 4)");
    return;
  }
  u_char* start_of_this_block = (u_char*)ptr - space_before;
  u_char* end_of_prev_block_data = start_of_this_block - space_after -1;
  // look for cushion in front of prev. block
  u_char* start_of_prev_block = find_cushion_backwards(end_of_prev_block_data);
  ptrdiff_t size = *size_addr_from_base(start_of_prev_block);
  u_char* obj = start_of_prev_block + space_before;
  if (size <= 0 || (size_t)size > largest_seen_so_far) {
    // start is bad; mayhave been confused by OS data inbetween objects
    // search one more backwards
    start_of_prev_block = find_cushion_backwards(start_of_prev_block);
    size = *size_addr_from_base(start_of_prev_block);
    obj = start_of_prev_block + space_before;  
  }

  if (start_of_prev_block + space_before + size + space_after == start_of_this_block) {
    tty->print_cr("### previous object: %p (%ld bytes)", obj, size);
  } else {
    tty->print_cr("### previous object (not sure if correct): %p (%ld bytes)", obj, size);
  }

  // now find successor block
  u_char* start_of_next_block = (u_char*)ptr + *size_addr_from_obj(ptr) + space_after;
  start_of_next_block = find_cushion_forwards(start_of_next_block);
  u_char* next_obj = start_of_next_block + space_before;
  ptrdiff_t next_size = *size_addr_from_base(start_of_next_block);
  if (start_of_next_block[0] == badResourceValue && 
      start_of_next_block[1] == badResourceValue && 
      start_of_next_block[2] == badResourceValue && 
      start_of_next_block[3] == badResourceValue) {
    tty->print_cr("### next object: %p (%ld bytes)", next_obj, next_size);
  } else {
    tty->print_cr("### next object (not sure if correct): %p (%ld bytes)", next_obj, next_size);
  }
}


void report_heap_error(void* memblock, void* bad, const char* where) {
  tty->print_cr("## nof_mallocs = %d, nof_frees = %d", os::num_mallocs, os::num_frees);
  tty->print_cr("## memory stomp: byte at %p %s object %p", bad, where, memblock);
  print_neighbor_blocks(memblock);
  fatal("memory stomping error");
}

void verify_block(void* memblock) {  
  size_t size = get_size(memblock);
  if (MallocCushion) {
    u_char* ptr = (u_char*)memblock - space_before;
    for (int i = 0; i < MallocCushion; i++) {
      if (ptr[i] != badResourceValue) {
        report_heap_error(memblock, ptr+i, "in front of");
      }
    }
    u_char* end = (u_char*)memblock + size + space_after;
    for (int j = -MallocCushion; j < 0; j++) {
      if (end[j] != badResourceValue) {
        report_heap_error(memblock, end+j, "after");
      }
    }
  }
}
#endif

void* os::malloc(size_t size) {
  NOT_PRODUCT(num_mallocs++);
  NOT_PRODUCT(alloc_bytes += size);

  if (size == 0) {
    // return a valid pointer if size is zero
    // if NULL is returned the calling functions assume out of memory.
    size = 1;
  }

  if (MallocVerifyInterval > 0) check_heap();
  u_char* ptr = (u_char*)::malloc(size + space_before + space_after);
#ifdef ASSERT
  if (ptr == NULL) return NULL;
  largest_seen_so_far = MAX2(largest_seen_so_far, size);
  if (MallocCushion) {
    for (u_char* p = ptr; p < ptr + MallocCushion; p++) *p = (u_char)badResourceValue;
    u_char* end = ptr + space_before + size;
    for (u_char* q = end; q < end + MallocCushion; q++) *q = (u_char)badResourceValue;
  }
  // put size just before data
  *size_addr_from_base(ptr) = size;
#endif
  u_char* memblock = ptr + space_before;
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    tty->print_cr("os::malloc caught, %lu bytes --> %p", size, memblock);
    breakpoint();
  }
  debug_only(if (paranoid) verify_block(memblock));
  if (PrintMalloc && tty != NULL) tty->print_cr("os::malloc %lu bytes --> %p", size, memblock);
  return memblock;
}


void* os::realloc(void *memblock, size_t size) {
  NOT_PRODUCT(num_mallocs++);
  NOT_PRODUCT(alloc_bytes += size);
#ifndef ASSERT
  return ::realloc(memblock, size);
#else
  if (memblock == NULL) {
    return os::malloc(size);
  }
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    tty->print_cr("os::realloc caught %p", memblock);
    breakpoint();
  }
  verify_block(memblock);
  if (MallocVerifyInterval > 0) check_heap();
  if (size == 0) return NULL;
  // always move the block
  void* ptr = malloc(size);
  if (PrintMalloc) tty->print_cr("os::remalloc %lu bytes, %p --> %p", size, memblock, ptr);
  // Copy to new memory if malloc didn't fail
  if ( ptr != NULL ) {
    memcpy(ptr, memblock, MIN2(size, get_size(memblock)));
    if (paranoid) verify_block(ptr);
    if ((intptr_t)ptr == (intptr_t)MallocCatchPtr) {
      tty->print_cr("os::realloc caught, %lu bytes --> %p", size, ptr);
      breakpoint();
    }
    free(memblock);
  }
  return ptr;
#endif
}


void  os::free(void *memblock) {
  NOT_PRODUCT(num_frees++);
#ifdef ASSERT
  if (memblock == NULL) return;
  if ((intptr_t)memblock == (intptr_t)MallocCatchPtr) {
    if (tty != NULL) tty->print_cr("os::free caught %p", memblock);
    breakpoint();
  }
  verify_block(memblock);
  if (PrintMalloc && tty != NULL) tty->print_cr("os::free %p", memblock);
  if (MallocVerifyInterval > 0) check_heap();
#endif
  ::free((char*)memblock - space_before);
}


void os::init_random(long initval) {
  _rand_seed = initval;
}


long os::random() {
  /* standard, well-known linear congruential random generator with
   * next_rand = (16807*seed) mod (2**31-1)
   * see
   * (1) "Random Number Generators: Good Ones Are Hard to Find",
   *      S.K. Park and K.W. Miller, Communications of the ACM 31:10 (Oct 1988),
   * (2) "Two Fast Implementations of the 'Minimal Standard' Random 
   *     Number Generator", David G. Carta, Comm. ACM 33, 1 (Jan 1990), pp. 87-88. 
  */
  const long a = 16807;
  const long m = 2147483647;
  const long q = m / a;        assert(q == 127773, "weird math");
  const long r = m % a;        assert(r == 2836, "weird math");

  // compute az=2^31p+q
  unsigned long lo = a * (long)(_rand_seed & 0xFFFF);
  unsigned long hi = a * (long)((unsigned long)_rand_seed >> 16);
  lo += (hi & 0x7FFF) << 16;

  // if q overflowed, ignore the overflow and increment q
  if (lo > m) {
    lo &= m;
    ++lo;
  }
  lo += hi >> 15;

  // if (p+q) overflowed, ignore the overflow and increment (p+q)
  if (lo > m) {
    lo &= m;
    ++lo;
  }
  return (_rand_seed = lo);
}

// The INITIALIZED state is distinguished from the SUSPENDED state because the
// conditions in which a thread is first started are different from those in which
// a suspension is resumed.  These differences make it hard for us to apply the
// tougher checks when starting threads that we want to do when resuming them.
// However, when start_thread is called as a result of Thread.start, on a Java
// thread, the operation is synchronized on the Java Thread object.  So there
// cannot be a race to start the thread and hence for the thread to exit while
// we are working on it.  Non-Java threads that start Java threads either have
// to do so in a context in which races are impossible, or should do appropriate
// locking.

void os::start_thread(Thread* thread) {
  // guard suspend/resume
  MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);

  assert(thread->is_baby_thread(), "thread has started");
  thread->clear_is_baby_thread();

  OSThread* osthread = thread->osthread();

  // A thread may be suspended in the presence of the profiler.
  // Only start thread when it's not suspended. 
  osthread->set_state(RUNNABLE);
  if (!thread->is_vm_suspended()) {
    pd_start_thread(thread);
  }
}

//--------------------------------------------------------------------------------------
// Fatal Error Handling

static struct fatal_error_info_struct {
  address     pc;
  int         sig;
  const char *error_fname;
  const char *function_name;
  int         function_offset;
  const char *library_name;
} fatal_error_info = {
  (address) -1,   /* pc              */
  -1,             /* sig             */
  NULL,           /* error_fname     */
  NULL,           /* function_name   */
  -1,             /* function_offset */
  NULL,           /* library_name    */
};

/* print out some basic information when the more powerful fatal error handler crashes */
static void report_fatal_error_simple(void)
{
   // NOTE: using puts is more stable than printf. amazingly, sprintf() together 
   //       with puts() doesn't crash in some of my testcases on Linux while a
   //       single printf will surely crash.
   if ((long)fatal_error_info.pc != -1){
        fputs("Dumping information about last error:\n", stderr); fflush(stderr);

        fputs("ERROR REPORT FILE = ", stderr);
        fputs((fatal_error_info.error_fname?fatal_error_info.error_fname:"(N/A)"), stderr);
        fputs("\n", stderr);
        fflush(stderr);

        char buf[80];
        sprintf(buf, "PC                = " INTPTR_FORMAT "\n", fatal_error_info.pc);
        fputs(buf, stderr);   fflush(stderr);
        sprintf(buf, "SIGNAL            = %d%s\n", fatal_error_info.sig,
            VM_Exit::vm_exited() ? " (VM shutdown)" : "");
        fputs(buf, stderr);   fflush(stderr);

        fputs("FUNCTION NAME     = ", stderr); fflush(stderr);
        fputs((fatal_error_info.function_name?fatal_error_info.function_name:"(N/A)"), stderr);
        fputs("\n", stderr); fflush(stderr);

        sprintf(buf, "OFFSET            = 0x%X\n", fatal_error_info.function_offset);
        fputs(buf, stderr);   fflush(stderr);

        fputs("LIBRARY NAME      = ", stderr); fflush(stderr);
        fputs((fatal_error_info.library_name?fatal_error_info.library_name:"(N/A)"), stderr);
        fputs("\n", stderr); fflush(stderr);
   }
   else{
        fputs("No information available.\n", stderr); fflush(stderr);
   }
}

/* NOTE: when having more information to print out in the future, don't forget */
/*       to "flush" after each "print". remember the system might crash at any */
/*       second. no output buffering.                                          */
void os::report_fatal_error(outputStream *st, address pc, int sig)
{
  ResourceMark rm;

  st->cr();
  st->flush();

  char sig_name[256];
  const char * excpt_name = os::exception_name(sig);
  if (excpt_name)
       sprintf(sig_name, "Unexpected Signal : %s (0x%x)", excpt_name, sig);
  else sprintf(sig_name, "Unexpected Signal : %d", sig);

  /* print PC and symbol names */

  const char * symname = NULL;
  int offset = -1;
  bool found_func = os::dll_address_to_function_name(pc, &symname, &offset);
  fatal_error_info.function_name   = symname;
  fatal_error_info.function_offset = offset;
  const char * symtype = "";

  if (os::dll_address_is_in_vm(pc)) {
    // Note:  If you get here, debug with "-XX:-UseTopLevelExceptionFilter".
    // Passes the exception name as the title, so it will also be printed out in PRODUCT mode

    if (!symname){
#ifndef CORE
       /* crash is not in a dynamic library, check if it's inside compiled Java code */
       if (CodeCache::contains(pc)){
          /* try symbol look-up in CodeCache */
           CodeBlob * blob = CodeCache::find_blob_unsafe(pc);
           if (blob && blob->is_nmethod()){
               /* how can we make sure this long dereference won't crash?! */
               symname = ((nmethod *)blob)->method()->name_and_sig_as_C_string();
               fatal_error_info.function_name   = symname;
               fatal_error_info.function_offset = -1;
               found_func = true;
               symtype = " (compiled Java code)";
           }
       }
#endif
    }
  }
  else{
    // We're confident it's not our bug.
     st->print_cr("An unexpected exception has been detected in native code outside the VM.");
     st->flush();
  }

  st->print_cr("%s occurred at PC=0x%lX%s", sig_name, (long)pc,
                VM_Exit::vm_exited() ? " during VM shutdown" : "");              
  st->flush();

  st->print("Function=");
  if (found_func) {
     if (offset >= 0) st->print_cr("%s+0x%X%s", symname, offset, symtype);   
     else st->print_cr("%s%s", symname, symtype);
  } else {
     st->print("[Unknown.");
     if (symname) st->print(" Nearest: %s+0x%X", symname, offset);
     st->print_cr("]%s", symtype);
  }
  st->flush();

  // library name
  const char * libname = os::dll_address_to_library_name(pc);
  fatal_error_info.library_name = libname;
  st->print_cr("Library=%s", (libname?libname:"(N/A)"));
  st->flush();

  // line number and filename
  char * filename = NULL;
  int lineno = os::dll_address_to_line_number(pc, &filename);

  if (lineno > 0 && filename){
        st->print_cr("Source file = %s : %d", filename, lineno);
        st->cr();
        st->flush();
  }

  if (!symname){
        st->cr();
        st->print_cr("NOTE: We are unable to locate the function name symbol for the error");
        st->print_cr("      just occurred. Please refer to release documentation for possible");
        st->print_cr("      reason and solutions.");
        st->cr();
        st->flush();
  }

  /* print stack trace */

  st->cr();
  st->flush();

  Thread * thread = ThreadLocalStorage::get_thread_slow();
  if (thread){
       if (thread->is_Java_thread()){
           /* java thread, print stack trace info for java frames */
           st->print_cr("Current Java thread:");
           /* temporarily redirect tty to st, yeah, it's dirty */
           outputStream *old_tty = tty;
           tty = st;
           ((JavaThread *)thread) -> print_stack();
           st->flush();
           tty = old_tty;
       }
       else{
           /* native thread, skip it, for now */
       }
  }
  else{
       /* crashed in native threads, JVM has no knowledge of them. */
       st->print_cr("Cannot obtain thread information");
       st->flush();
  }

  /* print library names */

  st->cr();
  st->flush();

  os::dump_dll_info(st);
  st->flush();

  /* print heap information before vm abort */
  os::print_vm_state_before_abort(st); 

  /* date and time */

  st->cr();
  st->flush();

  struct tm *ltime;
  time_t long_time;
  time(&long_time);

  /* ctime_r() puts a '\n' at the end. */
  
  { 
    struct tm ltbuf;
    char tbuf[50];

    // NOTE that the return values of localtime_r and asctime_r must be used 
    // as the buffer arguments are ignored on Windows
    // NOTE also that on Linux the asctime_r length arg is ignored
    ltime = os::localtime(&long_time, &ltbuf);

    st->print("Local Time = %s", os::asctime(ltime, tbuf, sizeof(tbuf)));
  }
  st->flush();

  double t = os::elapsedTime();
// NOTE: It tends to crash after a SEGV if we want to printf("%f",...) in
//       Linux. Must be a bug in glibc ? Workaround is to round "t" to int before
//       printf. We lost some precision, but who cares?
//  st->print("Elapsed Time = %f\n", t);
  st->print_cr("Elapsed Time = %d", (int)t);
  st->flush();

  if (os::dll_address_is_in_vm(pc)) {
    // Note:  If you get here, debug with "-XX:-UseTopLevelExceptionFilter".
    // Passes the exception name as the title, so it will also be printed out in PRODUCT mode

    st->print_cr("#");
    if (excpt_name)
         st->print_cr("# HotSpot Virtual Machine Error : %s", excpt_name);
    else st->print_cr("# HotSpot Virtual Machine Error : %d", sig);
    st->flush();

    char loc_buf[256];
    if (lineno > 0 && filename) {
        obfuscate_location(filename, lineno, loc_buf, sizeof(loc_buf));
    } else {
        obfuscate_location(__FILE__, __LINE__, loc_buf, sizeof(loc_buf));
    }
    st->print_cr("# Error ID : %s", loc_buf);
    st->flush();

    st->print_cr("# Please report this error at");
    st->print_cr("# http://java.sun.com/cgi-bin/bugreport.cgi");
    st->print_cr("#");
    st->flush();
  } else {
    // We're confident it's not our bug.
    st->print_cr("#");
    st->print_cr("# The exception above was detected in native code outside the VM");
    st->print_cr("#");
    st->flush();
  }

  st->print_cr("# Java VM: %s (%s %s)",
                Abstract_VM_Version::vm_name(),
                Abstract_VM_Version::vm_release(),
                Abstract_VM_Version::vm_info_string()
               );
  st->print_cr("#");
  st->flush();
}

void os::handle_recursive_fatal_error(int sig)
{
   if (ShowMessageBoxOnError) {
     infinite_sleep();
   }

   /* remove the signal handler, no longer used */
   os::set_exception_handler_during_fatal_error(sig, /* is_install */ 0);

   fputs("\n\n****************\n"
        "Another exception has been detected while we were handling last error.\n",
        stderr);
   fflush(stderr);


   report_fatal_error_simple();

   fputs("Please check ERROR REPORT FILE for further information, if there is any.\n"
         "Good bye.\n", stderr);
   fflush(stderr);

   os::abort();
}

# ifndef MAX_PATH
# define MAX_PATH (2 * K)
# endif

void os::handle_unexpected_exception(Thread* thread, int sig, address pc, void *extra_info) {
  static jint fatal_error_count = 0;

  switch(atomic::add(1, &fatal_error_count)){
  case 1: /* first time */      
    break;
  case 2: /* second time */     
    os::handle_recursive_fatal_error(sig);  // does not return
  default:/* more than twice */ 
    if (ShowMessageBoxOnError) {
      infinite_sleep();
    }
    os::abort();
  }

  /* disable report_error */
  inc_error_level();

  /* We will add a simple signal handler to print at least some information in */
  /* case this error handler crashes.                                          */
  os::set_exception_handler_during_fatal_error(sig, /* is_install */ 1);

  // In case error happens before init or during shutdown
  if (tty == NULL) ostream_init();

  /* save information for the exception handler */
  fatal_error_info.pc  = pc;
  fatal_error_info.sig = sig;

  /* allow user to attach this process to a debugger */
  if (ShowMessageBoxOnError) {
     char signame_buf[64];
     sprintf(signame_buf, "Unexpected Signal: %d, PC: %p, PID: %d", 
              sig, pc, os::current_process_id());
     const char*signame = os::exception_name(sig);
     if (os::message_box(signame ? signame : signame_buf,
         os::do_you_want_to_debug("An error has just occurred."))) {
           /* On Windows, this will pass the exception to the OS and */
           /* trigger the debugger. On Solaris and Linux, this has no*/
           /* effect.                                                */
           return;
     }
  }

  os::report_fatal_error(tty, pc, fatal_error_info.sig);

  /* also save a copy of fatal error report */
  char error_fname_buf[MAX_PATH];
  sprintf(error_fname_buf, "hs_err_pid%u.log", os::current_process_id());
  char *error_fname = error_fname_buf;
  FILE * fp = fopen(error_fname, "w");
  if (fp==NULL){
      const char * tmpdir = os::get_temp_directory();
      sprintf(error_fname_buf, "%shs_err_pid%u.log", (tmpdir?tmpdir:""), os::current_process_id());
      fp = fopen(error_fname, "w");
  }
  if (!fp) error_fname = NULL;

  if (error_fname){
      fatal_error_info.error_fname = error_fname;
      fileStream stream(fp);
      os::report_fatal_error(&stream, pc, sig);

          tty->print_cr("# An error report file has been saved as %s.", error_fname);
          tty->print_cr("# Please refer to the file for further information.");
          tty->print_cr("#");
  }

  tty->flush();

  os::abort();
}

void os::print_vm_state_before_abort(outputStream *st) {
  st->cr(); 
  st->print("Heap at VM Abort:");
  st->cr(); 
  Universe::print_on(st);
  st->flush();
}


#ifdef ASSERT
extern "C" void test_random() {
  const double m = 2147483647;
  double mean = 0.0, variance = 0.0, t;
  long reps = 10000;
  unsigned long seed = 1;

  tty->print_cr("seed %ld for %ld repeats...", seed, reps);
  os::init_random(seed);
  long num; 
  for (int k = 0; k < reps; k++) {
    num = os::random();
    double u = (double)num / m;
    assert(u >= 0.0 && u <= 1.0, "bad random number!");

    // calculate mean and variance of the random sequence 
    mean += u;
    variance += (u*u);
  }
  mean /= reps;
  variance /= (reps - 1);

  assert(num == 1043618065, "bad seed");
  tty->print_cr("mean of the 1st 10000 numbers: %f", mean);
  tty->print_cr("variance of the 1st 10000 numbers: %f", variance);
  const double eps = 0.0001;
  // fabs() not always available
  t = (mean - 0.5018) < 0.0 ? -(mean - 0.5018) : mean - 0.5018;
  assert(t < eps, "bad mean");
  t = (variance - 0.3355) < 0.0 ? -(variance - 0.3355) : variance - 0.3355;
  assert(t < eps, "bad variance");
}
#endif


// Set up the boot classpath.

bool os::set_boot_path(props_md_t* sprops, char fileSep, char pathSep) {

    assert((fileSep == '/' && pathSep == ':') ||
	   (fileSep == '\\' && pathSep == ';'), "unexpected seperator chars");

    const char* home = sprops->java_home;
    int home_len = (int)strlen(home);

    static const char classpathFormat[] =
	"%/lib/rt.jar:"
	"%/lib/i18n.jar:"
	"%/lib/sunrsasign.jar:"
	"%/lib/jsse.jar:"
	"%/lib/jce.jar:"
        "%/lib/charsets.jar:"
	"%/classes";

    // Scan the format string to determine the length of the actual
    // boot classpath, and handle platform dependencies as well.
    int sysclasspathLen = 0;
    const char* p;
    for (p = classpathFormat; *p != 0; ++p) {
	if (*p == '%') sysclasspathLen += home_len - 1;
	++sysclasspathLen;
    }

    char* sysclasspath = NEW_C_HEAP_ARRAY(char, sysclasspathLen + 1);
    if (sysclasspath == NULL) {
	return false;
    }

    // Create boot classpath from format, substituting separator chars and
    // java home directory.
    char* q = sysclasspath;
    for (p = classpathFormat; *p != 0; ++p) {
	switch (*p) {
	case '%':
	    strcpy(q, home);
	    q += home_len;
	    break;
	case '/':
	    *q++ = fileSep;
	    break;
	case ':':
	    *q++ = pathSep;
	    break;
	default:
	    *q++ = *p;
	}
    }
    *q = '\0';

    assert((q - sysclasspath) == sysclasspathLen, "sysclasspath size botched");
    sprops->sysclasspath = sysclasspath;

    return true;
}

// Returns true if the current stack pointer is above the stack shadow
// pages, false otherwise.

bool os::stack_shadow_pages_available(Thread *thread) {
  assert(StackYellowPages > 0,"Sanity check");
  assert(StackRedPages > 0,"Sanity check");
  address sp = current_stack_pointer();
  // Check if we have StackShadowPages above the yellow zone.  This parameter
  // is dependant on the depth of the maximum VM call stack possible from
  // the handler for stack overflow.  'instanceof' in the stack overflow
  // handler or a println uses at least 8k stack of VM and native code
  // respectively.
  int reserved_area = ((StackShadowPages + StackRedPages + StackYellowPages) 
                      * vm_page_size());
  // The very lower end of the stack
  address stack_limit = thread->stack_base() - thread->stack_size();
  return (sp > (stack_limit + reserved_area));
}
