#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_solaris_sparc.cpp	1.72 03/03/31 13:30:54 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file

# include <signal.h>        // needed first to avoid name collision for "std" with SC 5.0

# include "incls/_os_solaris_sparc.cpp.incl"

// put OS-includes here
# include <sys/types.h>
# include <sys/mman.h>
# include <pthread.h>
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

extern address JVM_FunctionAtEnd();

# define _STRUCTURED_PROC 1  //  this gets us the new structured proc interfaces of 5.6 & later
# include <sys/procfs.h>     //  see comment in <sys/procfs.h>

# ifdef PCSREG               //  5.6 or newer
# define SOLARIS_STRUCTURED_PROC
# endif

#define MAX_PATH (2 * K)

int os::Solaris::max_register_window_saves_before_flushing() {
  // We should detect this at run time. For now, filling
  // in with a constant.
  return 8;
}

static void handle_unflushed_register_windows(gwindows_t *win) {
  int restore_count = win->wbcnt;
  int i;

  for(i=0; i<restore_count; i++) {
    address sp = ((address)win->spbuf[i]) + STACK_BIAS;
    address reg_win = (address)&win->wbuf[i];
    memcpy(sp,reg_win,sizeof(struct rwindow));
  }
}


// Thanks, Mario Wolzcko for the code below: (Ungar 5/97)

char* os::Solaris::mmap_chunk(char *addr, size_t size, int flags, int prot) {
  char *b = mmap(addr, size, prot, flags, os::Solaris::_dev_zero_fd, 0);

  if (b == MAP_FAILED) {
    return NULL;
  }

  // QQQ TODO this could be moved back to os_solaris.cpp if high_half_mask was factored instead

  // of this entire routine

  // Ensure MP-correctness when we patch instructions containing addresses.
  const intptr_t high_half_mask = -1 << 10;   // sethi has a 22-bit immediate
  guarantee(((intptr_t)b & high_half_mask) != ((intptr_t)os::non_memory_address_word() & high_half_mask), "high half of address must not be all-zero");
  
  return b;
}


static char*  reserved_addr = 0;
static size_t reserved_increment = 0x1000000;

char* os::reserve_memory(size_t bytes) {
  // On Sparc, addr is not critical,
  // for Intel Solaris, though, must ensure heap is at address with
  // sentinel bit set -- assuming stack is up there, too.
  // - Ungar 11/97
  char * addr;

#ifdef _LP64
  // Lets' try to get the heap near the libjvm so we can use
  // the call instruction to call the runtime.
  if ( reserved_addr == 0 )
      addr = (char *)((uintptr_t)JVM_FunctionAtEnd() - (uintptr_t)0x80000000L);
  else
      addr = reserved_addr ;

  reserved_addr = (char*)((uintptr_t)addr + bytes );
#else
  addr = reserved_addr + 0x40000000; // A hint to mmap
  reserved_addr = (char*)(((intptr_t)reserved_addr + bytes + reserved_increment - 1) & ~(reserved_increment - 1));
#endif

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
  // On SPARC, 0 != %hi(any real address), because there is no
  // allocation in the first 1Kb of the virtual address space.
  return (char*) 0;
}

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
    address _sp   = (address)((intptr_t)suspect->uc_mcontext.gregs[REG_SP] + STACK_BIAS);
    if (!valid_stack_address(thread, _sp) ||
	!frame::is_valid_stack_pointer(((JavaThread*)thread)->base_of_stack_pointer(), NULL, (intptr_t*)_sp)) {
      DEBUG_ONLY(tty->print_cr("valid_ucontext: stackpointer not in thread stack");)
      return false;
    }
  }
  return true;
}

