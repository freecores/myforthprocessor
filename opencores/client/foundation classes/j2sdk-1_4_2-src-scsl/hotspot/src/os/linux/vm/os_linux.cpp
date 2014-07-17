#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_linux.cpp	1.126 03/05/22 13:46:25 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// do not include  precompiled  header file
# include "incls/_os_linux.cpp.incl"

// put OS-includes here
# include <sys/types.h>
# include <sys/mman.h>
# include <pthread.h>
# include <signal.h>
# include <errno.h>
# include <dlfcn.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/resource.h>
# include <pthread.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/utsname.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <pwd.h>
# include <poll.h>
# include <semaphore.h>

#define MAX_PATH    (2 * K)

////////////////////////////////////////////////////////////////////////////////
// global variables

int    os::Linux::_processor_count = 0;
julong os::Linux::_physical_memory = 0;

address   os::Linux::_initial_thread_stack_bottom = NULL;
uintptr_t os::Linux::_initial_thread_stack_size   = 0;
int       os::Linux::_heap_stack_gap = 0;


Mutex* os::Linux::_createThread_lock = NULL;
pthread_t os::Linux::_main_thread;
int os::Linux::_page_size = -1;
void* os::Linux::_pthread_getattr_func = NULL;


static jlong initial_time_count=0;

static pid_t _initial_pid = 0;

/* Signal number used to suspend/resume a thread */

/* do not use any signal number less than SIGSEGV, see 4355769 */
static int SR_signum = SIGUSR2;
sigset_t SR_sigset;

////////////////////////////////////////////////////////////////////////////////
// utility functions

static int SR_initialize();
static int SR_finalize();

julong os::physical_memory() {
  return Linux::physical_memory();
}

////////////////////////////////////////////////////////////////////////////////
// environment support

bool os::getenv(const char* name, char* buf, int len) {
  const char* val = ::getenv(name);
  if (val != NULL && strlen(val) < len) {
    strcpy(buf, val);
    return true;
  }
  if (len > 0) buf[0] = 0;  // return a null string
  return false;
}

void os::Linux::initialize_system_info() {
  _processor_count = sysconf(_SC_NPROCESSORS_CONF);
  _physical_memory = (julong)sysconf(_SC_PHYS_PAGES) * (julong)sysconf(_SC_PAGESIZE);
  assert(_processor_count > 0, "linux error");
}

props_md_t* os::get_system_properties() {
  if (_system_properties != NULL) {
    return _system_properties;
  }

  // create properties structure and fill it
  props_md_t* props = new props_md_t();
  memset(props, '\0', sizeof(*props));

//  char arch[12];
//  sysinfo(SI_ARCHITECTURE, arch, sizeof(arch));

  // The following extra steps are taken in the debugging version:
  //
  // Starting from the binary containing this code, look for a
  // symbolic link to JAVA_HOME.  If this code is linked into
  // gamma_g, for example, look for gamma_g.JAVA_HOME/{bin,lib}....
  //
  // If that fails, obtain the JAVA_HOME from the location of
  // libjvm(_g).so. This library should be located at
  // JAVA_HOME/lib/{sparc|i386}/hotspot/libjvm.so.
  // (This step is taken in the product version also.)
  //
  // If that fails, look for the environment variable $JAVA_HOME.
  // Note that this does __not__ override the result of the previous
  // step, if successful.  (This is different from the 1.1 JDK.)
  //
  // Important note. if the location of libjvm.so changes this 
  // code needs to be changed accordingly.

  // HotSpot integration note:
  // code below matches code in JDK1.2fcs K build.
  // (Some '#ifndef PRODUCT' lines are introduced.)
  // "$JDK/src/solaris/javavm/runtime/javai_md.c":80 (ver. 1.7 98/09/15)
  // The next few definitions allow the code to be verbatim:
  props_md_t& sprops = *props;
#define GetPropertiesMD os::get_system_properties
#define NATIVE 1
#define malloc(n) (char*)NEW_C_HEAP_ARRAY(char, (n))
#define getenv(n) ::getenv(n)

#define EXTENSIONS_DIR "/lib/ext"
#define ENDORSED_DIR "/lib/endorsed"

  char *v; /* tmp var */
  {
    /* sysclasspath, java_home, dll_dir */
    {
        char *home_path;
	char *dll_path;
        char buf[MAXPATHLEN];
#if 1 //HotSpot change
	os::jvm_path(buf, sizeof(buf));
#else //HotSpot change
	Dl_info dlinfo;
	dladdr((void *)GetPropertiesMD, &dlinfo);
	realpath((char *)dlinfo.dli_fname, buf);
#endif //HotSpot change
	*(strrchr(buf, '/')) = '\0';	/* get rid of /libjvm.so */
	*(strrchr(buf, '/')) = '\0';    /* get rid of classic    */
	dll_path = malloc(strlen(buf) + 1);
	if (dll_path == 0)
	    return NULL;
	strcpy(dll_path, buf);
	sprops.dll_dir = dll_path;

	*(strrchr(buf, '/')) = '\0';
	*(strrchr(buf, '/')) = '\0';

	home_path = malloc(strlen(buf) + 1);
	if (home_path == 0)
	    return NULL;
	strcpy(home_path, buf);
	sprops.java_home = home_path;

	if (!set_boot_path(&sprops, '/', ':'))
	    return NULL;
    }

    /* Where to look for native libraries */
    {
	/*
	 * Get the user setting of LD_LIBRARY_PATH
	 */
	v = getenv("LD_LIBRARY_PATH");

	if (v == NULL) {
	    v = "";
	}

	sprops.library_path = strdup(v);
    }

    /* Extensions directories */
    {
	char * buf;
	buf = malloc(strlen(sprops.java_home) + sizeof(EXTENSIONS_DIR));
	sprintf(buf, "%s" EXTENSIONS_DIR, sprops.java_home);
	sprops.ext_dirs = buf;
    }

    /* Endorsed standards default directory. */
    {
	char * buf;
	buf = malloc(strlen(sprops.java_home) + sizeof(ENDORSED_DIR));
	sprintf(buf, "%s" ENDORSED_DIR, sprops.java_home);
	sprops.endorsed_dirs = buf;
    }
  }

#undef GetPropertiesMD
#undef NATIVE
#undef malloc
#undef getenv
#undef EXTENSIONS_DIR
#undef ENDORSED_DIR

  // Done
  _system_properties = props;
  return props;
}

////////////////////////////////////////////////////////////////////////////////
// breakpoint support

void os::breakpoint() {
  BREAKPOINT;
}

extern "C" void breakpoint() {
  // use debugger to set breakpoint here
}

////////////////////////////////////////////////////////////////////////////////
// signal support

// These are signals that are unblocked while a thread is running Java.
// (For some reason, they get blocked by default.)
sigset_t* os::Linux::unblocked_signals() {
  static sigset_t sigs;
  static bool done = false;
  if (!done) {
    sigemptyset(&sigs);
    sigaddset  (&sigs, SIGINT);
    sigaddset  (&sigs, SIGCHLD);
    sigaddset  (&sigs, SIGTERM);
    /* these are normally unblocked but someone embedding a vm could block them */
    sigaddset(&sigs, SIGILL);
    sigaddset(&sigs, SIGSEGV);
    sigaddset(&sigs, SIGBUS);
    sigaddset(&sigs, SIGFPE);
    done = true;                // it's OK to re-enter this
  }
  return &sigs;
}

void os::Linux::hotspot_sigmask(bool main) {
  sigset_t sigset;

  sigemptyset(&sigset);
  if (!ReduceSignalUsage) {
    // Do not change the blocked/unblocked status of these signals if
    // -Xrs is specified. See signal_sets_init, os_solaris.cpp.
    if (!os::Linux::is_sig_ignored(SHUTDOWN1_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN1_SIGNAL);
    }
    if (!os::Linux::is_sig_ignored(SHUTDOWN2_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN2_SIGNAL);
    }
    if (!os::Linux::is_sig_ignored(SHUTDOWN3_SIGNAL)) {
       sigaddset  (&sigset, SHUTDOWN3_SIGNAL);
    }
    sigaddset  (&sigset, BREAK_SIGNAL);
  }
  // These are needed for correctness of the VM.
  sigaddset  (&sigset, SIGCHLD);
  sigaddset  (&sigset, SR_signum);
  pthread_sigmask(SIG_UNBLOCK, &sigset, NULL);

  if (!ReduceSignalUsage) {
    if (!main) {
      sigemptyset(&sigset);
      sigaddset  (&sigset, BREAK_SIGNAL);
      pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    }
  }
}

// bootstrap the main thread, need extra thoughts???
bool os::create_main_thread(Thread* thread) {
  assert(os::Linux::_main_thread == pthread_self(), "should be called inside main thread");
  return create_attached_thread(thread);
}

bool os::create_attached_thread(Thread* thread) {
  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);

  if (osthread == NULL) {
    return false;
  }
  osthread->clear_expanding_stack();

  // Store pthread info into the OSThread
  osthread->set_thread_id(::getpid());
  osthread->set_pthread_id(::pthread_self());

  // Make sure the thread has an alternate signal stack
  os::Linux::install_alternate_signal_stack(osthread);

  // initialize signal mask for this thread
  os::Linux::hotspot_sigmask(pthread_self() == os::Linux::_main_thread);

  // initialize floating point control register
  os::Linux::init_thread_fpu_state();

  // Initial thread state is RUNNABLE
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    thread->clear_is_baby_thread();
    osthread->set_state(RUNNABLE);
  }

  thread->set_osthread(osthread);

  return true;
}

