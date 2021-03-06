#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)mutexLocker.cpp	1.136 03/01/23 12:23:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_mutexLocker.cpp.incl"

// Mutexes used in the VM (see comment in mutexLocker.hpp)

Mutex*   Patching_lock                = NULL;
Mutex*   SystemDictionary_lock        = NULL;
Mutex*   PackageTable_lock            = NULL;
Mutex*   PointerMap_lock              = NULL;
Mutex*   CompiledIC_lock              = NULL;
Mutex*   InlineCacheBuffer_lock       = NULL;
Mutex*   VMStatistic_lock             = NULL;
Mutex*   JNIGlobalHandle_lock         = NULL;
Mutex*   JNIHandleBlockFreeList_lock  = NULL;
Mutex*   JNIIdentifier_lock           = NULL;
Monitor* JNICritical_lock             = NULL;
Mutex*   Heap_lock                    = NULL;
Mutex*   ExpandHeap_lock              = NULL;
Mutex*   RecompilationMonitor_lock    = NULL;
Mutex*   SignatureHandlerLibrary_lock = NULL;
Mutex*   VtableStubs_lock             = NULL;
Mutex*   SymbolTable_lock             = NULL;
Mutex*   StringTable_lock             = NULL;
Mutex*   AdapterCache_lock            = NULL;
Mutex*   CodeCache_lock               = NULL;
Mutex*   MethodData_lock              = NULL;
Mutex*   RetData_lock                 = NULL;
Monitor* VMOperationQueue_lock        = NULL;
Monitor* VMOperationRequest_lock      = NULL;
Monitor* Safepoint_lock               = NULL;
Monitor* Threads_lock                 = NULL;
Monitor* CMS_lock                     = NULL;
Monitor* SLT_lock                     = NULL;
Monitor* iCMS_lock                    = NULL;
Mutex*   ParGCRareEvent_lock          = NULL;
Mutex*	 DerivedPointerTableGC_lock   = NULL;
Mutex*   Compile_lock                 = NULL;
Monitor* AdapterCompileQueue_lock     = NULL;
Monitor* MethodCompileQueue_lock      = NULL;
Mutex*   CompileTaskAlloc_lock        = NULL;
Mutex*   CompileStatistics_lock       = NULL;
Mutex*   MultiArray_lock              = NULL;
Monitor* Terminator_lock              = NULL;
Monitor* BeforeExit_lock              = NULL;
Monitor* Notify_lock                  = NULL;
Monitor* Interrupt_lock               = NULL;
Monitor* SuspendChecker_lock          = NULL;
Monitor* ProfileVM_lock               = NULL;
Mutex*   ProfilePrint_lock            = NULL;
Mutex*   ExceptionCache_lock          = NULL;
Mutex*   OsrList_lock                 = NULL;
#ifndef PRODUCT
Mutex*   BytecodeTrace_lock           = NULL;
Mutex*   FullGCALot_lock              = NULL;
#endif

Mutex*   Debug1_lock                  = NULL;
Mutex*   Debug2_lock                  = NULL;
Mutex*   Debug3_lock                  = NULL;

Mutex*   Event_lock                   = NULL;
Mutex*   tty_lock                     = NULL;

Mutex*   MonitorCache_lock            = NULL;
Mutex*   PerfDataMemAlloc_lock        = NULL;
Mutex*   PerfDataManager_lock         = NULL;
Mutex*   OopMapCacheAlloc_lock        = NULL;

Monitor* GCTaskManager_lock           = NULL;

#ifdef ASSERT
void assert_locked_or_safepoint(const Mutex* lock) {
  // check if this thread owns the lock (common case)
  if (IgnoreLockingAssertions) return;
  if (lock->owned_by_self()) return;
  if (SafepointSynchronize::is_at_safepoint()) return;
  if (!Universe::is_fully_initialized()) return;
  // see if invoker of VM operation owns it
  VM_Operation* op = VMThread::vm_operation();
  if (op != NULL && op->calling_thread() == lock->owner()) return;
  fatal1("must own lock %s", lock->name());
}

// a stronger assertion than the above
void assert_lock_strong(const Mutex* lock) {
  if (IgnoreLockingAssertions) return;
  if (lock->owned_by_self()) return;
  fatal1("must own lock %s", lock->name());
}
#endif

