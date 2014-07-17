#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)os_win32_ia64.cpp	1.7 03/01/23 11:11:29 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

// do not include  precompiled  header file
# include "incls/_os_win32_ia64.cpp.incl"


char* os::reserve_memory(size_t bytes) {
  char* res = (char*) VirtualAlloc(NULL, bytes, MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  return !res ? NULL : res;
}

char* os::non_memory_address_word() {
  // Must never look like an address returned by reserve_memory,
  // even in its subfields (as defined by the CPU immediate fields,
  // if the CPU splits constants across multiple instructions).
  return (char*) -1;
}


// OS specific thread initialization
//
// For Itanium, we calculate and store the limits of the 
// register and memory stacks.  
//
void os::initialize_thread() {
  address mem_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // Initialize our register stack limit which is our guard
  JavaThread::enable_register_stack_guard();

  // Initialize our memory stack limit 
  mem_stk_limit = thread->stack_base() - thread->stack_size() +
                  ((StackShadowPages + StackYellowPages +
                  StackRedPages) * os::vm_page_size());
 
  thread->set_memory_stack_limit( mem_stk_limit );
}


// Atomic operations.

typedef jint     exchange_func_t                (jint, jint*);
typedef intptr_t exchange_ptr_func_t            (intptr_t, intptr_t*);
typedef jint     compare_and_exchange_func_t    (jint, jint*, jint);
typedef intptr_t compare_and_exchange_ptr_func_t(intptr_t, intptr_t*, intptr_t);
typedef jlong compare_and_exchange_long_func_t(jlong, jlong*, jlong);
typedef jint     increment_func_t               (jint, jint*);
typedef intptr_t increment_ptr_func_t           (intptr_t, intptr_t*);
typedef void     membar_func_t                  (void);

jint atomic::exchange(jint exchange_value, jint* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_exchange_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jint old_value = *dest;
    *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(exchange_func_t*, func))(exchange_value, dest);
}

intptr_t atomic::exchange_ptr(intptr_t exchange_value, intptr_t* dest) {
  // See if the stub is there.
  address func = StubRoutines::atomic_exchange_ptr_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    intptr_t old_value = *dest;
    *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(exchange_ptr_func_t*, func))(exchange_value, dest);
}

jint atomic::compare_and_exchange(jint exchange_value, jint* dest, jint compare_value) {
  // See if the stub is there.
  address func = StubRoutines::atomic_compare_and_exchange_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jint old_value = *dest;
    if (old_value == compare_value)
      *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(compare_and_exchange_func_t*, func))(exchange_value, dest, compare_value);
}

intptr_t atomic::compare_and_exchange_ptr(intptr_t exchange_value, intptr_t* dest, intptr_t compare_value) {
  // See if the stub is there.
  address func = StubRoutines::atomic_compare_and_exchange_ptr_entry();

  if (func == NULL) {
    // No stub.
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    intptr_t old_value = *dest;
    if (old_value == compare_value)
      *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(compare_and_exchange_ptr_func_t*, func))(exchange_value, dest, compare_value);
}

jlong atomic::compare_and_exchange_long(jlong exchange_value, jlong* dest, jlong compare_value) {
  // See if the stub is there.
  address func = StubRoutines::atomic_compare_and_exchange_long_entry();

  if (func == NULL) {
    // No stub.
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    jlong old_value = *dest;
    if (old_value == compare_value)
       *dest = exchange_value;
    return old_value;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(compare_and_exchange_long_func_t*, func))(exchange_value, dest, compare_value);
}

jint atomic::add(jint inc, jint* loc) {
  // See if the stub is there.
  address func = StubRoutines::atomic_increment_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return (*loc) += inc;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(increment_func_t*, func))(inc, loc);
}

intptr_t atomic::add_ptr(intptr_t inc, intptr_t* loc) {
  // See if the stub is there.
  address func = StubRoutines::atomic_increment_ptr_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return (*loc) += inc;
  }

  // Do the operation.
  return (*CAST_TO_FN_PTR(increment_ptr_func_t*, func))(inc, loc);
}

void atomic::membar(void) {
  // See if the stub is there.
  address func = StubRoutines::atomic_membar_entry();

  if (func == NULL) {
    assert(Threads::number_of_threads() == 0, "for bootstrap only");
    return;
  }

  // Do the operation.
  (*CAST_TO_FN_PTR(membar_func_t*, func))();
}

// Register Stack Management Routines

// Check to see if the current BSP is within our current guard
// page area.
bool JavaThread::register_stack_overflow()  {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return false;

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + 
                  StackRedPages) * os::vm_page_size());

  if ( StubRoutines::ia64::get_backing_store_pointer() > (reg_stk_limit - StackReguardSlack ) )
    return true;
  else
    return false;
}

// Set the guard page address to it's normal guarded position.
// Compiled code and interpreter entry compares the current
// BSP to this address to check for overflow.

void JavaThread::enable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the memory stack and that
  // it starts at the beginning stack address and grows higher.  The memory stack
  // grows to lower addresses.

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackYellowPages + 
                  StackRedPages) * os::vm_page_size());
  
  thread->set_register_stack_limit( reg_stk_limit );
}

// Reduce the guard page by YellowZonePages to allow for the processing
// of register stack overflow exceptions.
void JavaThread::disable_register_stack_guard() {
  address reg_stk_limit;
  JavaThread* thread = (JavaThread *)Thread::current();
  assert(thread != NULL,"Sanity check");

  if ( !thread->is_Java_thread() ) return;

  // We assume that the register stack is the same size as the memory stack and that
  // it starts at the beginning stack address and grows higher.  The memory stack
  // grows to lower addresses.

  reg_stk_limit = thread->stack_base() + thread->stack_size() -
                  ((StackShadowPages + StackRedPages) * os::vm_page_size());
 
  thread->set_register_stack_limit( reg_stk_limit );
}