void os::pd_start_thread(Thread* thread) {
  ((os::Linux::Event*) thread->osthread()->interrupt_event())->set();
}

// this may cause fatal error is some cases???
void os::exit_thread() {
  free_thread(Thread::current()->osthread());
  Thread::current()->set_osthread(NULL);

  pthread_exit(NULL);

  ShouldNotReachHere();
}

// Free Linux resources related to the OSThread
void os::free_thread(OSThread* osthread) {
  assert(osthread != NULL, "osthread not set");
  delete osthread;
}

address os::current_stack_pointer() {
  char dummy;
  return (address) &dummy;
}

int os::allocate_thread_local_storage() {
  pthread_key_t key;
  int rslt = pthread_key_create(&key, NULL);
  assert(rslt == 0, "cannot allocate thread local storage");
  return (int)key;
}

// Note: This is currently not used by VM, as we don't destroy TLS key
// on VM exit.
void os::free_thread_local_storage(int index) {
  int rslt = pthread_key_delete((pthread_key_t)index);
  assert(rslt == 0, "invalid index");
}

void os::thread_local_storage_at_put(int index, void* value) {
  int rslt = pthread_setspecific((pthread_key_t)index, value);
  assert(rslt == 0, "pthread_setspecific failed");
}

void* os::thread_local_storage_at(int index) {
  return pthread_getspecific((pthread_key_t)index);
}

Thread* ThreadLocalStorage::thread() {
  return (Thread *) os::thread_local_storage_at(thread_index());
}

extern "C" Thread* get_thread() {
  return ThreadLocalStorage::thread();
}

/* HACK: LinuxThreads does not provide the function similar to thr_main   */
/*       on Solaris. We have to play the trick here. One way to tell if   */
/*       we are in the process's initial thread is to check current stack */
/*       region. The other way is to check if pthread_self() returns a    */
/*       specific thread number, which may be different among pthread     */
/*       libraries.                                                       */
bool os::Linux::is_initial_thread(void) {
  char dummy;
  assert(initial_thread_stack_bottom() != NULL &&
         initial_thread_stack_size()   != 0,
         "os::init did not locate initial thread's stack region");
  if ((address)&dummy >= initial_thread_stack_bottom() &&
      (address)&dummy < initial_thread_stack_bottom() + initial_thread_stack_size())
       return true;
  else return false;
}

/* Initial thread (i.e., the main thread of the process, not necessarily  */
/* JVM's main_thread) is created by kernel.                               */
/* NOTE : We use tricks to get the value because in some (all?) thread    */
/* libraries pthread_get_attr_np returns the wrong values for the initial */
/* thread                                                                 */
/*                                                                        */
void os::Linux::capture_initial_stack(size_t max_size) {
  // locate initial thread's stack location
  struct rlimit rlim;
  FILE *fp = NULL;
  int pid;
  char state;
  int ppid;
  int pgrp;
  int session;
  int nr;
  int tpgrp;
  unsigned long flags;
  unsigned long minflt;
  unsigned long cminflt;
  unsigned long majflt;
  unsigned long cmajflt;
  unsigned long utime;
  unsigned long stime;
  long cutime;
  long cstime;
  long prio;
  long nice;
  long junk;
  long it_real;
  uintptr_t start;
  uintptr_t vsize;
  uintptr_t rss;
  unsigned long rsslim;
  uintptr_t scodes;
  uintptr_t ecode;
  uintptr_t stack_start;
  int i;

  getrlimit(RLIMIT_STACK, &rlim);
  // 4441425: avoid crash with "unlimited" stack size on SuSE 7.1 or Redhat
  //   7.1, in both cases we will get 2G in return value.
  // 4466587: glibc 2.2.x compiled w/o "--enable-kernel=2.4.0" (RH 7.0,
  //   SuSE 7.2, Debian) can not handle alternate signal stack correctly
  //   for initial thread if its stack size exceeds 6M. Cap it at 2M,
  //   in case other parts in glibc still assumes 2M max stack size.
#ifndef IA64
  if (rlim.rlim_cur > 2 * K * K) rlim.rlim_cur = 2 * K * K;
#else
  // Problem still exists RH7.2 (IA64 anyway) but 2MB is a little small
  if (rlim.rlim_cur > 4 * K * K) rlim.rlim_cur = 4 * K * K;
#endif

  _initial_thread_stack_size   = rlim.rlim_cur & ~(page_size() - 1);

  if (max_size && _initial_thread_stack_size > max_size) {
     _initial_thread_stack_size = max_size;
  }

  // Figure what the primordial thread stack base is. Code is inspired
  // by email from Hans Boehm. /proc/self/stat begins with current pid,
  // followed by command name surrounded by parentheses, state, etc.
  char stat[2048];
  int statlen;

  fp = fopen("/proc/self/stat", "r");
  if (fp == NULL) fatal("Can not open /proc/self/stat");
  statlen = fread(stat, 1, 2047, fp);
  stat[statlen] = '\0';
  fclose(fp);

  // Skip pid and the command string. Note that we could be dealing with
  // weird command names, e.g. user could decide to rename java launcher
  // to "java 1.4.2 :)", then the stat file would look like
  //                1234 (java 1.4.2 :)) R ... ... 
  // We don't really need to know the command string, just find the last 
  // occurrence of ")" and then start parsing from there. See bug 4726580.
  char * s = strrchr(stat, ')');

  i = 0;
  if (s) {
    // Skip blank chars
    do s++; while (isspace(*s));

    /*                                     1   1   1   1   1   1   1   1   1   1   2   2   2   2   2   2   2   2   2 */
    /*              3  4  5  6  7  8   9   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7   8 */
    i = sscanf(s, "%c %d %d %d %d %d %lu %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %lu %lu %ld %lu %lu %lu %lu", 
	     &state,          /* 3  %c  */
	     &ppid,           /* 4  %d  */
	     &pgrp,           /* 5  %d  */
	     &session,        /* 6  %d  */
	     &nr,             /* 7  %d  */
	     &tpgrp,          /* 8  %d  */
	     &flags,          /* 9  %lu  */
	     &minflt,         /* 10 %lu  */
	     &cminflt,        /* 11 %lu  */
	     &majflt,         /* 12 %lu  */
	     &cmajflt,        /* 13 %lu  */
	     &utime,          /* 14 %lu  */
	     &stime,          /* 15 %lu  */
	     &cutime,         /* 16 %ld  */
	     &cstime,         /* 17 %ld  */
	     &prio,           /* 18 %ld  */
	     &nice,           /* 19 %ld  */
	     &junk,           /* 20 %ld  */
	     &it_real,        /* 21 %ld  */
	     &start,          /* 22 %lu  */
	     &vsize,          /* 23 %lu  */
	     &rss,            /* 24 %ld  */
	     &rsslim,         /* 25 %lu  */
	     &scodes,         /* 26 %lu  */
	     &ecode,          /* 27 %lu  */
	     &stack_start);   /* 28 %lu  */
  }

  if (i != 28 - 2) {
     assert(false, "Bad conversion from /proc/self/stat");
     // product mode - assume we are the initial thread and rlim is alloc'ed
     // within the 1st page of the stack. Good luck in the embedded case.
     stack_start = (uintptr_t) &rlim;
  }

  // stack start is partially down the page so we align it to the top of the 
  // page the stack is in
  stack_start = align_size_up((intptr_t) stack_start, page_size());
  _initial_thread_stack_bottom = (address) stack_start - _initial_thread_stack_size;

  os::Linux::set_heap_stack_gap(0);
  fp = fopen("/proc/sys/vm/heap-stack-gap", "r");
  if (fp != NULL) {
     int gap;
     if (fscanf(fp, "%d", &gap) == 1 && gap > 0) {
        os::Linux::set_heap_stack_gap(gap);
     }
     fclose(fp);
  }
}


// ISM not avaliable for linux
char* os::reserve_memory_special(size_t bytes) {
  ShouldNotReachHere();
  return NULL;
}

bool os::set_mpss_range(char* addr, size_t size, size_t align) {
  ShouldNotReachHere();
  return false;
}

////////////////////////////////////////////////////////////////////////////////
// time support

// Time since start-up in seconds to a fine granularity.
// Used by VMSelfDestructTimer and the MemProfiler.
double os::elapsedTime() {

  return (double)(os::elapsed_counter()) * 0.000001;
}

jlong os::elapsed_counter() {
  timeval time;
  int status = gettimeofday(&time, NULL);
  return jlong(time.tv_sec) * 1000 * 1000 + jlong(time.tv_usec) - initial_time_count;
}

jlong os::elapsed_frequency() {
  return (1000 * 1000);
}

jlong os::javaTimeMillis() {
  timeval time;
  int status = gettimeofday(&time, NULL);
  assert(status != -1, "linux error");
  return jlong(time.tv_sec) * 1000  +  jlong(time.tv_usec / 1000);
}

////////////////////////////////////////////////////////////////////////////////
// runtime exit support

void os::abort(bool dump_core) {

  // allow PerfMemory to attempt cleanup of any persistent resources
  perfMemory_exit();

  // Check for abort hook
  abort_hook_t abort_hook = Arguments::abort_hook();
  if (abort_hook != NULL) {
    abort_hook();
  }
  if (dump_core) {
    NOT_PRODUCT(tty->print("Current thread is %p\n", pthread_self()));
    NOT_PRODUCT(tty->print("Dumping core ...\n"));
    ostream_exit();
    // 4450681 : kill all other threads before abort to make sure the 
    // calling thread is the one that gets dumped in core file. 
    pthread_kill_other_threads_np();
    ::abort(); // dump core
  }
  ostream_exit();
  ::exit(1);
}