// For Forte Analyzer AsyncGetCallTrace profiling support - thread is currently interrupted by SIGPROF
ExtendedPC os::Solaris::fetch_frame_from_ucontext(Thread* thread, ucontext_t* uc, intptr_t** ret_sp) {
  // Some versions of libthread will allow us to interrupt the libthread signal
  // handling. We never want to fetch a frame from libthread code so we always
  // look to see if we can find the java context (caller will validate if need
  // be anyway). We will only follow one level of uc_link
  // since there are libthread issues with ucontext linking and it is better
  // to be safe and just let caller retry later.

  assert(thread != NULL, "just checking");
  assert(ret_sp != NULL, "just checking");

  // Sometimes the register windows are not properly flushed.
  if (uc != NULL && uc->uc_mcontext.gwins != NULL) {
    ::handle_unflushed_register_windows(uc->uc_mcontext.gwins);
  }

  if (uc == NULL || (uc->uc_link != NULL && !os::Solaris::valid_ucontext(thread, uc, uc->uc_link))) {
    ExtendedPC new_addr(NULL, NULL);
    *ret_sp   = NULL;
    return new_addr;
  } else {
    if (uc->uc_link != NULL) uc = uc->uc_link;

    // Validate the ucontext after traversing one level down the uc->link chain
    if (uc == NULL || (uc->uc_link != NULL && !os::Solaris::valid_ucontext(thread, uc, uc->uc_link))) {
      // we have received a corrupted ucontext from the signal handler -- bail
      ExtendedPC new_addr(NULL, NULL);
      *ret_sp   = NULL;
      return new_addr;
    } else {
      ExtendedPC new_addr((address)uc->uc_mcontext.gregs[REG_PC],
			  (address)uc->uc_mcontext.gregs[REG_nPC]);
      *ret_sp   = (intptr_t*)((intptr_t)uc->uc_mcontext.gregs[REG_SP] + STACK_BIAS);
      return new_addr;
    }
  }
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
  if ( uc->uc_mcontext.gregs[REG_PC ] == (greg_t)_old_addr.pc() && 
       uc->uc_mcontext.gregs[REG_nPC] == (greg_t)_old_addr.npc()) {
    uc->uc_mcontext.gregs[REG_PC ] = (greg_t)_new_addr.pc();
    uc->uc_mcontext.gregs[REG_nPC] = (greg_t)_new_addr.npc();
    _result = true;
  } else if (uc->uc_link != NULL) {
    // Check (and validate) one level of stacked ucontext
    ucontext_t* linked_uc = uc->uc_link;
    if (os::Solaris::valid_ucontext(thread, uc, linked_uc) && 
       linked_uc->uc_mcontext.gregs[REG_PC] == (greg_t)_old_addr.pc() &&
       linked_uc->uc_mcontext.gregs[REG_nPC] == (greg_t)_old_addr.npc()) {
      linked_uc->uc_mcontext.gregs[REG_PC ] = (greg_t)_new_addr.pc();
      linked_uc->uc_mcontext.gregs[REG_nPC] = (greg_t)_new_addr.npc();
      _result = true;
    }
  }

  thread->safepoint_state()->notify_set_thread_pc_result(_result);

#ifdef ASSERT
  if (!_result) {
    if (uc->uc_link != NULL) {
      tty->print_cr("set_thread_pc: (nested) failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc());
    } else {
      tty->print_cr("set_thread_pc: failed to set pc " INTPTR_FORMAT " -> " INTPTR_FORMAT, _old_addr.pc(), _new_addr.pc());
    }
  }
#endif
}
#endif

void GetThreadPC_Callback::execute(OSThread::InterruptArguments *args) {
  Thread*     thread = args->thread();
  ucontext_t* uc     = args->ucontext();
  intptr_t* sp;

  assert(ProfileVM && thread->is_VM_thread(), "just checking");
    
  // Skip the mcontext corruption verification. If if occasionally
  // things get corrupt, it is ok for profiling - we will just get an unresolved 
  // function name
  ExtendedPC new_addr((address)uc->uc_mcontext.gregs[REG_PC],
		      (address)uc->uc_mcontext.gregs[REG_nPC]);
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
static ucontext_t*
get_ucontext_from_siglwp_handler(intptr_t* s_sp, Thread* thread) {
  ucontext_t *uc = NULL;

  address  s_pc        = os::Solaris::handler_end;  // outside the needed range
  intptr_t *younger_sp = NULL;
  intptr_t *orig_sp    = s_sp;
  address  stackStart  = (address)thread->stack_base();
  address  stackEnd    = (address)(stackStart - (address)thread->stack_size());
  int      i           = 0;
  
  // walk back on stack until the signal handler is found
  while (1) {
    // This part has been split from the workaround below to ease
    // debugging.
    if (++ i > 10)
      return NULL;
    // Temporary workaround for libthread bug:
    // 4335248 thr_getstate returns bad sp for flag TRS_NONVOLATILE
    // If we start with a bogus (usually somewhat stale) SP we may soon
    // go astray; so use a few sanity checks to detect and dodge such
    // situations:
    if (s_sp == NULL || s_pc == NULL ||
        (UseThrGetStateWorkaround &&
         ((intptr_t)s_sp & 0x7 != 0 || (intptr_t)s_pc & 0x3 != 0 ||
          (address)s_sp > stackStart - wordSize || (address)s_sp < stackEnd)) ||
          // The real stackStart seems to be one word less that what is returned by thr_stksegment.
          (younger_sp && s_sp <= younger_sp)) {
      return NULL;
    }

    if (s_pc >= os::Solaris::handler_start &&
        s_pc <  os::Solaris::handler_end) {
      frame sighandler_frame(s_sp, younger_sp);
      uc = *(ucontext_t **)sighandler_frame.param_addr(2, true);
      // Sometimes the register windows are not properly flushed.
      if(uc && uc->uc_mcontext.gwins != NULL)
        ::handle_unflushed_register_windows(uc->uc_mcontext.gwins);
      return uc;
    }
    younger_sp = s_sp;
    frame fr(s_sp, frame::unpatchable, s_pc);
    s_sp = fr.sender_sp();
    s_pc = fr.sender_pc();
  }
  ShouldNotReachHere();
}


// This is a "fast" implementation of fetch_top_frame that relies on the new libthread
// APIs to get pc for a suspended thread
ExtendedPC os::Solaris::fetch_top_frame_fast(Thread* thread, intptr_t** ret_younger_sp, intptr_t** ret_sp) {
#ifndef SOLARIS_STRUCTURED_PROC       // 5.5.1 or older
  ShouldNotReachHere();
  ExtendedPC noaddr;
  return noaddr;
#else
  intptr_t* sp = NULL;
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
    ExtendedPC new_addr(NULL, NULL);
    sp   = (intptr_t*)0;
    addr = new_addr;
    *ret_younger_sp = NULL;
    *ret_sp = sp;
    return addr;  // bail out
  }
  
  switch(flag) {
  case TRS_NONVOLATILE:   
    // Even worse, the register values that libthread returns are for the libthread's
    // own SIGLWP handler - a suspended thread sleeps on a synchronization object
    // inside this handler... 
    if (Arguments::has_profile() || UseStrictCompilerSafepoints || jvmpi::enabled()) {
      ucontext_t *uc = NULL;
     
      assert(rs[REG_SP] != NULL, "Invalid REG_SP in fetch_top_frame_fast");
      uc = get_ucontext_from_siglwp_handler((intptr_t*)((intptr_t)rs[REG_SP] + STACK_BIAS), thread);

      if (uc == NULL) {
        // this case has been added to handle a downstream "register windows not flushed" assert that
        // occurs in some cases when a thread enters the VM 

        sp   = (intptr_t*)((intptr_t)rs[REG_SP] + STACK_BIAS);
        if ((unsigned char*)sp < thread->stack_base() &&
	    (unsigned char*)sp >= thread->stack_base() - thread->stack_size()) {
          ExtendedPC new_addr((address)rs[REG_PC], (address)rs[REG_nPC]);
          addr = new_addr;
        }
        else {
	  // SP not within the thread's range
          // Refer to bug 4450877 for information related to this code.  It is not expected to
          // get a stack pointer outside the range of the thread's stack.  If it happens, it could either
          // be indicative of either a libthread bug or that more work should be done on this bug.
          DEBUG_ONLY(fatal1("thr_getstate reported SP=" INTPTR_FORMAT ", which is not within the stack range of the thread", sp - STACK_BIAS));
          ExtendedPC new_addr(NULL, NULL);
          sp   = (intptr_t*)0;
          addr = new_addr;
        }
      }
      else {
	address npc;
	if(UseStrictCompilerSafepoints) {
	  npc = (address) uc->uc_mcontext.gregs[REG_nPC];
	}
	else {
	  // set npc to zero to avoid using it for safepoint (good for profiling only)
	  npc = (address) 0;
	}
        ExtendedPC new_addr((address)uc->uc_mcontext.gregs[REG_PC], npc);
        addr = new_addr;
	sp = (intptr_t *)((intptr_t)uc->uc_mcontext.gregs[REG_SP] + STACK_BIAS);
      }
      break;
    }
    else
      ;// fallthrough to invalid case
  case TRS_INVALID:
    {
      ExtendedPC new_addr(NULL, NULL);
      sp   = (intptr_t*)0;
      addr = new_addr;
    }
  break;
  case TRS_VALID:         // the entire register set is cached for us by libthread
    {
      // TODO -- needs work - fastlane
      assert(rs[REG_SP]!= NULL, "stack point shouldn't be null in TRS_VALID case");
      ExtendedPC new_addr((address)rs[REG_PC], (address)rs[REG_nPC]);
      sp   = (intptr_t*)((intptr_t)rs[REG_SP] + STACK_BIAS);
      addr = new_addr;
    }
  break;
  case TRS_LWPID:        // got a full register set from the /proc interface
    // Fastlane?
    {
      assert(lwpstatus.pr_reg[R_SP] != 0, "stack point shouldn't be null in TRS_LWPID case");
      ExtendedPC new_addr((address)lwpstatus.pr_reg[R_PC], (address)lwpstatus.pr_reg[R_nPC]);
      sp   = (intptr_t*)((intptr_t)lwpstatus.pr_reg[R_SP] + STACK_BIAS);
      addr = new_addr;
    }
  break;
  default:
    break;
  }
  
  *ret_younger_sp = NULL; 
  *ret_sp = sp;

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
    if (thread) {
      OSThread::InterruptArguments args(thread, uc);
      thread->osthread()->do_interrupt_callbacks_at_interrupt(&args);
      return true;
    } else if (vmthread) {
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
  address npc         = NULL;
  address adjusted_pc = NULL;

  //%note os_trap_1
  if (info != NULL && thread != NULL) {
    // factor me: getPCfromContext
    pc = (address) uc->uc_mcontext.gregs[REG_PC];
    npc = (address) uc->uc_mcontext.gregs[REG_nPC];
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
	// Sometimes the register windows are not properly flushed.
	if(uc->uc_mcontext.gwins != NULL) {
	  ::handle_unflushed_register_windows(uc->uc_mcontext.gwins);
	}
	if (thread->thread_state() == _thread_in_Java) {
	  // Throw a stack overflow exception.  Guard pages will be reenabled
	  // while unwinding the stack.
	  stub = StubRoutines::sparc::handler_for_stack_overflow_entry();
	} else {
	  // Thread was in the vm or native code.  Return and try to finish.
	  return true;
	}
      } else if (thread->in_stack_red_zone(addr)) {
	// Fatal red zone violation.  Disable the guard pages and fall through
	// to handle_unexpected_exception way down below.
	thread->disable_stack_red_zone();
	tty->print_cr("An irrecoverable stack overflow has occurred.");
	// Sometimes the register windows are not properly flushed.
	if(uc->uc_mcontext.gwins != NULL) {
	  ::handle_unflushed_register_windows(uc->uc_mcontext.gwins);
	}
      }
    }

    if (thread->thread_state() == _thread_in_vm) {
      if (sig == SIGBUS && info->si_code == BUS_OBJERR && thread->doing_unsafe_access()) {
	stub = StubRoutines::sparc::handler_for_unsafe_access_entry();
      }
    }

    if (thread->thread_state() == _thread_in_Java) {
      // Java thread running in Java code => find exception handler if any
      // a fault inside compiled code, the interpreter, or a stub
#ifndef CORE
      if (sig == SIGILL && nativeInstruction_at(pc)->is_illegal()) {
	// debugging trap, or a safepoint
	COMPILER1_ONLY(stub = Runtime1::entry_for(Runtime1::illegal_instruction_handler_id);)
	COMPILER2_ONLY(stub = OptoRuntime::illegal_exception_handler_blob()->instructions_begin();)

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
	    // At this point we are done with the patched method.
	    // The appropriate handle_illegal_instruction_exception method
	    // will compute the real continuation point, and patch it
	    // into the frame.  No more action is required here.
	    #ifdef ASSERT
	    // The outgoing O7 will be used to link back to the real method.
	    // Make sure we do not depend on its value henceforth.
	    uc->uc_mcontext.gregs[REG_O7] = (greg_t) 0xbaadc0de;
	    #endif
	  }
	}
      }
#endif	// ! CORE

      // Not needed on x86 solaris because verify_oops doesn't generate
      // SEGV/BUS like sparc does.
      if ( (sig == SIGSEGV || sig == SIGBUS)
	   && pc >= MacroAssembler::_verify_oop_implicit_branch[0]
	   && pc <  MacroAssembler::_verify_oop_implicit_branch[1] ) {
	stub     =  MacroAssembler::_verify_oop_implicit_branch[2];
	warning("fixed up memory fault in +VerifyOops at address " INTPTR_FORMAT, info->si_addr);
      }

#ifndef CORE
      address implicit_null_handler;
      // This is not factored because on x86 solaris the patching for
      // zombies does not generate a SEGV.
      if (sig == SIGSEGV && nativeInstruction_at(pc)->is_zombie()) {
        // zombie method (ld [%g0],%o7 instruction)
        // "handle wrong method stub not implemented"
#ifdef COMPILER1
        // nmethod* nm = CodeCache::find_nmethod(adjusted_pc);
        // handle a problem in C++ code
        CodeBlob* cb = CodeCache::find_blob_unsafe(adjusted_pc);
        nmethod* nm = cb->is_nmethod() ? (nmethod*)cb : NULL;
        assert(nm != NULL, "no nmethod");
        // Note: we cannot ask method()->is_static() here because nmethod::_method is
        // set to NULL when nmethod is marked as unloaded.
        stub = nm->entry_point() == nm->verified_entry_point() ?
          Runtime1::entry_for(Runtime1::handle_wrong_static_method_id) :
          Runtime1::entry_for(Runtime1::handle_wrong_method_id);
#endif	// COMPILER1
	COMPILER2_ONLY(stub = OptoRuntime::handle_wrong_method_stub();)
        // At the stub it needs to look like a call from the caller of this
        // method (not a call from the segv site).
        pc = (address)uc->uc_mcontext.gregs[REG_O7];
      }   
#endif	// ! CORE

      else if (sig == SIGFPE && info->si_code == FPE_INTDIV) {
	// integer divide by zero
	stub = StubRoutines::sparc::handler_for_divide_by_zero_entry();
      }
      else if (sig == SIGFPE && info->si_code == FPE_FLTDIV) {
	// floating-point divide by zero
	stub = StubRoutines::sparc::handler_for_divide_by_zero_entry();
      }
#ifdef COMPILER2
      else if (sig == SIGSEGV && OptoRuntime::illegal_exception_handler_blob() != NULL &&
               OptoRuntime::illegal_exception_handler_blob()->contains(pc)) {
	ShouldNotReachHere();
      } else if (sig == SIGILL && nativeInstruction_at(pc)->is_ic_miss_trap()) {
	// Inline cache missed and user trap "Tne G0+ST_RESERVED_FOR_USER_0+2" taken.
	stub = OptoRuntime::handle_ic_miss_stub();
        // At the stub it needs to look like a call from the caller of this
        // method (not a call from the segv site).
        pc = (address)uc->uc_mcontext.gregs[REG_O7];
      }
#endif	// COMPILER2

      else if (sig == SIGSEGV && info->si_code > 0 && !MacroAssembler::needs_explicit_null_check((intptr_t)info->si_addr)) {
	// an access to the first page of VM--assume it is a null pointer
	stub = StubRoutines::sparc::handler_for_null_exception_entry();
#ifdef COMPILER2
	if (OptoRuntime::uncommon_trap_blob() != NULL && OptoRuntime::uncommon_trap_blob()->contains(pc)) {
	  ShouldNotReachHere();
	}
#endif  // COMPILER2
      }

#ifndef CORE
      else if (sig == SIGBUS && info->si_code == BUS_OBJERR) {
	// BugId 4454115: A read from a MappedByteBuffer can fault
	// here if the underlying file has been truncated.
	// Do not crash the VM in such a case.
        CodeBlob* cb = CodeCache::find_blob_unsafe(adjusted_pc);
        nmethod* nm = cb->is_nmethod() ? (nmethod*)cb : NULL;
        if (nm != NULL && nm->has_unsafe_access()) {
	  stub = StubRoutines::sparc::handler_for_unsafe_access_entry();
	}
      }
#endif //CORE
    }
  }

  if (stub != NULL) {
    // save all thread context in case we need to restore it

    thread->set_saved_exception_pc(pc);
    thread->set_saved_exception_npc(npc);

    // simulate a branch to the stub (a "call" in the safepoint stub case)
    // factor me: setPC
    uc->uc_mcontext.gregs[REG_PC ] = (greg_t)stub;
    uc->uc_mcontext.gregs[REG_nPC] = (greg_t)(stub + 4);

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

  // Now handle signals that are not chained

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
      void* sighand = oldAct.sa_sigaction ? CAST_FROM_FN_PTR(void*, oldAct.sa_sigaction)
					  : CAST_FROM_FN_PTR(void*, oldAct.sa_handler);
      warning("Unexpected Signal %d occured under user-defined signal handler " INTPTR_FORMAT, sig, (intptr_t)sighand);
    }
  }

  if (pc == NULL && uc != NULL) {
    pc = (address) uc->uc_mcontext.gregs[REG_PC];
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
       findpc((intptr_t)pc);
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

       tty->print_cr("Unexpected exception occured: signal=%d, pc=" INTPTR_FORMAT, sig, (intptr_t)pc);

       uc->uc_mcontext.gregs[REG_PC ] = (greg_t)&continue_with_dump;
       uc->uc_mcontext.gregs[REG_nPC] = uc->uc_mcontext.gregs[REG_PC] + 4;
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
    // Nothing needed on Sparc.
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

  uintptr_t sum = 0;
  sum += reg[R_SP];  sum += reg[R_PC];

  sum += reg[R_G1];  sum += reg[R_G2];  sum += reg[R_G3];  sum += reg[R_G4];
    
  sum += reg[R_O0];  sum += reg[R_O1];  sum += reg[R_O2];  sum += reg[R_O3];
  sum += reg[R_O4];  sum += reg[R_O5];

  sum += reg[R_I0];  sum += reg[R_I1];  sum += reg[R_I2];  sum += reg[R_I3];
  sum += reg[R_I4];  sum += reg[R_I5];  sum += reg[R_I6];  sum += reg[R_I7];

  sum += reg[R_L0];  sum += reg[R_L1];  sum += reg[R_L2];  sum += reg[R_L3];
  sum += reg[R_L4];  sum += reg[R_L5];  sum += reg[R_L6];  sum += reg[R_L7];

  if (tp->last_sum() == sum) {
    return false;
  } else {
    tp->set_last_sum(sum);
    return true;
  }
}

