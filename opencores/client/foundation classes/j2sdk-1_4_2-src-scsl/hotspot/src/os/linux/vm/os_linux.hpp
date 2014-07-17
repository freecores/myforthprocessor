#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)os_linux.hpp	1.36 03/05/22 13:46:26 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Linux_OS defines the interface to Linux operating systems

class Linux {
  friend class os;

  // For signal-chaining
#define MAXSIGNUM 32
  static struct sigaction sigact[MAXSIGNUM]; // saved preinstalled sigactions
  static unsigned int sigs;             // mask of signals that have
                                        // preinstalled signal handlers
  static bool libjsig_is_loaded;        // libjsig that interposes sigaction(),
                                        // __sigaction(), signal() is loaded
  static struct sigaction *(*get_signal_action)(int);
  static struct sigaction *get_preinstalled_handler(int);
  static void save_preinstalled_handler(int, struct sigaction&);

  static address   _initial_thread_stack_bottom;
  static uintptr_t _initial_thread_stack_size;

  static int _heap_stack_gap;

 protected:

  static int _processor_count;
  static julong _physical_memory;
  static pthread_t _main_thread;
  static Mutex* _createThread_lock;
  static int _page_size;
  static void *_pthread_getattr_func;

  static int processor_count() { return _processor_count; }
  static julong physical_memory() { return _physical_memory; }
  static void initialize_system_info();

 public:

  static void init_thread_fpu_state();
  static pthread_t main_thread(void)                                { return _main_thread; }
  static void set_pthread_getattr_func(void* fn)                    { _pthread_getattr_func = fn; }
  static void* pthread_getattr_func(void)                           { return _pthread_getattr_func; }
  static void set_createThread_lock(Mutex* lk)                      { _createThread_lock = lk; }
  static Mutex* createThread_lock(void)                             { return _createThread_lock; }
  static void hotspot_sigmask(bool main);

  static address   initial_thread_stack_bottom(void)                { return _initial_thread_stack_bottom; }
  static uintptr_t initial_thread_stack_size(void)                  { return _initial_thread_stack_size; }
  static bool is_initial_thread(void);

  static int heap_stack_gap(void)                                   { return _heap_stack_gap; }
  static void set_heap_stack_gap(int gap)                           { _heap_stack_gap = gap; }

  static int page_size(void)                                        { return _page_size; }
  static void set_page_size(int val)                                { _page_size = val; }

  static address ucontext_get_pc(ucontext_t * uc);
  static intptr_t*   ucontext_get_sp(ucontext_t * uc);
  static intptr_t*   ucontext_get_fp(ucontext_t * uc);

  // This boolean allows users to forward their own non-matching signals
  // to JVM_handle_linux_signal, harmlessly.
  static bool signal_handlers_are_installed;

  static void install_signal_handlers();
  static void set_signal_handler(int, bool);
  static bool is_sig_ignored(int sig);

  static sigset_t* unblocked_signals();

  // For signal-chaining
  static struct sigaction *get_chained_signal_action(int sig);
  static bool chained_handler(struct sigaction *actp, int sig,
                              siginfo_t *siginfo, void *context);
  // Tells whether we're running on an MP machine
  static bool is_MP() {
    return processor_count() > 1;
  };

  // Tells if we are running with floating stack pthread library
  inline static bool is_floating_stack() {
    return (pthread_getattr_func() != NULL);
  }

  // Minimum stack size a thread can be created with (allowing
  // the VM to completely create the thread and enter user code)
  static size_t min_stack_allowed;

  // Linux uses very large default stack size (2M before RH-7.1,
  // 8M on RH-7.1). We want to have a smaller default stack size.
  static size_t default_stack_size;
  static void capture_initial_stack(size_t max_size);

  // Stack overflow handling
  static bool manually_expand_stack(JavaThread * t, address addr);
  static void install_alternate_signal_stack(OSThread* osthread);
  static void remove_alternate_signal_stack();
  static int max_register_window_saves_before_flushing();

  // Stack repair handling

  // none present 

  // Workaround for LinuxThreads mutex granting problem (4461173, 4599479)
  static int safe_mutex_lock(pthread_mutex_t *_mutex);
  static int safe_cond_wait(pthread_cond_t *_cond, pthread_mutex_t *_mutex);
  static int safe_cond_timedwait(pthread_cond_t *_cond, pthread_mutex_t *_mutex, const struct timespec *_abstime);