intx os::current_thread_id() { return (intx)pthread_self(); }
int os::current_process_id() {

  assert(_initial_pid != 0, "called before os::init()");

  // linux gives each thread its own process id. because of this
  // each thread will return a differend pid if this method were
  // to return the result of getpid(2). Linux provides no api that
  // returns the pid of the primordial thread for the vm. This
  // implementation returns a unique pid, the pid for the thread that
  // invokes os::init(), for the vm 'process'.

  // if you are looking for the result of a call to getpid() that
  // returns a unique pid for the calling thread, then look at the
  // OSThread::thread_id() method in osThread_linux.hpp file

  return (int)_initial_pid;
}

// DLL functions

const char* os::dll_file_extension() { return ".so"; }

const char* os::get_temp_directory() { return "/tmp/"; }

// Returns true if the given PC was in the VM DLL; otherwise returns false.
bool os::dll_address_is_in_vm(address addr) {
  // These tests should be unified with the code in find(int, bool) in
  // debug.cpp.

  // Check generated code
  if (Interpreter::contains(addr)) {
    return true;
  }

  if (StubRoutines::contains(addr)) {
    return true;
  }

#ifndef CORE
  if (CodeCache::contains(addr)) {
    return true;
  }

  if (VtableStubs::contains(addr)) {
    return true;
  }
#endif

  // Check if inside libjvm
  Dl_info dlinfo;
  if (dladdr(CAST_FROM_FN_PTR(void *, os::dll_address_is_in_vm), &dlinfo)){
      void * fbase = dlinfo.dli_fbase;
      assert(fbase !=NULL, "Cannot obtain base address for libjvm");
      if (dladdr((void *)addr, &dlinfo)){
          if (fbase == dlinfo.dli_fbase) return true;
      }
  }
  else assert(NULL != NULL, "Cannot obtain base address for libjvm");

  return false;
}

bool os::dll_address_to_function_name(address addr, const char **symname,
                                             int *offset) {
  Dl_info dlinfo;

  if (dladdr((void*)addr, &dlinfo)) {
    if (symname) *symname = dlinfo.dli_sname;
    if (offset)  *offset  = addr - (address)dlinfo.dli_saddr;
    return true;
  } else {
    if (symname) *symname = NULL;
    if (offset)  *offset  = -1;
    return false;
  }
}

// Returns the name of the library that contains the specified addres
// or NULL if it's outside all known mapped object.
const char* os::dll_address_to_library_name(address addr){
  Dl_info dlinfo;
  const char* lib = (char *)NULL;

  if (dladdr((void*)addr, &dlinfo)){
     lib = dlinfo.dli_fname;
  }
  return lib;
}

// Maps PC to line number and source file name. Not implemented for
// Linux
int os::dll_address_to_line_number(address addr, char ** filename) {
  if (filename) *filename = NULL;
  return -1;
}

/*
 * splits a string _str_ into substrings with delimiter _delim_ by replacing old
 * delimiters with _new_delim_ (ideally, '\0'). the address of each substring
 * is stored in array _ptrs_ as the return value. the maximum capacity of _ptrs_
 * array is specified by parameter _n_.
 * RETURN VALUE: total number of substrings (always <= _n_)
 * NOTE: string _str_ is modified if _delim_!=_new_delim_
 */
static int split_n_str(char * str, int n, char ** ptrs, char delim, char new_delim)
{
   int i;
   for(i = 0; i < n; i++) ptrs[i] = NULL;
   if (str == NULL || n < 1 ) return 0;

   i = 0;

   // skipping leading blanks
   while(*str&&*str==delim) str++;

   while(*str&&i<n){
     ptrs[i++] = str;
     while(*str&&*str!=delim) str++;
     while(*str&&*str==delim) *(str++) = new_delim;
   }

   return i;
}

/*
 * fgets without storing '\n' at the end of the string
 */
static char * fgets_no_cr(char * buf, int n, FILE *fp)
{
   char * rslt = fgets(buf, n, fp);
   if (rslt && buf && *buf){
       char *p = strchr(buf, '\0');
       if (*--p=='\n') *p='\0';
   }
   return rslt;
}

/*
 * search string _str_ in a string list _strlist_. _n_ is the size of the list
 * RETURN VALUE: index in the list if found; otherwise -1.
 */
static int find_string(char ** strlist, int n, char * str)
{
   int i;
   for(i=0; i<n; i++)
       if (strcmp(strlist[i], str)==0)
           return i;
   return -1;
}


#define PRINT_PROC_MAP    /* do not attempt to analyze /proc/<pid>/maps, directly print the */
                          /* file to st. it's desirable for fatal error handling, as it's   */
                          /* less error-prone and actually can provide more information     */
                          /* for seasoned debuggers.                                        */
void os::dump_dll_info(outputStream *st) {
       pid_t pid = getpid();

       char fname[32];
       char buf[256];
       FILE *fp = NULL;

#ifndef PRINT_PROC_MAP
#      define MAX_HISTORY_SIZE 32
       char *history[MAX_HISTORY_SIZE];
       int  history_size = 0;
       char *execfile = "";
#endif

       st->print_cr("Dynamic libraries:");

       sprintf(fname, "/proc/%d/maps", pid);
       fp = fopen(fname, "r");

       if (fp == NULL){
           st->print("Can not get information for pid = %d\n", pid);
           return;
       }

       while(fgets_no_cr(buf, 256, fp)){
#ifndef PRINT_PROC_MAP
           char * word[6];
           int nwords = split_n_str(buf, 6, word, ' ', '\0');
#          define EXE_LOC "08048000-"
           if (strncmp(buf, EXE_LOC, strlen(EXE_LOC)) == 0){
                execfile = strdup(word[5]);
           }
           else if (nwords > 5 && strcmp(word[5], execfile)!=0
                    && find_string(history, history_size, word[5])<0){
                st->print("lib %s\n", word[5]); st->flush();
                if (history_size<MAX_HISTORY_SIZE-1)
                    history[history_size++] = strdup(word[5]);
           }
#else
           if (strlen(buf)>42){
             /* skip not so useful non-dll blocks */
              st->print_cr(buf);  st->flush();
           }
#endif
       }
       fclose(fp);
#undef EXE_LOC
#undef MAX_HISTORY_SIZE
}

static char saved_jvm_path[MAXPATHLEN] = {0};

// Find the full path to the current module, libjvm.so or libjvm_g.so
void os::jvm_path(char *buf, jint len) {
  // Error checking.
  if (len < MAXPATHLEN) {
    assert(false, "must use a large-enough buffer");
    buf[0] = '\0';
    return;
  }
  // Lazy resolve the path to current module.
  if (saved_jvm_path[0] != 0) {
    strcpy(buf, saved_jvm_path);
    return;
  }

  Dl_info dlinfo;
  int ret = dladdr(CAST_FROM_FN_PTR(void *, os::jvm_path), &dlinfo);
  assert(ret != 0, "cannot locate libjvm");
  realpath((char *)dlinfo.dli_fname, buf);

#ifndef PRODUCT
  // Support for the gamma launcher.  Typical value for buf is
  // "<JAVA_HOME>/jre/lib/<arch>/<vmtype>/libjvm.so".  If "/jre/lib/" appears at
  // the right place in the string, then assume we are installed in a JDK and
  // we're done.  Otherwise, check for a JAVA_HOME environment variable and fix
  // up the path so it looks like libjvm.so is installed there.
  const char *p = buf + strlen(buf) - 1;
  for (int count = 0; p > buf && count < 5; ++count) {
    for (--p; p > buf && *p != '/'; --p)
      /* empty */ ;
  }

  if (strncmp(p, "/jre/lib/", 9) != 0) {
    // Look for JAVA_HOME in the environment.
    char* java_home_var = ::getenv("JAVA_HOME");
    if (java_home_var != NULL && java_home_var[0] != 0) {
#ifdef IA64
      char cpu_arch[] = "ia64";
#else
      char cpu_arch[] = "i386";
#endif
      realpath(java_home_var, buf);
      sprintf(buf + strlen(buf), "/jre/lib/%s", cpu_arch);
      if (0 == access(buf, F_OK)) {
	sprintf(buf + strlen(buf), "/hotspot/libjvm%s.so", debug_only("_g")"");
      } else {
        // Go back to path of .so
        realpath((char *)dlinfo.dli_fname, buf);
      }
    }
  } 
#endif // #ifndef PRODUCT

  strcpy(saved_jvm_path, buf);
}

void os::print_jni_name_prefix_on(outputStream* st, int args_size) {
  // no prefix required, not even "_"
}

void os::print_jni_name_suffix_on(outputStream* st, int args_size) {
  // no suffix required
}

////////////////////////////////////////////////////////////////////////////////
// sun.misc.Signal support

static jint sigint_count = 0;

static void
UserHandler(int sig, void *siginfo, void *context) {
  // 4511530 - sem_post is serialized and handled by the manager thread. When
  // the program is interrupted by Ctrl-C, SIGINT is sent to every thread. We
  // don't want to flood the manager thread with sem_post requests.
  if (sig == SIGINT && atomic::add(1, &sigint_count) > 1) 
      return;
  os::signal_notify(sig);
}

void* os::user_handler() {
  return CAST_FROM_FN_PTR(void*, UserHandler);
}

