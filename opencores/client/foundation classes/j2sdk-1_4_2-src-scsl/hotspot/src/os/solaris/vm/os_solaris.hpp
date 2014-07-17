#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_solaris.hpp	1.80 03/04/23 15:08:35 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Solaris_OS defines the interface to Solaris operating systems

class Solaris {
  friend class os;
  friend class atomic;

 private:
  
  // Support for "new" libthread APIs for getting & setting thread context (2.8)
  #define TRS_VALID       0
  #define TRS_NONVOLATILE 1
  #define TRS_LWPID       2
  #define TRS_INVALID     3

  // _mutator_libthread is true if we have dynamically detected the presense of
  // the new SunSoft libthread.so
  // _T2_libthread is true if we believe we are running with the newer
  // SunSoft lib/lwp/libthread: default Solaris 9, available Solaris 8
  // which is a lightweight libthread that also supports all T1
  // If _T2_libthread is true, _mutator_libthread should also be true
  static bool _mutator_libthread;
  static bool _T2_libthread;
  // These refer to new libthread interface functions
  // They get intialized if we dynamically detect new libthread
  static int_fnP_thread_t_iP_uP_stack_tP_gregset_t _thr_getstate;
  static int_fnP_thread_t_i_gregset_t _thr_setstate;
  static int_fnP_thread_t_i _thr_setmutator;
  static int_fnP_thread_t _thr_suspend_mutator;
  static int_fnP_thread_t _thr_continue_mutator;
  // libthread_init sets the above, if the new functionality is detected

  // initialized to libthread or lwp synchronization primitives depending on UseLWPSychronization
  static int_fnP_mutex_tP _mutex_lock;
  static int_fnP_mutex_tP _mutex_trylock;
  static int_fnP_mutex_tP _mutex_unlock;
  static int_fnP_mutex_tP_i_vP _mutex_init;
  static int_fnP_mutex_tP _mutex_destroy;
  static int _mutex_scope;

  static int_fnP_cond_tP_mutex_tP_timestruc_tP _cond_timedwait;
  static int_fnP_cond_tP_mutex_tP _cond_wait;
  static int_fnP_cond_tP _cond_signal;
  static int_fnP_cond_tP _cond_broadcast;
  static int_fnP_cond_tP_i_vP _cond_init;
  static int_fnP_cond_tP _cond_destroy;
  static int _cond_scope;

  // Large Page Support 
  static char* _largepage_end_addr; 
  static char* _largepage_start_addr; 
  static void set_largepage_end_addr(char* addr) { _largepage_end_addr = addr; } 
  static char* largepage_end_addr() { return _largepage_end_addr; } 
  static void set_largepage_start_addr(char* addr) { _largepage_start_addr = addr; } 
  static char* largepage_start_addr() { return _largepage_start_addr; } 
  static bool largepage_range(char* addr, size_t size); 

  static ExtendedPC fetch_top_frame_fast(Thread* thread, intptr_t** ret_younger_sp, intptr_t** ret_sp);

  static void init_thread_fpu_state(void);

  // For signal-chaining
  static unsigned long sigs;                 // mask of signals that have
                                             // preinstalled signal handlers
  static struct sigaction *(*get_signal_action)(int);
  static struct sigaction *get_preinstalled_handler(int);
  static int (*get_libjsig_version)();
  static void save_preinstalled_handler(int, struct sigaction&);

  // For overridable signals
  static int _SIGinterrupt;                  // user-overridable INTERRUPT_SIGNAL
  static int _SIGasync;                      // user-overridable ASYNC_SIGNAL
  static void set_SIGinterrupt(int newsig) { _SIGinterrupt = newsig; }
  static void set_SIGasync(int newsig) { _SIGasync = newsig; }

 public:
  static int SIGinterrupt() { return _SIGinterrupt; }
  static int SIGasync() { return _SIGasync; }
  static address handler_start, handler_end; // start and end pc of thr_sighndlrinfo

  static bool valid_stack_address(Thread* thread, address sp);
  static bool valid_ucontext(Thread* thread, ucontext_t* valid, ucontext_t* suspect);