// cache_state -- this is called as part of the process of making
// a thread ready to take go to a safepoint.  This is usually called
// when this == JavaThread::current() (from save_state_before_block
// [i.e. StateSaver]) but can also in a certain circumstance (see
// the state() == SUSPENDED case) be called from the vm thread.

// Note -- We can completely skip flushing register windows provided
// we: 1. always suspend all java threads at a safepoint (a la
// thr_suspendallmutators) and 2. this kind of suspension is
// guaranteed to flush the thread's register windows and 3. we can
// get the thread's current sp from another thread (again a la the
// new libthread).  If these are true, then all of this work (in
// cache_state) can be done from the vm thread at safepoint time
// and no flushing should be necessary.

void JavaFrameAnchor::make_walkable(bool is_definitely_current_thread, JavaThread* thread) {
  if (!has_last_Java_frame()) {
    // we have no state to cache -- return
    return;

  } else if (!is_definitely_current_thread && thread->is_vm_suspended()) {
    if (_post_Java_state == NULL) {
      intptr_t* sp;
      intptr_t* younger_sp;
      ExtendedPC addr  = os::fetch_top_frame(thread, &younger_sp, &sp);
      if (sp == NULL) {
	return;
      }
      _post_Java_state = (frame::next_younger_sp(last_Java_sp(), sp));
    }
    // A side effect (undocumented) of suspending a Solaris thread
    // is that all of the register windows get flushed.  Therefore,
    // if another thread (i.e. the vm thread) has suspended a
    // thread, it can call cache_state to ensure that it is noted
    // that the windows are flushed.  The alternative would be to
    // send a signal to the thread and have it flush its own
    // windows.  Suspension seems to be faster.
    set_post_Java_state_has_flushed(true);

  } else if (_post_Java_state == NULL) {
    // The state has not been cached -- cache it -- i.e. find the
    // frame one younger than the last Java sp.  This frame's
    // register save area holds interesting things like the last
    // java frame's 'o' registers.

    assert(Thread::current() == thread, "only current thread can flush its own register windows");
    _post_Java_state = frame::next_younger_sp(last_Java_sp());

    // a side effect of calling next_younger_sp is that the register
    // windows get flushed -- note this.

    set_post_Java_state_has_flushed(true);

  } else if (!post_Java_state_has_flushed()) {
    // Sometimes we note the post java state but don't flush the
    // windows because it would be too expensive to do so all of the
    // time.  Flush them now.

    assert(Thread::current() == thread, "only current thread can flush its own register windows");
    (void)StubRoutines::sparc::flush_callers_register_windows_func()();
    set_post_Java_state_has_flushed(true);
  }

  assert(has_cached_state(), "sanity check");
}


#if !defined(COMPILER2) && !defined(_LP64)

// these routines are the initial value of atomic_exchange_entry(),
// atomic_compare_and_exchange_entry(), atomic_increment_entry()
// and atomic_membar_entry()
// until initialization is complete (see stubRoutines_<arch>.cpp).
// TODO - remove when the VM drops support for v8.

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

#endif // !_LP64 && !COMPILER2

// This is split from os_solaris.cpp because of platform issues but is related to os_win32.cpp
//Reconciliation History
// 1.287 99/04/01 18:31:08 os_win32.cpp
// 1.288 99/04/12 12:56:48 os_win32.cpp
// 1.293 99/07/06 16:36:49 os_win32.cpp
// 1.294 99/07/13 11:41:18 os_win32.cpp
// 1.296 99/08/05 16:45:47 os_win32.cpp
// 1.297 99/08/25 13:15:30 os_win32.cpp
//End
