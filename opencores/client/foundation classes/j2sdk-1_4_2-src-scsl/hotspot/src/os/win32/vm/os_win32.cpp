#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_win32.cpp	1.400 03/04/04 11:04:43 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifdef _M_IA64
// Must be at least Windows 2000 or XP to use VectoredExceptions
#define _WIN32_WINNT 0x500
#endif

// do not include precompiled header file
# include "incls/_os_win32.cpp.incl"

#ifdef _DEBUG
#include <crtdbg.h>
#endif


#include <windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <objidl.h>
#include <shlobj.h>

#include <signal.h>
#include <direct.h>
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>              // For _beginthreadex(), _endthreadex()
#include <imagehlp.h>             // For os::dll_address_to_function_name

/* for enumerating dll libraries */
#include <tlhelp32.h>
#include <vdmdbg.h>

static HANDLE main_process;
static HANDLE main_thread;
static int    main_thread_id;

static FILETIME process_creation_time;
static FILETIME process_exit_time;
static FILETIME process_user_time;
static FILETIME process_kernel_time;

static LPTOP_LEVEL_EXCEPTION_FILTER previous_toplevel_exception_filter = NULL;
#ifdef _M_IA64
PVOID  topLevelVectoredExceptionHandler = NULL;
#endif

#ifdef _M_IA64
#define __CPU__ ia64
#else
#define __CPU__ i486
#endif

// save DLL module handle, used by GetModuleFileName

HINSTANCE vm_lib_handle;

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      vm_lib_handle = hinst;
      if(ForceTimeHighResolution)
        timeBeginPeriod(1L);
      break;
    case DLL_PROCESS_DETACH:
      if(ForceTimeHighResolution)
        timeEndPeriod(1L);
#ifdef _M_IA64
      if (topLevelVectoredExceptionHandler != NULL) {
        RemoveVectoredExceptionHandler(topLevelVectoredExceptionHandler);
        topLevelVectoredExceptionHandler = NULL;
      }
#endif
      break;
    default:
      break;
  }
  return true;
}

static inline double fileTimeAsDouble(FILETIME* time) {
  const double high  = (double) ((unsigned int) ~0);
  const double split = 10000000.0;
  double result = (time->dwLowDateTime / split) + 
                   time->dwHighDateTime * (high/split);
  return result;
}

// Implementation of os

bool os::getenv(const char* name, char* buffer, int len) {
 int result = GetEnvironmentVariable(name, buffer, len);
 return result > 0 && result < len;
}


props_md_t* os::get_system_properties() {
  if (_system_properties != NULL) {
    return _system_properties;
  }
  // create properties structure and fill it
  props_md_t* sprops = new props_md_t();

  // HotSpot integration note:
  // code below matches code in win32 javai_md.c 1.4 98/09/15 from JDK1.3 beta H build

  /* sysclasspath, java_home, dll_dir */
  {
      char *home_path;
      char *dll_path;
      char *bin = "\\bin";
      char home_dir[MAX_PATH];

      if (!getenv("_ALT_JAVA_HOME_DIR", home_dir, MAX_PATH)) {
          GetModuleFileName(vm_lib_handle, home_dir, MAX_PATH);
          *(strrchr(home_dir, '\\')) = '\0';
          *(strrchr(home_dir, '\\')) = '\0';
          *(strrchr(home_dir, '\\')) = '\0';
      }

      home_path = NEW_C_HEAP_ARRAY(char, strlen(home_dir) + 1);
      if (home_path == NULL)
          return NULL;
      strcpy(home_path, home_dir);
      sprops->java_home = home_path;

      dll_path = NEW_C_HEAP_ARRAY(char, strlen(home_dir) + strlen(bin) + 1);
      if (dll_path == NULL)
          return NULL;
      strcpy(dll_path, home_dir);
      strcat(dll_path, bin);
      sprops->dll_dir = dll_path;

      if (!set_boot_path(sprops, '\\', ';'))
	  return NULL;
  }

  /* library_path */
  {
    /* Win32 library search order (See the documentation for LoadLibrary):
     *
     * 1. The directory from which application is loaded.
     * 2. The current directory
     * 3. System directory (GetSystemDirectory)
     * 4. Windows directory (GetWindowsDirectory)
     * 5. The PATH environment variable
     */

    char *library_path;
    char tmp[MAX_PATH];
    char *path_str = ::getenv("PATH");

    library_path = NEW_C_HEAP_ARRAY(char, MAX_PATH * 4 + (path_str ? strlen(path_str) : 0) + 10);

    library_path[0] = '\0';

    GetModuleFileName(NULL, tmp, sizeof(tmp));
    *(strrchr(tmp, '\\')) = '\0';
    strcat(library_path, tmp);

    strcat(library_path, ";.");

    GetSystemDirectory(tmp, sizeof(tmp));
    strcat(library_path, ";");
    strcat(library_path, tmp);

    GetWindowsDirectory(tmp, sizeof(tmp));
    strcat(library_path, ";");
    strcat(library_path, tmp);
  
    if (path_str) {
        strcat(library_path, ";");
        strcat(library_path, path_str);
    }

    sprops->library_path = os::strdup(library_path);
    FREE_C_HEAP_ARRAY(char, library_path);
  }

  /* Default extensions directory */
  {
      char buf[MAX_PATH + 10];
      sprintf(buf, "%s\\lib\\ext", sprops->java_home);
      sprops->ext_dirs = os::strdup(buf);
  }

  /* Default endorsed standards directory. */
  {
    #define ENDORSED_DIR "\\lib\\endorsed"
    size_t len = strlen(sprops->java_home) + sizeof(ENDORSED_DIR);
    char * buf = NEW_C_HEAP_ARRAY(char, len);
    sprintf(buf, "%s%s", sprops->java_home, ENDORSED_DIR);
    sprops->endorsed_dirs = buf;
    #undef ENDORSED_DIR
  }

  // Done
  _system_properties = sprops;
  return sprops;
}

void os::breakpoint() {
  DebugBreak();
}

// Invoked from the BREAKPOINT Macro
extern "C" void breakpoint() {
  os::breakpoint();
}

// Helper function to get access to the current thread's TIB
// (currently only needed for stack overflow handling)

inline static NT_TIB* get_tib() {
#if _M_IA64
  PNT_TIB tib;
  tib = (PNT_TIB)NtCurrentTeb();
#else
  NT_TIB* tib;
  __asm {
    // load thread information block (TIB) pointer from special segment
    mov eax, dword ptr FS:[18H];
    // make compiler happy
    mov tib, eax;
  }
#endif
  return tib;
}

inline static address current_stack_limit() {
  return (address)get_tib()->StackLimit;   
}

// Returns an estimate of the current stack pointer. Result must be guaranteed
// to point into the calling threads stack, and be no lower than the current
// stack pointer.

address os::current_stack_pointer() {
  int dummy;
  address sp = (address)&dummy;
  return sp;
}

address os::current_stack_base() {
  return (address)get_tib()->StackBase;   
}

size_t os::current_stack_size() {
  size_t sz;
  if (os::win32::is_nt()) {
    MEMORY_BASIC_INFORMATION minfo;
    DWORD cnt = VirtualQuery(&minfo, &minfo, sizeof(minfo));
    assert(cnt == sizeof(minfo), "problem running VirtualQuery");

    // BaseAddress + RegionSize is the stack base (highest addr), AllocationBase
    // is the stack end (lowest addr).
#ifdef _WIN64
    //  [RGV] TODO - Investigate this or just use os::current_stack_base
    address stack_base = (address) minfo.BaseAddress + 0x2000;
#else
    address stack_base = (address) minfo.BaseAddress + minfo.RegionSize;
#endif
    assert(stack_base == os::current_stack_base(), "just checking");
    sz = stack_base - (address) minfo.AllocationBase;
  } else {
    sz = (int)current_thread_id() == main_thread_id ||
      JavaThread::stack_size_at_create() == 0 ?
      os::win32::default_stack_size() : JavaThread::stack_size_at_create();
  }
  assert(sz != 0 && (sz & (vm_page_size() - 1)) == 0, "bad stack size");
  return sz;
}

// Thread start routine for all new Java threads
static unsigned __stdcall _start(Thread* thread) {
  OSThread* osthr = thread->osthread();
  assert(osthr->get_state() == RUNNABLE, "invalid os thread state");

  thread->run();
  // One less thread is executing
  // When the VMThread gets here, the main thread may have already exited
  // which frees the CodeHeap containing the atomic::decrement code
  if (thread != VMThread::vm_thread() && VMThread::vm_thread() != NULL) {
    atomic::decrement(&os::win32::_os_thread_count);
  }
  return 0;
}

static OSThread* create_os_thread(Thread* thread, HANDLE thread_handle, int thread_id) {
  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);
  if (osthread == NULL) return NULL;

  // Initial state is ALLOCATED but not INITIALIZED
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(ALLOCATED);
  }

  // Initialize support for Java interrupts
  HANDLE interrupt_event = CreateEvent(NULL, true, false, NULL);
  if (interrupt_event == NULL) {
    delete osthread;
    return NULL;
  }
  osthread->set_interrupt_event(interrupt_event);

  // Store info on the Win32 thread into the OSThread
  osthread->set_thread_handle(thread_handle);
  osthread->set_thread_id(thread_id);

  // Initial thread state is INITIALIZED, not SUSPENDED
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(INITIALIZED);
  }

  return osthread;
}


bool os::create_attached_thread(Thread* anThread) {
  HANDLE thread;
  if (!DuplicateHandle(main_process, GetCurrentThread(), GetCurrentProcess(),
                       &thread, THREAD_ALL_ACCESS, false, 0)) {
    fatal("DuplicateHandle failed\n");
  }
  OSThread* osthread = create_os_thread(anThread, thread, (int)current_thread_id());
  {
    MutexLockerEx ml(anThread->SR_lock(), Mutex::_no_safepoint_check_flag);
    anThread->clear_is_baby_thread();
    osthread->set_state(RUNNABLE);
  }
  anThread->set_osthread(osthread);
  return osthread != NULL;
}

bool os::create_main_thread(Thread* thread) {
  if (_starting_thread == NULL) {
    _starting_thread = create_os_thread(thread, main_thread, main_thread_id);
  }
  // The primordial thread is runnable from the start)
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    thread->clear_is_baby_thread();
    _starting_thread->set_state(RUNNABLE);
  }
  thread->set_osthread(_starting_thread);
  return _starting_thread != NULL;
}

// Allocate and initialize a new OSThread
// Note: thr_type and stack_size arguments are not used on Win32.
bool os::create_thread(Thread* thread, ThreadType thr_type, size_t stack_size) {
  unsigned thread_id;

  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);
  if (osthread == NULL) {
    return false;
  }

  // Initial state is ALLOCATED but not INITIALIZED
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(ALLOCATED);
  }
  
  // Initialize support for Java interrupts
  HANDLE interrupt_event = CreateEvent(NULL, true, false, NULL);
  if (interrupt_event == NULL) {
    delete osthread;
    return NULL;
  }
  osthread->set_interrupt_event(interrupt_event);
  osthread->set_interrupted(false);
  
  if (os::win32::_os_thread_count > os::win32::_os_thread_limit) {
    // We got lots of threads. Check if we still have some address space left.
    // Need to be at least 5Mb of unreserved address space. We do check by
    // trying to reserve some.
    const size_t VirtualMemoryBangSize = 20*K*K;
    char* mem = os::reserve_memory(VirtualMemoryBangSize);
    if (mem == NULL) { 
      delete osthread;    
      return NULL;
    } else {
      // Release the memory again
      (void)os::release_memory(mem, VirtualMemoryBangSize);
    }
  }

  thread->set_osthread(osthread);
  
  // Create the Win32 thread
  HANDLE thread_handle =
    (HANDLE)_beginthreadex(NULL,
                           UseDefaultStackSize ? 0 : (unsigned)JavaThread::stack_size_at_create(),
                           (unsigned (__stdcall *)(void*)) _start,
                           thread,
                           CREATE_SUSPENDED,
                           &thread_id);
  if (thread_handle == NULL) {
    // Need to clean up stuff we've allocated so far
    CloseHandle(osthread->interrupt_event());
    thread->set_osthread(NULL);
    delete osthread;
    return NULL;
  }
  
  atomic::increment(&os::win32::_os_thread_count);

  // Store info on the Win32 thread into the OSThread
  osthread->set_thread_handle(thread_handle);
  osthread->set_thread_id(thread_id);

  // Initial thread state is INITIALIZED, not SUSPENDED
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(INITIALIZED);
  }

  // The thread is returned suspended (in state INITIALIZED), and is started higher up in the call chain
  return true;
}


