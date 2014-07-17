#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_linux_ia64.cpp	1.30 03/05/22 13:47:29 JVM"
#endif
//
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_linux_ia64.cpp.incl"
# include "incls/_os_pd.hpp.incl"

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
# include <ucontext.h>

// On Itanium the SP is in r12
#ifndef REG_SP
#define REG_SP 12
#endif


char* os::reserve_memory(size_t size) {
  char *addr = (char*)::mmap(0, size, PROT_READ|PROT_WRITE|PROT_EXEC,
                      MAP_PRIVATE|MAP_NORESERVE|MAP_ANONYMOUS, -1, 0);

  return addr == MAP_FAILED ? NULL : addr;
}
  
char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).

  return (char*) -1;
}


address os::Linux::ucontext_get_pc(ucontext_t * uc) {
  return (address)uc->uc_mcontext.sc_ip;
}

intptr_t* os::Linux::ucontext_get_sp(ucontext_t * uc) {
  return (intptr_t*)uc->uc_mcontext.sc_gr[REG_SP];
}

intptr_t* os::Linux::ucontext_get_fp(ucontext_t * uc) {
  return (intptr_t*)uc->uc_mcontext.sc_ar_bsp;
}

#ifndef CORE
// return true on success, false otherwise
bool os::set_thread_pc_and_resume(JavaThread* thread, ExtendedPC old_addr, ExtendedPC new_addr) {
  assert(thread->is_in_compiled_safepoint(), "only for compiled safepoint");

  OSThread* osthread = thread->osthread();
  assert(thread->is_vm_suspended(), "must be suspended");

  // Note: self-suspended thread or thread suspended when waiting for
  // a mutex can not be repositioned, because it is not suspended
  // inside SR_handler. set_thread_pc() is only called when fetch_top_frame()
  // returns address in nmethod. That cannot be self-suspension nor
  // the mutex case.
  assert(!(thread->is_Java_thread()&&((JavaThread*)thread)->is_self_suspended())
       &&!thread->osthread()->sr.is_try_mutex_enter(),
         "cannot be self-suspended or suspended while waiting for mutex");

  ucontext_t* context = (ucontext*) osthread->ucontext();
  bool rslt = false;
  if (context->uc_mcontext.sc_ip == (intptr_t)old_addr.pc()) {
    context->uc_mcontext.sc_ip = (intptr_t)new_addr.pc();
    rslt = true;
  }

  thread->safepoint_state()->notify_set_thread_pc_result(rslt);
  thread->vm_resume(false);
  return rslt;
}
#endif

// Utility functions

julong os::allocatable_physical_memory(julong size) {
  return MIN2(size, (julong)M*M); // plenty for now
}


extern "C" void findpc(int x);
extern void set_error_reported();
extern void signalHandler(int sig, siginfo_t* info, ucontext_t* context);