  // For Analyzer Forte AsyncGetCallTrace profiling support
  static ExtendedPC fetch_frame_from_ucontext(Thread* thread, ucontext_t* uc, intptr_t** ret_sp);

 protected:
  // Solaris-specific interface goes here

  static int _processor_count;
  static int processor_count() { return _processor_count; }
  static julong physical_memory() { return _physical_memory; }
  static julong _physical_memory;
  static void initialize_system_info();
  static int _dev_zero_fd;
  static int get_dev_zero_fd() { return _dev_zero_fd; }
  static void set_dev_zero_fd(int fd) { _dev_zero_fd = fd; }
  static char* mmap_chunk(char *addr, size_t size, int flags, int prot);
  static void mpss_sanity_check();

  // Workaround for 4352906. thr_stksegment sometimes returns
  // a bad value for the primordial thread's stack base when
  // it is called more than one time. 
  // Workaround is to cache the initial value to avoid further 
  // calls to thr_stksegment. 
  // It appears that someone (Hotspot?) is trashing the user's
  // proc_t structure (note that this is a system struct).
  static address _main_stack_base;

 public:
  static void libthread_init();        
  static void synchronization_init();  

  // This boolean allows users to forward their own non-matching signals
  // to JVM_handle_solaris_signal, harmlessly.
  static bool signal_handlers_are_installed;

  static void install_signal_handlers();
  static void set_signal_handler(int sig, bool set_installed, bool oktochain);
  static void init_signal_mem();
  static bool is_sig_ignored(int sig);

  // For signal-chaining
  static bool libjsig_is_loaded; // libjsig that interposes sigaction(),
                                 // signal(), sigset() is loaded
  static struct sigaction *get_chained_signal_action(int sig);
  static bool chained_handler(struct sigaction *actp, int sig,
                              siginfo_t *siginfo, void *context);

  // The following allow us to link against both the old and new libthread (2.8)
  // and exploit the new libthread functionality if available.
  static bool mutator_libthread()                           { return _mutator_libthread; }
  static void set_mutator_libthread(bool mutator_libthread) { _mutator_libthread = mutator_libthread; }

  static bool T2_libthread()                                { return _T2_libthread; }
  static void set_T2_libthread(bool T2_libthread) { _T2_libthread = T2_libthread; }

  static int thr_getstate(thread_t tid, int *flag, unsigned *lwp, stack_t *ss, gregset_t rs)
    { return _thr_getstate(tid, flag, lwp, ss, rs); }
  static void set_thr_getstate(int_fnP_thread_t_iP_uP_stack_tP_gregset_t func)
    { _thr_getstate = func; }

  static int thr_setstate(thread_t tid, int flag, gregset_t rs)   { return _thr_setstate(tid, flag, rs); }
  static void set_thr_setstate(int_fnP_thread_t_i_gregset_t func) { _thr_setstate = func; }
  
  static int thr_setmutator(thread_t tid, int enabled)    { return _thr_setmutator(tid, enabled); }
  static void set_thr_setmutator(int_fnP_thread_t_i func) { _thr_setmutator = func; }

  static int  thr_suspend_mutator(thread_t tid)              { return _thr_suspend_mutator(tid); }
  static void set_thr_suspend_mutator(int_fnP_thread_t func) { _thr_suspend_mutator = func; }

  static int  thr_continue_mutator(thread_t tid)              { return _thr_continue_mutator(tid); }
  static void set_thr_continue_mutator(int_fnP_thread_t func) { _thr_continue_mutator = func; }

  // Allows us to switch between lwp and thread -based synchronization
  static int mutex_lock(mutex_t *mx)    { return _mutex_lock(mx); }
  static int mutex_trylock(mutex_t *mx) { return _mutex_trylock(mx); }
  static int mutex_unlock(mutex_t *mx)  { return _mutex_unlock(mx); }
  static int mutex_init(mutex_t *mx)    { return _mutex_init(mx, os::Solaris::mutex_scope(), NULL); }
  static int mutex_destroy(mutex_t *mx) { return _mutex_destroy(mx); }
  static int mutex_scope()              { return _mutex_scope; }