extern "C" {
  typedef void (*sa_handler_t)(int);
  typedef void (*sa_sigaction_t)(int, siginfo_t *, void *);
}

void* os::signal(int signal_number, void* handler) {
  struct sigaction sigAct, oldSigAct;

  sigfillset(&(sigAct.sa_mask));
  sigAct.sa_flags   = SA_RESTART|SA_SIGINFO;
  sigAct.sa_handler = CAST_TO_FN_PTR(sa_handler_t, handler);

  if (sigaction(signal_number, &sigAct, &oldSigAct)) {
    fatal2("sigaction: cannot set signal %d (%s)", signal_number, strerror(errno));
  }

  return CAST_FROM_FN_PTR(void*, oldSigAct.sa_handler);
}

void os::signal_raise(int signal_number) {
  ::raise(signal_number);
}

/*
 * The following code is moved from os.cpp for making this
 * code platform specific, which it is by its very nature.
 */

// Will be modified when max signal is changed to be dynamic
int os::sigexitnum_pd() {
  return NSIG;
}

// a counter for each possible signal value
static jint pending_signals[NSIG+1] = { 0 };

// Linux(POSIX) specific hand shaking semaphore.
static sem_t sig_sem;

void os::signal_init_pd() {
  // Initialize signal structures
  ::memset(pending_signals, 0, sizeof(pending_signals));

  // Initialize signal semaphore
  ::sem_init(&sig_sem, 0, 0);
}

void os::signal_notify(int sig) {
  atomic::increment(&pending_signals[sig]);
  ::sem_post(&sig_sem);
}

static int check_pending_signals(bool wait) {
  atomic::store(0, &sigint_count);
  for (;;) {
    for (int i = 0; i < NSIG + 1; i++) {
      jint n = pending_signals[i];
      if (n > 0 && n == atomic::compare_and_exchange(n - 1, &pending_signals[i], n)) {
        return i;
      }
    }
    if (!wait) {
      return -1;
    }
    JavaThread *thread = JavaThread::current();
    ThreadBlockInVM tbivm(thread);

    bool threadIsSuspended;
    do {
      thread->set_suspend_equivalent();
      // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
      ::sem_wait(&sig_sem);

      // were we externally suspended while we were waiting?
      threadIsSuspended = thread->handle_special_suspend_equivalent_condition();
      if (threadIsSuspended) {
        //
        // The semaphore has been incremented, but while we were waiting
        // another thread suspended us. We don't want to continue running
        // while suspended because that would surprise the thread that
        // suspended us.
        //
        ::sem_post(&sig_sem);

        StateSaver sv(thread);
        thread->java_suspend_self();
      }
    } while (threadIsSuspended);
  }
}

int os::signal_lookup() {
  return check_pending_signals(false);
}

int os::signal_wait() {
  return check_pending_signals(true);
}

////////////////////////////////////////////////////////////////////////////////
// Virtual Memory


int os::vm_page_size() {
  // Seems redundant as all get out
  assert(os::Linux::page_size() != -1, "must call os::init");
  return os::Linux::page_size();
}

// NOTE: Linux kernel does not really reserve the pages for us. 
//       All it does is to check if there are enough free pages 
//       left at the time of mmap(). This could be a potential  
//       problem.                                               
bool os::commit_memory(char* addr, size_t size) {
// return ::mmap(addr, size,
//               PROT_READ|PROT_WRITE|PROT_EXEC,
//               MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0)
//   != MAP_FAILED;

  uintptr_t res = (uintptr_t) ::mmap(addr, size,
		                     PROT_READ|PROT_WRITE|PROT_EXEC,
                                     MAP_PRIVATE|MAP_FIXED|MAP_ANONYMOUS, -1, 0);
  int err = errno;
  return res != (uintptr_t) MAP_FAILED;
}


bool os::uncommit_memory(char* addr, size_t size) {
  return ::mmap(addr, size,
		PROT_READ|PROT_WRITE|PROT_EXEC,
		MAP_PRIVATE|MAP_FIXED|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0)
    != MAP_FAILED;
}

bool os::release_memory(char* addr, size_t size) {
  return ::munmap(addr, size) == 0;
}

bool os::protect_memory(char* addr, size_t size) {
  return ::mprotect(addr, (size_t) size, PROT_READ) == 0;
}

bool os::guard_memory(char* addr, size_t size) {
  return ::mprotect(addr, (size_t) size, PROT_NONE) == 0;
}

bool os::unguard_memory(char* addr, size_t size) {
  return ::mprotect(addr, (size_t) size, PROT_READ|PROT_WRITE|PROT_EXEC) == 0;
}
/*
static int os_sleep(jlong millis) {
  const jlong limit = INT_MAX;
  while (millis > limit) {
    if (os_sleep(limit) == -1) {
      return -1;
    }
    millis -= limit;
  }

  struct timeval tv;
  tv.tv_sec  = millis / 1000;
  tv.tv_usec = (millis % 1000) * 1000;
  return select(0, NULL, NULL, NULL, &tv);
}
*/
int os::sleep(Thread* thread, jlong millis, bool interruptible) {
  assert(thread == Thread::current(),  "thread consistency check");

  OSThread* osthread = thread->osthread();
  if (interruptible) {
    if (os::is_interrupted(thread, true)) {
      return OS_INTRPT;
    }
    {
      StateSaver sv((JavaThread*)thread);
      ThreadBlockInVM tbivm((JavaThread*) thread);
      OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
      Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
      event->down(millis);

#if 0
      // XXX - This code was not exercised during the Merlin
      // pre-integration test cycle so it has been removed to
      // reduce risk.
      //
      // were we externally suspended while we were waiting?
      if (((JavaThread *) thread)->is_external_suspend_with_lock()) {
        //
        // While we were waiting in down() another thread suspended us.
        // We don't want to continue running while suspended because
        // that would surprise the thread that suspended us.
        //
        // StateSaver constructed above
        ((JavaThread *) thread)->java_suspend_self();
      }
#endif
    }
    if (os::is_interrupted(thread, true)) {
      return OS_INTRPT;
    }
  } else {
    OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    event->down(millis);
  }

  return OS_TIMEOUT;
}

// Sleep forever; naked call to OS-specific sleep; use with CAUTION
void os::infinite_sleep() {
  while (true) {    // sleep forever ...
    ::sleep(100);   // ... 100 seconds at a time
  }
}

// Used to convert frequent JVM_Yield() to nops
bool os::dont_yield() {
  return DontYieldALot;
}

void os::yield() {
  sched_yield();
}

void os::yield_all(int attempts) {
  // Yields to all threads, including threads with lower priorities
  // Threads on Linux are all with same priority. The Solaris style
  // os::yield_all() with nanosleep(1ms) is not necessary.
  sched_yield();
}

// Called from the tight loops to possibly influence time-sharing heuristics
void os::loop_breaker(int attempts) {
  os::yield_all(attempts);
}

////////////////////////////////////////////////////////////////////////////////
// thread priority support

// TODO: the entire priority issue needs to be overhauled.

#define T_PRI_MIN   1
#define T_PRI_NORM  1
#define T_PRI_MAX   1

int os::java_to_os_priority[MaxPriority + 1] = {
  0,              // 0 Entry should never be used

  1,              // 1 MinPriority
  2,              // 2
  3,              // 3

  4,              // 4
  5,              // 5 NormPriority
  6,              // 6

  7,              // 7
  8,              // 8
  9,              // 9 NearMaxPriority

  10              // 10 MaxPriority
};

// Note: LinuxThreads only honor thread priority for real time threads.
// sched_priority is ignored if policy is SCHED_OTHER. This function is
// equivalent to a "noop" on current Linux platforms.
OSReturn os::set_native_priority(Thread* thread, int newpri) {
  pthread_t thr = thread->osthread()->pthread_id();
  int policy = SCHED_OTHER;
  struct sched_param param;
  param.sched_priority = newpri;
  int ret = pthread_setschedparam(thr, policy, &param);
  return (ret == 0) ? OS_OK : OS_ERR;
}

OSReturn os::get_native_priority(Thread* thread, int *priority_ptr) {
  pthread_t thr = thread->osthread()->pthread_id();
  int policy = SCHED_OTHER;
  struct sched_param param;
  int ret = pthread_getschedparam(thr, &policy, &param);
  if (param.sched_priority < MinPriority) {
    *priority_ptr = MinPriority;
  } else if (param.sched_priority > MaxPriority) {
    *priority_ptr = MaxPriority;
  } else {
    *priority_ptr = param.sched_priority;
  }
  return (ret == 0) ? OS_OK : OS_ERR;
}

// Priority boosting/inheritence/inversion needed by safepoint code
bool os::boost_priority(Thread *thread, int max_java_priority) {
  return true;
}

bool os::deflate_priority(Thread *thread, int min_java_priority) {
  return true;
}

// Hint to the underlying OS that a task switch would not be good.
// Void return because it's a hint and can fail.
void os::hint_no_preempt() {}

////////////////////////////////////////////////////////////////////////////////
// suspend/resume/interrupt support

static void resume_clear_context(OSThread *osthread) {
  osthread->set_ucontext(NULL);
  osthread->set_siginfo(NULL);

  // notify the suspend action is completed, we have now resumed
  osthread->sr.clear_suspended();
}

static void suspend_save_context(OSThread *osthread, siginfo_t* siginfo, ucontext_t* context) {
  osthread->set_ucontext(context);
  osthread->set_siginfo(siginfo);
}