// Free Win32 resources related to the OSThread
void os::free_thread(OSThread* osthread) {
  assert(osthread != NULL, "osthread not set");
  CloseHandle(osthread->thread_handle());
  CloseHandle(osthread->interrupt_event());
  delete osthread;
}


void os::exit_thread() {
  free_thread(Thread::current()->osthread());

  _endthreadex(0);
  // NOT REACHED
}


static int    has_performance_count = 0;
static jlong initial_performance_count;
static jlong performance_frequency;


jlong as_long(LARGE_INTEGER x) {
  jlong result = 0; // initialization to avoid warning
  set_high(&result, x.HighPart);
  set_low(&result,  x.LowPart);
  return result;
}


jlong os::elapsed_counter() {
  LARGE_INTEGER count;  
  QueryPerformanceCounter(&count);
  return as_long(count) - initial_performance_count;
}


jlong os::elapsed_frequency() {
  return performance_frequency;
}


bool os::is_MP() {
  return win32::is_MP();
}

julong os::physical_memory() {
  return win32::physical_memory();
}

julong os::allocatable_physical_memory(julong size) {
  return MIN2(size, (julong)1400*M);
}


int os::active_processor_count() {
  DWORD lpProcessAffinityMask;
  DWORD lpSystemAffinityMask;
#ifdef _WIN64
  if (GetProcessAffinityMask(GetCurrentProcess(), (PDWORD_PTR)&lpProcessAffinityMask, (PDWORD_PTR)&lpSystemAffinityMask)) {
#else
  if (GetProcessAffinityMask(GetCurrentProcess(), &lpProcessAffinityMask, &lpSystemAffinityMask)) {
#endif
    // Nof active processors is number of bits in process affinity mask
    int bitcount = 0;
    while (lpProcessAffinityMask != 0) {
      lpProcessAffinityMask = lpProcessAffinityMask & (lpProcessAffinityMask-1);
      bitcount++;
    }
    return bitcount;
  } else {
    return win32::processor_count();
  }
}

bool os::distribute_processes(uint length, uint* distribution) {
  // Not yet implemented.
  return false;
}

bool os::bind_to_processor(uint processor_id) {
  // Not yet implemented.
  return false;
}

static void initialize_performance_counter() {
  LARGE_INTEGER count;
  if (QueryPerformanceFrequency(&count)) {
    has_performance_count = 1;
    performance_frequency = as_long(count);
    QueryPerformanceCounter(&count);
    initial_performance_count = as_long(count);
  } else {
    has_performance_count = 0;
  }
}


double os::elapsedTime() {
  if (!has_performance_count) return 0.0;
  LARGE_INTEGER current_count;  
  QueryPerformanceCounter(&current_count);
 
  jlong current = as_long(current_count);
  double count = (double) current - initial_performance_count;
  double freq  = (double) performance_frequency;
  return count/freq;
}


// Windows format:
//   The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601.
// Java format:
//   Java standards require the number of milliseconds since 1/1/1970 

void windows_error() {
  printf("Error = %d\n", GetLastError());
  fatal("Windows error");
}

// Cached offset
static int    _has_offset = 0;
static jlong  _offset     = 0;
// Fake time counter for reproducible results when debugging
static jlong  fake_time = 0;

jlong offset() {
  if (_has_offset) return _offset;
  SYSTEMTIME java_origin;
  java_origin.wYear          = 1970;
  java_origin.wMonth         = 1; 
  java_origin.wDayOfWeek     = 0; // ignored 
  java_origin.wDay           = 1; 
  java_origin.wHour          = 0; 
  java_origin.wMinute        = 0; 
  java_origin.wSecond        = 0; 
  java_origin.wMilliseconds  = 0; 
  FILETIME jot;
  if (!SystemTimeToFileTime(&java_origin, &jot)) {
    windows_error();
  }
  _offset = jlong_from(jot.dwHighDateTime, jot.dwLowDateTime);
  _has_offset = 1;
  return _offset;
}

jlong windows_to_java_time(FILETIME wt) {
  jlong a = jlong_from(wt.dwHighDateTime, wt.dwLowDateTime);
  return (a - offset()) / 10000;
}

FILETIME java_to_windows_time(jlong l) {
  jlong a = (l * 10000) + offset();
  FILETIME result;
  result.dwHighDateTime = high(a); 
  result.dwLowDateTime  = low(a);
  return result;
}


jlong os::javaTimeMillis() {
  if (UseFakeTimers) {
    return fake_time++;
  } else {
    FILETIME wt;
    GetSystemTimeAsFileTime(&wt);
    return windows_to_java_time(wt);
  }
}

void os::abort(bool dump_core) {  

  // allow PerfMemory to attempt cleanup of any persistent resources
  perfMemory_exit();

  // Check for abort hook
  abort_hook_t abort_hook = Arguments::abort_hook();
  if (abort_hook != NULL) {
    abort_hook();
  } else {
    ostream_exit();
    ::exit(1);
  }
}

// Directory routines copied from src/win32/native/java/io/dirent_md.c
//  * @(#)dirent_md.c	1.15 00/02/02
//
// The declarations for DIR and struct dirent are in jvm_win32.h.

/* Caller must have already run dirname through JVM_NativePath, which removes
   duplicate slashes and converts all instances of '/' into '\\'. */

DIR *
os::opendir(const char *dirname)
{
    assert(dirname != NULL, "just checking");	// hotspot change
    DIR *dirp = (DIR *)malloc(sizeof(DIR));
    DWORD fattr;				// hotspot change
    char alt_dirname[4] = { 0, 0, 0, 0 };

    if (dirp == 0) {
	errno = ENOMEM;
	return 0;
    }

    /*
     * Win32 accepts "\" in its POSIX stat(), but refuses to treat it
     * as a directory in FindFirstFile().  We detect this case here and
     * prepend the current drive name.
     */
    if (dirname[1] == '\0' && dirname[0] == '\\') {
	alt_dirname[0] = _getdrive() + 'A' - 1;
	alt_dirname[1] = ':';
	alt_dirname[2] = '\\';
	alt_dirname[3] = '\0';
	dirname = alt_dirname;
    }

    dirp->path = (char *)malloc(strlen(dirname) + 5);
    if (dirp->path == 0) {
	free(dirp);
	errno = ENOMEM;
	return 0;
    }
    strcpy(dirp->path, dirname);

    fattr = GetFileAttributes(dirp->path);
    if (fattr == 0xffffffff) {
	free(dirp->path);
	free(dirp);
	errno = ENOENT;
	return 0;
    } else if ((fattr & FILE_ATTRIBUTE_DIRECTORY) == 0) {
	free(dirp->path);
	free(dirp);
	errno = ENOTDIR;
	return 0;
    }

    /* Append "*.*", or possibly "\\*.*", to path */
    if (dirp->path[1] == ':'
	&& (dirp->path[2] == '\0'
	    || (dirp->path[2] == '\\' && dirp->path[3] == '\0'))) {
	/* No '\\' needed for cases like "Z:" or "Z:\" */
	strcat(dirp->path, "*.*");
    } else {
	strcat(dirp->path, "\\*.*");
    }

    dirp->handle = FindFirstFile(dirp->path, &dirp->find_data);
    if (dirp->handle == INVALID_HANDLE_VALUE) {
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
	    free(dirp->path);
	    free(dirp);
	    errno = EACCES;
	    return 0;
	}
    }
    return dirp;
}

/* parameter dbuf unused on Windows */

struct dirent *
os::readdir(DIR *dirp, dirent *dbuf)
{
    assert(dirp != NULL, "just checking");	// hotspot change
    if (dirp->handle == INVALID_HANDLE_VALUE) {
	return 0;
    }

    strcpy(dirp->dirent.d_name, dirp->find_data.cFileName);

    if (!FindNextFile(dirp->handle, &dirp->find_data)) {
	if (GetLastError() == ERROR_INVALID_HANDLE) {
	    errno = EBADF;
	    return 0;
	}
	FindClose(dirp->handle);
	dirp->handle = INVALID_HANDLE_VALUE;
    }

    return &dirp->dirent;
}

int
os::closedir(DIR *dirp)
{
    assert(dirp != NULL, "just checking");	// hotspot change
    if (dirp->handle != INVALID_HANDLE_VALUE) {
	if (!FindClose(dirp->handle)) {
	    errno = EBADF;
	    return -1;
	}
	dirp->handle = INVALID_HANDLE_VALUE;
    }
    free(dirp->path);
    free(dirp);
    return 0;
}

const char* os::dll_file_extension() { return ".dll"; }

const char * os::get_temp_directory()
{
        static char path_buf[MAX_PATH];
        if (GetTempPath(MAX_PATH, path_buf)>0)
                 return path_buf;
        else{
                 path_buf[0]='\0';
                 return path_buf;
        }
}

// int mod_enum_proc(int pid, char * module_file_name, address module_base_addr,
//                   unsigned module_size, void * param);
typedef int (* MODULEENUMPROC)( int, char *, address, unsigned, void * ) ;

// psapi.h doesn't come with Visual Studio 6; it can be downloaded as Platform SDK from
// Microsoft.  Here are the definitions copied from psapi.h
typedef struct _MODULEINFO {
    LPVOID lpBaseOfDll;
    DWORD SizeOfImage;
    LPVOID EntryPoint;
} MODULEINFO, *LPMODULEINFO;

/*
 * Enumerate all modules for a given process ID. Let pid be 0 for current
 * process.
 * NOTE: Because of different API implemented on Windows 95/98 and Windows
 *       NT, we have to explicitly load helper DLLs according to OS version.
 */