extern "C" int 
JVM_handle_linux_signal(int sig,
                        siginfo_t* info,
                        void* ucVoid,
                        int abort_if_unrecognized)
{



  ucontext_t* uc = (ucontext_t*) ucVoid;

  if (sig == SIGCHLD) {
    // cleanup zombie pid for child thread
    ::waitpid(-1, 0, WNOHANG);
    return 1;
  }

  JavaThread* thread = NULL;
  VMThread* vmthread = NULL;
  if (os::Linux::signal_handlers_are_installed) {
    // Thread* t = Thread::current();
    Thread* t = ThreadLocalStorage::get_thread_slow();  // slow & steady
    if (t != NULL ){
      if(t->is_Java_thread()) {
        thread = (JavaThread*)t;
      }
      else if(t->is_VM_thread()){
        vmthread = (VMThread *)t;
      }
    }
  }
/*
  NOTE: does not seem to work on linux.
  if (info == NULL || info->si_code <= 0 || info->si_code == SI_NOINFO) {
    // can't decode this kind of signal
    info = NULL;
  } else {
    assert(sig == info->si_signo, "bad siginfo");
  }
*/
  // decide if this trap can be handled by a stub
  address stub = NULL;

  address pc          = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.sc_ip;
    #ifndef CORE
    adjusted_pc = thread->safepoint_state()->compute_adjusted_pc(pc);
    #else
    adjusted_pc = pc;
    #endif

    // Handle ALL stack overflow variations here
    if (sig == SIGSEGV) {
      // The kernel delivers a SEGV on page faults to addresses slightly below
      // the esp, even if the addr is part of the stack.  Manually expand the
      // stack if needed.
      address addr = (address) info->si_addr;
      if (thread->osthread() != NULL && thread->osthread()->expanding_stack() == 0) {
	thread->osthread()->set_expanding_stack();
	if (os::Linux::manually_expand_stack(thread, addr)) {
	  thread->osthread()->clear_expanding_stack();
	  return 1;
	}
	thread->osthread()->clear_expanding_stack();
      } else {
	fatal("recursive segv. expanding stack.");
      }

      if (thread->in_stack_yellow_zone(addr)) {
	thread->disable_stack_yellow_zone();
	if (thread->thread_state() == _thread_in_Java) {
	  // Throw a stack overflow exception.  Guard pages will be reenabled
	  // while unwinding the stack.
	  stub = StubRoutines::ia64::handler_for_stack_overflow();
	} else {
	  // Thread was in the vm or native code.  Return and try to finish.
	  return 1;
	}
      } else if (thread->in_stack_red_zone(addr)) {
	// Fatal red zone violation.  Disable the guard pages and fall through
	// to handle_unexpected_exception way down below.
	thread->disable_stack_red_zone();
	tty->print_cr("An irrecoverable stack overflow has occurred.");
      } else if ( addr == NULL ) {
        // Check for a register stack overflow in compiled code.
        // The generated compiled code will write to 0 to signal
        // a register stack overflow.  Verify that this is indeed the
        // condition and start a stack overflow exception in motion.
        if (thread->thread_state() == _thread_in_Java) {
          if ((address)uc->uc_mcontext.sc_ar_bsp > thread->register_stack_limit() ) {
	    // 4826555: nsk test stack016 fails.  See stubGenerator_ia64.cpp.
	    // Linux kernel puts zero in GR4-7, so this assert always fires.
	    //            assert((address)uc->uc_mcontext.sc_gr[7] == thread->register_stack_limit(), 
	    //                   "GR7 not set to the register stack limit");
            if (thread->stack_yellow_zone_enabled()) {
              thread->disable_stack_yellow_zone();
            }
            // Throw a stack overflow exception.  Guard pages will be reenabled
            //  while unwinding the stack.
            stub = StubRoutines::ia64::handler_for_stack_overflow();
            thread->disable_register_stack_guard();
            // Give the compiled code the new register stack limit so we
            // dont' come back here again.
            uc->uc_mcontext.sc_gr[7] = (unsigned long)thread->register_stack_limit();
          }
        } 
      }
    }

    if (thread->thread_state() == _thread_in_Java) {
      // Java thread running in Java code => find exception handler if any
      // a fault inside compiled code, the interpreter, or a stub

#ifndef CORE
      if (sig == SIGILL && nativeInstruction_at(pc)->is_illegal()) {
#ifdef COMPILER1
       stub =  Runtime1::entry_for(Runtime1::illegal_instruction_handler_id);
#else
        // debugging trap, or a safepoint
        assert(OptoRuntime::illegal_exception_handler_blob() != NULL, "stub not created yet");
        stub = OptoRuntime::illegal_exception_handler_blob()->instructions_begin();
#endif	// COMPILER1
        CompiledCodeSafepointHandler *handler = (CompiledCodeSafepointHandler *) thread->safepoint_state()->handle();
        if (handler != NULL && thread->is_in_compiled_safepoint()) {
          nmethod* nm = handler->get_nmethod();
          assert(nm != NULL, "safepoint handler is not setup correctly");

          relocInfo::relocType type = nm->reloc_type_for_address(adjusted_pc);
          assert( ((NativeInstruction*)adjusted_pc)->is_call() || 
        	  (type == relocInfo::return_type) || (type == relocInfo::safepoint_type), 
        	  "Only calls, returns, and backward branches are patched at safepoint");

          if(type == relocInfo::return_type) {
            // The retl case: restore has already happened. The safepoint blob frame will appear 
            // immediately below caller of the orignal method that we have patched. The stack could 
            // be walked properly (the frame of the patched method is already popped). 
            // Will need to be revisited if and when we put restore in the delay slot of ret.
          }
          else {
            // This is the case when either a call or a branch was patched with an illegal instruction.
            // At this point we are done with the patched method and would like things to
            // appear as if the orignal method simply called the safepoint blob at a "safe point".
            // To achieve this, we set up the correct linkage by placing the adjusted trap pc in O7
            // and then "returning" from this trap handler to the safepoint blob.

            // QQQ TODO
#if 0
            // uc->uc_mcontext.sc_gr[REG_O7] = (unsigned long) adjusted_pc;
#endif
          }
        }
      }
      else 
#endif // NOT CORE 

      if (sig == SIGFPE /* && info->si_code == FPE_INTDIV */) {
#if 0
        // HACK: si_code does not work on linux 2.2.12-20!!!
        int op = pc[0];
        if (op == 0xDB) {
          // FIST
          // TODO: The encoding of D2I in ia64.ad can cause an exception
          // prior to the fist instruction if there was an invalid operation
          // pending. We want to dismiss that exception. From the win_32
          // side it also seems that if it really was the fist causing
          // the exception that we do the d2i by hand with different
          // rounding. Seems kind of weird.
          // NOTE: that we take the exception at the NEXT floating point instruction.
          assert(pc[0] == 0xDB, "not a FIST opcode");
          assert(pc[1] == 0x14, "not a FIST opcode");
          assert(pc[2] == 0x24, "not a FIST opcode");
          return true;
        } else if (op == 0xF7) {
          // IDIV
          stub = StubRoutines::ia64::handler_for_divide_by_zero();
        } else {
          // TODO: handle more cases if we are using other x86 instructions
          //   that can generate SIGFPE signal on linux.
          tty->print_cr("unknown opcode 0x%X with SIGFPE.", op);
          fatal("please update this code.");
        }
#else
          tty->print_cr("Fix SIGFPE handler, trying divide by zero handler.");
          stub = StubRoutines::ia64::handler_for_divide_by_zero();
#endif
      }

#ifdef COMPILER2
      else if (sig == SIGSEGV && OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
        // This is the case when we use a trap to restore the context at the end of safepoint
        // taken in compiled code. The "return" address is passed in O7
        // QQQ TODO
        ShouldNotReachHere();
#if 0
        unsigned long ret_pc = uc->uc_mcontext.sc_ip;
        // Recover global regs saved at the original illegal instruction trap. 
        // (%o are handled "automatically" and are possibly modified by the gc - don't touch them)
        thread->restore_global_regs_from_saved_context(uc);
        uc->uc_mcontext.sc_ip =  ret_pc;
        return true;
#endif // 0
      }
#endif	// COMPILER2
      else if (sig == SIGSEGV ) {
#ifndef CORE
	//
	// We only expect null pointers in the stubs (vtable only)
	//
	CodeBlob* cb = CodeCache::find_blob(adjusted_pc);
	if (cb != NULL) {
	  if (VtableStubs::stub_containing(adjusted_pc) != NULL) {
	    // We have to assume it is a NULL fault because info->si_addr
	    // is incorrect here
	    if (true || ((uintptr_t)info->si_addr) < os::vm_page_size() ) {
	      // an access to the first page of VM--assume it is a null pointer
	      stub = StubRoutines::ia64::handler_for_null_exception();
	    }
	  }
	}
#endif // CORE
      }
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it
    thread->set_saved_exception_pc(pc);
    uc->uc_mcontext.sc_ip = (unsigned long)stub;
    return true;
  }

  // signal-chaining
  if (UseSignalChaining) {
    bool chained = false;
    struct sigaction *actp = os::Linux::get_chained_signal_action(sig);
    if (actp != NULL) {
      chained = os::Linux::chained_handler(actp, sig, info, ucVoid);
    }
    if (chained) {
      // signal-chaining in effect. Continue.
      return true;
    }
  }

  if (sig == SIGPIPE) {
    if (PrintMiscellaneous && (WizardMode || Verbose)) {
      warning("Ignoring SIGPIPE - see bug 4229104");
    }
    return 1;
  }
  
  if (!abort_if_unrecognized) {
    // caller wants another chance, so give it to him
    return false;
  }

  if (pc == NULL && uc != NULL) {
    pc = (address) uc->uc_mcontext.sc_ip;
  }

#ifndef PRODUCT
  // tell the system that it is about to die
  set_error_reported();

  // Remove this handler now.  It is of no more use.
  os::Linux::set_signal_handler(sig, false);

  if (WizardMode) {
    if (pc != NULL) {
       printf("\n");
       printf("PC information:\n");
       findpc((uintptr_t)pc);
    }
    if (info != NULL) {
       printf("\n");
       printf("siginfo_t dump:\n");
       for (int i = 0; i < sizeof(info[0])/sizeof(int); i++) {
         printf(" 0x%08X", ((uintptr_t*)info)[i]);
         if ((i + 1) % 4 == 0)  printf("\n");
       }
       printf("\n");
    }
    if (uc != NULL) {
       printf("\n");
       printf("ucontext_t dump:\n");
       for (int i = 0; i < sizeof(uc[0])/sizeof(uintptr_t); i++) {
         printf(" 0x%08X", ((uintptr_t*)uc)[i]);
         if ((i + 1) % 4 == 0)  printf("\n");
       }
       printf("\n");

       tty->print_cr("Unexpected exception occured: signal=%d, pc=%lx", sig, (uintptr_t)pc);

       // QQQ will we need a stub to bridge into continue_with_dump so that a reasonable
       // return address will be present and the stack looks ok.
       //
       uc->uc_mcontext.sc_ip = (unsigned long)&continue_with_dump;
       return true;
    }
  }
#endif	// ! PRODUCT

  os::handle_unexpected_exception(thread, sig, pc, (void *)info);

  // reaches here only when the user wants to debug the problem in
  // -XX:+ShowMessageBoxOnError mode. But we can't launch debugger
  // as Windows does, just os::abort.
  os::abort();
}