  // An event is a condition variable with associated mutex.
  // (A cond_t is only usable in combination with a mutex_t.)
  class Event : public CHeapObj {
   private:
    pthread_mutex_t _mutex[1];
    pthread_cond_t  _cond[1];
    volatile int    _count;
    
   public:
    Event() {
      verify();
      int status;
      status = pthread_cond_init(_cond, NULL);
      assert(status == 0, "cond_init");
      status = pthread_mutex_init(_mutex, NULL);
      assert(status == 0, "mutex_init");
      _count = 0;
    }
    ~Event() {
      int status;
      status = pthread_cond_destroy(_cond);
      assert(status == 0, "cond_destroy");
      status = pthread_mutex_destroy(_mutex);
      assert(status == 0, "mutex_destroy");
    }
    // hook to check for mutex corruption:
    void verify() PRODUCT_RETURN;
    // for use in critical sections:
    void lock() {
      verify();
      int status = safe_mutex_lock(_mutex);
      assert(status == 0, "mutex_lock");
    }
    bool trylock() {
      verify();
      int status = pthread_mutex_trylock(_mutex);
      if (status == EBUSY) {
	return false;
      }
      assert(status == 0, "mutex_lock");
      return true;
    }
    void unlock() {
      verify();
      int status = pthread_mutex_unlock(_mutex);
      assert(status == 0, "mutex_unlock");
    }
    int timedwait(timespec* abstime) {
      verify();
      int status = safe_cond_timedwait(_cond, _mutex, abstime);
      assert(status == 0 || status == EINTR || status == ETIME || status == ETIMEDOUT, "cond_timedwait");
      return status;
    }
    int timedwait(jlong millis) {
      timespec abst;
      Event::compute_abstime(&abst, millis);
      return timedwait(&abst);
    }
    int wait() {
      verify();
      int status = safe_cond_wait(_cond, _mutex);
      // for some reason, under 2.7 lwp_cond_wait() may return ETIME ...
      // Treat this the same as if the wait was interrupted
      if(status == ETIME) {
	status = EINTR;
      }
      assert(status == 0 || status == EINTR, "cond_wait");
      return status;
    }
    void signal() {
      verify();
      int status = pthread_cond_signal(_cond);
      assert(status == 0, "cond_signal");
    }
    void broadcast() {
      verify();
      int status = pthread_cond_broadcast(_cond);
      assert(status == 0, "cond_broadcast");
    }
    // functions used to support monitor and interrupt
    void down() {
      verify();
      lock();
      while (_count <= 0) {
        wait();
      }
      _count = 0;
      unlock();
    }
    int down(jlong millis) {
      verify();
      int ret = OS_TIMEOUT;
      lock();
      if (_count <= 0) {
        timedwait(millis);
      }
      if (_count > 0) {
        _count = 0;
        ret = OS_OK;
      }
      unlock();
      return ret;
    }
    void set() {
      verify();
      lock();
      _count = 1;
      broadcast();
      unlock();
    }
    void reset() {
      verify();
      _count = 0;
    }
    // utility to compute the abstime argument to timedwait:
    static struct timespec* compute_abstime(timespec* abstime, jlong millis) {
      // millis is the relative timeout time
      // abstime will be the absolute timeout time
      if (millis < 0)  millis = 0;
      struct timeval now;
      int status = gettimeofday(&now, NULL);
      assert(status == 0, "gettimeofday");
      jlong seconds = millis / 1000;
      millis %= 1000;
      if (seconds > 50000000) { // see man cond_timedwait(3T)
        seconds = 50000000;
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
  };

  // An OSMutex is an abstraction used in the implementation of
  // ObjectMonitor; needed to abstract over the different thread
  // libraries' mutexes on Solaris.
  class OSMutex : public CHeapObj {
   private:
    #ifndef PRODUCT
    debug_only(volatile pthread_t _owner;)
    debug_only(volatile bool      _is_owned;)
    #endif
    pthread_mutex_t _mutex[1];