  static void set_mutex_lock(int_fnP_mutex_tP func)      { _mutex_lock = func; }
  static void set_mutex_trylock(int_fnP_mutex_tP func)   { _mutex_trylock = func; }
  static void set_mutex_unlock(int_fnP_mutex_tP func)    { _mutex_unlock = func; }
  static void set_mutex_init(int_fnP_mutex_tP_i_vP func) { _mutex_init = func; }
  static void set_mutex_destroy(int_fnP_mutex_tP func)   { _mutex_destroy = func; }
  static void set_mutex_scope(int scope)                 { _mutex_scope = scope; }

  static int cond_timedwait(cond_t *cv, mutex_t *mx, timestruc_t *abst)
                                                { return _cond_timedwait(cv, mx, abst); }
  static int cond_wait(cond_t *cv, mutex_t *mx) { return _cond_wait(cv, mx); }
  static int cond_signal(cond_t *cv)            { return _cond_signal(cv); }
  static int cond_broadcast(cond_t *cv)         { return _cond_broadcast(cv); }
  static int cond_init(cond_t *cv)              { return _cond_init(cv, os::Solaris::cond_scope(), NULL); }
  static int cond_destroy(cond_t *cv)           { return _cond_destroy(cv); }
  static int cond_scope()                       { return _cond_scope; }

  static void set_cond_timedwait(int_fnP_cond_tP_mutex_tP_timestruc_tP func)
                                                           { _cond_timedwait = func; }
  static void set_cond_wait(int_fnP_cond_tP_mutex_tP func) { _cond_wait = func; }
  static void set_cond_signal(int_fnP_cond_tP func)        { _cond_signal = func; }
  static void set_cond_broadcast(int_fnP_cond_tP func)     { _cond_broadcast = func; }
  static void set_cond_init(int_fnP_cond_tP_i_vP func)     { _cond_init = func; }
  static void set_cond_destroy(int_fnP_cond_tP func)       { _cond_destroy = func; }
  static void set_cond_scope(int scope)                    { _cond_scope = scope; }

  enum {
    clear_interrupted = true
  };
  static void setup_interruptible(JavaThread* thread);
  static JavaThread* setup_interruptible();
  static void cleanup_interruptible(JavaThread* thread);

  static sigset_t* unblocked_signals();
  static sigset_t* allowdebug_blocked_signals();

  // %%% Following should be promoted to os.hpp:
  // Trace number of created threads
  static jint  _os_thread_limit;
  static jint  _os_thread_count;

  // Tells whether we're running on an MP machine
  static bool is_MP() {
    return processor_count() > 1;
  };

  // Minimum stack size a thread can be created with (allowing
  // the VM to completely create the thread and enter user code)

  static size_t min_stack_allowed;

  // Stack overflow handling

  static void install_alternate_signal_stack();
  static void remove_alternate_signal_stack();
  static int max_register_window_saves_before_flushing();

  // Stack repair handling

  // none present 

  // naked sleep for v8
  static int naked_sleep();

  // An event is a condition variable with associated mutex.
  // (A cond_t is only usable in combination with a mutex_t.)
  class Event : public CHeapObj {
   private:
    #ifndef PRODUCT
    debug_only(unsigned _owner;)
    #endif
    mutex_t _mutex[1];
    cond_t  _cond[1];
    int     _count;