void os::Linux::init_thread_fpu_state(void) {

  // nothing to do on ia64
}

// Thread creation and stack handling
// Split from os_linux.cpp because thread creation and stack issues are handled very differently
// on ia32 and ia64. The fact that register windows flush area are placed at the bottom of
// the stack forced this split

// 64K is reserved for alternate signal stack and guard pages
size_t os::Linux::min_stack_allowed  = 768*K;
size_t os::Linux::default_stack_size = 1024*K;

/* pthread_getattr_np comes with LinuxThreads-0.9-7 on RedHat 7.1 */
typedef int (*pthread_getattr_func_type) (pthread_t, pthread_attr_t *);

////////////////////////////////////////////////////////////////////////////////
// thread creation support

static void* _start(Thread* thread) {
 assert(os::Linux::is_floating_stack(), "must be floating stack");

  // NOTE: it is safe to do this on linux.
  ThreadLocalStorage::set_thread(thread);

  OSThread* osthread = thread->osthread();

  // setup linux process id for thread
  osthread->set_thread_id(::getpid());

  int need_altstack =
    (osthread->thread_type() != os::vm_thread &&
     osthread->thread_type() != os::compiler_thread);

  if (need_altstack) {
    // Make sure the thread has an alternate signal stack
    os::Linux::install_alternate_signal_stack(osthread);
  }

  // initialize signal mask for this thread
  os::Linux::hotspot_sigmask(pthread_self() == os::Linux::main_thread());

  // initialize floating point control register
  os::Linux::init_thread_fpu_state();

  // notify parent thread
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(INITIALIZED);
  }

  // wake up parent thread
  osthread->startThread_event()->set();

  // wait until notified.
  ((os::Linux::Event*) osthread->interrupt_event())->down();

  // call one more level start routine
  thread->run();

  if (need_altstack) {
    // Make sure remove the alternate signal stack
    os::Linux::remove_alternate_signal_stack();
  }

  ThreadLocalStorage::set_thread(NULL);

  return 0;
}

