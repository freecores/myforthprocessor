#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadCookieManager.cpp	1.3 03/01/23 12:29:02 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_threadCookieManager.cpp.incl"

ThreadCookieManager::ThreadCookieManager(int capacity)
  : _capacity(capacity), _occupied(0), _lock(Mutex::leaf, "tcm lock") {
  _entries = NEW_C_HEAP_ARRAY(TCEntry, capacity);
}

ThreadCookieManager::~ThreadCookieManager() {
  FREE_C_HEAP_ARRAY(TCEntry, _entries);
}

ThreadCookieManager::TCEntry* ThreadCookieManager::find_entry(Thread* t) {
  for (int i = 0; i < _occupied; i++) {
    if (_entries[i]._thread == t) {
      return &_entries[i];
    }
  }
  return NULL;
}

void ThreadCookieManager::put(Thread* t, void* cookie) {
  MutexLockerEx x(lock(), Mutex::_no_safepoint_check_flag);

  TCEntry* entry = find_entry(t);
  if (entry != NULL) {
    entry->_cookie = cookie;
    return;
  }

  assert(_occupied < _capacity, "thread cookie manager overflow");
  _entries[_occupied]._thread = t;
  _entries[_occupied]._cookie = cookie;
  _occupied++;
}

void* ThreadCookieManager::get(Thread* t) {
  TCEntry* entry = find_entry(t);
  return (entry != NULL) ? entry->_cookie : NULL;
}

void ThreadCookieManager::iterate(ThreadCookieClosure* cl) {
  for (int i = 0; i < _occupied; i++) {
    if (cl->do_thread_cookie(_entries[i]._thread, _entries[i]._cookie)) {
      break;
    }
  }
}

ThreadCookieManager* ThreadCookieManager::_thread_cookie_manager = NULL;

void ThreadCookieManager::init_thread_cookie_manager(int capacity) {
  if (_thread_cookie_manager == NULL) {
    _thread_cookie_manager = new ThreadCookieManager(capacity);
  }
}

ThreadCookieManager* ThreadCookieManager::get_thread_cookie_manager() {
  assert(_thread_cookie_manager != NULL, "thread cookie manager not initialised");
  return _thread_cookie_manager;
}