// enumerate_modules for Windows NT, using PSAPI 
static int _enumerate_modules_winnt( int pid, MODULEENUMPROC lpProc, void * param)
{
       HINSTANCE      hInstLib ;
       HANDLE         hProcess ;
       DWORD          dwSize, dwIndex ;
       HMODULE        hModule;
       HMODULE       *lpdwModules;
       char           szFileName[ MAX_PATH ] ;

       int            result = 0;

       // PSAPI Function Pointers, for Windows NT
       BOOL (WINAPI *lpfEnumProcessModules)( HANDLE, HMODULE *, DWORD, LPDWORD );
       DWORD (WINAPI *lpfGetModuleFileNameEx)( HANDLE, HMODULE, LPTSTR, DWORD );
       BOOL (WINAPI *lpfGetModuleInformation)( HANDLE, HMODULE, LPMODULEINFO, DWORD );

       // Load library and get the procedures explicitly. We do
       // this so that we don't have to worry about modules using
       // this code failing to load under Windows 95, because
       // it can't resolve references to the PSAPI.DLL.
       hInstLib = LoadLibraryA( "PSAPI.DLL" ) ;
       if( hInstLib == NULL )
          return FALSE ;

       // Get procedure addresses.
       lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *,
            DWORD, LPDWORD)) GetProcAddress( hInstLib,
            "EnumProcessModules" ) ;
       lpfGetModuleFileNameEx =(DWORD (WINAPI *)(HANDLE, HMODULE,
            LPTSTR, DWORD )) GetProcAddress( hInstLib,
            "GetModuleFileNameExA" ) ;
       lpfGetModuleInformation=(BOOL (WINAPI *)(HANDLE, HMODULE,
            LPMODULEINFO, DWORD)) GetProcAddress( hInstLib,
           "GetModuleInformation");

       if( lpfEnumProcessModules == NULL ||
           lpfGetModuleFileNameEx == NULL ||
           lpfGetModuleInformation== NULL ) {
               // failed to locate functions
               FreeLibrary( hInstLib ) ;
               return FALSE ;
       }

       hProcess = OpenProcess(
            PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
            FALSE, pid ) ;

       if( hProcess != NULL ) {
            // Get the size of memory required to store module information
            lpfEnumProcessModules( hProcess, &hModule, sizeof(HMODULE), &dwSize );

            // Allocate memory for module information
            lpdwModules = (HMODULE *)HeapAlloc( GetProcessHeap(), 0, dwSize );
            if( lpdwModules == NULL ) {
                 FreeLibrary( hInstLib ) ;
                 return FALSE ;
            }

            // Here we call EnumProcessModules to get all open
            // modules for this process
            if( !lpfEnumProcessModules( hProcess, lpdwModules, dwSize, &dwSize ) ) {
                 HeapFree( GetProcessHeap(), 0, lpdwModules ) ;
                 FreeLibrary( hInstLib ) ;
                 CloseHandle( hProcess );
                 return FALSE ;
            }

            // How many ModuleID's did we get?
            dwSize /= sizeof( DWORD ) ;

            // to be used in GetModuleInformation()
            MODULEINFO modinfo;

            // Loop through each ModuleID.
            for( dwIndex = 0 ; dwIndex < dwSize ; dwIndex++ ) {
                // Get Full pathname:
                if( !lpfGetModuleFileNameEx( hProcess, lpdwModules[dwIndex],
                           szFileName, sizeof( szFileName ) ) ) {
                       szFileName[0] = 0 ;
                }

                if (!lpfGetModuleInformation(hProcess, lpdwModules[dwIndex],
                                        &modinfo, sizeof(modinfo))) {
                        modinfo.lpBaseOfDll = NULL;
                        modinfo.SizeOfImage = 0;
                }

                // Invoke callback function
                if((result=lpProc( pid, szFileName,
                                   (address)modinfo.lpBaseOfDll, modinfo.SizeOfImage, param ))
                         != 0)
                   break;
            } 

            HeapFree( GetProcessHeap(), 0, lpdwModules);
            CloseHandle( hProcess ) ;
       }

       FreeLibrary( hInstLib  );

       return result;
}

// enumerate_modules for Windows 95/98/ME, using TOOLHELP
static int _enumerate_modules_windows( int pid, MODULEENUMPROC lpProc, void *param)
{
      HINSTANCE      hInstLib ;
      HANDLE         hSnapShot ;
      MODULEENTRY32  modentry ;
      BOOL           bFlag ;

      int            result = 0;

      // ToolHelp Function Pointers, for Windows 95, 98 and ME
      HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD,DWORD) ;
      BOOL (WINAPI *lpfModule32First)(HANDLE,LPMODULEENTRY32) ;
      BOOL (WINAPI *lpfModule32Next)(HANDLE,LPMODULEENTRY32) ;

      hInstLib = LoadLibraryA( "Kernel32.DLL" ) ;
      if( hInstLib == NULL )
            return FALSE ;

      // Get procedure addresses.
      // We are linking to these functions of Kernel32
      // explicitly, because otherwise a module using
      // this code would fail to load under Windows NT before 5.0,
      // which does not have the Toolhelp32
      // functions in the Kernel 32.
      lpfCreateToolhelp32Snapshot=
            (HANDLE(WINAPI *)(DWORD,DWORD))
            GetProcAddress( hInstLib,
            "CreateToolhelp32Snapshot" ) ;
      lpfModule32First=
            (BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32))
            GetProcAddress( hInstLib, "Module32First" ) ;
      lpfModule32Next=
            (BOOL(WINAPI *)(HANDLE,LPMODULEENTRY32))
            GetProcAddress( hInstLib, "Module32Next" ) ;

      if( lpfModule32Next == NULL ||
          lpfModule32First == NULL ||
          lpfCreateToolhelp32Snapshot == NULL ) {
            // failed to locate functions
            FreeLibrary( hInstLib ) ;
            return FALSE ;
       }

       // Get a handle to a Toolhelp snapshot of the system
       hSnapShot = lpfCreateToolhelp32Snapshot(
            TH32CS_SNAPMODULE, pid ) ;
       if( hSnapShot == INVALID_HANDLE_VALUE ) {
            FreeLibrary( hInstLib ) ;
            return FALSE ;
       }

       // Get the first module's information.
       modentry.dwSize = sizeof(MODULEENTRY32) ;
       bFlag = lpfModule32First( hSnapShot, &modentry ) ;

       // While there are modules, keep looping.
       while( bFlag ) {
          // invoke the callback
          if((result=lpProc( pid, modentry.szExePath,
                            (address)modentry.modBaseAddr, modentry.modBaseSize, param ))!=0)
          {
               modentry.dwSize = sizeof(MODULEENTRY32) ;
               bFlag = lpfModule32Next( hSnapShot, &modentry );
          }else
               bFlag = FALSE ;
       }

       // Free the library.
       FreeLibrary( hInstLib ) ;

       return result;
}

int enumerate_modules( int pid, MODULEENUMPROC lpProc, void * param )
{
      // Get current process ID if caller doesn't provide it.
      if (!pid) pid = _getpid();

      // If Windows NT:
      if (os::win32::is_nt()) return _enumerate_modules_winnt  (pid, lpProc, param);
      else                    return _enumerate_modules_windows(pid, lpProc, param);
}

typedef BOOL (__stdcall *INIT_FUNC_TYPE)(HANDLE, LPSTR, BOOL);
typedef BOOL (__stdcall *MAP_ADDR_FUNC_TYPE)(HANDLE, DWORD, PDWORD, PIMAGEHLP_SYMBOL);
typedef BOOL (__stdcall *MAP_ADDR_TO_MODULE_FUNC_TYPE)(HANDLE, DWORD, PIMAGEHLP_MODULE);
typedef BOOL (__stdcall *MAP_ADDR_TO_LINE_FUNC_TYPE)(HANDLE, DWORD, PDWORD, PIMAGEHLP_LINE);
typedef DWORD(__stdcall *SYM_GET_OPTIONS_FUNC_TYPE)(VOID);
typedef DWORD(__stdcall *SYM_SET_OPTIONS_FUNC_TYPE)(DWORD);
typedef BOOL (__stdcall *SYM_SET_SEARCH_PATH_FUNC_TYPE)(HANDLE, PSTR);
typedef DWORD(__stdcall *SYM_LOAD_MODULE_FUNC_TYPE)(HANDLE, HANDLE, PSTR, PSTR, DWORD, DWORD);

/* functions from IMAGEHLP API (imagehlp.dll or dbghelp.dll) */
static HINSTANCE symbol_handler_library                                    = NULL;
static MAP_ADDR_FUNC_TYPE symbol_handler_map_addr_func                     = NULL;
static MAP_ADDR_TO_MODULE_FUNC_TYPE symbol_handler_map_addr_to_module_func = NULL;
static MAP_ADDR_TO_LINE_FUNC_TYPE symbol_handler_map_addr_to_line_func     = NULL;
static SYM_SET_SEARCH_PATH_FUNC_TYPE symbol_handler_set_search_path_func   = NULL;
static SYM_LOAD_MODULE_FUNC_TYPE symbol_handler_load_module_func           = NULL;

static HANDLE pseudo_process_handle;

#define RETRIEVE_LINE_NUMBER_INFO
#define MAX_ERROR_COUNT 100
#define SYS_THREAD_ERROR 0xffffffffUL


/* retrieves the directory name (including drive name) from _path_, and returns */
/* it in the buffer pointed by _dir_. caller is responsible to alloc the memory */
static void _path_get_directory(const char *path, char *dir, int max_size)
{
  if (dir == NULL) return;

  _splitpath(path, NULL, dir, NULL, NULL);
  if (*dir){
          char *ptr = strstr(path, dir);
          int  len = (ptr - path) + (int)strlen(dir);
          strncpy(dir, path, max_size);
          if (len < max_size) dir[len] = '\0';
          else dir[max_size-1] = '\0';
  }
  else *dir = '\0';
}

/* call back function to load symbol table for the specified module (DLL) */
/* libname is *full* path, base_addr is the desired base address.         */
static int sym_load_module_dll(int pid, char * libname,
                               address base_addr, unsigned size, void * param)
{
  /* NOTE: the directory that contains the module file (EXE, DLL, etc.) */
  /*       is not automatically on the search path for symbol table.    */
  /*       we have to set the search path manually.                     */
  char path[MAX_PATH];
  _path_get_directory(libname, path, _MAX_PATH);
  if (*path && symbol_handler_set_search_path_func!=NULL)
          symbol_handler_set_search_path_func(pseudo_process_handle, path);

  if (symbol_handler_load_module_func!=NULL){
      strncpy(path, libname, MAX_PATH);
      int rslt = symbol_handler_load_module_func(pseudo_process_handle, NULL,
                              path, NULL, (DWORD)base_addr, 0);
      #if 0
          if (rslt == 0){
                  rslt = GetLastError();
                  if (rslt!=0)
                          printf("SymLoadModule failed: %d\n", rslt);
          }
      #endif
  }

  return 0;  /* continue with module enumeration */
}

/* init pseudo_process_handle; load IMAGEHLP library; load symbol tables */
/* for all open modules. the actual library/symbol loading only executes */
/* once.                                                                 */
static BOOL load_symbol_handler_library()
{
  /* NOTE: While Windows NT requires a process handle for all Sym... function
           calls, Windows 95/98/ME actually requires pid as the input parameter.
           It's quite confusing, because the header file always says "HANDLE"
           for parameter 1. But in fact it should be pid for Windows 95/98/ME. */
  // Get pseudo handle
  if (os::win32::is_nt())
       pseudo_process_handle = GetCurrentProcess();
  else pseudo_process_handle = (HANDLE)_getpid();

  // Initialize symbol handler
  if (symbol_handler_library == NULL) {
    // Set to non-null value we don't retry unsuccessfull load attempts
    symbol_handler_library = (HINSTANCE) -1;
    // Load symbol handler library
    HINSTANCE library = LoadLibrary("imagehlp.dll");
    if (library) {
      // Lookup entry points
      INIT_FUNC_TYPE init_func =
           (INIT_FUNC_TYPE) GetProcAddress(library, "SymInitialize");
      MAP_ADDR_FUNC_TYPE map_addr_func =
           (MAP_ADDR_FUNC_TYPE) GetProcAddress(library, "SymGetSymFromAddr");
      MAP_ADDR_TO_MODULE_FUNC_TYPE map_addr_to_module =
           (MAP_ADDR_TO_MODULE_FUNC_TYPE) GetProcAddress(library, "SymGetModuleInfo");
#ifdef RETRIEVE_LINE_NUMBER_INFO
      MAP_ADDR_TO_LINE_FUNC_TYPE map_addr_to_line =
           (MAP_ADDR_TO_LINE_FUNC_TYPE) GetProcAddress(library, "SymGetLineFromAddr");
#endif
      SYM_GET_OPTIONS_FUNC_TYPE sym_get_options =
           (SYM_GET_OPTIONS_FUNC_TYPE) GetProcAddress(library, "SymGetOptions");
      SYM_SET_OPTIONS_FUNC_TYPE sym_set_options =
           (SYM_SET_OPTIONS_FUNC_TYPE) GetProcAddress(library, "SymSetOptions");
      SYM_SET_SEARCH_PATH_FUNC_TYPE sym_set_search_path =
           (SYM_SET_SEARCH_PATH_FUNC_TYPE) GetProcAddress(library, "SymSetSearchPath");
      SYM_LOAD_MODULE_FUNC_TYPE sym_load_module =
           (SYM_LOAD_MODULE_FUNC_TYPE) GetProcAddress(library, "SymLoadModule");

      if (library && init_func) {
#       ifdef RETRIEVE_LINE_NUMBER_INFO
            if (sym_get_options && sym_set_options){
                DWORD opt = sym_get_options();
                sym_set_options(opt | SYMOPT_LOAD_LINES);
            }
#       endif

        // we will manually load all module and symbols later
        BOOL init_result = init_func(pseudo_process_handle, NULL, false);
        if (init_result) {
                symbol_handler_library = library;
                symbol_handler_map_addr_func = map_addr_func;
                symbol_handler_map_addr_to_module_func = map_addr_to_module;
                symbol_handler_map_addr_to_line_func = map_addr_to_line;
                symbol_handler_set_search_path_func = sym_set_search_path;
                symbol_handler_load_module_func = sym_load_module;

                // load all modules and their symbols. note only loading the symbol
                // file for a particular module doesn't always work correctly in
                // symbol lookup, even the symbol is indeed inside that module.
                enumerate_modules(_getpid(), sym_load_module_dll, NULL);
        } else {
                /* failed in initialization */
                return FALSE;
        }
      }
    }
  } 

  return TRUE;
}