bool os::create_thread(Thread* thread, ThreadType thr_type, size_t stack_size) {
  assert(thread->osthread() == NULL, "caller responsible");

  // Allocate the OSThread object
  OSThread* osthread = new OSThread(NULL, NULL);
  if (osthread == NULL) {
    return false;
  }
  osthread->clear_expanding_stack();

  // set the correct thread state
  osthread->set_thread_type(thr_type);

  // Initial state is ALLOCATED but not INITIALIZED
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    osthread->set_state(ALLOCATED);
  }

  thread->set_osthread(osthread);

  // Create the Linux thread
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // variable stack size is only supported on floating stack LinuxThreads
  assert(os::Linux::is_floating_stack(), " must be floating");
  if (stack_size == 0) {
    stack_size = JavaThread::stack_size_at_create();
  }
  if (thr_type == java_thread && stack_size > 0) {
    stack_size = MAX2(stack_size, os::Linux::min_stack_allowed);
  } else if ((thr_type == vm_thread ||
              thr_type == gc_thread ||
              thr_type == pgc_thread ||
              thr_type == cms_thread) &&
	     VMThreadStackSize > 0) {
    stack_size = MAX2((size_t)(VMThreadStackSize * K), os::Linux::min_stack_allowed);
  }
#ifdef COMPILER2
  // Compiler2 requires a large stack size to handle recursive routines.
  else if (thr_type == compiler_thread) {
    // stack size in units of Kbytes; 2M total needed as default.
    int default_size = (CompilerThreadStackSize > 0) ? CompilerThreadStackSize : 2 * K;
    stack_size = MAX2((size_t)(default_size * K), os::Linux::min_stack_allowed);
  }