//
// Handler function invoked when a thread's execution is suspended or
// resumed. We have to be careful that only async-safe functions are
// called here (Note: most pthread functions are not async safe and 
// should be avoided.)
//
// Note: sigwait() is a more natural fit than sigsuspend() from an
// interface point of view, but sigwait() prevents the signal hander
// from being run. libpthread would get very confused by not having
// its signal handlers run and prevents sigwait()'s use with the
// mutex granting granting signal.
//
static void SR_handler(int sig, siginfo_t* siginfo, ucontext_t* context) {
  // Save and restore errno to avoid confusing native code with EINTR
  // after sigsuspend.
  int old_errno = errno;

  Thread* thread = Thread::current();
  OSThread* osthread = thread->osthread();

  // read current suspend action
  int action = osthread->sr.suspend_action();
  if (action == SR_SUSPEND) {

    assert(thread->is_Java_thread(), "Must be Java thread");
    JavaFrameAnchor orig;
    JavaThread* jt = (JavaThread*) thread;
    // If the thread is in native then this suspend is good point to make the
    // stack walkable until we leave native. In actual fact this suspend (on ia64
    // anyway) is most likely the vmThread suspending us to make the stack walkable
    // in trying to reach a safepoint. When we transition back to java the state
    // will be reset to not walkable

#ifdef IA64
    // This flush is needed for profiling since this is a forced
    // suspension.
    if ( Arguments::has_profile() ) {
      (void)StubRoutines::ia64::flush_register_stack()();
    }
#endif

    if (jt->thread_state() == _thread_in_native) {
      jt->frame_anchor()->make_walkable(true, jt);
    }
    // 
    //
    // We are only "logically" blocked.
    // If we are not in thread_in_native then the stack is only walkable (and stable)
    // while we are looping here. We save the current state and make the stack walkable
    // when this suspension ends we will mark stack as unwalkable (since the frame is
    // unstable) and restore this state.

    jt->save_state_before_block(&orig);
    suspend_save_context(osthread, siginfo, context);

    // Notify the suspend action is about to be completed. do_suspend()
    // waits until SR_SUSPENDED is set and then returns. We will wait
    // here for a resume signal and that completes the suspend-other
    // action. do_suspend() will return to eventually release the
    // SR_lock and return to the caller. At that point it is
    // possible for a racing resume request to get the SR_lock
    // and send a resume signal before we call sigsuspend() below.
    // Fortunately, the resume signal is blocked until we call
    // sigsuspend().
    //
    // A special case for suspend-other is when we come here
    // while waiting for a mutex, we don't want to get suspended
    // in this handler. Otherwise, VM would hang when Linux decides 
    // to grant that mutex to us. - We will return immediately and 
    // let safe_mutex_lock() handle this.

    // notify the caller
    osthread->sr.set_suspended();

    if (osthread->sr.is_try_mutex_enter()) {
      // thread suspension while waiting for a mutex is implemented as a 
      // spin inside safe_mutex_lock().

      // This ucontext/siginfo is no longer valid but we are still logically suspended
      osthread->set_ucontext(NULL);
      osthread->set_siginfo(NULL);
      errno = old_errno;
      return;
    }

    sigset_t suspend_set;  // signals for sigsuspend()

    // get current set of blocked signals and unblock resume signal
    pthread_sigmask(SIG_BLOCK, NULL, &suspend_set);
    sigdelset(&suspend_set, SR_signum);

    // wait here until we are resumed
    do {
      sigsuspend(&suspend_set);
      // ignore all returns until we get a resume signal
    } while (osthread->sr.suspend_action() != SR_CONTINUE);

    resume_clear_context(osthread);
    jt->restore_state_after_block(&orig);

  } else if (action == SR_CONTINUE && osthread->sr.is_try_mutex_enter()) {

    // Normally, we receive a resume signal in the sigsuspend()
    // call above and the SR_handler() call resulting from the resume
    // signal does no work because is_try_mutex_enter() is false.
    // However, if we were suspended while trying to enter a mutex,
    // then we are not suspended in the above sigsuspend() loop,
    // instead, we are doing a spin inside safe_mutex_lock(), we 
    // need to change suspend_action here.
    resume_clear_context(osthread);
  }

  errno = old_errno;
}


static int SR_initialize() {
  struct sigaction act;
  char *s;
  /* Get signal number to use for suspend/resume */
  if ((s = ::getenv("_JAVA_SR_SIGNUM")) != 0) {
    int sig = ::strtol(s, 0, 10);
    if (sig > 0 || sig < _NSIG) {
	SR_signum = sig;
    }
  }

  assert(SR_signum > SIGSEGV && SR_signum > SIGBUS,
        "SR_signum must be greater than max(SIGSEGV, SIGBUS), see 4355769");

  sigemptyset(&SR_sigset);
  sigaddset(&SR_sigset, SR_signum);

  /* Set up signal handler for suspend/resume */
  act.sa_flags = SA_RESTART|SA_SIGINFO;
  act.sa_handler = (void (*)(int)) SR_handler;
  // SR_signum is blocked by default. 
  // 4528190 - We also need to block pthread restart signal (32 on all
  // supported Linux platforms). Note that LinuxThreads need to block
  // this signal for all threads to work properly. So we don't have
  // to use hard-coded signal number when setting up the mask.
  pthread_sigmask(SIG_BLOCK, NULL, &act.sa_mask);
  assert(sigismember(&act.sa_mask, 32), "incompatible pthread library");
  if (sigaction(SR_signum, &act, 0) == -1) {
    return -1;
  }
  return 0;
}

static int SR_finalize() {
  return 0;
}


int do_suspend(OSThread* osthread, int action) {
  int ret;

  // set suspend action and send signal
  osthread->sr.set_suspend_action(action);

  ret = pthread_kill(osthread->pthread_id(), SR_signum);
  // check return code and wait for notification

  if (ret == 0) {
    for (int i = 0; !osthread->sr.is_suspended(); i++) {
      os::yield_all(i);
    }
  }

  osthread->sr.set_suspend_action(SR_NONE);
  return ret;
}

// Suspend another thread by one level. The VM code tracks suspend
// nesting and handles self-suspension via wait on SR_lock.
// Returns zero on success.
// Solaris found that non-compiler threads also needed ThreadCritical
// May also need to free malloc, etc. locks here. fence currently always true.
int os::pd_suspend_thread(Thread* thread, bool fence) {
    int ret;
    OSThread *osthread = thread->osthread();

    if (fence) {
      ThreadCritical tc;
      ret = do_suspend(osthread, SR_SUSPEND);
    } else {
      ret = do_suspend(osthread, SR_SUSPEND);
    }
    return ret;
}

// Resume a thread by one level.  This method assumes that consecutive
// suspends nest and require matching resumes to fully resume.  Note that
// this is different from Java's Thread.resume, which always resumes any
// number of nested suspensions.  The ability to nest suspensions is used
// by other facilities like safe points.  
// Resuming a thread that is not suspended is a no-op.
// Returns zero on success.

int os::pd_resume_thread(Thread* thread) {
  OSThread *osthread = thread->osthread();
  int ret;
  int i;

  // higher layers should catch resume of a thread that is not suspended
  assert(osthread->sr.is_suspended(), "thread should be suspended");
  if (!osthread->sr.is_suspended()) {
    return -1;  // robustness
  }

  osthread->sr.set_suspend_action(SR_CONTINUE);

  ret = pthread_kill(osthread->pthread_id(), SR_signum);

  if (ret == 0) {
    for (i = 0; osthread->sr.is_suspended(); i++) {
      os::yield_all(i);
    }
  }

  osthread->sr.set_suspend_action(SR_NONE);
  return ret;
}

