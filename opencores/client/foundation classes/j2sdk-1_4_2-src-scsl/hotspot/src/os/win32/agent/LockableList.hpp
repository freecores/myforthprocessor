#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)LockableList.hpp	1.4 03/01/23 11:06:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _LOCKABLE_LIST_
#define _LOCKABLE_LIST_

#include <windows.h>
#include "BasicList.hpp"

template<class T>
class LockableList : public BasicList<T> {
private:
  CRITICAL_SECTION crit;

public:
  LockableList() {
    InitializeCriticalSection(&crit);
  }

  ~LockableList() {
    DeleteCriticalSection(&crit);
  }

  void lock() {
    EnterCriticalSection(&crit);
  }

  void unlock() {
    LeaveCriticalSection(&crit);
  }
};

#endif  // #defined _LOCKABLE_LIST_