#endif

  if (stack_size != 0) {
    pthread_attr_setstacksize(&attr, stack_size);
  } else {
    pthread_attr_setstacksize(&attr, os::Linux::default_stack_size);
  }

  {
    pthread_t tid;
    int ret = pthread_create(&tid, &attr, (void* (*)(void*)) _start, thread);

    pthread_attr_destroy(&attr);

    if (ret != 0) {
      if (PrintMiscellaneous && (Verbose || WizardMode)) {
        perror("pthread_create()");
      }
      // Need to clean up stuff we've allocated so far
      thread->set_osthread(NULL);
      delete osthread;
      return false;
    }

    // Store pthread info into the OSThread
    osthread->set_pthread_id(tid);

    // Wait until child thread is initialized
    osthread->startThread_event()->down();
  }

  ThreadState state;
  {
    MutexLockerEx ml(thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    state = osthread->get_state();

    // the child thread should be ready
    assert(state == INITIALIZED, "race condition");
  }  

  // The thread is returned suspended (in state INITIALIZED),
  // and is started higher up in the call chain
  return true;
}

////////////////////////////////////////////////////////////////////////////////
// thread local storage, stack information, alternative signal stack support

//
// Here is the diagram that shows stack region as being used on floating
// stack.
//
//  Low memory addresses
//      +------------------------+
//      |                        |\
//      |Register Window Stack   | - 
//      |                        |/
//      +------------------------+
//      |                        |\
//      |      Unused Space      | - (necessary for RedHat 7.0)
//      |                        |/
//      +------------------------+ low + HOTSPOT_ALTSTACK_START
//      +------------------------+
//      |                        |\
//      |    Glibc Guard Pages   | - Right in the middle of stack segment, 2 pages
//      |                        |/
//      +------------------------+ stack low addr (stack bottom as far as current_stack_region say)
//      +------------------------+ low + HOTSPOT_ALTSTACK_START
//      |                        |\
//      | Alternate Signal Stack | - HOTSPOT_ALTSTACK_SIZE (Contains alternate memory & register stack
//      |                        |/
//      +------------------------+ low + HOTSPOT_GUARD_START
//      |                        |\
//      |  HotSpot Guard Pages   | - HOTSPOT_GUARD_SIZE
//      |                        |/
//      +------------------------+ low + HOTSPOT_STACK_START
//      |                        | \
//      |      Memory Stack      |  -
//      |                        | /
//      +------------------------+ stack base = stack bottom + stack size
//  Higher memory addresses


#ifdef HOTSPOT_STACK_SIZE
#error "please update the following code carefully"
#endif

// Zone sizes and zone offsets from the low address of thread stack (stack
// bottom). A START offset refers to the low address in the zone. 
#define HOTSPOT_ALTSTACK_SIZE     (  8*os::vm_page_size())
#define HOTSPOT_ALTSTACK_START    (  0 )

#define HOTSPOT_GUARD_PAGES       (StackYellowPages + StackRedPages)
#define HOTSPOT_GUARD_SIZE        ((HOTSPOT_GUARD_PAGES) * os::vm_page_size())