void os::interrupt(Thread* thread) {
  assert(Thread::current() == thread || Threads_lock->owned_by_self(),
    "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();

  if (!osthread->interrupted()) {
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    osthread->set_interrupted(true);
    event->set();
  }
}

bool os::is_interrupted(Thread* thread, bool clear_interrupted) {
  assert(Thread::current() == thread || Threads_lock->owned_by_self(),
    "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();

  bool interrupted = osthread->interrupted();

  if (interrupted && clear_interrupted) {
    Linux::Event* event = (Linux::Event*) osthread->interrupt_event();
    osthread->set_interrupted(false);
    event->reset();
  }

  return interrupted;
}

// Used for PostMortemDump
extern "C" void safepoints();
extern "C" void find(int x);
extern "C" void findpc(int x);
extern "C" void events();
extern void set_error_reported();
#ifndef PRODUCT
extern "C" void ps();
#endif

///////////////////////////////////////////////////////////////////////////////////
// signal handling (expect suspend/resume/interrupt)

#ifndef PRODUCT
void continue_with_dump(void) {
  tty->print_cr("Dumping thread state before exit....");
  ps();
  fatal("Exiting after thread state dump.");
}
#endif


// This routine may be used by user applications as a "hook" to catch signals.
// The user-defined signal handler must pass unrecognized signals to this
// routine, and if it returns true (non-zero), then the signal handler must
// return immediately.  If the flag "abort_if_unrecognized" is true, then this
// routine will never retun false (zero), but instead will execute a VM panic
// routine kill the process.
//
// If this routine returns false, it is OK to call it again.  This allows
// the user-defined signal handler to perform checks either before or after
// the VM performs its own checks.  Naturally, the user code would be making
// a serious error if it tried to handle an exception (such as a null check
// or breakpoint) that the VM was generating for its own correct operation.
//
// This routine may recognize any of the following kinds of signals:
//    SIGBUS, SIGSEGV, SIGILL, SIGFPE, SIGQUIT, SIGPIPE, SIGUSR1.
// It should be consulted by handlers for any of those signals.
//
// The caller of this routine must pass in the three arguments supplied
// to the function referred to in the "sa_sigaction" (not the "sa_handler")
// field of the structure passed to sigaction().  This routine assumes that
// the sa_flags field passed to sigaction() includes SA_SIGINFO and SA_RESTART.
//
// In order for stack overflow checking to operate correctly, the handler
// for SIGSEGV should also have to specify the SA_ONSTACK bit.
//
// Note that the VM will print warnings if it detects conflicting signal
// handlers, unless invoked with the option "-XX:+AllowUserSignalHandlers".
//
extern "C" int
JVM_handle_linux_signal(int signo, siginfo_t* siginfo,
                        void* ucontext, int abort_if_unrecognized);

void signalHandler(int sig, siginfo_t* info, void* uc) {
  assert(info != NULL && uc != NULL, "it must be old kernel");
  JVM_handle_linux_signal(sig, info, uc, true);
}


// This boolean allows users to forward their own non-matching signals
// to JVM_handle_linux_signal, harmlessly.
bool os::Linux::signal_handlers_are_installed = false;

bool os::Linux::is_sig_ignored(int sig) {
      struct sigaction oact;
      sigaction(sig, (struct sigaction*)NULL, &oact);
      void* ohlr = oact.sa_sigaction ? CAST_FROM_FN_PTR(void*,  oact.sa_sigaction)
                                     : CAST_FROM_FN_PTR(void*,  oact.sa_handler);
      if (ohlr == CAST_FROM_FN_PTR(void*, SIG_IGN))
           return true;
      else 
           return false;
}

// For signal-chaining
struct sigaction os::Linux::sigact[MAXSIGNUM];
unsigned int os::Linux::sigs = 0;
bool os::Linux::libjsig_is_loaded = false;
typedef struct sigaction *(*get_signal_t)(int);
get_signal_t os::Linux::get_signal_action = NULL;

struct sigaction* os::Linux::get_chained_signal_action(int sig) {
  struct sigaction *actp = NULL;
 
  if (libjsig_is_loaded) {
    // Retrieve the old signal handler from libjsig
    actp = (*get_signal_action)(sig);
  }
  if (actp == NULL) {
    // Retrieve the preinstalled signal handler from jvm
    actp = get_preinstalled_handler(sig);
  }

  return actp;
}

bool os::Linux::chained_handler(struct sigaction *actp, int sig,
                                  siginfo_t *siginfo, void *context) {
  // Call the old signal handler
  if (actp->sa_handler == SIG_DFL) {
    // It's more reasonable to let jvm treat it as an unexpected exception
    // instead of taking the default action.
    return false;
  } else if (actp->sa_handler != SIG_IGN) {
    if ((actp->sa_flags & SA_NODEFER) == 0) {
      // automaticlly block the signal
      sigaddset(&(actp->sa_mask), sig);
    }

    sa_handler_t hand;
    sa_sigaction_t sa;
    bool siginfo_flag_set = (actp->sa_flags & SA_SIGINFO) != 0;
    // retrieve the chained handler
    if (siginfo_flag_set) {
      sa = actp->sa_sigaction;
    } else {
      hand = actp->sa_handler;
    }

    if ((actp->sa_flags & SA_RESETHAND) != 0) {
      actp->sa_handler = SIG_DFL;
    }

    // try to honor the signal mask
    sigset_t oset;
    pthread_sigmask(SIG_SETMASK, &(actp->sa_mask), &oset);

    // call into the chained handler
    if (siginfo_flag_set) {
      (*sa)(sig, siginfo, context);
    } else {
      (*hand)(sig);
    }

    // restore the signal mask
    pthread_sigmask(SIG_SETMASK, &oset, 0);
  }
  // Tell jvm's signal handler the signal is taken care of.
  return true;
}

struct sigaction* os::Linux::get_preinstalled_handler(int sig) {
  if ((( (unsigned int)1 << sig ) & sigs) != 0) {
    return &sigact[sig];
  }
  return NULL;
}

void os::Linux::save_preinstalled_handler(int sig, struct sigaction& oldAct) {
  assert(sig > 0 && sig < MAXSIGNUM, "vm signal out of expected range");
  sigact[sig] = oldAct;
  sigs |= (unsigned int)1 << sig;
}

void os::Linux::set_signal_handler(int sig, bool set_installed) {
  // Check for overwrite.
  struct sigaction oldAct;
  sigaction(sig, (struct sigaction*)NULL, &oldAct);

  void* oldhand = oldAct.sa_sigaction
                ? CAST_FROM_FN_PTR(void*,  oldAct.sa_sigaction)
		: CAST_FROM_FN_PTR(void*,  oldAct.sa_handler);
  if (oldhand != CAST_FROM_FN_PTR(void*, SIG_DFL) &&
      oldhand != CAST_FROM_FN_PTR(void*, SIG_IGN) &&
      oldhand != CAST_FROM_FN_PTR(void*, (sa_sigaction_t)signalHandler)) {
    if (AllowUserSignalHandlers || !set_installed) {
      // Do not overwrite; user takes responsibility to forward to us.
      return;
    } else if (UseSignalChaining) {
      // save the old handler in jvm
      save_preinstalled_handler(sig, oldAct);
      // libjsig also interposes the sigaction() call below and saves the
      // old sigaction on it own.
    } else {
      fatal2("Encountered unexpected pre-existing sigaction handler %#lx for signal %d.", (long)oldhand, sig);
    }
  }

  struct sigaction sigAct;
  sigfillset(&(sigAct.sa_mask));
  sigAct.sa_handler = SIG_DFL;
  if (!set_installed) {
    sigAct.sa_flags = SA_SIGINFO|SA_RESTART;
  } else {
    sigAct.sa_sigaction = signalHandler;
    sigAct.sa_flags = SA_SIGINFO|SA_RESTART;
    if (sig == SIGCHLD) {
      sigAct.sa_flags |= SA_NOCLDSTOP;
    } else if (sig == SIGSEGV) {
      sigAct.sa_flags = SA_SIGINFO|SA_RESTART|SA_ONSTACK;
    }
  }

  int ret = sigaction(sig, &sigAct, &oldAct);
  assert(ret == 0, "check");

  void* oldhand2  = oldAct.sa_sigaction
                  ? CAST_FROM_FN_PTR(void*, oldAct.sa_sigaction)
                  : CAST_FROM_FN_PTR(void*, oldAct.sa_handler);
  assert(oldhand2 == oldhand, "no concurrent signal handler installation");
}

// install signal handlers for signals that HotSpot needs to
// handle in order to support Java-level exception handling.

void os::Linux::install_signal_handlers() {
  if (!signal_handlers_are_installed) {
    signal_handlers_are_installed = true;

    // signal-chaining
    typedef void (*signal_setting_t)();
    signal_setting_t begin_signal_setting = NULL;
    signal_setting_t end_signal_setting = NULL;
    begin_signal_setting = CAST_TO_FN_PTR(signal_setting_t,
                             dlsym(RTLD_DEFAULT, "JVM_begin_signal_setting"));
    if (begin_signal_setting != NULL) {
      end_signal_setting = CAST_TO_FN_PTR(signal_setting_t,
                             dlsym(RTLD_DEFAULT, "JVM_end_signal_setting"));
      get_signal_action = CAST_TO_FN_PTR(get_signal_t,
                            dlsym(RTLD_DEFAULT, "JVM_get_signal_action"));
      libjsig_is_loaded = true;
      assert(UseSignalChaining, "should enable signal-chaining");
    }
    if (libjsig_is_loaded) {
      // Tell libjsig jvm is setting signal handlers
      (*begin_signal_setting)();
    }

    set_signal_handler(SIGSEGV, true);
    set_signal_handler(SIGPIPE, true);
    set_signal_handler(SIGCHLD, true);
    set_signal_handler(SIGBUS, true);
    set_signal_handler(SIGILL, true);
    set_signal_handler(SIGFPE, true);

    if (libjsig_is_loaded) {
      // Tell libjsig jvm finishes setting signal handlers
      (*end_signal_setting)();
    }
  }
}

extern void report_error(char* file_name, int line_no, char* title, char* format, ...);

const char * os::exception_name(int exception_code)
{
  return NULL;
}

extern "C"{
static void exception_handler_during_fatal_error(int);
}

/* a very simple exception handler designed to print out at least some information */
/* when the fatal error handler crashes.                                           */
static void exception_handler_during_fatal_error(int sig)
{
   os::handle_recursive_fatal_error(sig);
   /* won't reach here */
   os::abort();
}

void os::set_exception_handler_during_fatal_error(int sig, int is_install)
{
   if (is_install)
        os::signal(sig, CAST_FROM_FN_PTR(void *, exception_handler_during_fatal_error));
   else os::signal(sig, CAST_FROM_FN_PTR(void *, SIG_DFL));
}

// this is called _before_ the global arguments have been parsed
void os::init(void)
{
  char dummy;	/* used to get a guess on initial stack address */
//  first_hrtime = gethrtime();

  _initial_pid = getpid();

  init_random(1234567);

  ThreadCritical::initialize();

  Linux::set_page_size(sysconf(_SC_PAGESIZE));

  if (Linux::page_size() == -1) {
    fatal1("os_linux.cpp: os::init: sysconf failed (%s)", strerror(errno));
  }

  Linux::initialize_system_info();

  // main_thread points to the aboriginal thread
  Linux::_main_thread = pthread_self();

  // locate pthread_getattr_np and test if we are on floating stack
  void * h = dlopen(NULL, RTLD_LAZY);
  assert(h != NULL, "just checking");
  Linux::set_pthread_getattr_func(dlsym(h, "pthread_getattr_np"));

  initial_time_count = os::elapsed_counter();
}

// To install functions for atexit system call
extern "C" {
  static void perfMemory_exit_helper() {
    perfMemory_exit();
  }
}

// this is called _after_ the global arguments have been parsed
jint os::init_2(void)
{
  if (PrintMiscellaneous && (Verbose || WizardMode)) {
     if (Linux::is_floating_stack()) {
       tty->print_cr("[HotSpot is running with floating stack LinuxThreads]");
     } else {
       tty->print_cr("[HotSpot is running with fixed stack LinuxThreads]");
     }
  }

  if (UseTopLevelExceptionFilter) {
    Linux::install_signal_handlers();
  }

  size_t threadStackSizeInBytes = ThreadStackSize * K;
  if (threadStackSizeInBytes != 0 &&
      threadStackSizeInBytes < Linux::min_stack_allowed) {
        tty->print_cr("\nThe stack size specified is too small, "
                      "Specify at least %dk",
                      Linux::min_stack_allowed / K);
        return JNI_ERR;
  }

  // Make the stack size a multiple of the page size so that
  // the yellow/red zones can be guarded.
  JavaThread::set_stack_size_at_create(round_to(threadStackSizeInBytes,
        vm_page_size()));

  Linux::capture_initial_stack(JavaThread::stack_size_at_create());

  // initialize suspend/resume support
  if (SR_initialize() != 0) {
    perror("SR_initialize failed");
    return JNI_ERR;
  }

  if (MaxFDLimit) {
    // set the number of file descriptors to max. print out error
    // if getrlimit/setrlimit fails but continue regardless.
    struct rlimit nbr_files;
    int status = getrlimit(RLIMIT_NOFILE, &nbr_files);
    if (status != 0) {
      if (PrintMiscellaneous && (Verbose || WizardMode))
        perror("os::init_2 getrlimit failed");
    } else {
      nbr_files.rlim_cur = nbr_files.rlim_max;
      status = setrlimit(RLIMIT_NOFILE, &nbr_files);
      if (status != 0) {
        if (PrintMiscellaneous && (Verbose || WizardMode))
          perror("os::init_2 setrlimit failed");
      }
    }
  }

  // Initialize lock used to serialize thread creation (see os::create_thread)
  Linux::set_createThread_lock(new Mutex(Mutex::leaf, "createThread_lock", false));

  // Initialize HPI.
  jint hpi_result = hpi::initialize();
  if (hpi_result != JNI_OK) {
    tty->print_cr("There was an error trying to initialize the HPI library.");
    tty->print_cr("Please check your installation, HotSpot does not work correctly");
    tty->print_cr("when installed in the JDK 1.2 Linux Production Release, or");
    tty->print_cr("with any JDK 1.1.x release.");
    return hpi_result;
  }

  // at-exit methods are called in the reverse order of their registration.
  // atexit functions are called on return from main or as a result of a
  // call to exit(3C). There can be only 32 of these functions registered
  // and atexit() does not set errno.

  if (PerfAllowAtExitRegistration) {
    // only register atexit functions if PerfAllowAtExitRegistration is set.
    // atexit functions can be delayed until process exit time, which
    // can be problematic for embedded VM situations. Embedded VMs should
    // call DestroyJavaVM() to assure that VM resources are released.

    // note: perfMemory_exit_helper atexit function may be removed in
    // the future if the appropriate cleanup code can be added to the
    // VM_Exit VMOperation's doit method.
    if (atexit(perfMemory_exit_helper) != 0) {
      warning("os::init2 atexit(perfMemory_exit_helper) failed");
    }
  }

  return JNI_OK;
}

void os::exit(void) {
  // cleanup in the reverse order

  SR_finalize();

  // Make sure remove the alternate signal stack
  Linux::remove_alternate_signal_stack();

  ThreadCritical::release();
}

int os::active_processor_count() {
  // Linux doesn't yet have a (official) notion of processor sets,
  // so just return the number of online processors.
  int online_cpus = ::sysconf(_SC_NPROCESSORS_ONLN);
  assert(online_cpus > 0 && online_cpus <= Linux::processor_count(), "sanity check");
  return online_cpus;
}

bool os::distribute_processes(uint length, uint* distribution) {
  // Not yet implemented.
  return false;
}

bool os::bind_to_processor(uint processor_id) {
  // Not yet implemented.
  return false;
}

/// 

// A lightweight implementation that does not suspend the target thread and
// thus returns only a hint. Used for profiling only!
ExtendedPC os::get_thread_pc(Thread* thread) {
  // Make sure that it is called by the watcher and the Threads lock is owned.
  assert(Thread::current()->is_Watcher_thread(), "Must be watcher and own Threads_lock");
  // For now, is only used to profile the VM Thread
  assert(thread->is_VM_thread(), "Can only be called for VMThread");

  ExtendedPC epc;

  thread->vm_suspend();
  OSThread* osthread = thread->osthread();
  if (osthread->ucontext() != NULL) {
    epc = os::Linux::ucontext_get_pc(osthread->ucontext());
  } else {
    assert(thread->is_Java_thread(), "Must be Java thread");
    JavaThread* jt = (JavaThread*)thread;
    // Does not need to be walkable sicne we just want pc for sampling
    assert(jt->has_last_Java_frame(), "must have last Java frame");
    epc = jt->frame_anchor()->last_Java_pc();
  }
  thread->vm_resume(false);  // don't yield

  return epc;
}

ExtendedPC os::fetch_top_frame(Thread* thread, intptr_t** sp, intptr_t** fp) {
  OSThread* osthread = thread->osthread();
  assert(thread->is_vm_suspended(), "must be suspended");
  ucontext* uc = osthread->ucontext();
  if ( uc != NULL) {
    *sp = os::Linux::ucontext_get_sp(uc);
    *fp = os::Linux::ucontext_get_fp(uc);
    ExtendedPC pc(os::Linux::ucontext_get_pc(uc));
    return pc;
  }
  assert(thread->is_Java_thread(), "Must be Java thread");
  JavaThread* jt = (JavaThread*)thread;

  // We used to have the following:
  //   assert(jt->has_last_Java_frame(), "must have last Java frame");
  //
  // When a new thread is attached via jni_AttachCurrentThread(), it
  // is possible for it to self-suspend in the JavaCallWrapper ctr
  // after it has changed state to _thread_in_Java. If the VMThread
  // is trying to safepoint at the same time as the thread is trying
  // to self-suspend, then we have a race. If the VMThread makes it
  // past its is_any_suspended_with_lock() check before the thread
  // self-suspends, then it will vm_suspend() the thread and try to
  // fetch the top frame. However, the thread hasn't run any Java
  // code yet so _last_Java_sp is NULL. Returning a NULL ExtendedPC
  // will cause the safepoint to retry this thread.
  if (!jt->has_last_Java_frame()) {
    *sp = NULL;
    *fp = NULL;
    ExtendedPC pc;  // create a NULL ExtendedPC
    return pc;
  }
  *sp = jt->last_Java_sp();
  *fp = jt->frame_anchor()->last_Java_fp();
  ExtendedPC pc(jt->frame_anchor()->last_Java_pc());
  return pc;
}

// Hotspot uses signal to implement suspend/resume on Linux. However,
// because LinuxThreads knows nothing about thread suspension in Hotspot,
// it can grant mutex to an already suspended thread. That may cause
// VM hang when it involves locks like SR_lock or ThreadCritical, 
// because the thread that can resume us may not be able to obtain the 
// mutex (it is now owned by us) to complete the task.
// To avoid such hangs, SR_handler() returns immediately if it discovers 
// the thread is waiting for a mutex. We will spin here - to let other 
// threads grab the mutex - as long as we are suspended.
// Note we cannot use any lock in this function nor use higher level self
// suspension logic - that will invalidate the whole idea.
//
// TODO: this function does not provide any help for native code that 
//       calls pthread_mutex_lock() directly, or for glibc function,
//       e.g. malloc(), that needs to lock mutex. A better implementation 
//       might let SR_handler() to walk stack and discover if it's safe
//       or not to suspend. Of course, polling would always be an attractive 
//       alternative to solve suspend/resume problems. Also, this 
//       function could be implemented with CAS, that probably can save a 
//       couple of suspend/resume signals flying around.

extern sigset_t SR_sigset;
#define BLOCK_SR_SIGNAL   pthread_sigmask(SIG_BLOCK, &SR_sigset, NULL)
#define UNBLOCK_SR_SIGNAL pthread_sigmask(SIG_UNBLOCK, &SR_sigset, NULL)

static inline int spin_if_suspended(OSThread* osthread, pthread_mutex_t*_mutex,
                                    int status)
{
  // It is possible that we are interrupted by a suspend signal when we
  // test flag SR_SUSPENDED and clear SR_TRY_MUTEX_ENTER. Fortunately this
  // function is implemented by CAS, so the "test" and "clear" are done
  // atomically.
  while(!osthread->sr.clear_try_mutex_enter_if_not_suspended()) {
    // LinuxThreads just granted the mutex to us, but we are still
    // suspended. We don't want to enter the mutex because otherwise
    // it would surprise the thread that suspended us.
    // Give the mutex to other thread(s) by unlocking it.
    status = pthread_mutex_unlock(_mutex);
    assert(status == 0, "pthread_mutex_unlock");

    // Don't contend the mutex as long as we are still suspended.
    // sigsuspend() will be interrupted by the resume signal.
    // It is necessary to mask suspend/resume signal because we
    // don't want to lose track of resume signal if it is delivered
    // right after we have read suspend_action but before sigsuspend().
    sigset_t suspend_set;
    pthread_sigmask(SIG_BLOCK, NULL, &suspend_set);
    BLOCK_SR_SIGNAL;
    while(osthread->sr.is_suspended()) {
       sigsuspend(&suspend_set);
    }
    UNBLOCK_SR_SIGNAL;

    // try to grab the mutex again
    status = pthread_mutex_lock(_mutex);
    assert(status == 0, "pthread_mutex_lock");
  }

  // if a suspend signal is delivered after this, the thread will be
  // suspended inside SR_handler, because try_mutex_enter is now cleared

  return status;
}

#undef BLOCK_SR_SIGNAL
#undef UNBLOCK_SR_SIGNAL

int os::Linux::safe_mutex_lock(pthread_mutex_t *_mutex)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;
   // Ok for non java threads??? QQQ
   if (!osthread || !t->is_Java_thread() ) {
      return pthread_mutex_lock(_mutex);
   } else {
      JavaFrameAnchor orig;
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->save_state_before_block(&orig);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      // normal mutex locking
      int status = pthread_mutex_lock(_mutex);
      assert(status == 0, "pthread_mutex_lock");

      status = spin_if_suspended(osthread, _mutex, status);
      jt->restore_state_after_block(&orig);

      return status;
   }
}

int os::Linux::safe_cond_wait(pthread_cond_t *_cond, pthread_mutex_t *_mutex)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;
   if (!osthread || !t->is_Java_thread()) {
      return pthread_cond_wait(_cond, _mutex);
   } else {
      JavaFrameAnchor orig;
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->save_state_before_block(&orig);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      int status = pthread_cond_wait(_cond, _mutex);

      spin_if_suspended(osthread, _mutex, status);
      jt->restore_state_after_block(&orig);

      // return the result from pthread_cond_wait().
      return status;
   }
}

