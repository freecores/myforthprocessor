#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_solaris.inline.hpp	1.14 03/01/23 11:05:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// return number of threads contending for this monitor
inline intptr_t ObjectMonitor::contentions() const {
  return _count;
}

inline void ObjectMonitor::set_owner(void* owner) {
  _owner = owner;
  _recursions = 0;
  _count = 0;
}
