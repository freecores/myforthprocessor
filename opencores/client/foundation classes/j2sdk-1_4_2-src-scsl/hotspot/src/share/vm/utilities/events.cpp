#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)events.cpp	1.28 03/01/23 12:27:56 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_events.cpp.incl"


#ifndef PRODUCT
// Events are the elements stored in the event buffer 

class Event VALUE_OBJ_CLASS_SPEC  {     
 private:
  TimeStamp   time;
  int         thread_id;
  const char* format;
  int         indent;
  int         arg_1;
  int         arg_2;
  int         arg_3;

  enum { vm_process              = -1,
         unknown_process         = -2,
         watcher_process         = -3,
         suspend_checker_process = -4
  };

 public:

  void clear() { format = NULL; }

  void fill(const char* rformat, int arg_1, int arg_2, int arg_3);

  void print() {    
    if (format == NULL) return;
    if (thread_id < 0) {
      const char* id =
	thread_id == vm_process ? " vm"
          : (thread_id == watcher_process ? "wat"
            : (thread_id == suspend_checker_process ? "sus"
              : "???"));
      tty->print("  %s", id);
    } else {
      tty->print("  %3d", thread_id);
    }
    tty->print("  %3.2g   ", time.seconds());
    tty->fill_to(10);
    for (int index = 0; index < indent; index++) {
      tty->print("| ");
    }
    tty->print_cr(format, arg_1, arg_2, arg_3);
  }
};

class EventBuffer : AllStatic {
 private:
  static Event* buffer;
  static int    size;
  static int    pos;

 public:
  static int    indent;

  static Event* next_event() {
    Event* result = &buffer[pos];
    pos = next_index(pos);
    return result;
  }

  static int next_index(int index) {
    return (index + 1) % size;
  }

  static int pred_index(int index) {
    if (index == 0) return size - 1;
    return index - 1;
  }

  static void print_all(); 

  static void print_last(int number) {
    tty->print_cr("[Last %d events in the event buffer]", number);
    tty->print_cr("-<thd>-<age sec>-<description>---------------------");

    int count = 0;
    for (int index = pred_index(pos); index != pos && count < number; index = pred_index(index)) {
      buffer[index].print();
      count++;
    }
  }

  static void init() {
    // Allocate the event buffer
    pos    = 0;
    size   = EventLogLength;
    buffer = NEW_C_HEAP_ARRAY(Event, size);

    // Clear the event buffer
    for (int index = 0; index < size; index++) {
      buffer[index].clear();
    }
  }
};

void EventBuffer::print_all() {
  tty->print_cr("[Events in the event buffer]");
  tty->print_cr("-<age sec>-<description>---------------------");
  for (int index = pred_index(pos); index != pos; index = pred_index(index)) {
    buffer[index].print();
  }
}

Event* EventBuffer::buffer  = NULL;
int    EventBuffer::size    = 0;
int    EventBuffer::pos     = 0;
int    EventBuffer::indent  = 0;

void Event::fill(const char* rformat, int rarg_1, int rarg_2, int rarg_3) {
  format = rformat;
  arg_1  = rarg_1;
  arg_2  = rarg_2;
  arg_3  = rarg_3;
  indent = EventBuffer::indent;
  Thread* t = ThreadLocalStorage::thread();

  if (t == NULL) {
    this->thread_id = unknown_process;
  } else if (t->is_Java_thread()) {
    this->thread_id = t->osthread()->thread_id();
  } else if (t->is_VM_thread()) {
    this->thread_id = vm_process;
  } else if (t->is_Watcher_thread()) {
    this->thread_id = watcher_process;
  } else if (t->is_SuspendChecker_thread()) {
    this->thread_id = suspend_checker_process;
  } else {
    this->thread_id = unknown_process;
  }
  time.update();
}

int Events::indent() {
  return EventBuffer::indent;
}

void Events::log(const char* format, ...) {
  if (LogEvents) {
    // Events should not introduce extra safepoints
    Event_lock->lock_without_safepoint_check();
    
    // Allocate a new event
    Event* event = EventBuffer::next_event();

    // Fill it out
    va_list ap;
    va_start(ap, format);
    int arg_1 = va_arg(ap, int);
    int arg_2 = va_arg(ap, int);
    int arg_3 = va_arg(ap, int);
    va_end(ap);
    event->fill(format, arg_1, arg_2, arg_3);

    Event_lock->unlock();
  }
}

void Events::print_all() {
 // Events should not introduce extra safepoints
  Event_lock->lock_without_safepoint_check();
  EventBuffer::print_all();  
  Event_lock->unlock();
}

void Events::print_last(int number) {
  EventBuffer::print_last(number);  
}

EventMark::EventMark(const char* format, ...) {
  if (LogEvents) {
    // Events should not introduce extra safepoints
    Event_lock->lock_without_safepoint_check();    
    Event* event = EventBuffer::next_event();
    va_list ap;
    va_start(ap, format);
    int arg_1 = va_arg(ap, int);
    int arg_2 = va_arg(ap, int);
    int arg_3 = va_arg(ap, int);
    va_end(ap);
    event->fill(format, arg_1, arg_2, arg_3);
    EventBuffer::indent++;    
    Event_lock->unlock();
  }
}

EventMark::~EventMark() {
  if (LogEvents) {
    { // Events should not introduce extra safepoints
      Event_lock->lock_without_safepoint_check();
      EventBuffer::indent--;
      Event_lock->unlock();
    }
    Events::log("done");
  }
}

void eventlog_init() {
  EventBuffer::init();
}

int print_all_events() {
  EventBuffer::print_all();
  return 1;
}
#else
void eventlog_init() {}
int print_all_events() { return 0; }
#endif // PRODUCT