// On IA64 the register windows reside at the bottom of the stack. 
// The pthread library divided this stack size in two. The top half
// is dedicated to the memory stack the bottom half is dedicated to
// the register window stack. The pthread library places a guard
// page in the middle of these two regions. The way we deal with
// this situation is basically to make the register window area
// invisible to the VM as far a stack allocation is concerned.
// This routine is key to making this are invisible. 
static void current_stack_region(address * bottom, size_t * size) {
  if ( os::Linux::is_initial_thread()) {
     /* 
        There is a hazard here since we don't know where the "middle" 
        guard page is located.  Experience has shown that we miss
        by one page when executing a java command with a lot
        of arguments.  We reserve three extra pages in order to
        avoid missing. 
      */
      #define MIDDLE_BUFFER_PAGE_COUNT 3

     *bottom = os::Linux::initial_thread_stack_bottom() + 
               (os::Linux::initial_thread_stack_size() / 2) + 
               (MIDDLE_BUFFER_PAGE_COUNT * os::Linux::page_size());
     *size   = (os::Linux::initial_thread_stack_size() / 2) - 
               (MIDDLE_BUFFER_PAGE_COUNT * os::Linux::page_size());
  } else {
    assert(os::Linux::is_floating_stack(), "only floating stack supported");
    pthread_getattr_func_type fn = CAST_TO_FN_PTR(pthread_getattr_func_type, os::Linux::pthread_getattr_func());
    assert(fn != NULL, "floating stack pthread must have pthread_getattr_np");
    pthread_attr_t attr;
    pthread_t tid = pthread_self();
    int rslt = fn(tid, &attr);
    if (rslt!=0) {
       /* what else can we do?? */
       fatal("Can not locate current stack region!");
    }

    void * top;
    size_t guard_size;
    if (pthread_attr_getstackaddr(&attr, &top) != 0 || pthread_attr_getstacksize(&attr, size) != 0 ||
	pthread_attr_getguardsize(&attr, &guard_size) != 0 ) {
       fatal("Can not locate current stack attributes!");
    }
    //
    // We calculate a false bottom that should be just above the guard page(s)
    // that the pthread library installs between the stack regions
    uintptr_t false_bottom =  (uintptr_t) top - ( *size / 2) + guard_size + os::Linux::page_size();
    *bottom = (address) align_size_up(false_bottom, os::Linux::page_size());
    *size   = (address)top - *bottom;
  }
}

//
// On x86 the alternate signal stack is allocated at the bottom of the 
// regular stack. This is becuase in some early kernels it was not possible
// to allocate this area into some arbitrary memory. Because of issues with
// the register window stack being at the bottom of the regular stack we
// go with the approach of allocating the alternate signal stack in a
// unique area. That way if this area is then subdivided for "alternate
// register window stack" we don't get a headachetrying to figure out
// how to fit all that into a single stack.
//
void os::Linux::install_alternate_signal_stack(OSThread* osthread) {
  stack_t stack;

  address   bottom;
  size_t    size;

  current_stack_region((address *)&bottom, &size);

  /* check if we have enough space to set up guard page and alternate */
  /* signal stack.                                                    */
  const uintptr_t safety_margin = 16 * K;
  address current_sp = os::current_stack_pointer();
  if (bottom + HOTSPOT_GUARD_SIZE + safety_margin > current_sp) {
     /* the thread is created or attached with stack size too small. */
     /* either way, we don't want to print this as a hotspot internal*/
     /* error. so don't fatal(...).                                  */
     tty->print_cr("Fatal: Stack size too small. Use 'java -Xss' to "
                   "increase default stack size.");
     os::abort(false);
  }

  /* setup alternate signal stack */
  /*
  NOTE: prior to LinuxThreads 0.9 (as shipped with RedHat 7.1),
        stack size is fixed to be 2M. Alternate signal stack
        MUST be installed within the 2M memory slot. If the
        alternate signal stack is installed outside the 2M thread
        stack region, system will hang upon signals!!
        This bug appears to have been fixed in LinuxThreads 0.9,
        we may use mmap(0,...) to allocate alternate signal
        stack in the future.
  */
  address astart = bottom + HOTSPOT_ALTSTACK_START;

  stack.ss_sp    = (void*)astart;
  stack.ss_size  = HOTSPOT_ALTSTACK_SIZE;
  stack.ss_flags = 0;

  if (!os::commit_memory((char*) astart, HOTSPOT_ALTSTACK_SIZE)) {
    warning("cannot allocate alternate signal stack"); 
    return;
  }

  if (::sigaltstack(&stack, NULL)) {
    warning("cannot install alternate signal stack");
  } else {
    osthread->set_alt_sig_stack(astart);
  }
}

