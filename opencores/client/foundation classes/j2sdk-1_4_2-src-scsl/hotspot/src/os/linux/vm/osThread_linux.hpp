#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)osThread_linux.hpp	1.25 03/01/23 11:03:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:
  int _thread_type;

 public:

  int thread_type() const {
    return _thread_type;
  }
  void set_thread_type(int type) {
    _thread_type = type;
  }

 private:

  pid_t     _thread_id;
  pthread_t _pthread_id;

 public:

  pid_t thread_id() const {
    return _thread_id;
  }
#ifndef PRODUCT
  // Used for debugging, return a unique integer for each thread.
  int thread_identifier() const   { return _thread_id; }
#endif
#ifdef ASSERT
  // We expect no reposition failures so kill vm if we get one.
  //
  bool valid_reposition_failure() {
    return false;
  }
#endif // ASSERT
  void set_thread_id(pid_t id) {
    _thread_id = id;
  }
  pthread_t pthread_id() const {
    return _pthread_id;
  }
  void set_pthread_id(pthread_t tid) {
    _pthread_id = tid;
  }

  // ***************************************************************
  // suspension support.
  // ***************************************************************

public:
  // flags that support signal based suspend/resume on Linux are in a
  // separate class to avoid confusion with many flags in OSThread that
  // are used by VM level suspend/resume.
  os::Linux::SuspendResume sr;

  // _ucontext and _siginfo are used by SR_handler() to save thread context,
  // and they will later be used to walk the stack or reposition thread PC.
  // If the thread is not suspended in SR_handler() (e.g. self suspend),
  // the value in _ucontext is meaningless, so we must use the last Java
  // frame information as the frame. This will mean that for threads
  // that are parked on a mutex the profiler (and safepoint mechanism)
  // will see the thread as if it were still in the Java frame. This
  // not a problem for the profiler since the Java frame is a close
  // enough result. For the safepoint mechanism when the give it the
  // Java frame we are not at a point where the safepoint needs the
  // frame to that accurate (like for a compiled safepoint) since we
  // should be in a place where we are native and will block ourselves
  // if we transition.
private:
  void* _siginfo;
  ucontext_t* _ucontext;
  int _expanding_stack;			/* non zero if manually expanding stack */
  address _alt_sig_stack;               /* address of base of alternate signal stack */

public:
  void* siginfo() const                   { return _siginfo;  }
  void set_siginfo(void* ptr)             { _siginfo = ptr;   }
  ucontext_t* ucontext() const            { return _ucontext; }
  void set_ucontext(ucontext_t* ptr)      { _ucontext = ptr;  }
  void set_expanding_stack(void)          { _expanding_stack = 1;  }
  void clear_expanding_stack(void)        { _expanding_stack = 0;  }
  int  expanding_stack(void)              { return _expanding_stack;  }

  void set_alt_sig_stack(address val)     { _alt_sig_stack = val; }
  address alt_sig_stack(void)             { return _alt_sig_stack; }

  // ***************************************************************
  // interrupt support.  interrupts (using signals) are used to get
  // the thread context (fetch_top_frame), to set the thread context
  // (set_thread_pc), and to implement java.lang.Thread.interrupt.
  // this implementation is borrowed from Win32 classic vm.
  // ***************************************************************

private:

  os::Linux::Event* _interrupt_event;
  os::Linux::Event* _startThread_event;

public:

  os::Linux::Event* interrupt_event() const {
    return _interrupt_event;
  }
  void set_interrupt_event(os::Linux::Event* ptr) {
    _interrupt_event = ptr;
  }

  os::Linux::Event* startThread_event() const {
    return _startThread_event;
  }

  // ***************************************************************
  // Platform dependent initialization and cleanup
  // ***************************************************************

private:

  void pd_initialize();
  void pd_destroy();

// Reconciliation History
// osThread_solaris.hpp	1.24 99/08/27 13:11:54
// End