   public:
    Event() {
      verify();
      int status;
      status = os::Solaris::cond_init(_cond);
      assert(status == 0, "cond_init");
      status = os::Solaris::mutex_init(_mutex);
      assert(status == 0, "mutex_init");
      #ifndef PRODUCT
      debug_only(_owner = 0;)
      #endif
      _count = 0;
    }
    ~Event() {
      int status;
      status = os::Solaris::cond_destroy(_cond);
      assert(status == 0, "cond_destroy");
      status = os::Solaris::mutex_destroy(_mutex);
      assert(status == 0, "mutex_destroy");
    }
    // for use in critical sections:
    void lock() {
      verify();
      int status = os::Solaris::mutex_lock(_mutex);
      assert(status == 0, "mutex_lock");
      #ifndef PRODUCT
      debug_only(_owner = thr_self();)
      #endif
    }
    bool trylock() {
      verify();
      int status = os::Solaris::mutex_trylock(_mutex);
      if (status == EBUSY)
	return false;
      assert(status == 0, "mutex_lock");
      #ifndef PRODUCT
      debug_only(_owner = thr_self();)
      #endif
      return true;
    }
    void unlock() {
      verify();
      int status = os::Solaris::mutex_unlock(_mutex);
      assert(status == 0, "mutex_unlock");
      #ifndef PRODUCT
      debug_only(_owner = 0;)
      #endif
    }
    int timedwait(timestruc_t* abstime) {
      verify();
      int status = os::Solaris::cond_timedwait(_cond, _mutex, abstime);
      assert(status == 0 || status == EINTR || status == ETIME || status == ETIMEDOUT, "cond_timedwait");
      return status;
    }
    int timedwait(jlong millis) {
      timestruc_t abst;
      Event::compute_abstime(&abst, millis);
      return timedwait(&abst);
    }
    int wait() {
      verify();
      int status = os::Solaris::cond_wait(_cond, _mutex);
      // for some reason, under 2.7 lwp_cond_wait() may return ETIME ...
      // Treat this the same as if the wait was interrupted
      // With usr/lib/lwp going to kernel, always handle ETIME
      if(status == ETIME) {
	status = EINTR;
      }
      assert(status == 0 || status == EINTR, "cond_wait");
      return status;
    }
    void signal() {
      verify();
      int status = os::Solaris::cond_signal(_cond);
      assert(status == 0, "cond_signal");
    }
    void broadcast() {
      verify();
      int status = os::Solaris::cond_broadcast(_cond);
      assert(status == 0, "cond_broadcast");
    }
    // functions used to support monitor and interrupt
    void down() {
      verify();
      if (!trylock()) {
        lock();
      }
      while (_count <= 0) {
        wait();
      }
      assert(_count > 0, "logic error");
      _count--;
      unlock();
    }
    int down(jlong millis) {
      timestruc_t abst;
      Event::compute_abstime(&abst, millis);
      verify();
      int raw ;
      int ret = OS_TIMEOUT;
      if (!trylock()) {
        lock();
      }
      // Object.wait(timo) will return because of
	  // (a) notification
	  // (b) timeout
	  // (c) thread.interrupt
	  // 
	  // Thread.interrupt and object.notify{All} both call Event::set.  	
	  // That is, we treat thread.interrupt as a special case of notification.  
	  // The underlying Solaris implementation, cond_timedwait, admits 
	  // spurious/premature wakeups, but the JLS/JVM spec prevents the
	  // JVM from making those visible to Java code.  As such, we must
	  // filter out spurious wakeups.  We assume all ETIME returns are valid. 
	  //
	  // TODO: properly differentiate simultaneous notify+interrupt. 
	  // In that case, we should propagate the notify to another waiter. 

	  while (_count <= 0) { 		
        raw = timedwait(&abst);
		if (!FilterSpuriousWakeups) break ; 		// previous semantics
		if (raw == ETIME || raw == ETIMEDOUT) break ; 
		// We consume and ignore EINTR and spurious wakeups.   
      }
      if (_count > 0) {
        _count--;
        ret = OS_OK;
      }
      unlock();
      return ret;
    }
    void post() {
      verify();
      if (!trylock()) {
        lock();
      }
      _count++;
      broadcast();
      unlock();
    }
    void set() {
      verify();
      if (!trylock()) {
        lock();
      }
      _count = 1;
      broadcast();
      unlock();
    }
    void reset() {
      verify();
      _count = 0;
    }

// value determined through experimentation
#define roundingFix 11    
    // utility to compute the abstime argument to timedwait:
    static timestruc_t* compute_abstime(timestruc_t* abstime, jlong millis) {
      // millis is the relative timeout time
      // abstime will be the absolute timeout time
      if (millis < 0)  millis = 0;
      struct timeval now;
      int status = gettimeofday(&now, NULL);
      assert(status == 0, "gettimeofday");
      jlong seconds = millis / 1000;
      jlong max_wait_period;
      
      if(UseLWPSynchronization) {
        // forward port of fix for 4275818 (not sleeping long enough)
        // There was a bug in Solaris 6, 7 and pre-patch 5 of 8 where
        // _lwp_cond_timedwait() used a round_down algorithm rather
        // than a round_up. For millis less than our roundfactor 
        // it rounded down to 0 which doesn't meet the spec.
        // For millis > roundfactor we may return a bit sooner, but
        // since we can not accurately identify the patch level and
        // this has already been fixed in Solaris 9 and 8 we will
        // leave it alone rather than always rounding down.

        if (millis > 0 && millis < roundingFix) millis = roundingFix;
	// It appears that when we go directly through Solaris _lwp_cond_timedwait()
	// the acceptable max time threshold is smaller than for libthread on 2.5.1 and 2.6
	max_wait_period = 21000000;  
      }
      else {
	max_wait_period = 50000000;
      }
      millis %= 1000;
      if (seconds > max_wait_period) {	// see man cond_timedwait(3T)
	seconds = max_wait_period;
      }
      abstime->tv_sec = now.tv_sec  + seconds;
      long       usec = now.tv_usec + millis * 1000;
      if (usec >= 1000000) {
	abstime->tv_sec += 1;
	usec -= 1000000;
      }
      abstime->tv_nsec = usec * 1000;
      return abstime;
    }

