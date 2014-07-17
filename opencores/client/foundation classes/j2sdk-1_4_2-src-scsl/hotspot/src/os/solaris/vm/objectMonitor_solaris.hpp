#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_solaris.hpp	1.21 03/01/23 11:05:26 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

 private:

  void          enter2(TRAPS);

  void          enqueue(ObjectWaiter* waiter);
  ObjectWaiter* dequeue();
  void          dequeue2(ObjectWaiter* waiter);

 public:
  intptr_t      contentions() const;
  ObjectWaiter* first_waiter();
  ObjectWaiter* next_waiter(ObjectWaiter* o);
  Thread*       thread_of_waiter(ObjectWaiter* o);

 private:
  os::Solaris::OSMutex _mutex; // platform specific mutex
