#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)BasicList.hpp	1.4 03/01/23 11:06:08 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _BASIC_LIST_
#define _BASIC_LIST_

#include <vector>

template<class T>
class BasicList {
protected:
  typedef std::vector<T> InternalListType;
  InternalListType internalList;

public:
  BasicList() {
  }
  virtual ~BasicList() {
  }

  void add(T arg) {
    internalList.push_back(arg);
  }

  bool remove(T arg) {
    for (InternalListType::iterator iter = internalList.begin();
         iter != internalList.end(); iter++) {
      if (*iter == arg) {
        internalList.erase(iter);
        return true;
      }
    }
    return false;
  }

  int size() {
    return internalList.size();
  }

  T get(int index) {
    return internalList[index];
  }
};

#endif  // #defined _BASIC_LIST_
