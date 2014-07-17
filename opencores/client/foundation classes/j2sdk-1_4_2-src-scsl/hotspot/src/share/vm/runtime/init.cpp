#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)init.cpp	1.98 03/01/23 12:23:00 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_init.cpp.incl"


// Add new initialization/finalization methods to the macro below; your
// functions must be parameterless and must be named after the .c file
// they belong to (e.g. "void foo_init(); void foo_exit()" for foo.c).
// The INIT_DO macro defines the initialization sequence; you need not
// add a corresponding entry to EXIT_DO if you don't need finalization.


// Initialization done by VM thread
# define VM_INIT_DO(template)         \
  template(check_ThreadShadow)        \
  template(check_basic_types)         \
  template(eventlog_init)             \
  template(mutex_init)                \
  template(chunkpool_init)            \
  template(perfMemory_init)


// Initialization done by Java thread
# define INIT_DO(template)            \
  template(vtune_init)                \
  template(bytecodes_init)            \
  template(classLoader_init)          \
  template(codeCache_init)            \
  template(icache_init)               \
  template(VM_Version_init)           \
  template(stubRoutines_init1)        \
  template(carSpace_init)             \
  template(universe_init)	      /* dependent on codeCache_init and stubRoutines_init */ \
  template(interpreter_init)          /* before any methods loaded */ \
  template(invocationCounter_init)    /* before any methods loaded */ \
  template(marksweep_init)	      \
  template(accessFlags_init)          \
  template(templateTable_init)        \
  template(InterfaceSupport_init)     \
  template(universe2_init)            /* dependent on codeCache_init and stubRoutines_init */ \
  template(referenceProcessor_init)   \
  template(jni_handles_init)          \
  template(vmStructs_init)


/* compiler related */
#ifdef CORE
# define COMPILER_INIT_DO(template)
#endif


#ifdef COMPILER1
# define COMPILER_INIT_DO(template)   \
  template(compiler1_init)            \
  template(vtableStubs_init)          \
  template(InlineCacheBuffer_init)    \
  template(compilerOracle_init)       \
  template(onStackReplacement_init)   \
  template(compilationPolicy_init)
#endif


#ifdef COMPILER2
# define COMPILER_INIT_DO(template)   \
  template(adapter_init)              \
  template(vtableStubs_init)          \
  template(InlineCacheBuffer_init)    \
  template(compilerOracle_init)       \
  template(compiler2_init)            /* needs ObjectArrayKlass vtables */ \
  template(onStackReplacement_init)   \
  template(compilationPolicy_init)
#endif


// Initialization after compiler initialization
# define INIT_AFTER_COMP_DO(template) \
  template(universe_post_init)        /* must happen after compiler_init */ \
  template(javaClasses_init)          /* must happen after vtable initialization */ \
  template(stubRoutines_init2)        /* note: StubRoutines need 2-phase init */

// Do not disable thread-local-storage, as it is important for some
// JNI/JVM/JVMPI/JVMDI functions and signal handlers to work properly
// during VM shutdown
# define EXIT_DO(template)            \
  template(perfMemory_exit)           \
  template(ostream_exit)


# define DEFINE_TEMPLATE(name)        \
  void name();


# define CALL_TEMPLATE(name)          \
  name();


VM_INIT_DO(DEFINE_TEMPLATE)
INIT_DO(DEFINE_TEMPLATE)
COMPILER_INIT_DO(DEFINE_TEMPLATE)
INIT_AFTER_COMP_DO(DEFINE_TEMPLATE)
EXIT_DO(DEFINE_TEMPLATE)


#ifdef CORE
void adapter_init()           {}
void invocationCounter_init() {}
void sweeper_init()           {}
#endif // CORE


void vm_init_globals() {
  VM_INIT_DO(CALL_TEMPLATE)
}


void init_globals() {  
  INIT_DO(CALL_TEMPLATE)
  COMPILER_INIT_DO(CALL_TEMPLATE)  
  INIT_AFTER_COMP_DO(CALL_TEMPLATE)
  // Although we'd like to, we can't easily do a heap verify
  // here because the main thread isn't yet a JavaThread, so
  // its TLAB may not be made parseable from the usual interfaces.
}


void exit_globals() {
  static bool destructorsCalled = false;
  if (!destructorsCalled) {
    destructorsCalled = true;
    EXIT_DO(CALL_TEMPLATE);
  }
}


static bool _init_completed = false;

bool is_init_completed() {
  return _init_completed;
}


void set_init_completed() {
  _init_completed = true;
}