int os::Linux::safe_cond_timedwait(pthread_cond_t *_cond, pthread_mutex_t *_mutex, const struct timespec *_abstime)
{
   // get current thread
   Thread *t = ThreadLocalStorage::thread();
   OSThread * osthread = t ? t->osthread() : NULL;

   if (!osthread || !t->is_Java_thread()) {
      return pthread_cond_timedwait(_cond, _mutex, _abstime);
   } else {
      JavaFrameAnchor orig;
      JavaThread* jt = (JavaThread*) t;
      // We are only "logically" blocked.
      jt->save_state_before_block(&orig);
      // try_mutex_enter is cleared in spin_if_suspended
      osthread->sr.set_try_mutex_enter();

      int status = pthread_cond_timedwait(_cond, _mutex, _abstime);

      spin_if_suspended(osthread, _mutex, status);
      jt->restore_state_after_block(&orig);

      // return the result from pthread_cond_timedwait();
      return status;
   }
}

// we need to be able to suspend ourself while at the same (atomic) time
// giving up the SR_lock -- we do this by using the
// SR_lock to implement a suspend_self
int os::pd_self_suspend_thread(Thread* thread) {
    assert(thread->is_Java_thread(), "must be Java thread");
    // We are only "logically" blocked.
    thread->SR_lock()->wait(Mutex::_no_safepoint_check_flag);
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
// debug support

#ifndef PRODUCT
static address same_page(address x, address y) {
  int page_bits = -os::vm_page_size();
  if ((jint(x) & page_bits) == (jint(y) & page_bits))
    return x;
  else if (x > y)
    return (address)(jint(y) | ~page_bits) + 1;
  else
    return (address)(jint(y) & page_bits);
}

bool os::find(address addr) {
  Dl_info dlinfo;
  memset(&dlinfo, 0, sizeof(dlinfo));
  if (dladdr(addr, &dlinfo)) {
    tty->print("0x%08x: ", addr);
    if (dlinfo.dli_sname != NULL)
      tty->print("%s+%#x", dlinfo.dli_sname, addr-(int)dlinfo.dli_saddr);
    else if (dlinfo.dli_fname)
      tty->print("<offset %#x>", addr-(int)dlinfo.dli_fbase);
    else
      tty->print("<absolute address>");
    if (dlinfo.dli_fname)  tty->print(" in %s", dlinfo.dli_fname);
    if (dlinfo.dli_fbase)  tty->print(" at 0x%08x", dlinfo.dli_fbase);
    tty->cr();

    if (Verbose) {
      // decode some bytes around the PC
      address begin = same_page(addr-40, addr);
      address end   = same_page(addr+40, addr);
      address       lowest = (address) dlinfo.dli_sname;
      if (!lowest)  lowest = (address) dlinfo.dli_fbase;
      if (begin < lowest)  begin = lowest;
      Dl_info dlinfo2;
      if (dladdr(end, &dlinfo2) && dlinfo2.dli_saddr != dlinfo.dli_saddr
	  && end > dlinfo2.dli_saddr && dlinfo2.dli_saddr > begin)
        end = (address) dlinfo2.dli_saddr;
      Disassembler::decode(begin, end);
    }
    return true;
  }
  return false;
}

#endif

////////////////////////////////////////////////////////////////////////////////
// misc

// This does not do anything on Linux. This is basically a hook for being
// able to use structured exception handling (thread-local exception filters)
// on, e.g., Win32.
void
os::os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method,
                         JavaCallArguments* args, Thread* thread) {
  f(value, method, args, thread);
}

