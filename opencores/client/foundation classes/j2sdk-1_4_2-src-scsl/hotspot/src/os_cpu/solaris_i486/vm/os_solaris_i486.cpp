#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_solaris_i486.cpp	1.71 03/03/31 13:30:50 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_solaris_i486.cpp.incl"
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
# include <thread.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/filio.h>
# include <sys/utsname.h>
# include <sys/systeminfo.h>
# include <sys/socket.h>
# include <sys/lwp.h>
# include <pwd.h>
# include <poll.h>
# include <sys/lwp.h>

# define _STRUCTURED_PROC 1  //  this gets us the new structured proc interfaces of 5.6 & later
# include <sys/procfs.h>     //  see comment in <sys/procfs.h>


# ifdef PCSREG               //  5.6 or newer
# define SOLARIS_STRUCTURED_PROC
# endif

#define MAX_PATH (2 * K)

static char*  reserved_addr = 0;
static size_t reserved_increment = 0x1000000;

// Thanks, Mario Wolzcko for the code below: (Ungar 5/97)

char* os::Solaris::mmap_chunk(char *addr, size_t size, int flags, int prot) {
  char *b = mmap(addr, size, prot, flags, os::Solaris::_dev_zero_fd, 0);

  if (b == MAP_FAILED) {
    return NULL;
  }

  // QQQ TODO this could be moved back to os_solaris.cpp if high_half_mask was factored instead
  // of this entire routine and reserve_memory was also moved backed to os_solaris.cpp

  // Ensure MP-correctness when we patch instructions containing addresses.
  const int high_half_mask = -1 << 24;   // misalignment can separate high byte
  guarantee(((long)b & high_half_mask) != ((long)os::non_memory_address_word() & high_half_mask), "high half of address must not be all-zero");
  
  return b;
}
char* os::reserve_memory(size_t bytes) {
  // On Sparc, addr is not critical,
  // for Intel Solaris, though, must ensure heap is at address with
  // sentinel bit set -- assuming stack is up there, too.
  // - Ungar 11/97

  char* addr = reserved_addr + 0x40000000; // A hint to mmap

  reserved_addr = (char*)(((intptr_t)reserved_addr + bytes + reserved_increment - 1) & ~(reserved_increment - 1));

  // Map uncommitted pages PROT_NONE so we fail early if we touch an
  // uncommitted page. Otherwise, the read/write might succeed if we
  // have enough swap space to back the physical page.
  return Solaris::mmap_chunk(addr, bytes,
			     MAP_PRIVATE | MAP_NORESERVE,
                             PROT_NONE);
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).

  // On Intel, virtual addresses never have the sign bit set.
  return (char*) -1;
}

//
// Validate a ucontext retrieved from walking a uc_link of a ucontext.
// There are issues with libthread giving out uc_links for different threads
// on the same uc_link chain and bad or circular links. 
//
bool os::Solaris::valid_ucontext(Thread* thread, ucontext_t* valid, ucontext_t* suspect) {
  if (valid >= suspect || 
      valid->uc_stack.ss_flags != suspect->uc_stack.ss_flags ||
      valid->uc_stack.ss_sp    != suspect->uc_stack.ss_sp    ||
      valid->uc_stack.ss_size  != suspect->uc_stack.ss_size) {
    DEBUG_ONLY(tty->print_cr("valid_ucontext: failed test 1");)
    return false;
  }

  if (thread->is_Java_thread()) {
    if (!valid_stack_address(thread, (address)suspect)) {
      DEBUG_ONLY(tty->print_cr("valid_ucontext: uc_link not in thread stack");)
      return false;
    }
    if (!valid_stack_address(thread,  (address) suspect->uc_mcontext.gregs[UESP])) {
      DEBUG_ONLY(tty->print_cr("valid_ucontext: stackpointer not in thread stack");)
      return false;
    }
  }
  return true;
}

// For Forte Analyzer AsyncGetCallTrace profiling support -
// the thread is currently interrupted by SIGPROF
ExtendedPC os::Solaris::fetch_frame_from_ucontext(Thread* thread, ucontext_t* uc, intptr_t** ret_sp) {
  // This will be implemented for Solaris i486 when Forte supports this platform.

  Unimplemented();

  ExtendedPC new_addr;
  return new_addr;
}