#define def(var, type, pri, vm_block) \
  var = new type(Mutex::##pri, #var, vm_block)

void mutex_init() {  
  def(Event_lock                   , Mutex  , event,       true ); // allow to lock in VM
  def(tty_lock                     , Mutex  , event,       true ); // allow to lock in VM

  if (UseConcMarkSweepGC) {
    def(CMS_lock                   , Monitor, special,     true ); // coordinate between fore- and background GC
    def(iCMS_lock                  , Monitor, special,     true ); // CMS incremental mode start/stop notification
  }
  def(ParGCRareEvent_lock          , Mutex  , leaf     ,   true );
  def(DerivedPointerTableGC_lock   , Mutex,   leaf,        true);
  def(CodeCache_lock               , Mutex  , special,     true );
  def(Interrupt_lock               , Monitor, special,     true ); // used for interrupt processing
  def(SuspendChecker_lock          , Monitor, special,     true ); // used for VMThread deadlock detection
  def(MonitorCache_lock            , Mutex,   special,     true ); // used for global monitor cache
  def(OopMapCacheAlloc_lock        , Mutex,   leaf,        true ); // used for oop_map_cache allocation.

  def(Patching_lock                , Mutex  , special,     true );  // used for safepointing and code patching.
  def(SystemDictionary_lock        , Mutex  , leaf,        true ); // lookups done by VM thread
  def(PackageTable_lock            , Mutex  , leaf,        false);
  def(AdapterCache_lock            , Mutex  , leaf,        true );
  def(PointerMap_lock              , Mutex  , leaf,        false);
  def(InlineCacheBuffer_lock       , Mutex  , leaf,        true );
  def(VMStatistic_lock             , Mutex  , leaf,        false);
  def(ExpandHeap_lock              , Mutex  , leaf,        true ); // Used during compilation by VM thread
  def(JNIHandleBlockFreeList_lock  , Mutex  , leaf,        true ); // handles are used by VM thread
  def(JNIIdentifier_lock           , Mutex  , leaf,        true);  // Used by VM_JVMPIPostObjAlloc VM_Operation
  def(RecompilationMonitor_lock    , Mutex  , leaf,        true );
  def(SignatureHandlerLibrary_lock , Mutex  , leaf,        false);
  def(SymbolTable_lock             , Mutex  , leaf,        true );
  def(StringTable_lock             , Mutex  , leaf,        true );
  def(ProfilePrint_lock            , Mutex  , leaf,        false); // serial profile printing
  def(ExceptionCache_lock          , Mutex  , leaf,        false); // serial profile printing
  def(OsrList_lock                 , Mutex  , leaf,        true );
  def(Debug1_lock                  , Mutex  , leaf,        true );
#ifndef PRODUCT
  def(BytecodeTrace_lock           , Mutex  , leaf,        false); // lock to make bytecode tracing mt safe
  def(FullGCALot_lock              , Mutex  , leaf,        false); // a lock to make FullGCALot MT safe
#endif
  def(BeforeExit_lock              , Monitor, leaf,        true );

  // CMS_modUnionTable_lock                   leaf
  // CMS_bitMap_lock                          leaf + 1
  // CMS_freeList_lock                        leaf + 2

  def(Safepoint_lock               , Monitor, safepoint,   true ); // locks SnippetCache_lock/Threads_lock

  def(Threads_lock                 , Monitor, barrier,     true ); 

  def(VMOperationQueue_lock        , Monitor, nonleaf,     true ); // VM_thread allowed to block on these     
  def(VMOperationRequest_lock      , Monitor, nonleaf,     true ); 
  def(RetData_lock                 , Mutex  , nonleaf,     false);
  def(Terminator_lock              , Monitor, nonleaf,     true );
  def(VtableStubs_lock             , Mutex  , nonleaf,     true );
  def(Notify_lock                  , Monitor, nonleaf,     true ); 
  def(JNIGlobalHandle_lock         , Mutex  , nonleaf,     true ); // locks JNIHandleBlockFreeList_lock
  def(JNICritical_lock             , Monitor, nonleaf,     true ); // used for JNI critical regions
  if (UseConcMarkSweepGC) {
    def(SLT_lock                   , Monitor, nonleaf,     false );
                    // used in CMS GC for locking PLL lock
  }
  def(Heap_lock                    , Mutex  , nonleaf+1,   false);

  def(CompiledIC_lock              , Mutex  , nonleaf+2,   false); // locks VtableStubs_lock, InlineCacheBuffer_lock
  def(CompileTaskAlloc_lock        , Mutex  , nonleaf+2,   true ); 
  def(CompileStatistics_lock       , Mutex  , nonleaf+2,   false); 
  def(MultiArray_lock              , Mutex  , nonleaf+2,   false); // locks SymbolTable_lock


  def(Compile_lock                 , Mutex  , nonleaf+3,   true ); 
  def(MethodData_lock              , Mutex  , nonleaf+3,   false);

  def(AdapterCompileQueue_lock     , Monitor, nonleaf+4,   true ); 
  def(MethodCompileQueue_lock      , Monitor, nonleaf+4,   true ); 
  def(Debug2_lock                  , Mutex  , nonleaf+4,   true );
  def(Debug3_lock                  , Mutex  , nonleaf+4,   true );
  def(ProfileVM_lock               , Monitor, nonleaf+4,   false); // used for profiling of the VMThread
  def(PerfDataMemAlloc_lock        , Mutex  , leaf,        true ); // used for allocating PerfData memory for performance data
  def(PerfDataManager_lock         , Mutex  , leaf,        true ); // used for synchronized access to PerfDataManager resources
}

GCMutexLocker::GCMutexLocker(Mutex* mutex) {
  if (SafepointSynchronize::is_at_safepoint()) {
    _locked = false;
  } else {
    _mutex = mutex;
    _locked = true;
    _mutex->lock();
  }
}

#ifndef PRODUCT
static Mutex *TraceLocking_lock = NULL;

void trace_locking(Mutex* mutex, char *type, char *state) {
  if (TraceLocking_lock == NULL) {
    TraceLocking_lock = new Mutex(1, "TraceLocking_lock");
  }
  TraceLocking_lock->lock(); // Do not use MutexLocker here (infinite recursion)
  tty->print("%s thread=" INTPTR_FORMAT "  lock=%s %s", type, ThreadLocalStorage::thread(), mutex->name(), state);  
  TraceLocking_lock->unlock();
}
#endif


