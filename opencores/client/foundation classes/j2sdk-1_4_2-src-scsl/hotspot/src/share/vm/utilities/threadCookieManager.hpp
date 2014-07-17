#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)threadCookieManager.hpp	1.3 03/01/23 12:29:04 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ThreadCookieClosure;

class ThreadCookieManager {
protected:
  class TCEntry {
  public:
    TCEntry()
      : _thread(NULL), _cookie(NULL) {}

    Thread* _thread;
    void*   _cookie;
  };

  TCEntry* _entries;
  int      _occupied;
  int      _capacity;

  Mutex    _lock;
  Mutex*   lock() { return &_lock; }

  ThreadCookieManager(int capacity);
  ~ThreadCookieManager();

  inline TCEntry* find_entry(Thread* t);

  // It's a singleton, i.e. there is only one
  // instance of a thread cookie manager.
  static ThreadCookieManager* _thread_cookie_manager;

public:
  // Add the given cookie for the given thread. If the thread is
  // already being managed then any existing cookie will be
  // overwritten with the new one. Otherwise the thread is added
  // to the manager with the given cookie.
  void  put(Thread* t, void* cookie);

  // Get the cookie belonging to the given thread.
  // If the thread isn't being managed then return NULL.
  void* get(Thread* t);

  // Apply the given closure to all managed threads and their cookies
  void iterate(ThreadCookieClosure* cl);

  // Initialise the (singleton) thread cookie manager. Capacity
  // is the number of threads for which to manage cookies.
  static void init_thread_cookie_manager(int capacity);

  // Get a reference to the (singleton) thread cookie manager.
  static ThreadCookieManager* get_thread_cookie_manager();
};

class ThreadCookieClosure : public CHeapObj {
public:
  // Should return true to stop iterating, false otherwise.
  virtual bool do_thread_cookie(Thread* t, void* cookie) = 0;
};