#ifndef CORE
// synchronous: cancelled by the sender thread
void SetThreadPC_Callback::execute(OSThread::InterruptArguments *args) {
  assert(args->thread()->is_Java_thread(), "must be a java thread");

  JavaThread* thread = (JavaThread*)args->thread();
  ucontext_t* uc     = args->ucontext();

  // In some instances with some versions of libthread we get this callback while
  // we are executing libthread signal handling code (during preemption/resumption) 
  // and we used to blindly try update the pc with a java code pc. This seriously
  // messes up the state of the world. Now that we have made the operation synchronous
  // we can examine the pc and either find the expected pc or not. We are willing to
  // examine at most a single nest ucontext. There are libthread issues with the
  // sanity of uc_link (cycles, cross thread stacks, ...) so the less we use uc_link
  // the better. If we find the expected pc we patch it and return (to VM thread typically)
  // success, if not we return failure and the caller can retry.
  // 
  _result = false;
  if (uc->uc_mcontext.gregs[EIP] == (greg_t)_old_addr.pc() ) {
    uc->uc_mcontext.gregs[EIP] = (greg_t)_new_addr.pc();
    _result = true;
  } else if (uc->uc_link != NULL) {
    // Check (and validate) one level of stacked ucontext
    ucontext_t* linked_uc = uc->uc_link;
    if (os::Solaris::valid_ucontext(thread, uc, linked_uc) && linked_uc->uc_mcontext.gregs[EIP] == (greg_t)_old_addr.pc()) {
      linked_uc->uc_mcontext.gregs[EIP] = (greg_t)_new_addr.pc();
      _result = true;
    }
  }

  thread->safepoint_state()->notify_set_thread_pc_result(_result);

#ifdef ASSERT
  if (!_result) { 
    if (uc->uc_link != NULL) { 
      tty->print_cr("set_thread_pc:(nested) failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc()); 
    } else { 
      tty->print_cr("set_thread_pc: failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc()); 
    } 
  } 
#endif
}
#endif

// This is a simple callback that just fetches a PC for an interrupted thread.
// The thread need not be suspended and the fetched PC is just a hint.
// Returned PC and nPC are not necessarily consecutive.
// This one is currently used for profiling the VMThread ONLY!

// Must be synchronous
void GetThreadPC_Callback::execute(OSThread::InterruptArguments *args) {
  Thread*     thread = args->thread();
  ucontext_t* uc     = args->ucontext();
  jint* sp;

  assert(ProfileVM && thread->is_VM_thread(), "just checking");
    
  ExtendedPC new_addr((address)uc->uc_mcontext.gregs[EIP]);
  _addr = new_addr;
}

#ifdef SOLARIS_STRUCTURED_PROC            // Solaris 5.6 and younger

static int threadgetstate(thread_t tid, int *flags, lwpid_t *lwp, stack_t *ss, gregset_t rs, lwpstatus_t *lwpstatus) {
  char lwpstatusfile[PROCFILE_LENGTH];
  int lwpfd, err;

  if (err = os::Solaris::thr_getstate(tid, flags, lwp, ss, rs))
    return (err);
  if (*flags == TRS_LWPID) {
    sprintf(lwpstatusfile, "/proc/%d/lwp/%d/lwpstatus", getpid(),
	    *lwp);
    if ((lwpfd = open(lwpstatusfile, O_RDONLY)) < 0) {
      perror("thr_mutator_status: open lwpstatus");
      return (EINVAL);
    }
    if (pread(lwpfd, lwpstatus, sizeof (lwpstatus_t), (off_t)0) !=
	sizeof (lwpstatus_t)) {
      perror("thr_mutator_status: read lwpstatus");
      (void) close(lwpfd);
      return (EINVAL);
    }
    (void) close(lwpfd);
  }
  return (0);
}

#endif // SOLARIS_STRUCTURED_PROC

// Returns a pointer to ucontext from SIGLWP handler by walking up the stack
// NULL indicates failure
static ucontext_t* get_ucontext_from_siglwp_handler(Thread* thread, address s_pc, jint* s_sp, jint* s_fp) {

// This routine is relatively fragile.
// If we wander back into a c2 compiled frame, we can't trust our frame pointer
// any more, as the compiler can use ebp as a general register.
// As a result we try very hard to ensure that no references will cause
// an accidental segv.
//
   ucontext_t *uc = NULL;
   address stackStart = (address) thread->stack_base();
   address stackEnd   = stackStart - thread->stack_size();
   int i = 0;
   jint* s_oldfp = s_fp - 1;  // initialize to smaller value
   intptr_t page_mask = (intptr_t) ~(os::vm_page_size() - 1);

   // walk back on stack until the signal handler is found
   // only follow legitimate frame pointers. Note: this can
   // still be fooled but as long as we don't segv we are ok.
   while (1) {

     // Check for bad pc's and bad fp's. We don't need sp and since
     // all but the initial sp are simply derived from fp it is a
     // waste to check 2..n. We have also seen a problem where the
     // sp that is delivered is stale (see bug 4335248 on sparc)
     // so we just completely ignore sp.

     if ( s_pc == (address) -1 || s_oldfp >= s_fp ||
	 (intptr_t) s_fp & 0x3 || (address) s_fp > stackStart - wordSize || (address) s_fp <= stackEnd ) {
         // The real stackStart seems to be one word less that what is returned by thr_stksegment.
       return NULL;
     }

     // The siglwp handler is known to be 4-7 frames back
     if (++i > 10) {
       return NULL;
     }
     if (s_pc >= os::Solaris::handler_start && s_pc < os::Solaris::handler_end) {
       frame sighandler_frame(s_sp, s_fp, s_pc);
       return *(ucontext_t **)sighandler_frame.native_param_addr(2);
     }

     frame fr(s_sp, s_fp, s_pc);
     s_sp = fr.sender_sp();
     s_oldfp = s_fp;

     s_fp = fr.link();
     s_pc = fr.sender_pc();
  }
  ShouldNotReachHere();

  return NULL;
}

// This is a "fast" implementation of fetch_top_frame that relies on the new libthread
// APIs to get pc for a suspended thread
ExtendedPC os::Solaris::fetch_top_frame_fast(Thread* thread, jint** sp, jint** fp) {
#ifndef SOLARIS_STRUCTURED_PROC       // 5.5.1 or older
  ShouldNotReachHere();
  ExtendedPC noaddr;
  return noaddr;
#else
  int flag;
  lwpid_t lwpid;
  gregset_t rs;
  lwpstatus_t lwpstatus;
  int res;
  ExtendedPC addr;
  
  #ifdef ASSERT
  memset(&rs[0], 0xff, sizeof(gregset_t));
  assert(os::Solaris::mutator_libthread() && GetThreadState, "Must use new libthread API");
  #endif

  thread_t tid = thread->osthread()->thread_id();
  res = threadgetstate(tid, &flag, &lwpid, NULL, rs, &lwpstatus);
  // Let caller know about failure 
  // thread self-suspension uses wait on SR_lock, so
  // thread is not t_stop or t_stopallmutators
//  assert(res == 0, "threadgetstate() failure"); // guarantee
  if(res) {
    ExtendedPC new_addr(NULL);
    *sp   = (jint*)0;
    *fp   = (jint*)0;
    addr = new_addr;
    return addr;  // bail out
  }
  
  switch(flag) {
  case TRS_NONVOLATILE:   
      // called when thread voluntarily gave up control via thr_suspend().
    // Even worse, the register values that libthread returns are for the libthread's
    // own SIGLWP handler - a suspended thread sleeps on a synchronization object
    // inside this handler... 
    if (Arguments::has_profile() || UseStrictCompilerSafepoints || jvmpi::enabled()) {
	ucontext_t *uc = NULL;

	// get_ucontext_from_siglwp_handler is too dangerous to call unless thread state
	// is _thread_in_Java. It is dangerous then too but we must use it if we expect
	// to be able to reach a safepoint with a long (infinite) loop in Java code.
	// 
	if (thread->is_Java_thread()) {
	  JavaThread* jthread = (JavaThread *)thread;
	  if (jthread->thread_state() == _thread_in_Java) {
	    uc = get_ucontext_from_siglwp_handler(thread, (address)rs[EIP], (jint*)rs[UESP], (jint*)rs[EBP]);
	  }
	}
        // if we can not find the signal handler on the stack,
        // set new_addr to null, which get_top_frame will use
        // to return false, which causes examine_state_of_thread to
        // roll_forward and SafepointSynchronize::begin() will
        // retry this thread again.
        if (uc == NULL) {
            ExtendedPC new_addr(NULL);
            *sp   = (jint*)0;
            *fp   = (jint*)0;
            addr = new_addr;
        }
        else {
            ExtendedPC new_addr((address)uc->uc_mcontext.gregs[EIP]);
            addr = new_addr;
	    assert(uc->uc_mcontext.gregs[EIP] != NULL, "cannot be null");
	    *sp = (jint *)uc->uc_mcontext.gregs[UESP];
	    *fp = (jint *)uc->uc_mcontext.gregs[EBP];
        }
        break;
    }
    else
      ;// fallthrough to invalid case
  case TRS_INVALID:
    {
      ExtendedPC new_addr(NULL);
      *sp   = (jint*)0;
      *fp   = (jint*)0;
      addr = new_addr;
    }
  break;


  case TRS_VALID:         // the entire register set is cached for us by libthread
    {
      assert(rs[UESP] != NULL, "stack point shouldn't be null in TRS_VALID case");
      ExtendedPC new_addr((address)rs[EIP]);
      *sp   = (jint*)rs[UESP];
      *fp   = (jint*)rs[EBP];
      assert(rs[EIP] != NULL, "cannot be null");
      addr = new_addr;
    }
  break;

  case TRS_LWPID:        // got a full register set from the /proc interface
    // Fastlane?
    {
      assert(lwpstatus.pr_reg[UESP] != 0, "stack point shouldn't be null in TRS_LWPID case");
      ExtendedPC new_addr((address)lwpstatus.pr_reg[EIP]);
      assert(lwpstatus.pr_reg[EIP] != NULL, "cannot be null");
      *sp   = (jint*)lwpstatus.pr_reg[UESP];
      *fp   = (jint*)lwpstatus.pr_reg[EBP];
      addr = new_addr;
    }
  break;
  default:
    break;
  }
  
  return addr;

#endif   // SOLARIS_STRUCTURED_PROC
}

extern "C" void findpc(int x);
extern void set_error_reported();

extern "C" int JVM_handle_solaris_signal(int signo, siginfo_t* siginfo, void* ucontext, int abort_if_unrecognized);

int JVM_handle_solaris_signal(int sig, siginfo_t* info, void* ucVoid, int abort_if_unrecognized) {
  ucontext_t* uc = (ucontext_t*) ucVoid;

  JavaThread* thread = NULL;
  VMThread* vmthread = NULL;
  if (os::Solaris::signal_handlers_are_installed) {
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

  guarantee(sig != os::Solaris::SIGinterrupt(), "Can not chain VM interrupt signal, try -Xusealtsigs");

  if (sig == os::Solaris::SIGasync()) {
    if(thread){
      OSThread::InterruptArguments args(thread, uc);
      thread->osthread()->do_interrupt_callbacks_at_interrupt(&args);
      return true; 
    } 
    else if(vmthread){
      OSThread::InterruptArguments args(vmthread, uc);
      vmthread->osthread()->do_interrupt_callbacks_at_interrupt(&args);
      return true;
    }
  }

  if (info == NULL || info->si_code <= 0 || info->si_code == SI_NOINFO) {
    // can't decode this kind of signal
    info = NULL;
  } else {
    assert(sig == info->si_signo, "bad siginfo");
  }

  // decide if this trap can be handled by a stub
  address stub = NULL;

  address pc          = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.gregs[EIP];
    #ifndef CORE
    adjusted_pc = thread->safepoint_state()->compute_adjusted_pc(pc);
    #else
    adjusted_pc = pc;
    #endif

    // Handle ALL stack overflow variations here
    if (sig == SIGSEGV && info->si_code == SEGV_ACCERR) {
      address addr = (address) info->si_addr;
      if (thread->in_stack_yellow_zone(addr)) {
	thread->disable_stack_yellow_zone();
	if (thread->thread_state() == _thread_in_Java) {
	  // Throw a stack overflow exception.  Guard pages will be reenabled
	  // while unwinding the stack.
	  stub = StubRoutines::i486::handler_for_stack_overflow();
	} else {
	  // Thread was in the vm or native code.  Return and try to finish.
	  return true;
	}
      } else if (thread->in_stack_red_zone(addr)) {
	// Fatal red zone violation.  Disable the guard pages and fall through
	// to handle_unexpected_exception way down below.
	thread->disable_stack_red_zone();
	tty->print_cr("An irrecoverable stack overflow has occurred.");
      }
    }

    if (thread->thread_state() == _thread_in_vm) {
      if (sig == SIGBUS && info->si_code == BUS_OBJERR && thread->doing_unsafe_access()) {
	stub = StubRoutines::i486::handler_for_unsafe_access();
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

	    // if(type == relocInfo::return_type) {
	    // The retl case: restore has already happened. The safepoint blob frame will appear 
	    // immediately below caller of the orignal method that we have patched. The stack could 
	    // be walked properly (the frame of the patched method is already popped). 
	    // Sparc: Will need to be revisited if and when we put restore in the delay slot of ret.
	    // }
	    // else {
	    // This is the case when either a call or a branch was patched with an illegal instruction.
	    // At this point we are done with the patched method and would like things to
	    // appear as if the orignal method simply called the safepoint blob at a "safe point".
	    // Sparc: To achieve this, we set up the correct linkage by placing the adjusted trap pc in O7
	    // and then "returning" from this trap handler to the safepoint blob.
            // Intel: to achieve this the handle_illegal_instruction_exception code 
            // called by the stub will place saved_exception_pc into %sp and return there

	    // }
	}
      }
      else if (sig == SIGFPE && info->si_code == FPE_INTDIV) {
#else
      if (sig == SIGFPE && info->si_code == FPE_INTDIV) {
#endif	// ! CORE
// TODO -- needs work - fastlane
// is_zombie needs factoring?
	// integer divide by zero
// TODO -- needs work - fastlane
        stub = StubRoutines::i486::handler_for_divide_by_zero();
      }
      else if (sig == SIGFPE && info->si_code == FPE_FLTDIV) {
	// floating-point divide by zero
// TODO -- needs work - fastlane
        stub = StubRoutines::i486::handler_for_divide_by_zero();
      }
      else if (sig == SIGFPE && info->si_code == FPE_FLTINV) {
	// The encoding of D2I in i486.ad can cause an exception prior
	// to the fist instruction if there was an invalid operation 
	// pending. We want to dismiss that exception. From the win_32
	// side it also seems that if it really was the fist causing
	// the exception that we do the d2i by hand with different
	// rounding. Seems kind of weird. QQQ TODO
	// Note that we take the exception at the NEXT floating point instruction.
	if (pc[0] == 0xDB) {
	    assert(pc[0] == 0xDB, "not a FIST opcode");
	    assert(pc[1] == 0x14, "not a FIST opcode");
	    assert(pc[2] == 0x24, "not a FIST opcode");
	    return true;
	} else {
	    assert(pc[-3] == 0xDB, "not an flt invalid opcode");
	    assert(pc[-2] == 0x14, "not an flt invalid opcode");
	    assert(pc[-1] == 0x24, "not an flt invalid opcode");
	}
      }
      else if (sig == SIGFPE ) {
        tty->print_cr("caught SIGFPE, info 0x%x.", info->si_code);
      }
	// QQQ It doesn't seem that we need to do this on x86 because we should be able
	// to return properly from the handler without this extra stuff on the back side.

#ifdef COMPILER2
      else if (sig == SIGSEGV && OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
	// This is the case when we use a trap to restore the context at the end of safepoint
	// taken in compiled code. The "return" address is passed in O7
	// QQQ TODO
        ShouldNotReachHere();

      }
#endif	// COMPILER2

      else if (sig == SIGSEGV && info->si_code > 0 && !MacroAssembler::needs_explicit_null_check((int)info->si_addr)) {
	// an access to the first page of VM--assume it is a null pointer
        stub = StubRoutines::i486::handler_for_null_exception();
      }

#ifndef CORE
      else if (sig == SIGBUS && info->si_code == BUS_OBJERR) {
	// BugId 4454115: A read from a MappedByteBuffer can fault
	// here if the underlying file has been truncated.
	// Do not crash the VM in such a case.
        CodeBlob* cb = CodeCache::find_blob_unsafe(adjusted_pc);
        nmethod* nm = cb->is_nmethod() ? (nmethod*)cb : NULL;
        if (nm != NULL && nm->has_unsafe_access()) {
	  stub = StubRoutines::i486::handler_for_unsafe_access();
	}
      }
#endif //CORE
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it

    thread->set_saved_exception_pc(pc);
    // 12/02/99: On Sparc it appears that the full context is alsosaved
    // but as yet, no one looks at or restores that saved context
    // factor me: setPC
    uc->uc_mcontext.gregs[EIP] = (greg_t)stub;
    return true;
  }

  // signal-chaining
  if (UseSignalChaining) {
    bool chained = false;
    struct sigaction *actp = os::Solaris::get_chained_signal_action(sig);
    if (actp != NULL) {
      chained = os::Solaris::chained_handler(actp, sig, info, ucVoid);
    }
    if (chained) {
      // signal-chaining in effect. Continue.
      return true;
    }
  }


// If os::Solaris::SIGasync not chained, and this is a non-vm and non-java thread
  if (sig == os::Solaris::SIGasync()) {
    return true;
  }

  if(sig == SIGPIPE) {
    if (PrintMiscellaneous && (WizardMode || Verbose)) {
      warning("Ignoring SIGPIPE - see bug 4229104");
    }
    return true;
  }

  if (!abort_if_unrecognized) {
    // caller wants another chance, so give it to him
    return false;
  }

  if (!os::Solaris::libjsig_is_loaded) {
    struct sigaction oldAct;
    sigaction(sig, (struct sigaction *)0, &oldAct);
    if (oldAct.sa_sigaction != signalHandler) {
      void* sighand = oldAct.sa_sigaction ? CAST_FROM_FN_PTR(void*,  oldAct.sa_sigaction)
					  : CAST_FROM_FN_PTR(void*, oldAct.sa_handler);
      warning("Unexpected Signal %d occured under user-defined signal handler %#lx", sig, (long)sighand);
    }
  }

  if (pc == NULL && uc != NULL) {
    pc = (address) uc->uc_mcontext.gregs[EIP];
  }

#ifndef PRODUCT
  // tell the system that it is about to die
  set_error_reported();

  // Remove this handler now.  It is of no more use.
  os::Solaris::set_signal_handler(sig, false, true);

  if (WizardMode) {
    if (pc != NULL) {
       printf("\n");
       printf("PC information:\n");
       findpc((int)pc);
    }
    if (info != NULL) {
       printf("\n");
       printf("siginfo_t dump:\n");
       for (int i = 0; i < sizeof(info[0])/sizeof(int); i++) {
	 printf(" 0x%08X", ((int*)info)[i]);
	 if ((i + 1) % 4 == 0)  printf("\n");
       }
       printf("\n");
    }
    if (uc != NULL) {
       printf("\n");
       printf("ucontext_t dump:\n");
       for (int i = 0; i < sizeof(uc[0])/sizeof(int); i++) {
	 printf(" 0x%08X", ((int*)uc)[i]);
	 if ((i + 1) % 4 == 0)  printf("\n");
       }
       printf("\n");

       tty->print_cr("Unexpected exception occured: signal=%d, pc=%lx", sig, (long)pc);

       // QQQ will we need a stub to bridge into continue_with_dump so that a reasonable
       // return address will be present and the stack looks ok.
       //
       uc->uc_mcontext.gregs[EIP ] = (greg_t)&continue_with_dump;
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

void os::Solaris::init_thread_fpu_state(void) {
  // Hardcoded to set fpu to 53 bit precision. This happens too early
  // to use a stub.
  //
  // pushl  $0x27f
  // fldcw  0(%esp)
  // popl   %eax
  // ret
  // nop ...

  static int code_template[] = {
    0x00027f68, 0x246cd900, 0x90c35800, 0x90909090
  };
  static void (*fixcw)(void) = CAST_TO_FN_PTR(void (*)(void), code_template);

  fixcw();
}

// JVMPI code
bool os::thread_is_running(JavaThread* tp) {
  int         flag;
  lwpid_t     lwpid;
  gregset_t   reg;
  lwpstatus_t lwpstatus;
  int         res;
  thread_t    tid = tp->osthread()->thread_id();
  res = threadgetstate(tid, &flag, &lwpid, NULL, reg, &lwpstatus);
  assert(res == 0, "threadgetstate() failure");
  if(res != 0) return false; // Safe return value

  unsigned int sum = 0;
  // give TRS_NONVOLATILE doesn't trust any other registers, just use these
  sum += reg[UESP];  sum += reg[EIP];   sum += reg[EBP];

  if (tp->last_sum() == sum) {
    return false;
  } else {
    tp->set_last_sum(sum);
    return true;
  }
}

// these routines are the initial value of atomic_exchange_entry(),
// atomic_compare_and_exchange_entry(), atomic_increment_entry()
// and atomic_membar_entry()
// until initialization is complete (see stubRoutines_<arch>.cpp).
// TODO - factor (or see win32 inline model?)

typedef jint exchange_func_t            (jint, jint*);
typedef jint compare_and_exchange_func_t(jint, jint*, jint);
typedef jlong compare_and_exchange_long_func_t(jlong, jlong*, jlong);
typedef jint increment_func_t           (jint, jint*);
typedef void membar_func_t              ();

jint os::atomic_exchange_bootstrap(jint exchange_value, jint* dest) {
  // try to use the stub:
  exchange_func_t* func = CAST_TO_FN_PTR(exchange_func_t*, StubRoutines::atomic_exchange_entry());

  if (func != NULL) {
    os::atomic_exchange_func = func;
    return (*func)(exchange_value, dest);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  *dest = exchange_value;
  return old_value;
}

jint os::atomic_compare_and_exchange_bootstrap(jint exchange_value, jint* dest, jint compare_value) {
  // try to use the stub:

  compare_and_exchange_func_t* func = CAST_TO_FN_PTR(compare_and_exchange_func_t*,
						       StubRoutines::atomic_compare_and_exchange_entry());

  if (func != NULL) {
    os::atomic_compare_and_exchange_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jint old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jlong os::atomic_compare_and_exchange_long_bootstrap(jlong exchange_value, jlong* dest, jlong compare_value) {
  // try to use the stub:

  compare_and_exchange_long_func_t* func = CAST_TO_FN_PTR(compare_and_exchange_long_func_t*,
						       StubRoutines::atomic_compare_and_exchange_long_entry());

  if (func != NULL) {
    os::atomic_compare_and_exchange_long_func = func;
    return (*func)(exchange_value, dest, compare_value);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  jlong old_value = *dest;
  if (old_value == compare_value)
    *dest = exchange_value;
  return old_value;
}

jint os::atomic_increment_bootstrap(jint inc, jint* loc) {
  // try to use the stub:
  increment_func_t* func = CAST_TO_FN_PTR(increment_func_t*, StubRoutines::atomic_increment_entry());

  if (func != NULL) {
    os::atomic_increment_func = func;
    return (*func)(inc, loc);
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  return (*loc) += inc;
}

void os::atomic_membar_bootstrap() {
  // try to use the stub:
  membar_func_t* func = CAST_TO_FN_PTR(membar_func_t*, StubRoutines::atomic_membar_entry());

  if (func != NULL) {
    os::atomic_membar_func = func;
    (*func)();
  }
  assert(Threads::number_of_threads() == 0, "for bootstrap only");

  // don't have to do anything for a single thread
}


exchange_func_t* os::atomic_exchange_func
  = os::atomic_exchange_bootstrap;
compare_and_exchange_func_t* os::atomic_compare_and_exchange_func
  = os::atomic_compare_and_exchange_bootstrap;
compare_and_exchange_long_func_t* os::atomic_compare_and_exchange_long_func
  = os::atomic_compare_and_exchange_long_bootstrap;
increment_func_t* os::atomic_increment_func
  = os::atomic_increment_bootstrap;
membar_func_t* os::atomic_membar_func
  = os::atomic_membar_bootstrap;


// This is split from os_solaris.cpp because of platform issues but is related to os_win32.cpp
//Reconciliation History
// 1.287 99/04/01 18:31:08 os_win32.cpp
// 1.288 99/04/12 12:56:48 os_win32.cpp
// 1.293 99/07/06 16:36:49 os_win32.cpp
// 1.294 99/07/13 11:41:18 os_win32.cpp
//End
