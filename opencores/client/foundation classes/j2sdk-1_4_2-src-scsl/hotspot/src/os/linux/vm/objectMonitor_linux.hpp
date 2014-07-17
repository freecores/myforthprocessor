#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objectMonitor_linux.hpp	1.8 03/01/23 11:03:40 JVM"
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
  os::Linux::OSMutex _mutex; // platform specific mutex