typedef struct _INTERNAL_MODULE_INFO {
        address addr;
        HMODULE hmod;
        char full_path[MAX_PATH];
}INTERNAL_MODULE_INFO, *PINTERNAL_MODULE_INFO;

/* call back function to check if the specified address is within the */
/* particular module. used by dll_address_to_library_name.            */
/* _param_ is a INTERNAL_MODULE_INFO structure.                       */
static int locate_module_by_address(int pid, char * mod_fname,
                                    address base_addr, unsigned size, void * param)
{
        INTERNAL_MODULE_INFO *pmod = (INTERNAL_MODULE_INFO *)param;
        if (!pmod) return -1;   /* error */

        if (base_addr<=pmod->addr &&
            base_addr+size > pmod->addr){
              strncpy(pmod->full_path, mod_fname, MAX_PATH);
              return 1;  /* found, no need to enumerate other modules */
        }
        return 0; /* continue with enumeration */
}

/* locates the library for the give address and returns full path to the DLL file */
const char* os::dll_address_to_library_name(address addr){
// NOTE: the reason we don't use SymGetModuleInfo() is it doesn't always
//       return the full path to the DLL file, sometimes it returns path
//       to the corresponding PDB file (debug info); sometimes it only 
//       returns partial path, which makes life painful.

        load_symbol_handler_library();

        static INTERNAL_MODULE_INFO mod_info;
        mod_info.addr = addr;
        int result = enumerate_modules(_getpid(), locate_module_by_address,
                                       (void *)&mod_info);
        if (result == 1){
                return mod_info.full_path;
        }
        else return NULL;
}

bool os::dll_address_to_function_name(address addr, const char **symname,
                                             int *offset) {
// The largest symbol that MS compilers generate is 256 chars, and
// the struct already contains one char for null termination.
# define MAX_SYMBOL_INFO_SIZE (sizeof(IMAGEHLP_SYMBOL)+256)
  static char symbol_info_buf[MAX_SYMBOL_INFO_SIZE];

  load_symbol_handler_library();

  if (symbol_handler_map_addr_func != NULL) {
    // Lookup symbol at address addr
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbol_info_buf;
    pSymbol->SizeOfStruct=MAX_SYMBOL_INFO_SIZE;
    pSymbol->MaxNameLength=MAX_SYMBOL_INFO_SIZE - sizeof(IMAGEHLP_SYMBOL);
    DWORD disp = 0;
    if (symbol_handler_map_addr_func(pseudo_process_handle, (DWORD) addr, &disp, pSymbol)) {
                if (offset)  *offset  = disp;
                if (symname) *symname = pSymbol->Name;
                return true;
    } else {
           // SymGetSymFromAddr failed
           // fprintf(stderr, "SymGetSymFromAddr returned error : %d\n", GetLastError());
    }
  }
  if (offset)  *offset  = -1;
  if (symname) *symname = NULL;
  return false;
}

// Looks up source file name and line number for the give address
// Returns line number if success. Filename is returned in statically
// allocated buffer, caller will have to copy the string between calls.
// If failed, returns -1.
int os::dll_address_to_line_number(address addr, char ** filename)
{
    const char * libname = dll_address_to_library_name(addr);

    IMAGEHLP_LINE line_info;
    line_info.SizeOfStruct=sizeof(IMAGEHLP_LINE);

    if (symbol_handler_map_addr_to_line_func != NULL){
           DWORD disp;
           if (symbol_handler_map_addr_to_line_func(pseudo_process_handle,
                                       (DWORD)addr, &disp, &line_info)) {
                   if (filename!=NULL) *filename = line_info.FileName;
                   return line_info.LineNumber;
            } else {
            //     fprintf(stderr, "SymGetLineFromAddr returned error : %d\n", GetLastError());
            }
    }

    return -1;
}

struct _jvm_border_struct {
        address jvm_begin_addr;
        address jvm_end_addr;
};

/* call back function to determine the border of jvm library, used by */
/* dll_address_is_in_vm()                                             */
/* _param_ is a _jvm_border_struct                                    */
int find_jvm_borders(int pid, char * mod_fname, address base_addr, unsigned size, void * param)
{
        if (!param) return -1;  /* error */

        if (base_addr<=(address)find_jvm_borders &&
            base_addr+size > (address)find_jvm_borders){
                ((struct _jvm_border_struct *)param)->jvm_begin_addr = (address)base_addr;
                ((struct _jvm_border_struct *)param)->jvm_end_addr = (address)base_addr + size;
                return 1; /* found, no need to further enum */
        }
        return 0; /* continue with enumeration */
}

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

  static address jvm_begin_addr = NULL;
  static address jvm_end_addr   = NULL;

  if (!jvm_begin_addr || !jvm_end_addr){
          struct _jvm_border_struct jb;
          if (enumerate_modules(_getpid(), find_jvm_borders, (void *)&jb) == 1){
                  jvm_begin_addr = jb.jvm_begin_addr;
                  jvm_end_addr   = jb.jvm_end_addr;
          }
          else{
                  /* can't find jvm library, then what should we do?? */
          }
  }

  if ((jvm_begin_addr <= addr) && (addr < jvm_end_addr)){
          return true;
  }
  return false;
}

/* call back function to print out information about a module */
/* _param_ is an outputstream                                 */
static int print_module( int pid, char * fname, address base, unsigned size, void * param)
{
   if (!param) return -1; /* error */

   address end_addr = base + size;
   ((outputStream *)param)->print("0x%p - 0x%p \t%s\n",
                                   base, end_addr, fname);
   return 0;  /* continue with enumeration */
}

void os::dump_dll_info(outputStream *st) {
   int pid = _getpid();
   if (!st) st=tty;
   st->print_cr("Dynamic libraries:");  st->flush();
   enumerate_modules(pid, print_module, (void *)st);
}

void os::jvm_path(char *buf, jint buflen) {
  GetModuleFileName(vm_lib_handle, buf, buflen);
}


void os::print_jni_name_prefix_on(outputStream* st, int args_size) {
#ifndef _WIN64
  st->print("_");
#endif
}


void os::print_jni_name_suffix_on(outputStream* st, int args_size) {
#ifndef _WIN64
  st->print("@%d", args_size  * sizeof(int));
#endif
}

// sun.misc.Signal
// NOTE that this is a workaround for an apparent kernel bug where if
// a signal handler for SIGBREAK is installed then that signal handler
// takes priority over the console control handler for CTRL_CLOSE_EVENT.
// See bug 4416763.
static void (*sigbreakHandler)(int) = NULL;

static void UserHandler(int sig, void *siginfo, void *context) {
  os::signal_notify(sig);
  // We need to reinstate the signal handler each time... 
  os::signal(sig, (void*)UserHandler);
}

void* os::user_handler() {
  return (void*) UserHandler;
}

void* os::signal(int signal_number, void* handler) {
  if ((signal_number == SIGBREAK) && (!ReduceSignalUsage)) {
    void (*oldHandler)(int) = sigbreakHandler;
    sigbreakHandler = (void (*)(int)) handler;
    return (void*) oldHandler;
  } else {
    return (void*)::signal(signal_number, (void (*)(int))handler);
  }
}

void os::signal_raise(int signal_number) {
  raise(signal_number);
}

// The Win32 C runtime library maps all console control events other than ^C
// into SIGBREAK, which makes it impossible to distinguish ^BREAK from close,
// logoff, and shutdown events.  We therefore install our own console handler
// that raises SIGTERM for the latter cases.
//
static BOOL WINAPI consoleHandler(DWORD event) {
  switch(event) {
    case CTRL_C_EVENT:
      os::signal_raise(SIGINT);
      return TRUE;
      break;
    case CTRL_BREAK_EVENT:
      if (sigbreakHandler != NULL) {
        (*sigbreakHandler)(SIGBREAK);
      }
      return TRUE;
      break;
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
      os::signal_raise(SIGTERM);
      return TRUE;
      break;
    default:
      break;
  }
  return FALSE;
}

/*
 * The following code is moved from os.cpp for making this
 * code platform specific, which it is by its very nature.
 */

// Return maximum OS signal used + 1 for internal use only
// Used as exit signal for signal_thread
int os::sigexitnum_pd(){
  return NSIG;
}

// a counter for each possible signal value, including signal_thread exit signal
static int pending_signals[NSIG+1] = { 0 };
static HANDLE sig_sem;

void os::signal_init_pd() {
  // Initialize signal structures
  memset(pending_signals, 0, sizeof(pending_signals));
  
  sig_sem = ::CreateSemaphore(NULL, 0, NSIG+1, NULL);

  // Programs embedding the VM do not want it to attempt to receive
  // events like CTRL_LOGOFF_EVENT, which are used to implement the
  // shutdown hooks mechanism introduced in 1.3.  For example, when
  // the VM is run as part of a Windows NT service (i.e., a servlet
  // engine in a web server), the correct behavior is for any console
  // control handler to return FALSE, not TRUE, because the OS's
  // "final" handler for such events allows the process to continue if
  // it is a service (while terminating it if it is not a service).
  // To make this behavior uniform and the mechanism simpler, we
  // completely disable the VM's usage of these console events if -Xrs
  // (=ReduceSignalUsage) is specified.  This means, for example, that
  // the CTRL-BREAK thread dump mechanism is also disabled in this
  // case.  See bugs 4323062, 4345157, and related bugs.
  
  if (!ReduceSignalUsage) {
    // Add a CTRL-C handler
    SetConsoleCtrlHandler(consoleHandler, TRUE);
  }
}

void os::signal_notify(int signal_number) {
  BOOL ret;

  atomic::increment(&pending_signals[signal_number]);
  ret = ::ReleaseSemaphore(sig_sem, 1, NULL);
  assert(ret != 0, "ReleaseSemaphore() failed");
}  