    // hook to check for mutex corruption:
    void verify() PRODUCT_RETURN;
  };

  // An OSMutex is an abstraction over the different thread libraries'
  // mutexes. NOTE that this assumes that mutex_t is identical for
  // libthread, liblwp, and pthreads.
  class OSMutex : public CHeapObj {
   private:
    #ifndef PRODUCT
    debug_only(volatile unsigned _owner;)
    debug_only(volatile bool      _is_owned;)
    #endif
    mutex_t _mutex[1];

   public:
    OSMutex() {
      verify();
      int status = os::Solaris::mutex_init(_mutex);
      assert(status == 0, "mutex_init");
      #ifndef PRODUCT
      debug_only(_is_owned = false;)
      #endif
    }
    ~OSMutex() {
      int status = os::Solaris::mutex_destroy(_mutex);
      assert(status == 0, "mutex_destroy");
    }
    // for use in critical sections:
    void lock() {
      verify();
      int status = os::Solaris::mutex_lock(_mutex);
      assert(status == 0, "mutex_lock");
      #ifndef PRODUCT
      assert(_is_owned == false, "mutex_lock should not have had owner");
      debug_only(_owner = thr_self();)
      debug_only(_is_owned = true;)
      #endif
    }
    bool trylock() {
      verify();
      int status = os::Solaris::mutex_trylock(_mutex);
      if (status == EBUSY)
	return false;
      assert(status == 0, "mutex_trylock");
      #ifndef PRODUCT
      debug_only(_owner = thr_self();)
      debug_only(_is_owned = true;)
      #endif
      return true;
    }
    void unlock() {
      verify();
      #ifndef PRODUCT
      debug_only(int my_id = thr_self();)
      assert(_owner == my_id, "mutex_unlock");
      debug_only(_is_owned = false;)
      #endif
      int status = os::Solaris::mutex_unlock(_mutex);
      assert(status == 0, "mutex_unlock");
    }

    // hook to check for mutex corruption:
    void verify() PRODUCT_RETURN;
    void verify_locked() PRODUCT_RETURN;
  };
};

//Reconciliation History
// 1.17 99/03/24 14:00:00 os_win32.hpp
// 1.19 99/06/22 16:39:06 os_win32.hpp
// 1.20 99/07/30 14:35:24 os_win32.hpp
// 1.22 99/08/25 11:16:05 os_win32.hpp
//End