void os::Linux::remove_alternate_signal_stack() {
  stack_t old_stack;

  ::sigaltstack(NULL, &old_stack);
  if ((old_stack.ss_flags & SS_DISABLE) == 0) {
    stack_t new_stack;

    new_stack.ss_sp    = NULL;
    new_stack.ss_size  = 0;
    new_stack.ss_flags = SS_DISABLE;

    if (sigaltstack(&new_stack, NULL) != 0) {
      warning("cannot uninstall alt signal stack");
    }
    // TODO: shall we munmap the guard page and altstack.
    // I am not sure the linuxthreads will do it for us.
  }
}

address os::current_stack_base() {
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return (bottom + size);
}

size_t os::current_stack_size() {
  // Note:  this includes the normal stack + guard pages, but not the alternate
  // signal stack (see the code in class JavaThread that deals with yellow and
  // red zones before changing).
  address bottom;
  size_t size;
  current_stack_region(&bottom, &size);
  return (size - HOTSPOT_ALTSTACK_SIZE);
}

bool os::Linux::manually_expand_stack(JavaThread * t, address addr) {
  assert(t!=NULL, "just checking");
  assert(t->osthread()->expanding_stack(), "expand should be set");
  assert(t->stack_base() != NULL, "stack_base and stack_size was not initialized");
  
  // Thread::stack_base() is the stack top on Linux.
  // Thread::stack_size() includes normal stack plus Hotspot guard pages
  if (addr <  t->stack_base() && 
      addr >= t->stack_base() - (t->stack_size() - HOTSPOT_GUARD_SIZE )) { 
    // The stack banging code reached the end of the stack pages that are
    // currently mapped, and the kernel gives us a SEGV instead of expanding
    // the stack because addr is 'too close' to our sp.  Expand the stack
    // manually, since now we're on the alternate signal stack. (Dangerous!!!)
    *addr = 0;
    return true;
  }
  return false;
}

// OS specific thread initialization
//
// For Itanium, we calculate and store the limits of the 
// register and memory stacks.  
//
void os::initialize_thread() {
  address mem_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // Initialize our register stack limit.  This is our
  // guard. 
  JavaThread::enable_register_stack_guard();

  // Initialize the memory stack limit
  mem_stk_limit = thread->stack_base() - thread->stack_size() +
                  ((StackShadowPages + StackYellowPages + StackRedPages) 
                  * os::vm_page_size());
 
  thread->set_memory_stack_limit( mem_stk_limit );
}


// Check to see if the current BSP is within our guard
// page area.
bool JavaThread::register_stack_overflow()  {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return false;

  reg_stk_limit = thread->stack_base() - thread->stack_size() - 
                  HOTSPOT_ALTSTACK_SIZE - (2 * os::vm_page_size()) -
                  ((StackShadowPages + StackYellowPages + StackRedPages) 
                  * os::vm_page_size());

  if ( StubRoutines::ia64::get_backing_store_pointer() > 
                           (reg_stk_limit - StackReguardSlack ) )
    return true;
  else
    return false;
}

// Set the guard page address to it's normal guarded position.
// Compiled code and interpreter entry compares the current
// BSP to this address to check for overflow.

void JavaThread::enable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");


  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the memory stack and that
  // it is located sequentially below (lower addresses than) the memory stack.  
  // The register stack doesn't have an alternate signal stack.  

  reg_stk_limit = thread->stack_base() - thread->stack_size() - 
                  HOTSPOT_ALTSTACK_SIZE - (2 * os::vm_page_size()) -
                  ((StackShadowPages + StackYellowPages + StackRedPages) * os::vm_page_size());
  
  thread->set_register_stack_limit( reg_stk_limit );
}

// Reduce the guard page by StackShadowPages to allow for the processing
// of register stack overflow exceptions.
void JavaThread::disable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  reg_stk_limit = thread->stack_base() - thread->stack_size() - 
                  HOTSPOT_ALTSTACK_SIZE - (2 * os::vm_page_size()) -
                  ((StackYellowPages + StackRedPages) * os::vm_page_size());

  thread->set_register_stack_limit( reg_stk_limit );
}