static int check_pending_signals(bool wait_for_signal) {
  DWORD ret;
  while (true) {
    for (int i = 0; i < NSIG + 1; i++) {
      jint n = pending_signals[i];
      if (n > 0 && n == atomic::compare_and_exchange(n - 1, &pending_signals[i], n)) {
        return i;
      }
    }
    if (!wait_for_signal) {
      return -1;
    }

#ifdef _M_IA64
    // This flush is needed for profiling since this is a forced
    // async suspension.
    if ( Arguments::has_profile() ) {
      (void)StubRoutines::ia64::flush_register_stack()();
    }
#endif

    JavaThread *thread = JavaThread::current();

    // Flush the register windows if we are about to block in the VM.
    // Since we are currently in native, we know that the flushed window
    // flags will be reset when we return to Java.
    if (thread->thread_state() == _thread_in_native) {
      thread->frame_anchor()->make_walkable(true, thread);
    }

    ThreadBlockInVM tbivm(thread);

    bool threadIsSuspended;
    do {
      thread->set_suspend_equivalent();
      // cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()
      ret = ::WaitForSingleObject(sig_sem, INFINITE);
      assert(ret == WAIT_OBJECT_0, "WaitForSingleObject() failed");

      // were we externally suspended while we were waiting?
      threadIsSuspended = thread->handle_special_suspend_equivalent_condition();
      if (threadIsSuspended) {
        //
        // The semaphore has been incremented, but while we were waiting
        // another thread suspended us. We don't want to continue running
        // while suspended because that would surprise the thread that
        // suspended us.
        //
        ret = ::ReleaseSemaphore(sig_sem, 1, NULL);
        assert(ret != 0, "ReleaseSemaphore() failed");

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

// Implicit OS exception handling

LONG Handle_Exception(struct _EXCEPTION_POINTERS* exceptionInfo, address handler) {
  JavaThread* thread = JavaThread::current();
  // Save pc in thread
#ifdef _WIN64
  thread->set_saved_exception_pc((address)exceptionInfo->ContextRecord->StIIP);
  // Set pc to handler
  exceptionInfo->ContextRecord->StIIP = (DWORD64)handler;  
#else
  thread->set_saved_exception_pc((address)exceptionInfo->ContextRecord->Eip);
  // Set pc to handler
  exceptionInfo->ContextRecord->Eip = (LONG)handler;  
#endif

  // Continue the execution
  return EXCEPTION_CONTINUE_EXECUTION;
}


// Used for PostMortemDump
extern "C" void safepoints();
extern "C" void find(int x);
extern "C" void events();

// According to Windows API documentation, an illegal instruction sequence should generate
// the 0xC000001C exception code. However, real world experience shows that occasionnaly
// the execution of an illegal instruction can generate the exception code 0xC000001E. This
// seems to be an undocumented feature of Win NT 4.0 (and probably other Windows systems).

#define EXCEPTION_ILLEGAL_INSTRUCTION_2 0xC000001E

const char* os::exception_name(int exception_code) {
  switch (exception_code) {
    case EXCEPTION_ACCESS_VIOLATION        : return "EXCEPTION_ACCESS_VIOLATION";
    case EXCEPTION_DATATYPE_MISALIGNMENT   : return "EXCEPTION_DATATYPE_MISALIGNMENT";
    case EXCEPTION_BREAKPOINT              : return "EXCEPTION_BREAKPOINT";
    case EXCEPTION_SINGLE_STEP             : return "EXCEPTION_SINGLE_STEP";
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED   : return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
    case EXCEPTION_FLT_DENORMAL_OPERAND    : return "EXCEPTION_FLT_DENORMAL_OPERAND";
    case EXCEPTION_FLT_DIVIDE_BY_ZERO      : return "EXCEPTION_FLT_DIVIDE_BY_ZERO";
    case EXCEPTION_FLT_INEXACT_RESULT      : return "EXCEPTION_FLT_INEXACT_RESULT";
    case EXCEPTION_FLT_INVALID_OPERATION   : return "EXCEPTION_FLT_INVALID_OPERATION";
    case EXCEPTION_FLT_OVERFLOW            : return "EXCEPTION_FLT_OVERFLOW";
    case EXCEPTION_FLT_STACK_CHECK         : return "EXCEPTION_FLT_STACK_CHECK";
    case EXCEPTION_FLT_UNDERFLOW           : return "EXCEPTION_FLT_UNDERFLOW";
    case EXCEPTION_INT_DIVIDE_BY_ZERO      : return "EXCEPTION_INT_DIVIDE_BY_ZERO";
    case EXCEPTION_INT_OVERFLOW            : return "EXCEPTION_INT_OVERFLOW";
    case EXCEPTION_PRIV_INSTRUCTION        : return "EXCEPTION_PRIV_INSTRUCTION";
    case EXCEPTION_IN_PAGE_ERROR           : return "EXCEPTION_IN_PAGE_ERROR";
    case EXCEPTION_ILLEGAL_INSTRUCTION     : return "EXCEPTION_ILLEGAL_INSTRUCTION";
    case EXCEPTION_ILLEGAL_INSTRUCTION_2   : return "EXCEPTION_ILLEGAL_INSTRUCTION_2";
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION";
    case EXCEPTION_STACK_OVERFLOW          : return "EXCEPTION_STACK_OVERFLOW";
    case EXCEPTION_INVALID_DISPOSITION     : return "EXCEPTION_INVALID_DISPOSITION";
    case EXCEPTION_GUARD_PAGE              : return "EXCEPTION_GUARD_PAGE";
    case EXCEPTION_INVALID_HANDLE          : return "EXCEPTION_INVALID_HANDLE";
    default                                : return "unknown exception code";
  }
}


//-----------------------------------------------------------------------------
LONG Handle_IDiv_Exception(struct _EXCEPTION_POINTERS* exceptionInfo) {
  // handle exception caused by idiv; should only happen for -MinInt/-1
  // (division by zero is handled explicitly)
#ifdef _M_IA64
  assert(0, "Fix Handle_IDiv_Exception");
#else
  PCONTEXT ctx = exceptionInfo->ContextRecord;
  address pc = (address)ctx->Eip;
  NOT_PRODUCT(Events::log("idiv overflow exception at 0x%lx", pc));
  assert(pc[0] == 0xF7, "not an idiv opcode");
  assert((pc[1] & ~0x7) == 0xF8, "cannot handle non-register operands");
  assert(ctx->Eax == min_jint, "unexpected idiv exception");
  // set correct result values and continue after idiv instruction
  ctx->Eip = (DWORD)pc + 2;        // idiv reg, reg  is 2 bytes
  ctx->Eax = (DWORD)min_jint;      // result
  ctx->Edx = (DWORD)0;             // remainder
  // Continue the execution
#endif
  return EXCEPTION_CONTINUE_EXECUTION;
}

//-----------------------------------------------------------------------------
/* set/unset the simple signal handler to catch signals raised during fatal error handling */
void os::set_exception_handler_during_fatal_error(int sig, int is_install)
{
   /* on Windows, do nothing. recursive exceptions will not be blocked and will be */
   /* handled by default fatal error handler                                       */
}

LONG WINAPI topLevelExceptionFilter(struct _EXCEPTION_POINTERS* exceptionInfo) {
  if (InterceptOSException) return EXCEPTION_CONTINUE_SEARCH;  

  DWORD exception_code = exceptionInfo->ExceptionRecord->ExceptionCode;
  Thread* t = ThreadLocalStorage::thread();
  if (t != NULL && t->is_Java_thread()) {
    JavaThread* thread = (JavaThread*) t;
    bool in_java = thread->thread_state() == _thread_in_Java;

    // Handle potential stack overflows up front.
    if (exception_code == EXCEPTION_STACK_OVERFLOW) {
      if (os::uses_stack_guard_pages()) {
#ifdef _M_IA64
        //
        // If it's a legal stack address continue, Windows will map it in. 
        //
        PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;
        address addr = (address) exceptionRecord->ExceptionInformation[1];
        if (addr > thread->stack_yellow_zone_base() && addr < thread->stack_base() ) 
          return EXCEPTION_CONTINUE_EXECUTION; 

        // The register save area is the same size as the memory stack
        // and starts at the page just above the start of the memory stack.
        // If we get a fault in this area, we've run out of register
        // stack.  If we are in java, try throwing a stack overflow exception.
        if (addr > thread->stack_base() && 
                      addr <= (thread->stack_base()+thread->stack_size()) ) {

          tty->print_cr("Register stack overflow, addr:%p, stack_base:%p\n",
                            addr, thread->stack_base() );
	  // If not in java code, return and hope for the best.
	  return in_java ? Handle_Exception(exceptionInfo,
            StubRoutines::__CPU__::handler_for_stack_overflow())
            :  EXCEPTION_CONTINUE_EXECUTION;
        }
#endif
        if (thread->stack_yellow_zone_enabled()) {
          // Yellow zone violation.  The o/s has unprotected the first yellow
          // zone page for us.  Note:  must call disable_stack_yellow_zone to
          // update the enabled status, even if the zone contains only one page.
          thread->disable_stack_yellow_zone();
	  // If not in java code, return and hope for the best.
	  return in_java ? Handle_Exception(exceptionInfo,
            StubRoutines::__CPU__::handler_for_stack_overflow())
            :  EXCEPTION_CONTINUE_EXECUTION;
        } else {
          // Fatal red zone violation. 
          thread->disable_stack_red_zone();
          tty->print_cr("An unrecoverable stack overflow has occurred.");
          os::handle_unexpected_exception(thread, 
                        exception_code,
                        (address) exceptionInfo->ExceptionRecord->ExceptionAddress,
                        (void *) exceptionInfo);
          return EXCEPTION_CONTINUE_SEARCH;
        }
      } else if (in_java) {
        // JVM-managed guard pages cannot be used on win95/98.  The o/s provides
        // a one-time-only guard page, which it has released to us.  The next
        // stack overflow on this thread will result in an ACCESS_VIOLATION.
        return Handle_Exception(exceptionInfo,
          StubRoutines::__CPU__::handler_for_stack_overflow());
      } else {
        // Can only return and hope for the best.  Further stack growth will
        // result in an ACCESS_VIOLATION.
	return EXCEPTION_CONTINUE_EXECUTION;
      }
    } else if (exception_code == EXCEPTION_ACCESS_VIOLATION) {
      // Either stack overflow or null pointer exception.
      if (in_java) {
        PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;
	address addr = (address) exceptionRecord->ExceptionInformation[1];
	address stack_end = thread->stack_base() - thread->stack_size();
	if (addr < stack_end && addr >= stack_end - os::vm_page_size()) {
	  // Stack overflow.
	  assert(!os::uses_stack_guard_pages(),
	    "should be caught by red zone code above.");
	  return Handle_Exception(exceptionInfo,
	    StubRoutines::__CPU__::handler_for_stack_overflow());
	} else {
#ifdef _M_IA64
          //
	  // If it's a legal stack address map the entire region in
          //
          PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;
	  address addr = (address) exceptionRecord->ExceptionInformation[1];
	  if (addr > thread->stack_yellow_zone_base() && addr < thread->stack_base() ) {
                  addr = (address)((uintptr_t)addr & 
                         (~((uintptr_t)os::vm_page_size() - (uintptr_t)1)));  
                  os::commit_memory( (char *)addr, thread->stack_base() - addr );
	          return EXCEPTION_CONTINUE_EXECUTION;
          }
          else 
#endif
          {
	    // Null pointer exception.
#ifdef _M_IA64
            // We catch register stack overflows in compiled code by doing
            // an explicit compare and executing a st8(G0, G0) if the
            // BSP enters into our guard area.  We test for the overflow
            // condition and fall into the normal null pointer exception 
            // code if BSP hasn't overflowed.
            if ( in_java ) {
              if(thread->register_stack_overflow()) {
                assert((address)exceptionInfo->ContextRecord->IntS3 == 
                                thread->register_stack_limit(), 
                               "GR7 doesn't contain register_stack_limit");
                // Disable the yellow zone which sets the state that 
                // we've got a stack overflow problem.
                if (thread->stack_yellow_zone_enabled()) {
                  thread->disable_stack_yellow_zone();
                }
                // Give us some room to process the exception
                thread->disable_register_stack_guard();
                // Update GR7 with the new limit so we can continue running
                // compiled code.
                exceptionInfo->ContextRecord->IntS3 = 
                               (ULONGLONG)thread->register_stack_limit();
                return Handle_Exception(exceptionInfo, 
                       StubRoutines::__CPU__::handler_for_stack_overflow());
              } else {
#ifndef CORE
		//
		// Check for implicit null
		// We only expect null pointers in the stubs (vtable)
		// the rest are checked explicitly now.
		//
		address pc = (address) exceptionInfo->ContextRecord->StIIP;
		CodeBlob* cb = CodeCache::find_blob(pc);
		if (cb != NULL) {
		  if (VtableStubs::stub_containing(pc) != NULL) {
		    if (((uintptr_t)addr) < os::vm_page_size() ) {
		      // an access to the first page of VM--assume it is a null pointer
		      return Handle_Exception(exceptionInfo,
			StubRoutines::__CPU__::handler_for_null_exception());
		    }
		  }
		}
#endif
	      }
            } // in_java

            // IA64 doesn't use implicit null checking yet. So we shouldn't
            // get here.
            tty->print_cr("Access violation, possible null pointer exception");
            os::handle_unexpected_exception(thread,
                        exception_code,
                        (address) exceptionInfo->ExceptionRecord->ExceptionAddress,
                        (void *) exceptionInfo);
#else
	    return Handle_Exception(exceptionInfo,
	      StubRoutines::__CPU__::handler_for_null_exception());
#endif
          }
	}
      }

      // Stack overflow or null pointer exception in native code.
      os::handle_unexpected_exception(thread, exception_code,
        (address) exceptionInfo->ExceptionRecord->ExceptionAddress,
	(void *) exceptionInfo);
      return EXCEPTION_CONTINUE_SEARCH;
    }

    if (in_java) {
      switch (exception_code) {
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return Handle_Exception(exceptionInfo, StubRoutines::__CPU__::handler_for_divide_by_zero());

      case EXCEPTION_INT_OVERFLOW:
        return Handle_IDiv_Exception(exceptionInfo);
      
#ifndef CORE
      // Only used for safepoints in compiler
      case EXCEPTION_ILLEGAL_INSTRUCTION: // fall-through
      case EXCEPTION_ILLEGAL_INSTRUCTION_2: {       
#ifdef _M_IA64
        assert(0, "Fix Exception Filter");
        {
#else
        NativeInstruction *inst  = (NativeInstruction *)exceptionInfo->ContextRecord->Eip;
        if (inst->is_illegal()) {   
#ifdef COMPILER1
          return Handle_Exception(exceptionInfo, Runtime1::entry_for(Runtime1::illegal_instruction_handler_id));
#else
          assert(OptoRuntime::illegal_exception_handler_blob() != NULL, "stub not created yet");
          return Handle_Exception(exceptionInfo, (address)OptoRuntime::illegal_exception_handler_blob()->instructions_begin());
#endif
#endif // else _M_IA64
        }        
        break;      
      }
#endif      
      } // switch
    }
  }

  if (previous_toplevel_exception_filter != NULL) {
    // Call an already existing toplevel exception handler
    return previous_toplevel_exception_filter(exceptionInfo);
  } else if (exception_code != EXCEPTION_BREAKPOINT) {    
#ifndef _M_IA64
    os::handle_unexpected_exception(t,
                        exception_code,
                        (address) exceptionInfo->ExceptionRecord->ExceptionAddress,
                        (void *) exceptionInfo);
#else
    // Itanium Windows uses a VectoredExceptionHandler
    // Which means that C++ programatic exception handlers (try/except)
    // will get here.  Continue the search for the right except block if
    // the exception code is not a fatal code.
    switch ( exception_code ) {
      case EXCEPTION_ACCESS_VIOLATION:
      case EXCEPTION_STACK_OVERFLOW:
      case EXCEPTION_ILLEGAL_INSTRUCTION:
      case EXCEPTION_ILLEGAL_INSTRUCTION_2:
      case EXCEPTION_INT_OVERFLOW:
      case EXCEPTION_INT_DIVIDE_BY_ZERO:
        os::handle_unexpected_exception(t,
                          exception_code,
                          (address) exceptionInfo->ExceptionRecord->ExceptionAddress,
                          (void *) exceptionInfo);
        break;
      default:
        break;
    }
#endif
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

// Install a win32 structured exception handler around thread.
void os::os_exception_wrapper(java_call_t f, JavaValue* value, methodHandle* method, JavaCallArguments* args, Thread* thread) {
#ifdef _M_IA64
    f(value, method, args, thread);
#else
  __try {
    f(value, method, args, thread);
  } __except(topLevelExceptionFilter((_EXCEPTION_POINTERS*)_exception_info())) {
      // Nothing to do.
  }
#endif
}

// Virtual Memory

int os::vm_page_size() { return os::win32::vm_page_size(); }


// ISM not avaliable for win32
char* os::reserve_memory_special(size_t bytes) {
  ShouldNotReachHere();
  return NULL;
}
 
bool os::set_mpss_range(char* addr, size_t size, size_t align) {
  ShouldNotReachHere();
  return false;
}

void os::print_statistics() {
}

bool os::commit_memory(char* addr, size_t bytes) {
  if (bytes == 0) {
    // Don't bother the OS with noops.
    return true;
  }
  assert((size_t) addr % os::vm_page_size() == 0, "commit on page boundaries");
  assert(bytes % os::vm_page_size() == 0, "commit in page-sized chunks");
  // Don't attempt to print anything if the OS call fails. We're
  // probably low on resources, so the print itself may cause crashes.
  return VirtualAlloc(addr, bytes, MEM_COMMIT, PAGE_EXECUTE_READWRITE) != NULL;
}


bool os::uncommit_memory(char* addr, size_t bytes) {
  if (bytes == 0) {
    // Don't bother the OS with noops.
    return true;
  }
  assert((size_t) addr % os::vm_page_size() == 0, "uncommit on page boundaries");
  assert(bytes % os::vm_page_size() == 0, "uncommit in page-sized chunks");
  return VirtualFree(addr, bytes, MEM_DECOMMIT);
}


bool os::release_memory(char* addr, size_t bytes) {
  return VirtualFree(addr, 0, MEM_RELEASE);
}


bool os::protect_memory(char* addr, size_t bytes) {
  DWORD old_status;
  return VirtualProtect(addr, bytes, PAGE_READONLY, &old_status);
}


bool os::guard_memory(char* addr, size_t bytes) {
  DWORD old_status;
  return VirtualProtect(addr, bytes, PAGE_EXECUTE_READWRITE | PAGE_GUARD, &old_status);
}


bool os::unguard_memory(char* addr, size_t bytes) {
  DWORD old_status;
  return VirtualProtect(addr, bytes, PAGE_EXECUTE_READWRITE, &old_status);
}


void os::pd_start_thread(Thread* thread) {
  DWORD ret = ResumeThread(thread->osthread()->thread_handle());
  // Returns previous suspend state:
  // 0:  Thread was not suspended
  // 1:  Thread is running now
  // >1: Thread is still suspended.
  assert(ret != SYS_THREAD_ERROR, "StartThread failed"); // should propagate back
}

//Helper function trying to suspend thread for a reasonable 
//number of iterations. This becomes neccessary because 
//SuspendThread may fail to suspend a thread that makes 
//Windows kernel call.  
static DWORD try_to_suspend(void* handle) { 
    DWORD ret = 0;
    int err_count = 0; 
    while(SuspendThread(handle) == SYS_THREAD_ERROR) { 
        if(err_count++ >= MAX_ERROR_COUNT) {
            return SYS_THREAD_ERROR;
        }
        Sleep(0); 
    } 
    return ret;
}

// Suspend a thread by one level. The VM code tracks the
// nesting despite Win32 SuspendThread support for nesting.
// This aids in debugging and assertions and more shared code.
// SuspendThread returns previous suspend count if successful
// pd_suspend_thread returns 0 on success.

int os::pd_suspend_thread(Thread* thread, bool fence) {
    HANDLE handle = thread->osthread()->thread_handle();    
    DWORD ret;
    if (fence) {
      ThreadCritical tc;
      ret = try_to_suspend(handle);
    } else {
      ret = try_to_suspend(handle);
    }
    assert(ret != SYS_THREAD_ERROR, "SuspendThread failed"); // should propagate back
    assert(ret == 0, "Win32 nested suspend");

    return ret;
}

// Resume a thread by one level.  This method assumes that consecutive
// suspends nest and require matching resumes to fully resume.  Note that
// this is different from Java's Thread.resume, which always resumes any
// number of nested suspensions.  The ability to nest suspensions is used
// by other facilities like safe points.  
// Multiple-level suspends are handled by common code in the Thread class.
// Resuming a thread that is not suspended is a no-op.
// ResumeThread returns -1 failure, 0 not suspended, else
// previous suspend count, i.e. 1 for now restarted.
// pd_resume_thread returns 0 on success
// vm_resume() handles nesting suspend/resume requests, pd_resume_thread()
// is only called when nesting level reaches 0. 

int os::pd_resume_thread(Thread* thread) {
  OSThread* osthread = thread->osthread();
  DWORD ret = ResumeThread(osthread->thread_handle());
  assert(ret != SYS_THREAD_ERROR, "ResumeThread failed"); // should propagate back
  assert(ret != 0, "Cannot resume unsuspended thread");
  if (ret == SYS_THREAD_ERROR) {
    return -1;
  }
  return 0;
}

// we need to be able to suspend ourself while at the same (atomic) time
// giving up the SR_lock -- we do this by using the
// SR_lock to implement a suspend_self
int os::pd_self_suspend_thread(Thread* thread) {
  thread->SR_lock()->wait(Mutex::_no_safepoint_check_flag);
  return 0;
}

class HighResolutionInterval {
  // The default timer resolution seems to be 10 milliseconds.
  // (Where is this written down?)
  // If someone wants to sleep for only a fraction of the default,
  // then we set the timer resolution down to 1 millisecond for 
  // the duration of their interval.
  // We carefully set the resolution back, since otherwise we 
  // seem to incur an overhead (3%?) that we don't need.
private:
    jlong resolution;
public:
  HighResolutionInterval(jlong ms) {
    resolution = ms % 10L;
    if (resolution != 0) {
      MMRESULT result = timeBeginPeriod(1L);
    }
  }
  ~HighResolutionInterval() {
    if (resolution != 0) {
      MMRESULT result = timeEndPeriod(1L);
    }
    resolution = 0L;
  }
};

int os::sleep(Thread* thread, jlong ms, bool interruptable) {
  jlong limit = (jlong) MAXDWORD;

  while(ms > limit) {
    int res;
    if ((res = sleep(thread, limit, interruptable)) != OS_TIMEOUT)
      return res;
    ms -= limit;
  }

  assert(thread == Thread::current(),  "thread consistency check");
  OSThread* osthread = thread->osthread();
  OSThreadWaitState osts(osthread, false /* not Object.wait() */);
  int result;
  if (interruptable) {    
    assert(thread->is_Java_thread(), "must be java thread");
    StateSaver sv((JavaThread*)thread);
    ThreadBlockInVM tbivm((JavaThread*) thread);
    HANDLE events[1];
    events[0] = osthread->interrupt_event();     
    HighResolutionInterval *phri=NULL;
    if(!ForceTimeHighResolution)
      phri = new HighResolutionInterval( ms );
    if (WaitForMultipleObjects(1, events, FALSE, (DWORD)ms) == WAIT_TIMEOUT) {
      result = OS_TIMEOUT;
    } else {
      osthread->set_interrupted(false);
      ResetEvent(osthread->interrupt_event());
      result = OS_INTRPT;
    }
    delete phri; //if it is NULL, harmless

#if 0
    // XXX - This code was not exercised during the Merlin RC1
    // pre-integration test cycle so it has been removed to
    // reduce risk.
    //
    // were we externally suspended while we were waiting?
    if (((JavaThread *) thread)->is_external_suspend_with_lock()) {
      //
      // While we were waiting in WaitForMultipleObjects() another thread
      // suspended us. We don't want to continue running while suspended
      // because that would surprise the thread that suspended us.
      //
      ((JavaThread *) thread)->java_suspend_self();
    }
#endif
  } else {    
    assert(!thread->is_Java_thread(), "must not be java thread");
    Sleep((long) ms);
    result = OS_TIMEOUT;
  }
  return result;
}

// Sleep forever; naked call to OS-specific sleep; use with CAUTION
void os::infinite_sleep() {
  while (true) {    // sleep forever ...
    Sleep(100000);  // ... 100 seconds at a time
  }
}

void os::yield() {  
  // Yields to all threads with same priority  
  Sleep(0);
}

void os::yield_all(int attempts) {
  // Yields to all threads, including threads with lower priorities    
  Sleep(1);
}

// Win32 only gives you access to seven real priorities at a time,
// so we compress Java's ten down to seven.  It would be better
// if we dynamically adjusted relative priorities. 

int os::java_to_os_priority[MaxPriority + 1] = {
  THREAD_PRIORITY_IDLE,                         // 0  Entry should never be used
  THREAD_PRIORITY_LOWEST,                       // 1  MinPriority
  THREAD_PRIORITY_LOWEST,                       // 2
  THREAD_PRIORITY_BELOW_NORMAL,                 // 3
  THREAD_PRIORITY_BELOW_NORMAL,                 // 4
  THREAD_PRIORITY_NORMAL,                       // 5  NormPriority
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 6
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 7
  THREAD_PRIORITY_HIGHEST,                      // 8
  THREAD_PRIORITY_HIGHEST,                      // 9  NearMaxPriority
  THREAD_PRIORITY_TIME_CRITICAL                 // 10 MaxPriority
};

OSReturn os::set_native_priority(Thread* thread, int priority) {
  bool ret = SetThreadPriority(thread->osthread()->thread_handle(), priority);
  return ret ? OS_OK : OS_ERR;
}

OSReturn os::get_native_priority(Thread* thread, int* priority_ptr) {
  int os_prio = GetThreadPriority(thread->osthread()->thread_handle());
  if (os_prio == THREAD_PRIORITY_ERROR_RETURN) {
    assert(false, "GetThreadPriority failed");
    return OS_ERR;
  }
  *priority_ptr = os_prio;
  return OS_OK;
}


// Increase priority of a thread by one OS level, but not higher than
// the os-level corresponding to the given java priority.
// Returns 'true' is priority is changed
bool os::boost_priority(Thread *thread, int max_java_priority) {
  int cur_os_prio   = GetThreadPriority(thread->osthread()->thread_handle());
  int max_os_prio   = java_to_os_priority[max_java_priority];
    
  // Boost priority (NT priorities are not just 1,2,3, hence, the case statement)
  int new_os_prio;
  switch(cur_os_prio) {
    case THREAD_PRIORITY_LOWEST         : new_os_prio = THREAD_PRIORITY_BELOW_NORMAL;   break;
    case THREAD_PRIORITY_BELOW_NORMAL   : new_os_prio = THREAD_PRIORITY_NORMAL;         break;
    case THREAD_PRIORITY_NORMAL         : new_os_prio = THREAD_PRIORITY_ABOVE_NORMAL;   break;
    case THREAD_PRIORITY_ABOVE_NORMAL   : new_os_prio = THREAD_PRIORITY_HIGHEST;        break;
    case THREAD_PRIORITY_HIGHEST        : new_os_prio = THREAD_PRIORITY_TIME_CRITICAL;  break;
    case THREAD_PRIORITY_TIME_CRITICAL  : return false; // Cannot boost at max priority.
  }

  // Check if boosting is allowed
  if (new_os_prio > max_os_prio) return false;

  if (!SetThreadPriority(thread->osthread()->thread_handle(), new_os_prio)) return false;
  return true;
}

// Decrease priority of a thread by one OS level, but not lower than
// the os-level corresponding to the given java priority
bool os::deflate_priority(Thread *thread, int min_java_priority) {
  int cur_os_prio = GetThreadPriority(thread->osthread()->thread_handle());
  int min_os_prio = java_to_os_priority[min_java_priority];
  
  // Deflate priority (NT priorities are not just 1,2,3, hence, the case statement)
  int new_os_prio;
  switch(cur_os_prio) {
    case THREAD_PRIORITY_LOWEST         : return false; // Cannot deflate lowest priority
    case THREAD_PRIORITY_BELOW_NORMAL   : new_os_prio = THREAD_PRIORITY_LOWEST;       break;
    case THREAD_PRIORITY_NORMAL         : new_os_prio = THREAD_PRIORITY_BELOW_NORMAL; break;
    case THREAD_PRIORITY_ABOVE_NORMAL   : new_os_prio = THREAD_PRIORITY_NORMAL;       break;
    case THREAD_PRIORITY_HIGHEST        : new_os_prio = THREAD_PRIORITY_ABOVE_NORMAL; break;
    case THREAD_PRIORITY_TIME_CRITICAL  : new_os_prio = THREAD_PRIORITY_HIGHEST;      break;    
  }
  if (new_os_prio < min_os_prio) return false;

  if (!SetThreadPriority(thread->osthread()->thread_handle(), new_os_prio)) return false;  
  return true;
}

// Hint to the underlying OS that a task switch would not be good.
// Void return because it's a hint and can fail.
void os::hint_no_preempt() {}

void os::interrupt(Thread* thread) {
  assert(!thread->is_Java_thread() || Thread::current() == thread || Threads_lock->owned_by_self(),
         "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();
  osthread->set_interrupted(true);
  SetEvent(osthread->interrupt_event());
}


bool os::is_interrupted(Thread* thread, bool clear_interrupted) {
  assert(!thread->is_Java_thread() || Thread::current() == thread || Threads_lock->owned_by_self(),
         "possibility of dangling Thread pointer");

  OSThread* osthread = thread->osthread();
  bool interrupted;
  interrupted = osthread->interrupted();
  if (clear_interrupted == true) {
    osthread->set_interrupted(false);
    ResetEvent(osthread->interrupt_event());
  } // Otherwise leave the interrupted state alone

  return interrupted;
}


ExtendedPC os::fetch_top_frame(Thread* thread, intptr_t** sp, intptr_t** fp) {
  CONTEXT context;
  context.ContextFlags = CONTEXT_CONTROL;
  ExtendedPC addr;
#ifdef _M_IA64
  if (GetThreadContext(thread->osthread()->thread_handle(), &context)) {
    *sp  = (intptr_t*)context.IntSp;
    *fp  = (intptr_t*)context.RsBSP;
    addr = ExtendedPC((address)context.StIIP);
  } else {
    ShouldNotReachHere();    
  }
  return addr;
#else
  if (GetThreadContext(thread->osthread()->thread_handle(), &context)) {
    *sp  = (jint*)context.Esp;
    *fp  = (jint*)context.Ebp;
    addr = ExtendedPC((address)context.Eip);
  } else {
    ShouldNotReachHere();    
  }
  return addr;
#endif
}

#ifndef CORE
static bool _set_thread_pc(Thread* thread, ExtendedPC old_pc, ExtendedPC new_pc) {
  CONTEXT context;
  context.ContextFlags = CONTEXT_CONTROL;  
  HANDLE handle = thread->osthread()->thread_handle();
#ifdef _M_IA64
  assert(0, "Fix set_thread_pc");
  return false;
#else
  if (GetThreadContext(handle, &context)) {
    if (context.Eip != (unsigned long)old_pc.pc()) return false;
    context.Eip = (unsigned long)new_pc.pc();
    if (!SetThreadContext(handle, &context)) {    
      // SetThreadContext failed
      ShouldNotReachHere();
      return false;
    }
  } else {
    // GetThreadContext failed
    ShouldNotReachHere();
    return false;
  }    
  return true;
#endif
}

bool os::set_thread_pc_and_resume(JavaThread* thread, ExtendedPC old_pc, ExtendedPC new_pc) {
  assert(thread->is_in_compiled_safepoint(), "only for compiled safepoint");

  bool rslt = _set_thread_pc(thread, old_pc, new_pc);
  thread->safepoint_state()->notify_set_thread_pc_result(rslt);
  thread->vm_resume(false);
  return rslt;
}
#endif

// Get's a pc (hint) for a running thread. Currently used only for profiling.
ExtendedPC os::get_thread_pc(Thread* thread) {
  CONTEXT context;
  context.ContextFlags = CONTEXT_CONTROL;  
  HANDLE handle = thread->osthread()->thread_handle();
#ifdef _M_IA64
  assert(0, "Fix get_thread_pc");
  return ExtendedPC(NULL);
#else
  if (GetThreadContext(handle, &context)) {
    return ExtendedPC((address) context.Eip);
  } else {
    return ExtendedPC(NULL);
  }
#endif
}

// GetCurrentThreadId() returns DWORD
intx os::current_thread_id()          { return GetCurrentThreadId(); }

int os::current_process_id()
{
  return _getpid();
}

int    os::win32::_vm_page_size       = 0;
int    os::win32::_processor_type     = 0;
int    os::win32::_processor_level    = 0;
int    os::win32::_processor_count    = 0;
julong os::win32::_physical_memory    = 0;
size_t os::win32::_default_stack_size = 0;
jint   os::win32::_os_thread_limit    = 0;
jint   os::win32::_os_thread_count    = 0;

bool   os::win32::_is_nt              = false;

// hard code assembler for access cpuid
static const unsigned char call_cpuid[] = {
  // mov eax, 1;
  0xB8, 0x01, 0x00, 0x00, 0x00,
  // cpuid;
  0x0F, 0xA2,
  // ret;
  0xC3,
  // int 3; debug break
  0xCC
};

// Find the executable file that started this process and read its "Portable
// Executable" (COFF) headers.
void os::win32::read_executable_headers(PIMAGE_NT_HEADERS nt_hdrs) {
  int rc;
  char pe_filename[MAX_PATH + 1];
  size_t len = GetModuleFileName(NULL, pe_filename, sizeof(pe_filename));
  assert(len > 0 && len < sizeof(pe_filename), "problem w/GetModuleFileName");
  FILE *pe_file = fopen(pe_filename, "rb");
  guarantee (pe_file != NULL, "error opening portable executable");

  // First, read the so-called DOS compatibility header.
  IMAGE_DOS_HEADER dos_hdr;
  rc = (int)fread(&dos_hdr, sizeof(IMAGE_DOS_HEADER), 1, pe_file);
  guarantee(rc == 1, "error reading portable executable header [1]");
  guarantee(dos_hdr.e_magic == IMAGE_DOS_SIGNATURE,
    "bad magic number in DOS .EXE header"); 

  // The PE signature (PE00), starts at the offset specified by e_lfanew.  It is
  // followed immediately by the file header and optional header.
  rc = fseek(pe_file, dos_hdr.e_lfanew, SEEK_SET);
  guarantee(rc == 0, "error reading portable executable header [2]");

  // Read the PE signature, file header and optional header in one chunk.
  rc = (int)fread(nt_hdrs, sizeof(IMAGE_NT_HEADERS), 1, pe_file);
  guarantee(rc == 1, "error reading portable executable header [3]");
  fclose(pe_file);

  guarantee(nt_hdrs->FileHeader.SizeOfOptionalHeader != 0,
    "the optional header is required for an executable");
  guarantee(nt_hdrs->Signature == IMAGE_NT_SIGNATURE,
    "bad portable executable signature");
  guarantee(nt_hdrs->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC,
    "bad magic number in optional header");
}

void os::win32::initialize_system_info() {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  _vm_page_size    = si.dwPageSize;
  _processor_type  = si.dwProcessorType;
  _processor_level = si.wProcessorLevel;
  _processor_count = si.dwNumberOfProcessors;

  MEMORYSTATUS ms;
  // also returns dwAvailPhys (free physical memory bytes), dwTotalVirtual, dwAvailVirtual,
  // dwMemoryLoad (% of memory in use)
  GlobalMemoryStatus(&ms);
  _physical_memory = ms.dwTotalPhys;

  OSVERSIONINFO oi;
  oi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx(&oi);
  switch(oi.dwPlatformId) {
    case VER_PLATFORM_WIN32_WINDOWS: _is_nt = false; break;
    case VER_PLATFORM_WIN32_NT:      _is_nt = true;  break;
    default: fatal("Unknown platform");
  }  

  if (is_nt()) {
    _default_stack_size = os::current_stack_size();
// The assert below was firing when running java_g 
// under the MKS shell running on Itanium systems
#ifndef _M_IA64
#ifdef ASSERT
    IMAGE_NT_HEADERS nt_hdrs;
    read_executable_headers(&nt_hdrs);
    assert(_default_stack_size == nt_hdrs.OptionalHeader.SizeOfStackReserve,
      "stack size mismatch - os::current_stack_size() vs. executable headers");
#endif
#endif
  } else {
    // The VirtualQuery call used by os::current_stack_size() yields the wrong
    // value on win98; read the executable.
    IMAGE_NT_HEADERS nt_hdrs;
    read_executable_headers(&nt_hdrs);
    _default_stack_size = nt_hdrs.OptionalHeader.SizeOfStackReserve;
  }
  assert(_default_stack_size > (size_t) _vm_page_size, "invalid stack size");
  assert((_default_stack_size & (_vm_page_size - 1)) == 0,
    "stack size not a multiple of page size");

  initialize_performance_counter();

  // Win95/Win98 scheduler bug work-around. The Win95/98 scheduler is
  // known to deadlock the system, if the VM issues to thread operations with
  // a too high frequency, e.g., such as changing the priorities. 
  // The 6000 seems to work well - no deadlocks has been notices on the test
  // programs that we have seen experience this problem.
  if (!os::win32::is_nt()) {    
    StarvationMonitorInterval = 6000;
  }

  if (!os::win32::is_nt() && _processor_type == PROCESSOR_INTEL_PENTIUM) {
    typedef int (__cdecl *function_t)(void);
    function_t function = (function_t) (void*) call_cpuid;
    int ret = function();
    // see Intel Pentium Instruction Set Reference
    // The format of return value is
    // 31          14  13 12  11 10 9 8  7 6 5 4  3 2 1 0
    // [   reserved  ][type ][  level  ][ model ][step id]
    _processor_level = ((ret >> 8) & 0xF);
  }

  // Turn on spin-lock only on multiprocessor machines.
  if (_processor_count > 1) {
    UseSpinning = true;
  }
}


void os::win32::setmode_streams() {
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
  _setmode(_fileno(stderr), _O_BINARY);
}


int os::message_box(const char* title, const char* message) {
  int result = MessageBox(NULL, message, title,
                          MB_YESNO | MB_ICONERROR | MB_SYSTEMMODAL | MB_DEFAULT_DESKTOP_ONLY);
  return result == IDYES;
}

char *os::do_you_want_to_debug(const char *message) {
  // os::handle_unexpected_exception() guarantees only one thread can call
  // this function 
  static char buf[256];
  jio_snprintf(buf, sizeof(buf), "%s Do you want to debug the problem?", message);
  return buf;
}

int os::allocate_thread_local_storage() {
  return TlsAlloc();
}


void os::free_thread_local_storage(int index) {
  TlsFree(index);
}


void os::thread_local_storage_at_put(int index, void* value) {
  TlsSetValue(index, value);
  assert(thread_local_storage_at(index) == value, "Just checking");
}


void* os::thread_local_storage_at(int index) {
  return TlsGetValue(index);
}

// this is called _before_ the global arguments have been parsed
void os::init(void) {
  init_random(1234567);

  win32::initialize_system_info();
  win32::setmode_streams();

  // For better scalability on MP systems (must be called after initialize_system_info)
#ifndef PRODUCT
  if (win32::is_MP()) {    
    NoYieldsInMicrolock = true;
  }
#endif
  // Initialize main_process and main_thread
  main_process = GetCurrentProcess();  // Remember main_process is a pseudo handle
  if (!DuplicateHandle(main_process, GetCurrentThread(), main_process,
                       &main_thread, THREAD_ALL_ACCESS, false, 0)) {
    fatal("DuplicateHandle failed\n");
  }
  main_thread_id = (int) GetCurrentThreadId();
}
 
// To install functions for atexit processing
extern "C" {
  static void perfMemory_exit_helper() {
    perfMemory_exit();
  }
}


// this is called _after_ the global arguments have been parsed
jint os::init_2(void) {
  // Setup Windows Exceptions

  // On Itanium systems, Structured Exception Handling does not
  // work since stack frames must be walkable by the OS.  Since
  // much of our code is dynamically generated, and we do not have
  // proper unwind .xdata sections, the system simply exits
  // rather than delivering the exception.  To work around
  // this we use VectorExceptions instead.
#ifdef _M_IA64
  topLevelVectoredExceptionHandler = AddVectoredExceptionHandler( 1, topLevelExceptionFilter);
#else
  if (UseTopLevelExceptionFilter && !UseNewOSExceptions) {
    previous_toplevel_exception_filter = SetUnhandledExceptionFilter(topLevelExceptionFilter);
  }  
#endif

  // for debugging float code generation bugs
  if (ForceFloatExceptions) {
#ifndef  _M_IA64
    static long fp_control_word = 0;
    __asm { fstcw fp_control_word }
    // see Intel PPro Manual, Vol. 2, p 7-16
    const long precision = 0x20;
    const long underflow = 0x10;
    const long overflow  = 0x08;
    const long zero_div  = 0x04;
    const long denorm    = 0x02;
    const long invalid   = 0x01;
    fp_control_word |= invalid;
    __asm { fldcw fp_control_word }
#endif
  }

  // Initialize HPI.
  jint hpi_result = hpi::initialize();
  if (hpi_result != JNI_OK) { return hpi_result; }

  size_t stack_create_size = round_to(ThreadStackSize*K, os::vm_page_size());
  JavaThread::set_stack_size_at_create(stack_create_size);

  // Calculate theoretical max. size of Threads to guard gainst artifical
  // out-of-memory situations, where all available address-space has been
  // reserved by thread stacks.
  size_t pre_thread_stack_size =
    stack_create_size > 0 ? stack_create_size : os::win32::default_stack_size();
  assert(pre_thread_stack_size != 0, "Must have a stack");
  // Win NT has a maximum of 2Gb of user programs. Calculate the thread limit when
  // we should start doing Virtual Memory banging. Currently when the threads will
  // have used all but 200Mb of space.
  size_t max_address_space = ((size_t)2 * K * K * K) - (200 * K * K);
  win32::_os_thread_limit = (jint)(max_address_space / pre_thread_stack_size);

  // at exit methods are called in the reverse order of their registration.
  // there is no limit to the number of functions registered. atexit does
  // not set errno.

  if (PerfAllowAtExitRegistration) {
    // only register atexit functions if PerfAllowAtExitRegistration is set.
    // atexit functions can be delayed until process exit time, which
    // can be problematic for embedded VM situations. Embedded VMs should
    // call DestroyJavaVM() to assure that VM resources are released.

    // note: perfMemory_exit_helper atexit function may be removed in
    // the future if the appropriate cleanup code can be added to the
    // VM_Exit VMOperation's doit method.
    if (atexit(perfMemory_exit_helper) != 0) {
      warning("os::init_2 atexit(perfMemory_exit_helper) failed");
    }
  }

  return JNI_OK;
}


void os::exit(void) {
  ThreadCritical::release();
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


// JVMPI code

#define FT2INT64(ft) \
  ((jlong)(ft).dwHighDateTime << 32 | (jlong)(ft).dwLowDateTime)

jlong os::thread_cpu_time() {
  // This code is copy from clasic VM -> hpi::sysThreadCPUTime
  if (os::win32::is_nt()) {
    FILETIME CreationTime;
    FILETIME ExitTime;
    FILETIME KernelTime;
    FILETIME UserTime;

    GetThreadTimes(GetCurrentThread(), 
                    &CreationTime, & ExitTime, &KernelTime, &UserTime);
    return FT2INT64(UserTime) * 100;
  } else {
    return (jlong) timeGetTime() * 1000000;
  }
}


bool os::thread_is_running(JavaThread* tp) {
#ifdef _M_IA64
  assert(0, "Fix thread_is_running");
  return  false;
#else
  // this code is a copy from classic VM -> hpi::sysThreadIsRunning
  unsigned int sum = 0;
  unsigned int *p;
  CONTEXT context;
  
  context.ContextFlags = CONTEXT_FULL;
  GetThreadContext(tp->osthread()->thread_handle(), &context);
  p = (unsigned int*)&context.SegGs;
  while (p <= (unsigned int*)&context.SegSs) {
    sum += *p;
    p++;
  }
  if (sum == tp->last_sum()) {
    return false;
  }
  tp->set_last_sum(sum);
  return true;
#endif
}


// DontYieldALot=false by default: dutifully perform all yields as requested by JVM_Yield()
bool os::dont_yield() {
  return DontYieldALot;
}

//--------------------------------------------------------------------------------------------------
// Non-product code

#ifdef PRODUCT
bool os::check_heap(bool force) { return true; }
#else
static int mallocDebugIntervalCounter = 0;
static int mallocDebugCounter = 0;
bool os::check_heap(bool force) {
  if (++mallocDebugCounter < MallocVerifyStart && !force) return true;
  if (++mallocDebugIntervalCounter >= MallocVerifyInterval || force) {
    // Note: HeapValidate executes two hardware breakpoints when it finds something
    // wrong; at these points, eax contains the address of the offending block (I think).
    // To get to the exlicit error message(s) below, just continue twice.
    HANDLE heap = GetProcessHeap();
    { HeapLock(heap);
      PROCESS_HEAP_ENTRY phe;
      phe.lpData = NULL;
      while (HeapWalk(heap, &phe) != 0) {
        if ((phe.wFlags & PROCESS_HEAP_ENTRY_BUSY) &&
            !HeapValidate(heap, 0, phe.lpData)) {
          tty->print_cr("C heap has been corrupted (time: %d allocations)", mallocDebugCounter);
          tty->print_cr("corrupted block near address %#x, length %d", phe.lpData, phe.cbData);
          fatal("corrupted C heap");
        }
      }
      int err = GetLastError();
      if (err != ERROR_NO_MORE_ITEMS && err != ERROR_CALL_NOT_IMPLEMENTED) {
        fatal1("heap walk aborted with error %d", err);
      }
      HeapUnlock(heap);
    }
    mallocDebugIntervalCounter = 0;
  }
  return true;
}
#endif


#ifndef PRODUCT
bool os::find(address addr) {
  // Nothing yet
  return false;
}
#endif 

typedef jlong compare_and_exchange_long_func_t(jlong, jlong*, jlong);

jlong os::win32::atomic_compare_and_exchange_long_bootstrap(jlong exchange_value, jlong* dest, jlong compare_value) {
  // try to use the stub:

  compare_and_exchange_long_func_t* func = CAST_TO_FN_PTR(compare_and_exchange_long_func_t*,
                                                       StubRoutines::atomic_compare_and_exchange_long_entry());

  if (func != NULL) {
    os::win32::atomic_compare_and_exchange_long_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jlong old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

compare_and_exchange_long_func_t* os::win32::atomic_compare_and_exchange_long_func
  = os::win32::atomic_compare_and_exchange_long_bootstrap;

// (Note:  The debugging machinery for malloc is in os.cpp.)