   public:
    OSMutex() {
      verify();
      int status = pthread_mutex_init(_mutex, NULL);
      assert(status == 0, "pthread_mutex_init");
      #ifndef PRODUCT
      debug_only(_is_owned = false;)
      #endif
    }
    ~OSMutex() {
      int status = pthread_mutex_destroy(_mutex);
      assert(status == 0, "pthread_mutex_destroy");
    }
    // for use in critical sections:
    void lock() {
      verify();
      int status = safe_mutex_lock(_mutex);
      assert(status == 0, "pthread_mutex_lock");
      #ifndef PRODUCT
      assert(_is_owned == false, "mutex_lock should not have had owner");
      debug_only(_owner = pthread_self();)
      debug_only(_is_owned = true;)
      #endif
    }
    bool trylock() {
      verify();
      int status = pthread_mutex_trylock(_mutex);
      if (status == EBUSY)
	return false;
      assert(status == 0, "pthread_mutex_trylock");
      #ifndef PRODUCT
      debug_only(_owner = pthread_self();)
      debug_only(_is_owned = true;)
      #endif
      return true;
    }
    void unlock() {
      verify();
      #ifndef PRODUCT
      debug_only(pthread_t my_id = pthread_self();)
      assert(pthread_equal(_owner, my_id), "mutex_unlock");
      debug_only(_is_owned = false;)
      #endif
      int status = pthread_mutex_unlock(_mutex);
      assert(status == 0, "pthread_mutex_unlock");
    }

    // hook to check for mutex corruption:
    void verify() PRODUCT_RETURN;
    void verify_locked() PRODUCT_RETURN;
  };

  // Linux suspend/resume support
  class SuspendResume {
  private:
    volatile int _suspend_action;
    // values for suspend_action:
    #define SR_NONE               (0x00)
    #define SR_SUSPEND            (0x01)  // suspend request
    #define SR_CONTINUE           (0x02)  // resume request

    volatile jint _state;
    // values for _state: (clear_try_mutex_enter_if_not_suspended() and
    // set_try_mutex_enter() must be rewritten if adding more states)
    #define SR_TRY_MUTEX_ENTER    (0x10)
    #define SR_SUSPENDED          (0x20)
  public:
    SuspendResume() { _suspend_action = SR_NONE; _state = SR_NONE; }

    int suspend_action() const     { return _suspend_action; }
    void set_suspend_action(int x) { _suspend_action = x;    }

    void set_try_mutex_enter() { 
      // SR_NONE is the only possible state when we get here.
      // flag SR_SUSPENDED is set by signal handler SR_handler. Because
      // SR_TRY_MUTEX_ENTER is not set, thread will be suspended inside
      // SR_handler. When it finally returns and we can continue to run,
      // SR_SUSPENDED is already cleared.
      jint rslt = atomic::compare_and_exchange(SR_TRY_MUTEX_ENTER,
                                               (jint*)&_state, SR_NONE);
      assert(rslt == SR_NONE, "invalid SuspendResume::_state");
      // after this, we could be in state "SR_TRY_MUTEX_ENTER" or in
      // "SR_TRY_MUTEX_ENTER | SR_SUSPENDED"
      assert((_state & SR_TRY_MUTEX_ENTER) != 0, "invalid state");
    }

    // one assumption for suspend-other is that the thread will always 
    // be suspended inside SR_handler if it's not is_try_mutex_enter(). 
    // we can only clear SR_TRY_MUTEX_ENTER when the thread is not suspended.

    // clear SR_TRY_MUTEX_ENTER and return true only if not SR_SUSPENDED.
    // return false and do nothing if the thread is suspended.
    // Note: this function can only be used when SR_TRY_MUTEX_ENTER is set.
    bool clear_try_mutex_enter_if_not_suspended() {
      assert(is_try_mutex_enter(), "must have SR_TRY_MUTEX_ENTER set");

      // Only two possible states: SR_TRY_MUTEX_ENTER | SR_SUSPENDED or
      // SR_TRY_MUTEX_ENTER. Thread is not suspended if
      // _state == SR_TRY_MUTEX_ENTER

      // CAS is necessary, because otherwise a racing suspend signal
      // delivered after we have read _state can cause troubles.
      return atomic::compare_and_exchange(SR_NONE, (jint*)&_state, SR_TRY_MUTEX_ENTER)
                                         == SR_TRY_MUTEX_ENTER;
    }

    void set_suspended()           { 
      jint temp, temp2;
      do {
	temp = _state;
	temp2 = atomic::compare_and_exchange(temp | SR_SUSPENDED, (jint*)&_state, temp);
      } while (temp2 != temp);
    }
    void clear_suspended()        { 
      jint temp, temp2;
      do {
	temp = _state;
	temp2 = atomic::compare_and_exchange(temp & ~SR_SUSPENDED, (jint*)&_state, temp);
      } while (temp2 != temp);
    }

    bool is_try_mutex_enter() const{ return _state & SR_TRY_MUTEX_ENTER; }
    bool is_suspended()            { return _state & SR_SUSPENDED;       }

    #undef SR_TRY_MUTEX_ENTER
    #undef SR_SUSPENDED
  };
};
