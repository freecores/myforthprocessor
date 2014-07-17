#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)events.hpp	1.13 03/01/23 12:27:58 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Events and EventMark provide interfaces to log events taking place in the vm.
// This facility is extremly useful for post-mortem debugging. The eventlog
// often provides crucial information about events leading up to the crash.
//
// To log a single event use:
//    Events::log("New nmethod has been created 0x%lx", nm);
//
// To log a block of events use:
//    EventMark m("GarbageCollecting %d", gc_number);
//
// The constructor to eventlog indents the eventlog until the
// destructor has been executed.
//
// IMPLEMENTATION RESTRICTION:
//   Max 3 arguments are saved for each logged event.
//
class Events : AllStatic {  
 public:
  // Logs an event, format as printf
  static void log(const char* format, ...) PRODUCT_RETURN;

  // Prints all events in the buffer
  static void print_all() PRODUCT_RETURN;

  // Prints last number events from the event buffer
  static void print_last(int number) PRODUCT_RETURN;

  // returns indent level
  static int  indent() PRODUCT_RETURN0;
};

class EventMark : public StackObj {
 public:
  // log a begin event, format as printf
  EventMark(const char* format, ...) PRODUCT_RETURN;
  // log an end event
  ~EventMark() PRODUCT_RETURN;
};

int print_all_events();