void os::print_statistics() {
}

static char *get_program_name(char * name_buf) {
  if (realpath("/proc/self/exe", name_buf) == NULL) {
     name_buf[0] = '\0';
  }
  return name_buf;
}

int os::message_box(const char* title, const char* message) {
  ::write(2, title, strlen(title));
  ::write(2, "\n", 1);
  ::write(2, message, strlen(message));
  ::write(2, "\n", 1);

  char buf[256];
  // Prevent process from exiting upon "read error" without consuming all CPU
  while (::read(0, buf, 256) <= 0) { ::sleep(100); }

  return buf[0] == 'y' || buf[0] == 'Y';
}

// append a short instruction on how to debug the dying VM
char *os::do_you_want_to_debug(const char *message) {
  // PATH_MAX is 4096, that's too much stack space to ask for in fatal error
  // handler. Use static buffer instead. os::handle_unexpected_exception() 
  // will guarantee only one thread can call this function.
  static char buf[PATH_MAX + 256];

  sprintf(buf, "%s\nTo debug, use 'gdb ", message);
  get_program_name(buf + strlen(buf));
  
  int len = strlen(buf);

  jio_snprintf(buf + len, sizeof(buf) - len, " %d'; then switch to thread "INTX_FORMAT"\n"
              "Otherwise, press RETURN to abort...",
              current_process_id(), current_thread_id());

  return buf;
}

int os::stat(const char *path, struct stat *sbuf) {
  char pathbuf[MAX_PATH];
  if (strlen(path) > MAX_PATH - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  hpi::native_path(strcpy(pathbuf, path));
  return ::stat(pathbuf, sbuf);
}

bool os::check_heap(bool force) {
  return true;
}

int local_vsnprintf(char* buf, size_t count, const char* format, va_list args) {
  return ::vsnprintf(buf, count, format, args);
}

// JVMPI code
jlong os::thread_cpu_time() {
  //
  // gethrtime() is used in Solaris for this function. However, we
  // don't have gethrtime(). It is also used in the Solaris version
  // of os::elapsed_counter() so we leverage off our implementation
  // of os::elapsed_counter().
  //
  return os::elapsed_counter();
}

bool os::thread_is_running(JavaThread* tp) {
  Unimplemented();
  return false;
}

#ifndef PRODUCT
void os::Linux::Event::verify() {
  guarantee(!Universe::is_fully_initialized() ||
            !Universe::heap()->is_in_reserved((oop)this),
            "Mutex must be in C heap only.");
}

void os::Linux::OSMutex::verify() {
  guarantee(!Universe::is_fully_initialized() || 
    	    !Universe::heap()->is_in_reserved((oop)this), 
    	    "OSMutex must be in C heap only.");
}

void os::Linux::OSMutex::verify_locked() {
  pthread_t my_id = pthread_self();
  assert(_is_owned, "OSMutex should be locked");
  assert(pthread_equal(_owner, my_id), "OSMutex should be locked by me");
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * NOTE: the following code is to keep the green threads code
 * in the libjava.so happy. Once the green threads is removed,
 * these code will no longer be needed.
 */
int
jdk_waitpid(pid_t pid, int* status, int options) {
    return waitpid(pid, status, options);
}

int
fork1() {
    return fork();
}

int
jdk_sem_init(sem_t *sem, int pshared, unsigned int value) {
    return sem_init(sem, pshared, value);
}

int
jdk_sem_post(sem_t *sem) {
    return sem_post(sem);
}

int
jdk_sem_wait(sem_t *sem) {
    return sem_wait(sem);
}

int
jdk_pthread_sigmask(int how , const sigset_t* newmask, sigset_t* oldmask) {
    return pthread_sigmask(how , newmask, oldmask);
}

#ifdef __cplusplus
}
#endif

// Reconciliation History
//
// End
