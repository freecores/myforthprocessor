#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)jvmdi.cpp	1.140 03/01/23 12:20:03 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_jvmdi.cpp.incl"

static int count_thread_num_frames(JavaThread* thr);
static void recompute_method_entry_and_exit_on(); 

// Wrapper to trace JVMDI functions

#ifdef ASSERT
  
//
// JvmdiTraceWrapper
//

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* str) {
      ResourceMark rm;
      const char *thread_name = JvmdiInternal::safe_get_thread_name(JavaThread::current()->threadObj());
      tty->print_cr("JVMDI Call %30s [%s]", str, thread_name);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, JvmdiMonitor *monitor) {
      ResourceMark rm;
      tty->print("  %s = ", parm);
      monitor->print(*tty);
      tty->print_cr("");
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, jlong l) {
      ResourceMark rm;
      tty->print_cr("  %s = %d", parm, (int) l);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, const char *s) {
      ResourceMark rm;
      tty->print_cr("  %s = %s", parm, s);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, oop obj) {
    ResourceMark rm;
    const char *class_name = (obj != NULL) ? obj->blueprint()->external_name() : "NULL";
    tty->print_cr("   %s = %s",parm,class_name);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, JavaThread *thread) {
      ResourceMark rm;
      const char *thread_name = (thread == NULL) 
                              ? "<NULL>" 
                              : JvmdiInternal::safe_get_thread_name(thread->threadObj()); 

      tty->print_cr("  %s = %s", parm, thread_name);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, int event_type, bool enabled) {
    ResourceMark rm;
    tty->print("  %s = ", parm);
    JvmdiEvent::write_kind(*tty,event_type);
    tty->print_cr(" %s ", enabled ? "enabled" : "disabled" );
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char* parm, methodOop m, jint bci) {
    ResourceMark rm;
    tty->print("  %s = ", parm);    
    const char *klass_name  = (m == NULL) ? "NULL" : m->klass_name()->as_C_string();
    const char *method_name = (m == NULL) ? "NULL" : m->name()->as_C_string();
    tty->print_cr("[%s.%s:%d]", klass_name, method_name, bci);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char *parm, JvmdiEvent& event) {
      ResourceMark rm;
      const char *thread_name = JvmdiInternal::safe_get_thread_name(JavaThread::current()->threadObj());
      tty->print("%s = ", parm);
      event.write(*tty);
      tty->print_cr("[%s]", thread_name);
  }

  JvmdiTraceWrapper::JvmdiTraceWrapper(const char *parm, JvmdiBreakpoint& bp) {
      ResourceMark rm;
      tty->print("  %s = ",parm);
      bp.print();
      tty->print_cr("");
  }

#endif 


//
// class GrowableCache - private methods
//

void GrowableCache::recache() {
  int len = _elements->length();

  FREE_C_HEAP_ARRAY(address, _cache);
  _cache = NEW_C_HEAP_ARRAY(address,len+1);

  for (int i=0; i<len; i++) {
    _cache[i] = _elements->at(i)->getCacheValue();
    //
    // The cache entry has gone bad. Without a valid frame pointer
    // value, the entry is useless so we simply delete it in product
    // mode. The call to remove() will rebuild the cache again
    // without the bad entry.
    //
    if (_cache[i] == NULL) {
      assert(false, "cannot recache NULL elements");
      remove(i);
      return;
    }
  }
  _cache[len] = NULL;

  _listener_fun(_this_obj,_cache);
}

bool GrowableCache::equals(void* v, GrowableElement *e2) {
  GrowableElement *e1 = (GrowableElement *) v;
  assert(e1 != NULL, "e1 != NULL");
  assert(e2 != NULL, "e2 != NULL");

  return e1->equals(e2);
}

//
// class GrowableCache - public methods
//

GrowableCache::GrowableCache() {
  _this_obj       = NULL;
  _listener_fun   = NULL;    
  _elements       = NULL;
  _cache          = NULL;
}

GrowableCache::~GrowableCache() {
  clear();
  _elements->clear_and_deallocate();
  FreeHeap(_elements);
  FREE_C_HEAP_ARRAY(address, _cache);
}

void GrowableCache::initialize(void *this_obj, void listener_fun(void *, address*) ) {
  _this_obj       = this_obj;
  _listener_fun   = listener_fun;    
  _elements       = new (ResourceObj::C_HEAP) GrowableArray<GrowableElement*>(5,true);
  recache();
}

// number of elements in the collection
int GrowableCache::length() { 
  return _elements->length(); 
}

// get the value of the index element in the collection
GrowableElement* GrowableCache::at(int index) {
  GrowableElement *e = (GrowableElement *) _elements->at(index);
  assert(e != NULL, "e != NULL");
  return e;
}
 
int GrowableCache::find(GrowableElement* e) {
  return _elements->find(e, GrowableCache::equals);
}

// append a copy of the element to the end of the collection
void GrowableCache::append(GrowableElement* e) {
  GrowableElement *new_e = e->clone();
  _elements->append(new_e);
  recache();
}

// insert a copy of the element using lessthan()
void GrowableCache::insert(GrowableElement* e) {
  GrowableElement *new_e = e->clone();
  _elements->append(new_e);

  int n = length()-2;
  for (int i=n; i>=0; i--) {
    GrowableElement *e1 = _elements->at(i);
    GrowableElement *e2 = _elements->at(i+1);
    if (e2->lessThan(e1)) {
      _elements->at_put(i+1, e1);
      _elements->at_put(i,   e2);
    }
  }

  recache();
}

// remove the element at index
void GrowableCache::remove (int index) {
  GrowableElement *e = _elements->at(index);
  assert(e != NULL, "e != NULL");
  _elements->remove(e);
  delete e;
  recache();
}

// clear out all elements, release all heap space and
// let our listener know that things have changed.
void GrowableCache::clear() {
  int len = _elements->length();
  for (int i=0; i<len; i++) {
    delete _elements->at(i);
  }
  _elements->clear();
  recache();
}

void GrowableCache::oops_do(OopClosure* f) {
  int len = _elements->length();
  for (int i=0; i<len; i++) {
    GrowableElement *e = _elements->at(i);
    e->oops_do(f);
  }
}

void GrowableCache::gc_epilogue() {
  int len = _elements->length();
  // recompute the new cache value after GC
  for (int i=0; i<len; i++) {
    _cache[i] = _elements->at(i)->getCacheValue();
  }
}

//
// class JvmdiObj
//

void *JvmdiObj::operator new(size_t size)   { return JvmdiInternal::jvmdiMalloc(size); }
void  JvmdiObj::operator delete(void *p)    { JvmdiInternal::jvmdiFree((jbyte *) p); }


//
// class JvmdiMonitor
//

JvmdiMonitor::JvmdiMonitor(const char *name, const int magic) {
#ifdef ASSERT
  _name = strcpy(NEW_C_HEAP_ARRAY(char, strlen(name) + 1), name);
#else
  _name = NULL;
#endif
  _magic = magic;
}

JvmdiMonitor::~JvmdiMonitor() {
#ifdef ASSERT
  FreeHeap(_name);
#endif
  _magic = 0;
}


//
// class JvmdiBreakpoint
//

JvmdiBreakpoint::JvmdiBreakpoint() {
  _method = NULL;
  _bci    = 0;
}

JvmdiBreakpoint::JvmdiBreakpoint(jclass clazz, jmethodID method, jlocation location) {
  klassOop klass = JvmdiInternal::get_klassOop(clazz);
  assert(klass != NULL, "klass != NULL");
  _method        = JvmdiInternal::get_methodOop(method, klass);
  assert(_method != NULL, "_method != NULL");
  _bci           = (int) location;  

  assert(_method->method_holder() == klass, "method holder not the klass passed in" );
  assert(_bci >= 0, "_bci >= 0"); 
}

void JvmdiBreakpoint::copy(JvmdiBreakpoint& bp) {
    _method   = bp._method;
    _bci      = bp._bci;
}

bool JvmdiBreakpoint::lessThan(JvmdiBreakpoint& bp) {
 Unimplemented();
 return false;
}

bool JvmdiBreakpoint::equals(JvmdiBreakpoint& bp) {
    return _method   == bp._method
      &&   _bci      == bp._bci;
}

bool JvmdiBreakpoint::is_valid() {
  return _method != NULL &&
         _bci >= 0;
}

address JvmdiBreakpoint::getBcp() {
  return _method->bcp_from(_bci);
}

void JvmdiBreakpoint::each_method_version_do(method_action meth_act) {
  assert(UseFastBreakpoints, "unexpected fast breakpoint");
  (_method->*meth_act)(_bci);

  // add/remove breakpoint to/from all EMCP previous
  // versions of the method
  Thread *thread = Thread::current();
  ResourceMark rm(thread);
  instanceKlassHandle ikh = instanceKlassHandle(thread, _method->method_holder());
  symbolOop meth_name = _method->name();
  symbolOop meth_signature = _method->signature();
  while (ikh->has_previous_version()) {
    ikh = instanceKlassHandle(thread, ikh->previous_version());
    methodOop m = ikh->find_method(meth_name, meth_signature);
    if (m == NULL) {
      assert(0, "method addition isn't implemented yet");
      return;  // method must have been added
    } else {
      // matching method found
      if (m->is_non_emcp_with_new_version()) {
        return; // don't set/clear in directly or transitively non-EMCP
      } else {
        (m->*meth_act)(_bci);
      }
    }
  }
}

void JvmdiBreakpoint::set() {
  each_method_version_do(&methodOopDesc::set_breakpoint);
}

void JvmdiBreakpoint::clear() {
  each_method_version_do(&methodOopDesc::clear_breakpoint);
}

void JvmdiBreakpoint::print() {
#ifndef PRODUCT
  const char *class_name  = (_method == NULL) ? "NULL" : _method->klass_name()->as_C_string();
  const char *method_name = (_method == NULL) ? "NULL" : _method->name()->as_C_string();

  tty->print("Breakpoint(%s,%s,%d,%p)",class_name, method_name, _bci, getBcp());
#endif
}




//
// class  BreakpointStepping - helper class,
//  turns bytecode_stepping on  when number of breakpoints changes from zero to non-zero
//  turns bytecode_stepping off when number of breakpoints changes from non-zero to zero
// Only active if !UseFastBreakpoints
//

class BreakpointStepping {
  JvmdiBreakpointCache *_bps;
  int _length;

public:
  BreakpointStepping(JvmdiBreakpointCache *bps) {
    assert(bps != NULL, "BreakpointStepping called with empty bkp list");
    _bps    = bps;
    _length = _bps->length();
  }

  ~BreakpointStepping() {
    // transition from no breakpoints to some breakpoints
    if (!UseFastBreakpoints) {
      if ((_length == 0) && (_bps->length() > 0)) {
        jvmdi::set_bytecode_stepping(true);
      } else if ((_length > 0) && (_bps->length() == 0) &&
                 !JvmdiEventDispatcher::is_enabled(JVMDI_EVENT_SINGLE_STEP)) {
        // transition from some breakpoints to no breakpoints
        // unless we are also single stepping
        jvmdi::set_bytecode_stepping(false);
      }
    }
  }

};

//
// class VM_ChangeBreakpoints
//
// Modify the Breakpoints data structure at a safepoint
//

void VM_ChangeBreakpoints::doit() {
  switch (_operation) {
   case SET_BREAKPOINT:
     _breakpoints->set_at_safepoint(*_bp);
     break;
   case CLEAR_BREAKPOINT:
     _breakpoints->clear_at_safepoint(*_bp);
     break;
   case CLEAR_ALL_BREAKPOINT:
     _breakpoints->clearall_at_safepoint();
     break;
  default:
    assert(false, "Unknown operation");
  }
}

//
// class JvmdiBreakpoints 
//
// a JVMDI internal collection of JvmdiBreakpoint
//

JvmdiBreakpoints::JvmdiBreakpoints(void listener_fun(void *,address *)) {
  _bps.initialize(this,listener_fun);
}

JvmdiBreakpoints:: ~JvmdiBreakpoints() {}

  void  JvmdiBreakpoints::oops_do(OopClosure* f) {  
    _bps.oops_do(f);
  } 

  void  JvmdiBreakpoints::gc_epilogue() {  
    _bps.gc_epilogue();
  } 

  void  JvmdiBreakpoints::print() {
#ifndef PRODUCT
    ResourceMark rm;

    int n = _bps.length();
    for (int i=0; i<n; i++) {
      JvmdiBreakpoint& bp = _bps.at(i);
      tty->print("%d: ", i);
      bp.print();
      tty->print_cr("");
    }
#endif
  }


void JvmdiBreakpoints::set_at_safepoint(JvmdiBreakpoint& bp) {
  BreakpointStepping bs(&_bps);
  int i = _bps.find(bp);
  if (i == -1) { 
    _bps.append(bp);
    if (UseFastBreakpoints)  bp.set();
  }
}

void JvmdiBreakpoints::clear_at_safepoint(JvmdiBreakpoint& bp) {
  BreakpointStepping bs(&_bps);
  int i = _bps.find(bp);
  if (i != -1) {
    _bps.remove(i);
     if (UseFastBreakpoints)  bp.clear();
  }
}

void JvmdiBreakpoints::clearall_at_safepoint() {
  BreakpointStepping bs(&_bps);
  if (UseFastBreakpoints) {
    int len = _bps.length();
    for (int i=0; i<len; i++) {
      _bps.at(i).clear();
    }
  }
  _bps.clear();
}
 
int JvmdiBreakpoints::length() { return _bps.length(); }

int JvmdiBreakpoints::set(JvmdiBreakpoint& bp) {
  if ( _bps.find(bp) != -1) {
     return JVMDI_ERROR_DUPLICATE;
  }
  VM_ChangeBreakpoints set_breakpoint(this,VM_ChangeBreakpoints::SET_BREAKPOINT, &bp);
  VMThread::execute(&set_breakpoint);
  return JVMDI_ERROR_NONE;
}

int JvmdiBreakpoints::clear(JvmdiBreakpoint& bp) {
  if ( _bps.find(bp) == -1) {
     return JVMDI_ERROR_NOT_FOUND;
  }

  VM_ChangeBreakpoints clear_breakpoint(this,VM_ChangeBreakpoints::CLEAR_BREAKPOINT, &bp);
  VMThread::execute(&clear_breakpoint);
  return JVMDI_ERROR_NONE;
}

#ifdef HOTSWAP
void JvmdiBreakpoints::clearall_in_class_at_safepoint(klassOop klass) {
  bool changed = true;
  // We are going to run thru the list of bkpts
  // and delete some.  This deletion probably alters
  // the list in some implementation defined way such
  // that when we delete entry i, the next entry might
  // no longer be at i+1.  To be safe, each time we delete
  // an entry, we'll just start again from the beginning.
  // We'll stop when we make a pass thru the whole list without
  // deleting anything.
  while (changed) {
    int len = _bps.length();
    changed = false;
    for (int i = 0; i < len; i++) {
      JvmdiBreakpoint& bp = _bps.at(i);
      if (bp.method()->method_holder() == klass) {
        bp.clear();
        _bps.remove(i);
        // This changed 'i' so we have to start over.
        changed = true;
        break;
      }
    }
  }
}
#endif HOTSWAP

void JvmdiBreakpoints::clearall() {
  VM_ChangeBreakpoints clearall_breakpoint(this,VM_ChangeBreakpoints::CLEAR_ALL_BREAKPOINT);
  VMThread::execute(&clearall_breakpoint);
}

//
// class JvmdiCurrentBreakpoints
// 

JvmdiBreakpoints *JvmdiCurrentBreakpoints::_jvmdi_breakpoints  = NULL;
address *         JvmdiCurrentBreakpoints::_breakpoint_list    = NULL;

void JvmdiCurrentBreakpoints::initialize() {}

void JvmdiCurrentBreakpoints::destroy() {
  delete _jvmdi_breakpoints;
  _jvmdi_breakpoints = NULL;
}


JvmdiBreakpoints& JvmdiCurrentBreakpoints::get_jvmdi_breakpoints() {
    if (_jvmdi_breakpoints != NULL) return (*_jvmdi_breakpoints);
    _jvmdi_breakpoints = new JvmdiBreakpoints(listener_fun);
    assert(_jvmdi_breakpoints != NULL, "_jvmdi_breakpoints != NULL");
    return (*_jvmdi_breakpoints);
}

void  JvmdiCurrentBreakpoints::listener_fun(void *this_obj, address *cache) { 
   JvmdiBreakpoints *this_jvmdi = (JvmdiBreakpoints *) this_obj;
   assert(this_jvmdi != NULL, "this_jvmdi != NULL");

   debug_only(int n = this_jvmdi->length(););
   assert(cache[n] == NULL, "cache must be NULL terminated");

   set_breakpoint_list(cache);
}


void JvmdiCurrentBreakpoints::oops_do(OopClosure* f) { 
  if (_jvmdi_breakpoints != NULL) {
    _jvmdi_breakpoints->oops_do(f);
  }
}

void JvmdiCurrentBreakpoints::gc_epilogue() { 
  if (_jvmdi_breakpoints != NULL) {
    _jvmdi_breakpoints->gc_epilogue();
  }
}


//
// class JvmdiEventEnabled
//

JvmdiEventEnabled::JvmdiEventEnabled(bool init_val) {
  for (int i=0; i<(JVMDI_MAX_EVENT_TYPE_VAL+1); i++) {
    _event_enabled[i] = init_val;
  }
}

bool JvmdiEventEnabled::is_enabled(jint event_type) {
  assert(jvmdi::is_valid_event_type(event_type), "invalid event type");
  return _event_enabled[event_type];
}

void JvmdiEventEnabled::set_enabled(jint event_type, bool enabled) {  
  assert(jvmdi::is_valid_event_type(event_type), "invalid event type");
  _event_enabled[event_type] = enabled;
}

//
// class VM_ChangeMethodEntryOrExit
//

// This class is only needed when fullspeed debugging is enabled.
// Enabling method entry or exit for a given thread requires switching
// that thread to interpreted-only mode; enabling either event
// globally requires all threads to be changed to interpreted-only
// mode, and furthermore, that all newly-created threads start in
// interpreted-only mode. In addition, these events can be enabled for
// threads which are not suspended. 

class VM_ChangeMethodEntryOrExit : public VM_Operation {
public:
  // A NULL thread indicates that the event is being enabled or
  // disabled globally
  VM_ChangeMethodEntryOrExit(JavaThread* thread,
                             int event,   // JVMDI_EVENT_METHOD_ENTRY or EXIT
                             bool enabled);
  
  // This can cause a VM_Deoptimize to be run
  bool allow_nested_vm_operations() const { return true; }
  const char* name() const { return "change method entry/exit"; }
  void doit();

private:
  JavaThread* _thread;
  int         _event;
  bool        _enabled;

  void change_interp_only_mode(JavaThread* thr, bool enabled);
};

VM_ChangeMethodEntryOrExit::VM_ChangeMethodEntryOrExit(JavaThread* thread,
                                                       int event,
                                                       bool enabled)
  : _thread(thread)
  , _event(event)
  , _enabled(enabled)
{
  assert(_event == JVMDI_EVENT_METHOD_ENTRY || _event == JVMDI_EVENT_METHOD_EXIT, "incorrect event");
}
  

void VM_ChangeMethodEntryOrExit::doit() {
  // Change event state

  if (_thread != NULL) {
    // Per-thread enable/disable
    bool changed = (_thread->jvmdi_thread_state()->is_enabled(_event) != _enabled);
    _thread->jvmdi_thread_state()->set_enabled(_event, _enabled);
    if (changed) {
      change_interp_only_mode(_thread, _enabled);
    }
  } else {
    // Global enable/disable
    bool changed = (JvmdiEventDispatcher::_global_event_enabled.is_enabled(_event) != _enabled);
    JvmdiEventDispatcher::_global_event_enabled.set_enabled(_event, _enabled);
    if (changed) {
      // Threads_lock is already held as part of this VM operation
      for (JavaThread* thr = Threads::first(); thr != NULL; thr = thr->next()) {
        JavaThreadState state = thr->thread_state();
        // skip threads that haven't run yet
        if (state == _thread_new || state == _thread_new_trans) {
          continue;
        }
        change_interp_only_mode(thr, _enabled);
      }
    }
  }
}

void VM_ChangeMethodEntryOrExit::change_interp_only_mode(JavaThread* thr, bool enabled) {
  if (enabled) {
    thr->jvmdi_thread_state()->enable_interp_only_mode();
  } else {
    thr->jvmdi_thread_state()->disable_interp_only_mode();
  }
}

//
// class VM_ChangeFieldWatch
//

// This class is only needed when fullspeed debugging is enabled.
// Setting a watchpoint requires switching all threads to 
// interpreted-only mode; and furthermore, that all
// newly-created threads start in interpreted-only mode. In
// addition, watchpoints may be set while
// threads are not suspended. 

class VM_ChangeFieldWatch : public VM_Operation {
public:
  VM_ChangeFieldWatch(int event,   // JVMDI_EVENT_FIELD_MODIFICATION or ACCESS
                      bool enabled);
  
  // This can cause a VM_Deoptimize to be run
  bool allow_nested_vm_operations() const { return true; }
  const char* name() const { return "change field watch"; }
  void doit();

private:
  int         _event;
  bool        _enabled;

  void update();
  void change_interp_only_mode(JavaThread* thr, bool enabled);
};

VM_ChangeFieldWatch::VM_ChangeFieldWatch(int event,
                                         bool enabled)
  : _event(event)
  , _enabled(enabled)
{
  assert(_event == JVMDI_EVENT_FIELD_MODIFICATION || _event == JVMDI_EVENT_FIELD_ACCESS, 
         "incorrect event");
}
  

void VM_ChangeFieldWatch::doit() {
  int *count_addr;
  switch (_event) {
    case JVMDI_EVENT_FIELD_MODIFICATION:
      count_addr = (int *)jvmdi::get_field_modification_count_addr();
      break;
    case JVMDI_EVENT_FIELD_ACCESS:
      count_addr = (int *)jvmdi::get_field_access_count_addr();
      break;
    default:
      /* see assert in constructor */
      return;
  }
  if (_enabled) {
    (*count_addr)++;
    if ((*count_addr) == 1) {
      update();
    }
  } else {
    if ((*(count_addr)) > 0) {
      (*count_addr)--;
      if ((*count_addr) == 0) {
        update();
      }
    } else {
      assert(false, "field watch out of phase");
    }
  }
}

void VM_ChangeFieldWatch::update() {
  // Change event state
  if (FullSpeedJVMDI) {
    for (JavaThread* thr = Threads::first(); thr != NULL; thr = thr->next()) {
      change_interp_only_mode(thr, _enabled);
    }
  }
}

void VM_ChangeFieldWatch::change_interp_only_mode(JavaThread* thr, bool enabled) {
  if (enabled) {
    thr->jvmdi_thread_state()->enable_interp_only_mode();
  } else {
    thr->jvmdi_thread_state()->disable_interp_only_mode();
  }
}

//
// class JvmdiEventDispatcher
//

// create with all global events enabled, turn some off in initialize()
JvmdiEventEnabled JvmdiEventDispatcher::_global_event_enabled(true);
debug_only(static bool jvmdi_event_dispatcher_initialized = false;)

void JvmdiEventDispatcher::initialize() {
  assert(!jvmdi_event_dispatcher_initialized, "Should only initialize JvmdiEventDispatcher once");
  // set some global events off by default (from JVMDI spec)
  _global_event_enabled.set_enabled(JVMDI_EVENT_SINGLE_STEP,false);
  _global_event_enabled.set_enabled(JVMDI_EVENT_EXCEPTION_CATCH,false);
  _global_event_enabled.set_enabled(JVMDI_EVENT_METHOD_ENTRY,false);
  _global_event_enabled.set_enabled(JVMDI_EVENT_METHOD_EXIT,false);
  debug_only(jvmdi_event_dispatcher_initialized = true;)
}

void JvmdiEventDispatcher::destroy() {
}

static void get_current_location(JavaThread* thread, JNIid** method_id, int* bci) {
  ResourceMark rmark; // thread != Thread::current()
  RegisterMap rm(thread, false);
  javaVFrame* vf = thread->last_java_vframe(&rm);
  assert(vf != NULL, "must have last java frame");
  methodOop method = vf->method();
  *method_id = method->jni_id();
  *bci       = vf->bci();
}


void JvmdiEventDispatcher::reset_thread_location(JavaThread *thread, jint event, bool enabled) {
  assert(event == JVMDI_EVENT_SINGLE_STEP || event == JVMDI_EVENT_BREAKPOINT, "must be single-step or breakpoint event");

  // Thread location is used to detect the following:
  // 1) a breakpoint event followed by single-stepping to the same bci
  // 2) single-step to a bytecode that will be transformed to a fast version
  // We skip to avoid posting the duplicate single-stepping event.

  // If single-stepping is disabled, reset thread location so that 
  // single-stepping to the same method and bcp at a later time will be
  // detected if single-stepping is enabled at that time (see 4388912).

  // If single-stepping is enabled, set the thread location to the
  // current method and bcp. This covers the following type of case,
  // e.g., the debugger stepi command:
  // - bytecode single stepped
  // - SINGLE_STEP event posted and SINGLE_STEP event disabled
  // - SINGLE_STEP event reenabled
  // - bytecode rewritten to fast version

  // If breakpoint event is disabled, reset thread location only if
  // single-stepping is not enabled.  Otherwise, keep the thread location
  // to detect any duplicate events.

  // If enabling breakpoint, no need to reset.
  if (event == JVMDI_EVENT_BREAKPOINT && enabled) {
    return;
  }

  // Reset thread-specific location info
  if (thread != NULL) {
    JvmdiThreadState *state = thread->jvmdi_thread_state();
    JNIid* method_id = NULL;
    int bci = 0;
    if (enabled && thread->has_last_Java_frame()) {
      assert(event == JVMDI_EVENT_SINGLE_STEP, "enable BREAKPOINT should be done already");
      get_current_location(thread, &method_id, &bci);
    }
    state->set_current_location(method_id, bci);
  } else {
    // reset location info for all threads when stepping or breakpoint is disabled globally.
    MutexLocker tl(Threads_lock);
    for(JavaThread *jthread = Threads::first(); jthread != NULL; jthread = jthread->next()) {
      JvmdiThreadState *state = jthread->jvmdi_thread_state();
      if (enabled && jthread->has_last_Java_frame()) {
        assert(event == JVMDI_EVENT_SINGLE_STEP, "enable BREAKPOINT should be done already");
        JNIid* method_id = NULL;
        int bci = 0;
        get_current_location(jthread, &method_id, &bci);
        state->set_current_location(method_id, bci);
      } else if (!enabled && !state->is_enabled(event)) {
        // The event is disabled globally but we need to check the event is also not enabled
        // in thread level.  If this is to disable breakpoint, also check if single-step
        // is not enabled in the thread level.
        if (event == JVMDI_EVENT_SINGLE_STEP || !JvmdiEventDispatcher::is_enabled(jthread, JVMDI_EVENT_SINGLE_STEP)) {
          state->set_current_location(NULL, 0);
        }
      }
 
    }
  }
}

void JvmdiEventDispatcher::set_enabled(JavaThread *thread, jint event_type, bool enabled) {
  // Method entry and exit events require special handling in fullspeed mode
  if (FullSpeedJVMDI) {
    if (event_type == JVMDI_EVENT_METHOD_ENTRY || event_type == JVMDI_EVENT_METHOD_EXIT) {
      VM_ChangeMethodEntryOrExit op(thread, event_type, enabled);
      VMThread::execute(&op);
      return;
    }
  }

  if (thread == NULL) {
    _global_event_enabled.set_enabled(event_type, enabled);   
  } else if (!(is_global_event(event_type))) { // global events cannot be toggled on threads
    thread->jvmdi_thread_state()->set_enabled(event_type, enabled);   
  }

  // Both single stepping and breakpoints use bytecode stepping
  if (event_type == JVMDI_EVENT_SINGLE_STEP ||
      event_type == JVMDI_EVENT_BREAKPOINT) {
    bool should_step = is_enabled(JVMDI_EVENT_SINGLE_STEP) || (!UseFastBreakpoints && is_enabled(JVMDI_EVENT_BREAKPOINT));
    jvmdi::set_bytecode_stepping(should_step);
    // reset thread-specific location info when single-stepping or breakpoint event is enabled/disabled.
    reset_thread_location(thread, event_type, enabled);
  }
}

bool JvmdiEventDispatcher::is_enabled(jint event_type) {
  if (_global_event_enabled.is_enabled(event_type)) {
    return true;
  }

  if (is_global_event(event_type)) {
    return false; // no need to check on individual threads
  }

  { 
    bool need_threads_lock = !Thread::current()->is_VM_thread();
    assert(need_threads_lock || Threads_lock->owner()->is_VM_thread(), "VM thread must own Threads_lock");
    MutexLockerEx mu(need_threads_lock ? Threads_lock : NULL);
    JavaThread *java_thread = Threads::first();
    while (java_thread != NULL) {
      JvmdiThreadState *thread_state = java_thread->jvmdi_thread_state();
      if ((thread_state != NULL) && (thread_state->is_enabled(event_type))) {
        return true;
      }    
      java_thread = java_thread->next();
    }
  }

  return false;
}

bool JvmdiEventDispatcher::is_enabled(JavaThread *thread, jint event_type) {
  if (_global_event_enabled.is_enabled(event_type)) {
    return true;
  }
 
  if (is_global_event(event_type)) {
    return false;  // no need to check on the thread 
  }
  return thread->jvmdi_thread_state()->is_enabled(event_type);
}

bool JvmdiEventDispatcher::is_globally_enabled(jint event_type) {
  return (_global_event_enabled.is_enabled(event_type));
}

//
// class JvmdiInternal
//

bool              JvmdiInternal::_jvmdi_initialized = false;
JVMDI_Interface_1 JvmdiInternal::_jvmdi_interface;
JVMDI_EventHook   JvmdiInternal::_event_hook  = (JVMDI_EventHook)JvmdiInternal::default_event_hook;
JavaVM*           JvmdiInternal::_vm          = NULL;
JVMDI_AllocHook   JvmdiInternal::_ahook       = NULL;
JVMDI_DeallocHook JvmdiInternal::_dhook       = NULL;


//
// Useful for debugging jvmdi.cpp:
// default_event_hook is only called when SetEventHook is not used.
//
void JvmdiInternal::default_event_hook(JNIEnv *env, JVMDI_Event *event) {
  assert(jvmdi::enabled(), "jvmdi::enabled()");  
  Unimplemented();
}

void JvmdiInternal::jvmdi_init() {
  assert(jvmdi::enabled(), "jvmdi::enabled()" );
}

void JvmdiInternal::jvmdi_init_done() {
  _jvmdi_initialized = true;
}

void JvmdiInternal::jvmdi_death() {
  assert(jvmdi::enabled(), "jvmdi::enabled()");
  jvmdi::destroy();
}

void  JvmdiInternal::set_allocation_hooks(JVMDI_AllocHook ahook, JVMDI_DeallocHook dhook) {
  _ahook = ahook;
  _dhook = dhook;
}


//
// Utility functions that manage memory by calling into JVMDI.
// You should Never use os::malloc/free directly since the allocation
// hooks may have been redirected.
//

jbyte *JvmdiInternal::jvmdiMalloc(jlong size) {
  jbyte *mem;
  jvmdiError result = AllocateInternal(size,&mem);
  assert(result == JVMDI_ERROR_NONE, "Allocate failed");
  return mem;
}

void JvmdiInternal::jvmdiFree(jbyte *mem) {
  DeallocateInternal(mem);
}

// Called from JVMDI entry points which perform stack walking. If the
// associated JavaThread is the current thread, then wait_for_suspend
// is not used. Otherwise, it determines if we should wait for the
// "other" thread to complete external suspension. (NOTE: in future
// releases the suspension mechanism should be reimplemented so this
// is not necessary.)
//
bool JvmdiInternal::is_thread_fully_suspended(JavaThread* thr, bool wait_for_suspend) {
  NEEDS_CLEANUP

  // "other" threads require special handling
  if (thr != JavaThread::current()) {
    if (wait_for_suspend) {
      // We are allowed to wait for the external suspend to complete
      // so give the other thread a chance to get suspended.
      if (!thr->wait_for_ext_suspend_completion(SUSPENDRETRYCOUNT)) {
        // didn't make it so let the caller know
        return false;
      }
    }
    // We aren't allowed to wait for the external suspend to complete
    // so if the other thread isn't externally suspended we need to
    // let the caller know.
    else if (!thr->is_ext_suspended_equivalent_with_lock()) {
      return false;
    }
  }

  return true;
}

///////////////////////////////////////////////////////////////



//
// class JvmdiFrame - public methods
//

vframe *JvmdiFrame::get_vframe() {
  RegisterMap reg_map(get_thread());
  vframe *vf = get_thread()->last_java_vframe(&reg_map);
  int d = 0;
  while ((vf != NULL) && (d < get_depth())) {
    vf = vf->java_sender();
    d++;
  }
  return vf;
}

bool JvmdiFrame::verify() {
  return (get_thread()->has_last_Java_frame());
}

// local setter / getters
// Todo: Add type-checking
// Todo: make setters set

jvmdiError JvmdiFrame::get_local(jint n, oop *o) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, T_OBJECT);
  VMThread::execute(&op);
  *o = op.oop_value();
  return op.result();
}

jvmdiError JvmdiFrame::get_local(jint n, jint *i) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, T_INT);
  VMThread::execute(&op);
  *i = op.value().i;
  return op.result();
}

jvmdiError JvmdiFrame::get_local(jint n, jlong *l) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, T_LONG);
  VMThread::execute(&op);
  *l = op.value().j;
  return op.result();
}

jvmdiError JvmdiFrame::get_local(jint n, jfloat *f) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, T_FLOAT);
  VMThread::execute(&op);
  *f = op.value().f;
  return op.result();
}

jvmdiError JvmdiFrame::get_local(jint n, jdouble *d) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, T_DOUBLE);
  VMThread::execute(&op);
  *d = op.value().d;
  return op.result();
}

jvmdiError JvmdiFrame::set_local(jint n, Handle* o) {
  MutexUnlocker mu(Threads_lock);
  VM_GetOrSetLocal op(this, n, o);
  VMThread::execute(&op);
  return op.result();
}

jvmdiError JvmdiFrame::set_local(jint n, jint i) {
  MutexUnlocker mu(Threads_lock);
  jvalue val;
  val.i = i;
  VM_GetOrSetLocal op(this, n, T_INT, val);
  VMThread::execute(&op);
  return op.result();
}

jvmdiError JvmdiFrame::set_local(jint n, jlong l) {
  MutexUnlocker mu(Threads_lock);
  jvalue val;
  val.j = l;
  VM_GetOrSetLocal op(this, n, T_LONG, val);
  VMThread::execute(&op);
  return op.result();
}

jvmdiError JvmdiFrame::set_local(jint n, jfloat f) {
  MutexUnlocker mu(Threads_lock);
  jvalue val;
  val.f = f;
  VM_GetOrSetLocal op(this, n, T_FLOAT, val);
  VMThread::execute(&op);
  return op.result();
}

jvmdiError JvmdiFrame::set_local(jint n, jdouble d) {
  MutexUnlocker mu(Threads_lock);
  jvalue val;
  val.d = d;
  VM_GetOrSetLocal op(this, n, T_DOUBLE, val);
  VMThread::execute(&op);
  return op.result();
}


VM_GetOrSetLocal::VM_GetOrSetLocal(JvmdiFrame *f, int index, BasicType type)
  : _f(f)
  , _index(index)
  , _type(type)
  , _set(false)
{  
}

VM_GetOrSetLocal::VM_GetOrSetLocal(JvmdiFrame *f, int index, BasicType type, jvalue value)
  : _f(f)
  , _index(index)
  , _type(type)
  , _value(value)
  , _set(true)
{
}

VM_GetOrSetLocal::VM_GetOrSetLocal(JvmdiFrame *f, int index, Handle* value)
  : _f(f)
  , _index(index)
  , _type(T_OBJECT)
  , _obj(value)
  , _set(true)
{
}

bool VM_GetOrSetLocal::check_index(StackValueCollection* c, int i) {
  if (i < 0 || i >= c->size()) {
    _result = JVMDI_ERROR_INVALID_SLOT;
    return false;
  }
  return true;
}

#ifndef CORE
static bool can_be_deoptimized(vframe* vf) {
  return (vf->is_compiled_frame() && ((compiledVFrame*) vf)->code()->can_be_deoptimized());
}
#endif /* CORE */


void VM_GetOrSetLocal::doit() {
  vframe* vf = _f->get_vframe();
  if (vf == NULL) {
    _result = JVMDI_ERROR_INVALID_FRAMEID;
    return;
  }

  if (vf->is_java_frame()) {
    javaVFrame *jvf = (javaVFrame*)vf;

    if (jvf->method()->is_native()) {
      _result = JVMDI_ERROR_OPAQUE_FRAME;
      return;
    }

    if (_set) {
#ifndef CORE
      // Force deoptimization of frame if compiled because it's
      // possible the compiler emitted some locals as constant values,
      // meaning they are not mutable.
      if (FullSpeedJVMDI) {
        if (can_be_deoptimized(jvf)) {
          ((compiledVFrame*) jvf)->code()->mark_for_deoptimization();
          VM_Deoptimize op;
          VMThread::execute(&op);
          // Re-fetch vframe after deoptimization
          vf = _f->get_vframe();
          assert(vf->is_java_frame(), "check");
          jvf = (javaVFrame*) vf;
        }
      }
#endif /* CORE */
      StackValueCollection *locals = jvf->locals();

      if (!check_index(locals, _index)) { return; }

      switch (_type) {
      case T_INT:    locals->set_int_at   (_index, _value.i); break;
      case T_LONG:   locals->set_long_at  (_index, _value.j); break;
      case T_FLOAT:  locals->set_float_at (_index, _value.f); break;
      case T_DOUBLE: locals->set_double_at(_index, _value.d); break;
      case T_OBJECT: locals->set_obj_at   (_index, *_obj);  break;
      default: ShouldNotReachHere();
      }
      jvf->set_locals(locals);
    } else {
      StackValueCollection *locals = jvf->locals();

      if (!check_index(locals, _index)) { return; }

      switch (_type) {
      case T_INT:    _value.i = locals->int_at   (_index);   break;
      case T_LONG:   _value.j = locals->long_at  (_index);   break;
      case T_FLOAT:  _value.f = locals->float_at (_index);   break;
      case T_DOUBLE: _value.d = locals->double_at(_index);   break;
      case T_OBJECT: _oop_result = locals->obj_at(_index)(); break;
      default: ShouldNotReachHere();
      }
    }
    
    _result = JVMDI_ERROR_NONE;
  } else {
    // Actually should not reach here
    _result = JVMDI_ERROR_OPAQUE_FRAME;
  }
}


bool VM_GetOrSetLocal::allow_nested_vm_operations() const {
  return true; // May need to deoptimize
}

/////////////////////////////////////////////////////////////////////////////////////////

//
// class JvmdiThreads - see comments in jvmdi_impl.hpp
//

jframeID JvmdiThreads::_last_jframeID = 0;

//
// private methods (class JvmdiThreads)
//


JvmdiFrame *JvmdiThreads::get_frame(jframeID fid) {
  JavaThread *java_thread = get_thread(fid);
  if (java_thread == NULL) return NULL;  // can't find walkable thread

  JvmdiFrame *jvmdi_frame = java_thread->jvmdi_thread_state()->from_jframeID(fid);
  assert(jvmdi_frame != NULL, "jvmdi_frame != NULL");
  return jvmdi_frame;
}


JavaThread *JvmdiThreads::get_thread(jframeID fid) {  
  assert_locked_or_safepoint(Threads_lock);

  for (JavaThread *thr = Threads::first(); thr != NULL; thr = thr->next()) {
    if (thr->jvmdi_thread_state()->from_jframeID(fid) != NULL) return thr;
  }
  return NULL;
}


bool JvmdiThreads::check_thread(JavaThread* thread) {
  return (thread->jvmdi_thread_state()->may_be_walked());
}


//
// public (synchronized) methods (class JvmdiThreads)
//


jframeID JvmdiThreads::next_jframeID() {
  jint next = 1 + (intptr_t)(_last_jframeID);
  if (next < 0) {
    next = 1;
  }
  _last_jframeID = (jframeID)next;
  return _last_jframeID;
}

bool JvmdiThreads::suspend(JavaThread *java_thread) {  
    		// hSuspendThread should have caught suspending a thread twice

  // Immediate suspension required for jdb so JVM/DI agent threads do
  // not deadlock due to later suspension on transitions while holding
  // raw monitors.  Passing true causes the immediate suspension.
  // java_suspend() will catch threads in the process of exiting
  // and will ignore them.
  java_thread->java_suspend(UseForcedSuspension);

  // It would be nice to have the following assertion in all the time,
  // but it is possible for a racing resume request to have resumed
  // this thread right after we suspended it. Temporarily enable this
  // assertion if you are chasing a different kind of bug.
  //
  // assert(java_lang_Thread::thread(java_thread->threadObj()) == NULL ||
  //   java_thread->is_being_ext_suspended(), "thread is not suspended");

  if (java_lang_Thread::thread(java_thread->threadObj()) == NULL) {
    // check again because we can get delayed in java_suspend():
    // the thread is in process of exiting.
    return false;
  }

  return true;
}

bool JvmdiThreads::resume(JavaThread *java_thread) {  
		// hResumeThread should have caught resuming a thread twice
  assert(java_thread->is_being_ext_suspended(), "thread should be suspended");

  java_thread->jvmdi_thread_state()->clear_cached_frames();

  // resume thread
  {
    // must always grab Threads_lock, see JVM_SuspendThread
    MutexLocker ml(Threads_lock);  
    java_thread->java_resume(); 
  }
 
  return true;
}

bool JvmdiThreads::suspended(JavaThread *java_thread) {
  MutexLocker mu(Threads_lock);

  return (java_thread->is_being_ext_suspended());
}

jframeID JvmdiThreads::current_frame(JavaThread *java_thread) {
  MutexLocker mu(Threads_lock);

  if (!check_thread(java_thread)) return JvmdiThreadState::invalid_jframeID();
  return java_thread->jvmdi_thread_state()->current_frame();
}

int JvmdiThreads::nFrames(JavaThread *java_thread) {
  MutexLocker mu(Threads_lock);

  if (!check_thread(java_thread)) return -1;
  return java_thread->jvmdi_thread_state()->nFrames();
}

jframeID JvmdiThreads::get_caller(jframeID fid) {
  MutexLocker mu(Threads_lock);

  JavaThread *java_thread = get_thread(fid);
  if (java_thread == NULL) {
    // can't find walkable thread
    return JvmdiThreadState::invalid_jframeID();
  }

  return java_thread->jvmdi_thread_state()->get_caller(fid);
}

bool JvmdiThreads::is_opaque(jframeID fid, bool return_true_for_native_frames) {
  MutexLocker mu(Threads_lock);

  JavaThread *java_thread = get_thread(fid);
  if (java_thread == NULL) {
    // fid is not related to a walkable thread so consider it opaque.
    // fid could refer to a non-Java frame or to an invalid frame.
    return true;
  }
  return java_thread->jvmdi_thread_state()->is_opaque(fid, return_true_for_native_frames);
}


bool JvmdiThreads::get_frame_location(jframeID fid, jclass *klass, jmethodID *method, jlocation *location) {
  jint         bci;
  HandleMark   hm;
  methodHandle hmethod;

  {
    MutexLocker mu(Threads_lock);

    JavaThread *java_thread = get_thread(fid);
    if (java_thread == NULL) return false;  // can't find walkable thread


    if (!java_thread->jvmdi_thread_state()->get_frame_location(fid, &hmethod, &bci)) {
      return false;
    }
  }
  JVMDI_TRACE_ARG3("returns location", hmethod(), bci);

  *location = bci;
  
  if (bci >= 0) {
    *klass  = JvmdiInternal::get_jni_class_non_null(hmethod->method_holder());
#ifdef HOTSWAP
    if (hmethod->is_non_emcp_with_new_version())
      // For old non-EMCP versions of methods of evolved classes we return obsolete
      *method = OBSOLETE_METHOD_ID;
    else
      *method = JvmdiInternal::get_jni_method_non_null(hmethod());      
#else
    *method = JvmdiInternal::get_jni_method_non_null(hmethod());
#endif HOTSWAP
  } else {
    *klass  = NULL;
    *method = NULL;
  }

  return true;
}


bool JvmdiThreads::notify_frame_pop(jframeID fid) {
  if (TraceJVMDICalls) {
    ResourceMark rm;
    print();
  }

  ResourceMark rm;
  JvmdiThreadState *thread_state = NULL;
  JvmdiFrame *      jvmdi_frame  = NULL;
  vframe *                   vf  = NULL;
  frame *                     f  = NULL;
  JavaThread *       java_thread = NULL;

  { MutexLocker mu(Threads_lock);

    jvmdi_frame = get_frame(fid);
    if (jvmdi_frame == NULL) { return false; }

    vf = jvmdi_frame->get_vframe();
    assert(vf != NULL, "stale frame should have been caught above");

    f = vf->frame_pointer();
    assert(f != NULL, "f != NULL");

    java_thread = jvmdi_frame->get_thread();
    assert(java_thread != NULL, "java_thread != NULL");

    thread_state = java_thread->jvmdi_thread_state();
    assert(thread_state != NULL, "thread_state != NULL");
  }

  { // set the frame pop in our threadstate
    JvmdiFramePop fp(count_thread_num_frames(java_thread) - jvmdi_frame->get_depth() - 1);
    thread_state->get_jvmdi_frame_pops()->set(fp);
  }

  // The method_exit_on flag is enabled when either a FramePop is set
  // or interp_only_mode is set by other events.
  // Check here to make sure.
  assert(*((bool*) jvmdi::get_method_exit_on_addr()), "method_exit_on must be set");

#ifndef PRODUCT
  if (TraceJVMDICalls) {
    tty->print_cr("JvmdiThreads::notify_frame_pop:");
    print_frame_pops(fid);
  }
#endif

  return true;
}

jvmdiError JvmdiThreads::get_local(jframeID fid, jint n, Handle *h) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }

  oop obj;
  jvmdiError res;
  if ((res = jvmdi_frame->get_local(n, &obj)) == JVMDI_ERROR_NONE) {
    *h = Handle(Thread::current(), obj);
  }

  return res;
}

jvmdiError JvmdiThreads::get_local(jframeID fid, jint n, jint *i) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->get_local(n, i);
}

jvmdiError JvmdiThreads::get_local(jframeID fid, jint n, jlong *l) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->get_local(n, l);
}


jvmdiError JvmdiThreads::get_local(jframeID fid, jint n, jfloat *f) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->get_local(n, f);
}

jvmdiError JvmdiThreads::get_local(jframeID fid, jint n, jdouble *d) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->get_local(n, d);
}

jvmdiError JvmdiThreads::set_local(jframeID fid, jint n, Handle* object_handle) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->set_local(n, object_handle);
}

jvmdiError JvmdiThreads::set_local(jframeID fid, jint n, jint i) {
  MutexLocker mu(Threads_lock);
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->set_local(n, i);
}

jvmdiError JvmdiThreads::set_local(jframeID fid, jint n, jlong l) {
  MutexLocker mu(Threads_lock); 
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->set_local(n, l);
}

jvmdiError JvmdiThreads::set_local(jframeID fid, jint n, jfloat f) {
  MutexLocker mu(Threads_lock); 
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->set_local(n, f);
}

jvmdiError JvmdiThreads::set_local(jframeID fid, jint n, jdouble d) {
  MutexLocker mu(Threads_lock); 
  JvmdiFrame * jvmdi_frame = get_frame(fid);
  if (jvmdi_frame == NULL) { return JVMDI_ERROR_INVALID_FRAMEID; }
  return jvmdi_frame->set_local(n, d);
}


void JvmdiThreads::print_frame_pops(jframeID fid) {
#ifndef PRODUCT
  MutexLocker mu(Threads_lock);
  ResourceMark rm;

  JavaThread *java_thread = get_thread(fid);
  if (java_thread != NULL) {
    java_thread->jvmdi_thread_state()->get_jvmdi_frame_pops()->print();
  }
#endif
}

// Todo: This could replace JavaThread::get_thread_name() if the extra test is acceptable
const char *JvmdiInternal::safe_get_thread_name(oop threadObj) {
    if (threadObj == NULL) {
      return "NULL";
    }
    typeArrayOop name = java_lang_Thread::name(threadObj);
    if (name == NULL) {
      return "<NOT FILLED IN>";
    }
    return UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length());
}

void JvmdiThreads::print() {
#ifndef PRODUCT
  MutexLocker mu(Threads_lock);
  ResourceMark rm;

  tty->print("Suspended Threads: [");
  JavaThread *st      = Threads::first();
  while (st != NULL) {

    const char *name   = JvmdiInternal::safe_get_thread_name(st->threadObj());
    int nFrames    = st->jvmdi_thread_state()->nFrames();

    tty->print("%s(%c ",name, st->is_being_ext_suspended() ? 'S' : '_');
    if (!st->jvmdi_thread_state()->has_last_frame()) {
      tty->print("no stack");
    }
    tty->print(") ");
    st = st->next();
  }
  tty->print_cr("]");
  
#endif  
}


JRT_LEAF(void, jvmdi::set_cur_thread_in_native_code())
{
  JavaThread* thread = JavaThread::current();
  thread->jvmdi_thread_state()->set_in_native_code();
}
JRT_END


JRT_LEAF(void, jvmdi::clear_cur_thread_in_native_code())
{
  JavaThread* thread = JavaThread::current();
  thread->jvmdi_thread_state()->clear_in_native_code();
}
JRT_END


JRT_LEAF(bool, jvmdi::is_cur_thread_in_native_code())
{
  JavaThread* thread = JavaThread::current();
  return thread->jvmdi_thread_state()->is_in_native_code();
}
JRT_END


//
// class JvmdiPostEventMark
//

class JvmdiPostEventMark : public ResourceObj {
private:
  StateSaver _state_saver;
  JavaThread *_thread;

public:
  JvmdiPostEventMark(JavaThread *thread)
  : _state_saver(thread)
  , _thread(thread) {
    assert(thread == JavaThread::current(), "thread must be current!");
  }

  ~JvmdiPostEventMark()     {  
    _thread->jvmdi_thread_state()->clear_cached_frames();
  }

  jframeID current_frame()       {
    return JvmdiThreads::current_frame(_thread); 
  }
};

// Helper routine used in several places below
static int count_thread_num_frames(JavaThread* thr) {
  if (!thr->has_last_Java_frame()) return 0;  // no Java frames

  ResourceMark rm;
  RegisterMap reg_map(thr);
  vframe *vf = thr->last_java_vframe(&reg_map);
  int n = 0;
  while (vf != NULL) {
    vf = vf->java_sender();
    n++;
  }
  return n;
}

//
// class JvmdiFramePop 
//

#ifndef PRODUCT
void JvmdiFramePop::print() {
  tty->print_cr("_frame_number=%d", _frame_number);
}
#endif  


//
// class VM_ChangeFramePops
//
// Alter a JvmdiFramePops object at a safepoint
//

void VM_ChangeFramePops::doit() {
  _frame_pops->set_at_safepoint(_fp);
}

bool VM_ChangeFramePops::allow_nested_vm_operations() const {
  return true;
}

//
// class JvmdiFramePops - private methods
//

void JvmdiFramePops::set_at_safepoint(JvmdiFramePop& fp) {
  if (_pops.find(fp.frame_number()) < 0) {
    _pops.append(fp.frame_number());
    threadstate()->enable_interp_only_mode();
  }
}

//
// class JvmdiFramePops - public methods
//

JvmdiFramePops::JvmdiFramePops(JvmdiThreadState *threadstate)
  : _threadstate(threadstate)
  , _pops(2, true)
{
}

void JvmdiFramePops::clear(JvmdiFramePop& fp) {
  assert(_pops.length() > 0, "No more frame pops");

  assert(JvmdiInternal::is_thread_fully_suspended(_threadstate->get_thread(), false /* don't wait for suspend completion */),
         "Only thread itself can clear frame pop or when a thread is suspended");

  _pops.remove(fp.frame_number());
  threadstate()->disable_interp_only_mode();
}

void JvmdiFramePops::clear_to(JvmdiFramePop& fp) {
  assert(JavaThread::current() == _threadstate->get_thread(), "Only thread itself can clear frame pop");

  int index = 0;
  while (index < _pops.length()) {
    JvmdiFramePop pop = JvmdiFramePop(_pops.at(index));
    if (pop.above_on_stack(fp)) {
      _pops.remove_at(index);
      threadstate()->disable_interp_only_mode();
    } else {
      ++index;
    }
  }
}
    
void JvmdiFramePops::set(JvmdiFramePop& fp) {
  VM_ChangeFramePops set_frame_pop(this, fp);
  VMThread::execute(&set_frame_pop);

#ifndef PRODUCT
  if (TraceJVMDICalls) {
    tty->print_cr("JvmdiFramePops::set:");
    print();
  }
#endif
}

#ifndef PRODUCT
void JvmdiFramePops::print() {
  ResourceMark rm;

  int n = _pops.length();
  for (int i=0; i<n; i++) {
    JvmdiFramePop fp = JvmdiFramePop(_pops.at(i));
    tty->print("%d: ", i);
    fp.print();
    tty->print_cr("");
  }
}
#endif


///////////////////////////////////////////////////////////////
//
// class JvmdiThreadState
//
// Instances of JvmdiThreadState hang off of each thread.
// Thread local storage for the debugger.
//

JvmdiThreadState::JvmdiThreadState(JavaThread *thread)   { 
  _local_event_enabled = NULL;
  _thread              = thread;
  _jvmdi_frame_pops    = NULL;
  _exception_detected  = false;
  _exception_caught    = false;
  _in_native_code      = false;
  _debuggable          = true;
  _current_bci         = 0;
  _current_method_id   = NULL;
  _breakpoint_posted   = false;
  _single_stepping_posted = false;
  _hide_single_stepping = false;
  _hide_level          = 0;
  _interp_only_mode    = 0;
  _pending_step_for_popframe = false;
  
  
#ifndef PRODUCT
  _monitors            = NULL;
  _jvmdi_count         = 0;
#endif

  _frames              = new (ResourceObj::C_HEAP) GrowableArray<JvmdiFrame *>(5,true);
}

JvmdiThreadState::~JvmdiThreadState()   { 
  delete _local_event_enabled;
  _local_event_enabled = NULL;
  delete _jvmdi_frame_pops;
  _jvmdi_frame_pops = NULL;

#ifndef PRODUCT
  delete _monitors;
  _monitors = NULL;
#endif

  clear_cached_frames();
  _frames->clear_and_deallocate();
  FreeHeap(_frames);
  _frames = NULL;
}


// If no local events have been set, always return true;
bool JvmdiThreadState::is_enabled(jint event_type) const {
  if (_local_event_enabled == NULL) return false;
  return _local_event_enabled->is_enabled(event_type);
}

void JvmdiThreadState::set_enabled(jint event_type, bool enabled) {
  assert(!JvmdiEventDispatcher::is_global_event(event_type), "should not be a global event");

  if (_local_event_enabled == NULL) {
    // threads start out with all events disabled
    _local_event_enabled = new JvmdiEventEnabled(false);
  }

  if (_local_event_enabled->is_enabled(event_type) == enabled) {
    // current event state matches requested state so nothing more to do
    return;
  }

  _local_event_enabled->set_enabled(event_type, enabled);  

  if (event_type == JVMDI_EVENT_SINGLE_STEP) {
    if (enabled) {
      enable_interp_only_mode();
    } else {
      disable_interp_only_mode();
    }
  }
}


void JvmdiThreadState::enable_interp_only_mode() {
  ++_interp_only_mode;

  if (_interp_only_mode == 1) {
    // Set up the current stack depth for later tracking
    set_cur_stack_depth(count_thread_num_frames(get_thread()));

    // Set method_entry_on and method_exit_on the first time
    // interp_only_mode is set.  So cur_stack_depth will
    // maintained properly.
    jvmdi::set_method_entry_on(true);
    jvmdi::set_method_exit_on(true);
  }

#ifndef CORE
  if (FullSpeedJVMDI) {
    if ((_interp_only_mode == 1) && (get_thread()->has_last_Java_frame())) {
      // If running in fullspeed mode, single stepping is implemented
      // as follows: first, the interpreter does not dispatch to
      // compiled code for threads that have single stepping enabled;
      // second, we deoptimize all methods on the thread's stack when
      // interpreted-only mode is enabled the first time for a given
      // thread (nothing to do if no Java frames yet).
      int num_marked = 0;
      ResourceMark resMark;
      RegisterMap rm(get_thread(), false);
      for (vframe* vf = get_thread()->last_java_vframe(&rm); vf; vf = vf->sender()) {
        if (can_be_deoptimized(vf)) {
          ((compiledVFrame*) vf)->code()->mark_for_deoptimization();
          ++num_marked;
        }
      }
      if (num_marked > 0) {
        VM_Deoptimize op;
        VMThread::execute(&op);
      }
    }
  }
#endif /* CORE */
}


void JvmdiThreadState::disable_interp_only_mode() {
  --_interp_only_mode;
  assert(_interp_only_mode >= 0, "enable/disable_interp_only_mode mismatch");
}


// Maintain the current JVM/DI location on a per-thread basis
// and use it to filter out duplicate events:
// - instruction rewrites
// - breakpoint followed by single step
// - single step at a breakpoint
void JvmdiThreadState::compare_and_set_current_location(methodOop new_method, 
  address new_location, jint event) {

  int new_bci = new_location - new_method->code_base();

  // The method is identified and stored as a JNI id which is safe in this
  // case because the class cannot be unloaded while a method is executing.
  JNIid *new_method_id = new_method->jni_id();

  bool step_for_popframe = false;
  if (event == JVMDI_EVENT_SINGLE_STEP && is_pending_step_for_popframe()) {
    // We are single stepping as the last part of the PopFrame() dance
    // so we have some house keeping to do.
    step_for_popframe = true;

    JavaThread *thr = get_thread();
    if (thr->popframe_condition() != JavaThread::popframe_inactive) {
      // If the popframe_condition field is not popframe_inactive, then
      // we missed all of the popframe_field cleanup points:
      //
      // - unpack_frames() was not called (nothing to deopt)
      // - remove_activation_preserving_args_entry() was not called
      //   (did not get suspended in a call_vm() family call and did
      //   not complete a call_vm() family call on the way here)
      //
      // One legitimate way for us to miss all the cleanup points is
      // if we got here right after handling a compiled return. If that
      // is the case, then we consider our return from compiled code
      // to complete the PopFrame() request and we clear the condition.
      if (!thr->was_popframe_compiled_return()) {
        guarantee(false, "unexpected popframe_condition value");
      }
      thr->clear_popframe_condition();
    }

    // clearing the flag indicates we are done with the PopFrame() dance
    clear_pending_step_for_popframe();
  }

  // the last breakpoint or single step was at this same location
  if (_current_bci == new_bci && _current_method_id == new_method_id) {
    switch (event) {
    case JVMDI_EVENT_BREAKPOINT:
      // Repeat breakpoint is complicated. If we previously posted a breakpoint
      // event at this location and if we also single stepped at this location
      // then we skip the duplicate breakpoint.
      _breakpoint_posted = _breakpoint_posted && _single_stepping_posted;
      break;
    case JVMDI_EVENT_SINGLE_STEP:
      // Repeat single step is easy: just don't post it again.
      // If step is pending for popframe then it may not be
      // a repeat step. The new_bci and method_id is same as current_bci
      // and current method_id after pop and step for recursive calls.
      _single_stepping_posted = !step_for_popframe;
      break;
    default:
      assert(false, "invalid event value passed");
      break;
    }
    return;
  }

  set_current_location(new_method_id, new_bci);
  _breakpoint_posted = false;
  _single_stepping_posted = false;
}


JvmdiFramePops* JvmdiThreadState::get_jvmdi_frame_pops() {
  if (_jvmdi_frame_pops != NULL) return _jvmdi_frame_pops;
  _jvmdi_frame_pops = new JvmdiFramePops(this);
  assert(_jvmdi_frame_pops != NULL, "_jvmdi_frame_pops != NULL");
  return _jvmdi_frame_pops;
}


void JvmdiThreadState::set_in_native_code() {
  _in_native_code = true;
}


void JvmdiThreadState::clear_in_native_code() {
  clear_cached_frames();
  _in_native_code = false;
}


bool JvmdiThreadState::is_in_native_code() {
  return _in_native_code;
}


bool JvmdiThreadState::may_be_walked() {
  return (get_thread()->is_being_ext_suspended() || (JavaThread::current() == get_thread()));
}


static int JVMDI_FR_MAGIC = (int)(('D' << 24) | ('I' << 16) | ('F' << 8) | 'R');

bool JvmdiThreadState::verify_jvmdi_frame(JvmdiFrame *jvmdi_frame) {
  if (jvmdi_frame == NULL || jvmdi_frame->magic() != JVMDI_FR_MAGIC || (!jvmdi_frame->verify())) {
    // quick sanity check of the JvmdiFrame
    return false;
  }
  return true;
}


bool JvmdiThreadState::equalJFrameID(void* fid, JvmdiFrame *jvmdi_frame) {
  jframeID f1 = (jframeID)fid;
  jframeID f2 = jvmdi_frame->get_jframeID();

  return (f1 == f2);
}

JvmdiFrame *JvmdiThreadState::from_jframeID(jframeID fid) {
  assert(!is_error_jframeID(fid), "jframeID must not be error marker");
  int index = _frames->find(fid, JvmdiThreadState::equalJFrameID);
  if (index == -1) return NULL;

  JvmdiFrame *jvmdi_frame = _frames->at(index);
  if (!verify_jvmdi_frame(jvmdi_frame)) return NULL;
  return jvmdi_frame;
}

jframeID JvmdiThreadState::to_jframeID(JvmdiFrame *jvmdi_frame) {
  if (!verify_jvmdi_frame(jvmdi_frame)) return invalid_jframeID();

  return (jframeID)jvmdi_frame->get_jframeID();
}

jframeID JvmdiThreadState::invalid_jframeID() {
  return (jframeID)-1;
}

jframeID JvmdiThreadState::no_more_frames_jframeID() {
  return (jframeID)-2;
}

bool JvmdiThreadState::is_error_jframeID(jframeID fid) {
  return ((intptr_t)fid) < 0;
}

int JvmdiThreadState::nFrames() {
  JvmdiThreadState* state = get_thread()->jvmdi_thread_state();
  if (state->is_interp_only_mode()) {
    return state->cur_stack_depth();
  }

  if (!has_last_frame()) return 0;  // no Java frames

  ResourceMark rm;
  RegisterMap reg_map(get_thread());
  vframe *vf = get_thread()->last_java_vframe(&reg_map);
  int n = 0;
  while (vf != NULL) {
    vf = vf->java_sender();
    n++;
  }
  return n;
}

bool JvmdiThreadState::assure_depth(int depth) {
  int initial_length = _frames->length();
  if (initial_length > depth) {
    return true;
  }
  ResourceMark rm;
  RegisterMap reg_map(get_thread());
  vframe *vf = get_thread()->last_java_vframe(&reg_map);
  int d = 0;
  while (true) {
    if (vf == NULL) {
      return false;
    }
    if (d >= initial_length) {
      JvmdiFrame *jf = new JvmdiFrame(get_thread(), d, JVMDI_FR_MAGIC,
                                      JvmdiThreads::next_jframeID());
      _frames->append(jf);
    }
    if (d >= depth) {
      return true;
    }
    vf = vf->java_sender();
    d++;
  }
}

jframeID JvmdiThreadState::depth_to_jframeID(int depth) {
  // The thread being queried does not have any Java frames
  if (!has_last_frame()) {
      return no_more_frames_jframeID();
  }

  if (assure_depth(depth)) {
    return to_jframeID(_frames->at(depth));
  } else {
    return no_more_frames_jframeID();
  }
}

jframeID JvmdiThreadState::current_frame() {
  return depth_to_jframeID(0);
}

jframeID JvmdiThreadState::get_caller(jframeID fid) {
  JvmdiFrame *jvmdi_frame = from_jframeID(fid);
  if (jvmdi_frame == NULL) {
    assert(false, "invalid jframeID");
    return invalid_jframeID();
  }
  return depth_to_jframeID(1+jvmdi_frame->get_depth());
}

bool JvmdiThreadState::is_opaque(jframeID fid, bool return_true_for_native_frames) {
  ResourceMark rm;
  JvmdiFrame *jvmdi_frame = from_jframeID(fid);
  if (jvmdi_frame == NULL) {
    // Currently we are only called by JvmdiThreadState::is_opaque().
    // The caller used from_jframeID() to find our JvmdiThreadState
    // object so the fact that from_jframeID() returns NULL now is bad.
    assert(false, "JvmdiFrame must not be NULL");
    return true;  // be pessimistic and don't crash in product mode
  }
  vframe *vf = jvmdi_frame->get_vframe();
  assert(vf != NULL, "stale frames should have been caught above");
  
  return (!vf->is_java_frame() || (return_true_for_native_frames && ((javaVFrame*) vf)->method()->is_native()));
}

bool JvmdiThreadState::get_frame_location(jframeID fid, methodHandle *method_handle, jint *bci) {
  ResourceMark rm;
  JvmdiFrame *jvmdi_frame = from_jframeID(fid);
  if (jvmdi_frame == NULL) {
    assert(false, "JvmdiFrame must not be NULL");
    return false;  // be pessimistic in product mode
  }
  vframe *vf = jvmdi_frame->get_vframe();
  assert(vf != NULL, "stale frames should have been caught above");

  if (vf->is_java_frame()) {
    javaVFrame *jvf = (javaVFrame*)vf;
    *method_handle = methodHandle(Thread::current(), jvf->method());
    *bci           = jvf->bci();
    return true;
  } else {
    return false;
  }
}


void JvmdiThreadState::clear_cached_frames() {
  int len = _frames->length();
  for (int i=0; i< len; i++) {
    delete _frames->at(i);
  }
  _frames->clear();
}

// Class:     JvmdiThreadState
// Function:  pop_top_frame
// Description:
//   This function decrements the _cur_stack_depth count, removes
//   framepop for this frame, if set, and also invalidates
//   all cached frameIDs due to PopFrame.
//
// Called by: PopFrame
//
void JvmdiThreadState::pop_top_frame() {
  assert(JvmdiInternal::is_thread_fully_suspended(get_thread(), false /* don't wait for suspend completion */),
         "Must be the current thread or it's externally suspended");

  if (is_interp_only_mode()) {
    // Decrement the cur_stack_depth and remove framepop, if any, if interp_only_mode is enabled.
    int popframe_number = cur_stack_depth() - 1;
    decr_cur_stack_depth();
    if (is_frame_pop(popframe_number)) {
      JvmdiFramePops* fps = get_jvmdi_frame_pops();
      JvmdiFramePop fp(popframe_number);
      fps->clear(fp);
      // recompute method entry and exit flags if no more frame pops
      if (fps->length() == 0) {
        recompute_method_entry_and_exit_on();
      }
    }
  } else {
    assert(get_jvmdi_frame_pops()->length() == 0, "Must have no framepops set");
  }

  // Invalidate all frame IDs
  clear_cached_frames();
}

///////////////////////////////////////////////////////////////


//
// Wrapper function for calling the event hook
//

// Todo: make handle code efficient, HandleMarkCleaner, HandleMark, ???
void JvmdiInternal::post_event(JvmdiEvent& jvmdi_e) {
  Thread *thread  = Thread::current();
  assert(thread->is_Java_thread(), "must be called by JavaThread");

  JVMDI_EventHook hook = JvmdiInternal::get_event_hook();

  if (hook == NULL || hook == (JVMDI_EventHook)default_event_hook) {
      JVMDI_TRACE_ARG2("Dropping Event (hook not set)", jvmdi_e);
      return;
  }

  ResourceMark rm;

  JavaThread *jthread = (JavaThread *)thread;
  JNIEnv     *env     = jthread->jni_environment();

  JVMDI_Event event;
  // convert VM handles in jvmdi_e to JNIhandles in event
  jvmdi_e.to_event(event);

  ThreadToNativeFromVM transition(jthread);
  HandleMark hm(jthread);

  JVMDI_TRACE_ARG2("*** Starting Event Hook ***", jvmdi_e);
  hook(env, &event);
  JVMDI_TRACE_ARG2("*** Finished Event Hook ***", jvmdi_e);

  // Delete the JNIHandle references allocated in to_event().
  jvmdi_e.destroy_local_JNI_handles(event);

}


// Todo: make handle code efficient, HandleMarkCleaner, HandleMark, ???
void JvmdiInternal::post_proxy_event(JvmdiEventProxy& jvmdi_e, JavaThread *jthread) {
  assert(Thread::current()->is_VM_thread(), "must be called by VMThread");

  JVMDI_EventHook hook = JvmdiInternal::get_event_hook();

  if (hook == NULL || hook == (JVMDI_EventHook)default_event_hook) {
      JVMDI_TRACE_ARG2("Dropping Proxy Event (hook not set)", jvmdi_e);
      return;
  }

  ResourceMark rm;
  JNIEnv       *env = jthread->jni_environment();

  JVMDI_Event event;
  //
  // Convert VM handles in jvmdi_e to JNIhandles in event. The VMThread
  // is proxying on behalf of a JavaThread and we are adding local JNI
  // handles to the JavaThread. This really isn't a problem since the
  // JavaThread is at a safepoint, but we have to release those local
  // JNI handles before we return to VMThread work.
  //
  jvmdi_e.to_event(event, jthread);

  // Before we call the JVM/DI agent, we have to set the state in the
  // thread for which we are proxying.
  JavaThreadState prev_state = jthread->thread_state();
  assert(prev_state == _thread_blocked, "JavaThread should be at safepoint");
  jthread->set_thread_state(_thread_in_native);

  HandleMark hm;
  JVMDI_TRACE_ARG2("*** Starting Proxy Event Hook ***", jvmdi_e);
  hook(env, &event);
  JVMDI_TRACE_ARG2("*** Finished Proxy Event Hook ***", jvmdi_e);

  // Since we are the VMThread, we need to clean up the JNIhandles that
  // were allocated above.
  jvmdi_e.destroy_local_JNI_handles(event);
  assert(jthread->thread_state() == _thread_in_native, "JavaThread should be in native");
  jthread->set_thread_state(prev_state);
}


jvmdiError JvmdiInternal::AllocateInternal(jlong size, jbyte** memPtr) {  
  if (_ahook == NULL) {
    *memPtr = (jbyte *)os::malloc((size_t)size); 
  } else {
    JavaThread* thread = JavaThread::current();
    ThreadToNativeFromVM transition(thread);
    HandleMark hm(thread);
    _ahook(size, memPtr);
  }

  if (*memPtr == NULL) {
    return JVMDI_ERROR_OUT_OF_MEMORY;
  }
  return JVMDI_ERROR_NONE;
}

jvmdiError JvmdiInternal::DeallocateInternal(jbyte* mem) {
  if (mem == NULL) {
    return JVMDI_ERROR_NULL_POINTER;
  }
  if (_dhook == NULL) {
    os::free(mem);
  } else {
    JavaThread* thread = JavaThread::current();
    ThreadToNativeFromVM transition(thread);
    HandleMark hm(thread);
    _dhook(mem);
  }
  
  return JVMDI_ERROR_NONE;
}


///////////////////////////////////////////////////////
//
// JVMDI functions that are called by the debugger
//


//
// Memory Management
//

JVMDI_ENTER(jvmdiError, SetAllocationHooks, (JVMDI_AllocHook ahook, JVMDI_DeallocHook dhook))
  JvmdiInternal::set_allocation_hooks(ahook, dhook);
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER_NO_TRACE(jvmdiError, Allocate, (jlong size, jbyte** memPtr))
  JVMDI_RETURN(JvmdiInternal::AllocateInternal(size,memPtr));
JVMDI_END

JVMDI_ENTER_NO_TRACE(jvmdiError, Deallocate, (jbyte* mem))
  JVMDI_RETURN(JvmdiInternal::DeallocateInternal(mem));
JVMDI_END

//
// Events
// 

JVMDI_ENTER(jvmdiError, SetEventHook, (JVMDI_EventHook hook))
  // setup the event_hook function
  JvmdiInternal::set_event_hook(hook);
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


static void set_method_event_flags(JavaThread *thread, jint event_type, bool enabled);

JVMDI_ENTER(jvmdiError, SetEventNotificationMode, (jint mode, jint event_type, jthread jni_thread, ...))
  
  va_list argPtr;
  va_start(argPtr, jni_thread);
  va_end(argPtr);

  if (!jvmdi::is_valid_event_type(event_type)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_EVENT_TYPE);
  }
  
  JavaThread* java_thread = NULL;
  if (jni_thread != NULL) {
    // If a thread is specified, SetEventNotificationMode only works for threads that has run.
    java_thread = JvmdiInternal::get_JavaThread(jni_thread);
    NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);
  }

  JVMDI_TRACE_ARG3("event_type",event_type,mode == JVMDI_ENABLE);
  JVMDI_TRACE_ARG2("thread",java_thread);

  // Check if the event can be controlled at thread level.
  if (java_thread != NULL && ((event_type == JVMDI_EVENT_VM_INIT)      || 
                              (event_type == JVMDI_EVENT_VM_DEATH)     ||
			      (event_type == JVMDI_EVENT_THREAD_START) || 
			      (event_type == JVMDI_EVENT_CLASS_UNLOAD))) {
    JVMDI_RETURN(JVMDI_ERROR_ILLEGAL_ARGUMENT);
  }

  bool enabled = (mode == JVMDI_ENABLE);
  JvmdiEventDispatcher::set_enabled(java_thread, event_type, enabled);
  
  if ((event_type == JVMDI_EVENT_METHOD_EXIT)   ||
      (event_type == JVMDI_EVENT_FRAME_POP)     ||
      (event_type == JVMDI_EVENT_METHOD_ENTRY)) {
    set_method_event_flags(java_thread, event_type, enabled);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JavaThread *JvmdiInternal::get_JavaThread(jthread jni_thread) {
  oop t = JNIHandles::resolve_external_guard(jni_thread);
  if (t == NULL || !t->is_a(SystemDictionary::thread_klass())) {
    return NULL;
  }
  // The following returns NULL if the thread has not yet run or is in
  // process of exiting
  return java_lang_Thread::thread(t);
}


jobject *JvmdiInternal::new_jobjectArray(int length, Handle *handles) {
  if (length == 0) { 
    return NULL;
  }

  jobject *objArray = (jobject *) jvmdiMalloc(sizeof(jobject) * length);
  for (int i=0; i<length; i++) {
    objArray[i] = JNIHandles::make_global(handles[i]);
  }  
  return objArray;
}

jthread *JvmdiInternal::new_jthreadArray(int length, Handle *handles) {
  return (jthread *) new_jobjectArray(length,handles);
}

jthreadGroup *JvmdiInternal::new_jthreadGroupArray(int length, Handle *handles) {
  return (jthreadGroup *) new_jobjectArray(length,handles);
}



//
// Thread Execution Functions
//


JVMDI_ENTER(jvmdiError, GetThreadStatus, (jthread jni_thread,
                                          jint *threadStatusPtr, 
                                          jint *suspendStatusPtr))
  // We don't use get_JavaThread() here because the target thread may not
  // have run yet. get_JavaThread() only works on threads that have run.
  NULL_CHECK(jni_thread, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(threadStatusPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(suspendStatusPtr, JVMDI_ERROR_NULL_POINTER);
  oop thread_oop = JNIHandles::resolve_external_guard(jni_thread);
  if (thread_oop == NULL || !thread_oop->is_a(SystemDictionary::thread_klass())) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_THREAD);
  }
  // at this point, we have a valid Thread object
  JavaThread* thr = java_lang_Thread::thread(thread_oop);    
  OSThread* osThread;
  if (thr == NULL) {
    // the thread hasn't run yet or is in the process of exiting
    *threadStatusPtr = JVMDI_THREAD_STATUS_UNKNOWN;
    *suspendStatusPtr = 0;
  } else if ((osThread = thr->osthread()) == NULL) {
    *threadStatusPtr = JVMDI_THREAD_STATUS_ZOMBIE;
    *suspendStatusPtr = 0;
  } else {
    ThreadState state = osThread->get_state();

    // compute suspend status - or together relevant bits
    *suspendStatusPtr  = 
        ((state == BREAKPOINTED)? JVMDI_SUSPEND_STATUS_BREAK : 0) |
        // internal suspend doesn't count for this flag
        (thr->is_being_ext_suspended()? JVMDI_SUSPEND_STATUS_SUSPENDED : 0);
  
    switch (state) {
      case SLEEPING:
	*threadStatusPtr = JVMDI_THREAD_STATUS_SLEEPING;
	break;
      case OBJECT_WAIT:
	*threadStatusPtr = JVMDI_THREAD_STATUS_WAIT;
	break;
      case MONITOR_WAIT:
	*threadStatusPtr = JVMDI_THREAD_STATUS_MONITOR;
	break;
      case ZOMBIE:
	*threadStatusPtr = JVMDI_THREAD_STATUS_ZOMBIE;
	break;
      case RUNNABLE:
      case BREAKPOINTED:  // if at breakpoint, must have been running
      case CONDVAR_WAIT:  // only Object.wait()'s use of a condition
                          // variable counts as STATUS_WAIT
	*threadStatusPtr = JVMDI_THREAD_STATUS_RUNNING;
	break;
      default:
	*threadStatusPtr = JVMDI_THREAD_STATUS_UNKNOWN;
    }
  }
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, GetAllThreads, (jint *threadsCountPtr, jthread **threadsPtr))
  NULL_CHECK(threadsCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(threadsPtr, JVMDI_ERROR_NULL_POINTER);

  int nthreads        = 0;
  int ndebugthreads   = 0;
  Handle *thread_objs = NULL;

  ResourceMark rm;
  HandleMark hm;

  { MutexLocker mu(Threads_lock);

    nthreads = Threads::number_of_threads();
    if (nthreads == 0) {
      *threadsCountPtr = 0;
      *threadsPtr      = 0;
      JVMDI_RETURN(JVMDI_ERROR_NONE);
    }
    thread_objs = NEW_RESOURCE_ARRAY(Handle, nthreads);
    NULL_CHECK(thread_objs, JVMDI_ERROR_OUT_OF_MEMORY);

    for (JavaThread *tp = Threads::first(); tp != NULL; tp = tp->next()) {
      assert(tp->is_Java_thread(), "Must be JavaThread");
      assert(ndebugthreads < nthreads, "Number of threads doesn't match");
      if (ndebugthreads >= nthreads) break;  // robustness: no more room in the array

      JavaThreadState state = tp->thread_state();
      // skip threads that haven't run yet
      if (state == _thread_new || state == _thread_new_trans) continue;
      // ignore JavaThreads in the process of exiting or attached via JNI
      // Filter hidden java threads such as compiler  thread.
      if (tp->threadObj() == NULL   ||
          !java_lang_Thread::is_alive(tp->threadObj())   ||
          tp->is_hidden_from_external_view()) {
        continue;
      }

      thread_objs[ndebugthreads++] = Handle(tp->threadObj());
    }
  }

  // have to make global handles outside of Threads_lock
  jthread *jthreads  = JvmdiInternal::new_jthreadArray(ndebugthreads,thread_objs);
  NULL_CHECK(jthreads, JVMDI_ERROR_OUT_OF_MEMORY);

  #ifndef PRODUCT
  if (TraceJVMDICalls) {
    for (int i=0; i<ndebugthreads; i++) {
      JVMDI_TRACE_ARG2("thread",java_lang_Thread::thread(thread_objs[i]()));
    }
  }
  #endif  

  *threadsCountPtr = ndebugthreads;
  *threadsPtr      = jthreads;

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, hSuspendThread, (jthread jni_thread))

  JavaThread *java_thread = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);
  // the thread has not yet run or has exited (not on threads list)
  NULL_CHECK(java_thread->threadObj(), JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(java_lang_Thread::thread(java_thread->threadObj()), JVMDI_ERROR_INVALID_THREAD);
  JVMDI_TRACE_ARG2("thread", java_thread);

  // don't allow hidden thread suspend request.
  if (java_thread->is_hidden_from_external_view()) {
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }

  {
    MutexLockerEx ml(java_thread->SR_lock(), Mutex::_no_safepoint_check_flag);
    if (java_thread->is_external_suspend()) {
      // don't allow nested external suspend requests.
      JVMDI_RETURN(JVMDI_ERROR_THREAD_SUSPENDED);
    }
    if (java_thread->is_exiting()) { // thread is in the process of exiting
      JVMDI_RETURN(JVMDI_ERROR_INVALID_THREAD);
    }
    java_thread->set_external_suspend();
  }

  if (!JvmdiThreads::suspend(java_thread)) {
    // the thread was in the process of exiting
    JVMDI_RETURN(JVMDI_ERROR_INVALID_THREAD);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, SuspendThreadList, (jint reqCnt, jthread *reqList, jvmdiError *results))

  NULL_CHECK(reqList, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(results, JVMDI_ERROR_NULL_POINTER);
  if (reqCnt <= 0) {
    JVMDI_RETURN(JVMDI_ERROR_ILLEGAL_ARGUMENT);
  }

  int needSafepoint = 0;  // > 0 if we need a safepoint

  for (int i = 0; i < reqCnt; i++) {
    JavaThread *java_thread = JvmdiInternal::get_JavaThread(reqList[i]);
    if (java_thread == NULL) {
      results[i] = JVMDI_ERROR_INVALID_THREAD;
      continue;
    }
    // the thread has not yet run or has exited (not on threads list)
    if (java_thread->threadObj() == NULL) {
      results[i] = JVMDI_ERROR_INVALID_THREAD;
      continue;
    }
    if (java_lang_Thread::thread(java_thread->threadObj()) == NULL) {
      results[i] = JVMDI_ERROR_INVALID_THREAD;
      continue;
    }
    // don't allow hidden thread suspend request.
    if (java_thread->is_hidden_from_external_view()) {
      results[i] = JVMDI_ERROR_NONE;  // indicate successful suspend
      continue;
    }
    JVMDI_TRACE_ARG2("thread", java_thread);

    {
      MutexLockerEx ml(java_thread->SR_lock(), Mutex::_no_safepoint_check_flag);
      if (java_thread->is_external_suspend()) {
        // don't allow nested external suspend requests.
        results[i] = JVMDI_ERROR_THREAD_SUSPENDED;
        continue;
      }
      if (java_thread->is_exiting()) { // thread is in the process of exiting
        results[i] = JVMDI_ERROR_INVALID_THREAD;
        continue;
      }
      java_thread->set_external_suspend();
    }

    if (java_thread->thread_state() == _thread_in_native) {
      // We need to try and suspend native threads here. Threads in
      // other states will self-suspend on their next transition.
      if (!JvmdiThreads::suspend(java_thread)) {
        // The thread was in the process of exiting. Force another
        // safepoint to make sure that this thread transitions.
        needSafepoint++;
        results[i] = JVMDI_ERROR_INVALID_THREAD;
        continue;
      }
    } else {
      needSafepoint++;
    }

    results[i] = JVMDI_ERROR_NONE;  // indicate successful suspend
  }

  if (needSafepoint > 0) {
    VM_ForceSafepoint vfs;
    VMThread::execute(&vfs);
  }

  // per-thread suspend results returned via results parameter
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, hResumeThread, (jthread jni_thread))

  JavaThread* java_thread = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);
  JVMDI_TRACE_ARG2("thread", java_thread);

  // don't allow hidden thread resume request.
  if (java_thread->is_hidden_from_external_view()) {
    JVMDI_RETURN(JVMDI_ERROR_NONE);
  }

  if (!java_thread->is_being_ext_suspended()) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }

  if (!JvmdiThreads::resume(java_thread)) {
    JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, ResumeThreadList, (jint reqCnt, jthread *reqList, jvmdiError *results))

  NULL_CHECK(reqList, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(results, JVMDI_ERROR_NULL_POINTER);
  if (reqCnt <= 0) {
    JVMDI_RETURN(JVMDI_ERROR_ILLEGAL_ARGUMENT);
  }

  for (int i = 0; i < reqCnt; i++) {
    JavaThread *java_thread = JvmdiInternal::get_JavaThread(reqList[i]);
    if (java_thread == NULL) {
      results[i] = JVMDI_ERROR_INVALID_THREAD;
      continue;
    }
    // don't allow hidden thread resume request.
    if (java_thread->is_hidden_from_external_view()) {
      results[i] = JVMDI_ERROR_NONE;  // indicate successful resume
      continue;
    }
    if (!java_thread->is_being_ext_suspended()) {
      results[i] = JVMDI_ERROR_THREAD_NOT_SUSPENDED;
      continue;
    }
    JVMDI_TRACE_ARG2("thread", java_thread);

    if (!JvmdiThreads::resume(java_thread)) {
      results[i] = JVMDI_ERROR_INTERNAL;
      continue;
    }

    results[i] = JVMDI_ERROR_NONE;  // indicate successful suspend
  }

  // per-thread resume results returned via results parameter
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, StopThread, (jthread jni_thread, jobject exception))
  JavaThread* thr = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(thr, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(thr->threadObj(), JVMDI_ERROR_INVALID_THREAD);
  oop e = JNIHandles::resolve_external_guard(exception);
  NULL_CHECK(e, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("thread", thr);

  JavaThread::send_async_exception(thr->threadObj(), e);
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


// Todo: this is a duplicate of JVM_Interrupt; share code in future
JVMDI_ENTER(jvmdiError, InterruptThread, (jthread jni_thread))
  // Ensure that the C++ Thread and OSThread structures aren't freed before we operate
  JavaThread* java_thread = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);

  MutexLockerEx ml(thread->threadObj() == java_thread->threadObj() ? NULL : Threads_lock);
  // We need to re-resolve the java_thread, since a GC might have happened during the
  // acquire of the lock
  java_thread = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(java_thread, JVMDI_ERROR_INVALID_THREAD);
  
  JVMDI_TRACE_ARG2("thread", java_thread);
  
  Thread::interrupt(java_thread);

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, GetThreadInfo, (jthread jni_thread, JVMDI_thread_info *infoPtr))
  // We don't use get_JavaThread() here because the target thread may not
  // have run yet. get_JavaThread() only works on threads that have run.
  NULL_CHECK(jni_thread, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(infoPtr, JVMDI_ERROR_NULL_POINTER);
  oop thread_oop = JNIHandles::resolve_external_guard(jni_thread);
  if (thread_oop == NULL || !thread_oop->is_a(SystemDictionary::thread_klass())) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_THREAD);
  }

  ResourceMark rm;
  HandleMark hm;

  Handle thread_obj(thread, thread_oop);
  typeArrayHandle    name;
  ThreadPriority priority;
  Handle     thread_group;
  bool          is_daemon;

  JVMDI_TRACE_ARG2("thread", thread_oop);
  
  { MutexLocker mu(Threads_lock);

        name = typeArrayHandle(thread, java_lang_Thread::name(thread_obj()));
        priority = java_lang_Thread::priority(thread_obj());
    thread_group = Handle(thread, java_lang_Thread::threadGroup(thread_obj()));
       is_daemon = java_lang_Thread::is_daemon(thread_obj());
  }

  { const char *n;
    
    if (name() != NULL) {
      n = UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length()); 
    } else {
      n = UNICODE::as_utf8(NULL, 0); 
    }
    infoPtr->name = (char *) JvmdiInternal::jvmdiMalloc(strlen(n)+1);
    NULL_CHECK(infoPtr->name, JVMDI_ERROR_OUT_OF_MEMORY);
    strcpy(infoPtr->name, n);
  }

  infoPtr->is_daemon = is_daemon;
  infoPtr->priority  = priority;
  infoPtr->context_class_loader = NULL;
  infoPtr->thread_group = JNIHandles::make_global(thread_group);

  JVMDI_TRACE_ARG2("returns name", infoPtr->name);
  JVMDI_TRACE_ARG2("returns is_daemon", infoPtr->is_daemon ? "true" : "false");
  JVMDI_TRACE_ARG2("returns priority", infoPtr->priority);

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, GetOwnedMonitorInfo, (jthread jni_thread, 
            JVMDI_owned_monitor_info *infoPtr))
  JavaThread *jt = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(jt, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(infoPtr, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("thread", jt);

  if (!JvmdiInternal::is_thread_fully_suspended(jt, true)) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }

  ResourceMark rm;
  // allocate growable array of locked object handles on the C-heap
  GrowableArray<jobject> *handle_list = new GrowableArray<jobject>(1, true);

  // save the locked object handles (if any)
  jt->save_lock_entries(handle_list);

  JVMDI_owned_monitor_info ret;
  ret.owned_monitor_count = handle_list->length();
  ret.owned_monitors = NULL;

  if (ret.owned_monitor_count > 0) {
    // convert locked object handles into global JNI handles
    ret.owned_monitors = (jobject *)JvmdiInternal::jvmdiMalloc(
      ret.owned_monitor_count * sizeof(jobject *));

    for (int i = 0; i < ret.owned_monitor_count; i++) {
      ret.owned_monitors[i] = (jobject)handle_list->at(i);
    }
  }

  // we don't update return parameter unless everything worked
  *infoPtr = ret;

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, GetCurrentContendedMonitor, (jthread jni_thread,
	    jobject *monitor))
  JavaThread *jt = JvmdiInternal::get_JavaThread(jni_thread);
  NULL_CHECK(jt, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(monitor, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("thread", jt);

  if (!JvmdiInternal::is_thread_fully_suspended(jt, true)) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }

  oop obj = NULL;
  ObjectMonitor *mon = jt->current_waiting_monitor();

  if (mon == NULL) {
    // thread is not doing an Object.wait() call
    mon = jt->current_pending_monitor();
    if (mon != NULL) {
      // The thread is trying to enter() or raw_enter() an ObjectMonitor.
      obj = (oop)mon->object();
      // If obj == NULL, then ObjectMonitor is raw which doesn't count
      // as contended for this API
    }
    // implied else: no contended ObjectMonitor
  } else {
    // thread is doing an Object.wait() call
    obj = (oop)mon->object();
    assert(obj != NULL, "Object.wait() should have an object");
  }

  if (obj == NULL) {
    *monitor = NULL;
  } else {
    HandleMark hm;
    Handle     hobj(obj);
    *monitor = (jobject)JNIHandles::make_global(hobj);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

DebuggerThread::DebuggerThread(JVMDI_StartFunction start_fn, void *start_arg)
  : JavaThread(start_function_wrapper) {
  _start_fn = start_fn;
  _start_arg = start_arg;
}

void DebuggerThread::start_function_wrapper(JavaThread *thread, TRAPS)
{
  // It is expected that any debugger threads will be created as
  // Java Threads.  If this is the case, notification of the creation
  // of the thread is given in JavaThread::thread_main().
  assert(thread->is_Java_thread(), "debugger thread should be a Java Thread");

  DebuggerThread *dthread = (DebuggerThread *)thread;
  JavaThread *current_thread = JavaThread::current();
  HandleMark hm(thread);
  ThreadToNativeFromVM transition(current_thread);
  StateSaver ss(thread);
  dthread->_start_fn(dthread->_start_arg);
}


// XXX - This function is similar to create_system_thread(); share code
// in the future.
JVMDI_ENTER(jvmdiError, RunDebugThread, (jthread jni_thread, 
                                         JVMDI_StartFunction proc, 
                                         void *arg,
                                         int priority))

  // We don't use get_JavaThread() here because the debug thread has not
  // run yet. get_JavaThread() only works on threads that have run.
  oop t = JNIHandles::resolve_external_guard(jni_thread);
  if (t == NULL || !t->is_a(SystemDictionary::thread_klass())) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_THREAD);
  }
  
  Handle thread_oop(thread, t);
  {
    MutexLocker mu(Threads_lock); // grab Threads_lock

    DebuggerThread *new_thread = new DebuggerThread(proc, arg);

    // At this point it may be possible that no osthread was created for the
    // JavaThread due to lack of memory. We would have to throw an exception
    // in that case. However, since this must work and we do not allow
    // exceptions anyway, just use an assertion here.
    guarantee(new_thread->osthread() != NULL, "thread creation failed");

    java_lang_Thread::set_thread(thread_oop(), new_thread);
    java_lang_Thread::set_priority(thread_oop(), (ThreadPriority)priority);
    java_lang_Thread::set_daemon(thread_oop());

    new_thread->set_threadObj(thread_oop());
    Threads::add(new_thread);
    JVMDI_TRACE_ARG2("new_thread", new_thread);
    Thread::start(new_thread);
  } // unlock Threads_lock

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

//
// Thread Groups
//

JVMDI_ENTER(jvmdiError, GetTopThreadGroups, (jint *groupCountPtr, 
                                             jthreadGroup **groupsPtr))
  NULL_CHECK(groupCountPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(groupsPtr, JVMDI_ERROR_NULL_POINTER);

  // Only one top level thread group now.
  *groupCountPtr = 1;
  
  // Allocate memory to store global-refs to the thread groups.
  // Assume this area is freed by caller.
  *groupsPtr = (jthreadGroup *) JvmdiInternal::jvmdiMalloc((sizeof(jthreadGroup)) * (*groupCountPtr));
  
  NULL_CHECK(*groupsPtr, JVMDI_ERROR_OUT_OF_MEMORY);

  // Convert oop to Handle, then convert Handle to global-ref.
  {
    Handle system_thread_group(thread, Universe::system_thread_group()); 
    *groupsPtr[0] = JNIHandles::make_global(system_thread_group);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, GetThreadGroupInfo, (jthreadGroup group, 
                                             JVMDI_thread_group_info *infoPtr))
  NULL_CHECK(infoPtr, JVMDI_ERROR_NULL_POINTER);

  ResourceMark rm;
  HandleMark hm;

  Handle group_obj (thread, JNIHandles::resolve_external_guard(group));
  NULL_CHECK(group_obj, JVMDI_ERROR_INVALID_THREAD_GROUP);

  typeArrayHandle name;
  Handle parent_group;
  bool is_daemon;
  ThreadPriority max_priority;

  { MutexLocker mu(Threads_lock);

    name         = typeArrayHandle(thread,
                                   java_lang_ThreadGroup::name(group_obj()));
    parent_group = Handle(thread, java_lang_ThreadGroup::parent(group_obj()));
    is_daemon    = java_lang_ThreadGroup::is_daemon(group_obj());
    max_priority = java_lang_ThreadGroup::maxPriority(group_obj());
  }

  infoPtr->is_daemon    = is_daemon;
  infoPtr->max_priority = max_priority;
  infoPtr->parent       = JNIHandles::make_global(parent_group);

  { const char *n = UNICODE::as_utf8((jchar*) name->base(T_CHAR), name->length()); 
  infoPtr->name = (char *) JvmdiInternal::jvmdiMalloc(strlen(n)+1);    

    NULL_CHECK(infoPtr->name, JVMDI_ERROR_OUT_OF_MEMORY);

    strcpy(infoPtr->name, n);
  }
   
  JVMDI_TRACE_ARG2("returns name",         infoPtr->name);
  JVMDI_TRACE_ARG2("returns max_priority", infoPtr->max_priority);
  JVMDI_TRACE_ARG2("returns isDaemon",     infoPtr->is_daemon);


  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, GetThreadGroupChildren, (jthreadGroup group, 
                                                 jint *threadCountPtr, 
                                                 jthread **threadsPtr,
                                                 jint *groupCountPtr, 
                                                 jthreadGroup **groupsPtr))

 oop group_obj = (oop) JNIHandles::resolve_external_guard(group);
 NULL_CHECK(group_obj, JVMDI_ERROR_INVALID_THREAD_GROUP);
 NULL_CHECK(threadCountPtr, JVMDI_ERROR_NULL_POINTER);
 NULL_CHECK(threadsPtr, JVMDI_ERROR_NULL_POINTER);
 NULL_CHECK(groupCountPtr, JVMDI_ERROR_NULL_POINTER);
 NULL_CHECK(groupsPtr, JVMDI_ERROR_NULL_POINTER);

 Handle *thread_objs = NULL;
 Handle *group_objs  = NULL;
 int nthreads = 0;
 int ngroups = 0;
 int hidden_threads = 0;

 ResourceMark rm;
 HandleMark hm;

 { MutexLocker mu(Threads_lock);

   nthreads = java_lang_ThreadGroup::nthreads(group_obj);
   ngroups  = java_lang_ThreadGroup::ngroups(group_obj);

   if (nthreads > 0) {
     objArrayOop threads = java_lang_ThreadGroup::threads(group_obj);
     assert(nthreads <= threads->length(), "too many threads");
     thread_objs = NEW_RESOURCE_ARRAY(Handle,nthreads);
     for (int i=0, j=0; i<nthreads; i++) {
       oop thread_obj = threads->obj_at(i);
       assert(thread_obj != NULL, "thread_obj is NULL");
       JavaThread *javathread = java_lang_Thread::thread(thread_obj);
       // Filter out hidden java threads.
       if (javathread != NULL && javathread->is_hidden_from_external_view()) {
	 hidden_threads++;
	 continue;
       }
       thread_objs[j++] = Handle(thread, thread_obj);
     }
     nthreads -= hidden_threads;
   }
   

   if (ngroups > 0) {
     objArrayOop groups = java_lang_ThreadGroup::groups(group_obj);
     assert(ngroups <= groups->length(), "too many threads");
     group_objs = NEW_RESOURCE_ARRAY(Handle,ngroups);
     for (int i=0; i<ngroups; i++) {
       oop group_obj = groups->obj_at(i);
       assert(group_obj != NULL, "group_obj != NULL");
       group_objs[i] = Handle(thread, group_obj);
     }
   }
  }

  // have to make global handles outside of Threads_lock
  *groupCountPtr  = ngroups;
  *threadCountPtr = nthreads;
  *threadsPtr     = JvmdiInternal::new_jthreadArray(nthreads, thread_objs);
  *groupsPtr      = JvmdiInternal::new_jthreadGroupArray(ngroups, group_objs);

  if ((nthreads > 0) && (*threadsPtr == NULL)) {
     JVMDI_RETURN(JVMDI_ERROR_OUT_OF_MEMORY);
  }

  if ((ngroups > 0) && (*groupsPtr == NULL)) {
     JVMDI_RETURN(JVMDI_ERROR_OUT_OF_MEMORY);
  }

   JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

//
// Stack Frame Access
//

JVMDI_ENTER(jvmdiError, GetFrameCount, (jthread jni_thread, jint *countPtr))

  JavaThread* thr = JvmdiInternal::get_JavaThread(jni_thread); 
  NULL_CHECK(thr, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(countPtr, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("thread",thr);

  int nFrames;
  if (thr == JavaThread::current()) {
    nFrames = JvmdiThreads::nFrames(thr);
  } else {
    if (!JvmdiInternal::is_thread_fully_suspended(thr, true)) {
      JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
    }

    nFrames = JvmdiThreads::nFrames(thr);
  }
  JVMDI_TRACE_ARG2("returns nFrames",nFrames);

  if (nFrames == -1) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }
    
  *countPtr = nFrames;
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, GetCurrentFrame, (jthread jni_thread, jframeID *framePtr))

  JavaThread* thr = JvmdiInternal::get_JavaThread(jni_thread); 
  NULL_CHECK(thr, JVMDI_ERROR_INVALID_THREAD);
  NULL_CHECK(framePtr, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("thread",thr);

  jframeID current = NULL;
  if (thr == JavaThread::current()) {
    current = JvmdiThreads::current_frame(thr);
  } else {
    if (!JvmdiInternal::is_thread_fully_suspended(thr, true)) {
      JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
    }

    current = JvmdiThreads::current_frame(thr);
  }

  if (current == JvmdiThreadState::no_more_frames_jframeID()) {
    JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
  }

  if (current == JvmdiThreadState::invalid_jframeID()) {
    JVMDI_RETURN(JVMDI_ERROR_THREAD_NOT_SUSPENDED);
  }

  *framePtr = current;
  JVMDI_TRACE_ARG2("returned framePtr", (jlong)current);

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, GetCallerFrame, (jframeID called, jframeID *framePtr))
  NULL_CHECK(called, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(framePtr, JVMDI_ERROR_NULL_POINTER);

  if (JvmdiThreadState::is_error_jframeID(called)) {
    // should never have gone external
    assert(false, "invalid (error marker) jframeID");
    JVMDI_RETURN(JVMDI_ERROR_INVALID_FRAMEID);
  } 

  jframeID parent = JvmdiThreads::get_caller(called);
  if (parent == JvmdiThreadState::invalid_jframeID()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_FRAMEID);
  }

  if (parent == JvmdiThreadState::no_more_frames_jframeID()) {
      JVMDI_RETURN(JVMDI_ERROR_NO_MORE_FRAMES);
  }

  *framePtr = parent;
  JVMDI_TRACE_ARG2("returned framePtr", (jlong)parent);

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

// Todo: do these global handles have to get collected?!?
JVMDI_ENTER(jvmdiError, GetFrameLocation, (jframeID fid, 
                                           jclass *classPtr, 
                                           jmethodID *methodPtr,
                                           jlocation *locationPtr))
  NULL_CHECK(fid, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(classPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(methodPtr, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(locationPtr, JVMDI_ERROR_NULL_POINTER);

  JVMDI_TRACE_ARG2("frame", (jlong)fid);

  if (JvmdiThreads::is_opaque(fid, false)) {
    JVMDI_RETURN(JVMDI_ERROR_OPAQUE_FRAME);
  } else if (!JvmdiThreads::get_frame_location(fid, classPtr, methodPtr, locationPtr)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_FRAMEID);
  } 

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER(jvmdiError, NotifyFramePop, (jframeID frame))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  JVMDI_TRACE_ARG2("frame", (jlong)frame);

  if (JvmdiThreads::is_opaque(frame, true)) {
    JVMDI_RETURN(JVMDI_ERROR_OPAQUE_FRAME);
  } else if (!JvmdiThreads::notify_frame_pop(frame)) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_FRAMEID);
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);

JVMDI_END

//
// Local Variable Access
//
 
JVMDI_ENTER(jvmdiError, GetLocalObject, (jframeID frame, 
                                         jint slot, 
				         jobject *valuePtr))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(valuePtr, JVMDI_ERROR_NULL_POINTER);

  HandleMark hm;
  Handle h;

  jvmdiError res = JvmdiThreads::get_local(frame, slot, &h);
  if (res != JVMDI_ERROR_NONE) JVMDI_RETURN(res);

  *valuePtr = JvmdiInternal::get_jni_object(h());
  
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, GetLocalInt, (jframeID frame, 
                                      jint slot, 
                                      jint *valuePtr))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(valuePtr, JVMDI_ERROR_NULL_POINTER);

  JVMDI_RETURN(JvmdiThreads::get_local(frame, slot, valuePtr));
JVMDI_END

JVMDI_ENTER(jvmdiError, GetLocalLong, (jframeID frame, 
                                       jint slot, 
                                       jlong *valuePtr))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(valuePtr, JVMDI_ERROR_NULL_POINTER);

  JVMDI_RETURN(JvmdiThreads::get_local(frame, slot, valuePtr));
JVMDI_END

JVMDI_ENTER(jvmdiError, GetLocalFloat, (jframeID frame, 
                                        jint slot, 
                                        jfloat *valuePtr))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(valuePtr, JVMDI_ERROR_NULL_POINTER);

  JVMDI_RETURN(JvmdiThreads::get_local(frame, slot, valuePtr));
JVMDI_END

JVMDI_ENTER(jvmdiError, GetLocalDouble, (jframeID frame, 
                                         jint slot, 
                                         jdouble *valuePtr))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);
  NULL_CHECK(valuePtr, JVMDI_ERROR_NULL_POINTER);

  JVMDI_RETURN(JvmdiThreads::get_local(frame, slot, valuePtr));
JVMDI_END

JVMDI_ENTER(jvmdiError, SetLocalObject, (jframeID frame, 
                                         jint slot, jobject value))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);

  HandleMark hm;
  Handle object_handle = Handle(THREAD, JvmdiInternal::get_Oop(value) );

  JVMDI_RETURN(JvmdiThreads::set_local(frame, slot, &object_handle));
JVMDI_END

JVMDI_ENTER(jvmdiError, SetLocalInt, (jframeID frame, 
                                      jint slot, 
                                      jint value))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);

  JVMDI_RETURN(JvmdiThreads::set_local(frame, slot, value));
JVMDI_END

JVMDI_ENTER(jvmdiError, SetLocalLong, (jframeID frame, 
                                       jint slot, 
                                       jlong value))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);

  JVMDI_RETURN(JvmdiThreads::set_local(frame, slot, value));
JVMDI_END

JVMDI_ENTER(jvmdiError, SetLocalFloat, (jframeID frame, 
                                        jint slot, 
                                        jfloat value))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);

  JVMDI_RETURN(JvmdiThreads::set_local(frame, slot, value));
JVMDI_END

JVMDI_ENTER(jvmdiError, SetLocalDouble, (jframeID frame, 
                                         jint slot, 
                                         jdouble value))
  NULL_CHECK(frame, JVMDI_ERROR_INVALID_FRAMEID);

  JVMDI_RETURN(JvmdiThreads::set_local(frame, slot, value));
JVMDI_END

// 
// Raw Monitor Support
//

static int JVMDI_RM_MAGIC = (int)(('D' << 24) | ('I' << 16) | ('R' << 8) | 'M');
#define JVMDI_RM_CHECK(m)                                 \
    if ((m) == NULL || (m)->magic() != JVMDI_RM_MAGIC) {  \
	JVMDI_RETURN(JVMDI_ERROR_INVALID_MONITOR);        \
    }

JVMDI_ENTER(jvmdiError, CreateRawMonitor, (char *name, 
                                           JVMDI_RawMonitor *monitorPtr))
  NULL_CHECK(name, JVMDI_ERROR_NULL_POINTER);
  NULL_CHECK(monitorPtr, JVMDI_ERROR_NULL_POINTER);
  JVMDI_TRACE_ARG2("name", name);

  JvmdiMonitor* monitor = new JvmdiMonitor(name, JVMDI_RM_MAGIC);
  NULL_CHECK(monitor, JVMDI_ERROR_OUT_OF_MEMORY);
  
  *monitorPtr = monitor;

  JVMDI_TRACE_ARG2("returns", (JvmdiMonitor *) monitor);
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, DestroyRawMonitor, (JVMDI_RawMonitor _monitor))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);

  JVMDI_TRACE_ARG2("monitor", monitor);

  if (monitor->monitor().is_entered(THREAD)) {
    // The caller owns this monitor which we are about to destroy.
    // We exit the underlying synchronization object so that the
    // "delete monitor" call below can work without an assertion
    // failure on systems that don't like destroying synchronization
    // objects that are locked.
    int r;
    r = monitor->monitor().raw_exit(THREAD, false);
    assert(r == ObjectMonitor::OM_OK, "raw_exit should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }
  if (monitor->monitor().owner() != NULL) {
    // The caller is trying to destroy a monitor that is locked by
    // someone else. While this is not forbidden by the JVM/DI 1.0
    // spec, it will cause an assertion failure on systems that don't
    // like destroying synchronization objects that are locked.
    // We indicate a problem with the error return (and leak the
    // monitor's memory).
    JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
  }

  delete monitor;
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER_IN_NATIVE_NO_TRACE(jvmdiError, RawMonitorEnter, (JVMDI_RawMonitor _monitor))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);

//JVMDI_TRACE_ARG2("monitor", monitor);

  { int r;
    r = monitor->monitor().raw_enter(THREAD, false);    
    assert(r == ObjectMonitor::OM_OK, "raw_enter should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END



JVMDI_ENTER_IN_NATIVE_NO_TRACE(jvmdiError, RawMonitorExit, (JVMDI_RawMonitor _monitor))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);

//JVMDI_TRACE_ARG2("monitor", monitor);

  { int r;
    r = monitor->monitor().raw_exit(THREAD, false);
    if (r == ObjectMonitor::OM_ILLEGAL_MONITOR_STATE) {
      JVMDI_RETURN(JVMDI_ERROR_NOT_MONITOR_OWNER);
    }
    assert(r == ObjectMonitor::OM_OK, "raw_exit should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER_IN_NATIVE_NO_TRACE(jvmdiError, RawMonitorWait, (JVMDI_RawMonitor _monitor, 
                                                 jlong millis))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);
//JVMDI_TRACE_ARG2("monitor", monitor);

  { int r;

    r = monitor->monitor().raw_wait(millis, true, THREAD);
    switch (r) {
    case ObjectMonitor::OM_INTERRUPTED:
      JVMDI_RETURN(JVMDI_ERROR_INTERRUPT);
    case ObjectMonitor::OM_ILLEGAL_MONITOR_STATE:
      JVMDI_RETURN(JVMDI_ERROR_NOT_MONITOR_OWNER);
    }
    assert(r == ObjectMonitor::OM_OK, "raw_wait should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER_NO_TRACE(jvmdiError, RawMonitorNotify, (JVMDI_RawMonitor _monitor))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);

//JVMDI_TRACE_ARG2("monitor", monitor);

  { int r; 
    r = monitor->monitor().raw_notify(THREAD);
    if (r == ObjectMonitor::OM_ILLEGAL_MONITOR_STATE) {
      JVMDI_RETURN(JVMDI_ERROR_NOT_MONITOR_OWNER);
    }
    assert(r == ObjectMonitor::OM_OK, "raw_notify should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


JVMDI_ENTER_NO_TRACE(jvmdiError, RawMonitorNotifyAll, (JVMDI_RawMonitor _monitor))
  JvmdiMonitor *monitor = (JvmdiMonitor *)_monitor;
  JVMDI_RM_CHECK(monitor);

//JVMDI_TRACE_ARG2("monitor", monitor);

  { int r;
    r = monitor->monitor().raw_notifyAll(THREAD);
    if (r == ObjectMonitor::OM_ILLEGAL_MONITOR_STATE) {
      JVMDI_RETURN(JVMDI_ERROR_NOT_MONITOR_OWNER);
    }
    assert(r == ObjectMonitor::OM_OK, "raw_notifyAll should have worked");
    if (r != ObjectMonitor::OM_OK) {  // robustness
      JVMDI_RETURN(JVMDI_ERROR_INTERNAL);
    }
  }
  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


//
// Breakpoints
//

JVMDI_ENTER(jvmdiError, SetBreakpoint, (jclass clazz, jmethodID method, jlocation location))
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  if (location < 0) {	// simple invalid location check first
    JVMDI_RETURN(JVMDI_ERROR_INVALID_LOCATION);
  }

  // verify that the breakpoint is not past the end of the method
  if (location >= (jlocation) m->code_size()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_LOCATION);
  }

  ResourceMark rm;
  JvmdiBreakpoint bp(clazz, method, location);
  JVMDI_TRACE_ARG2("breakpoint", bp);

  /////////////////////////////////////////////////////////////////////////////
  //
  // BEGIN HACK ALERT (4/8/99)
  //
  // Currently, our mechanism for seeing when we hit a breakpoint
  // requires a check at every bytecode. We would like to not have to
  // pay this cost unless we actually have user breakpoints set.
  // However, JDI sets its own internal breakpoint on
  // java.lang.Thread.resume so user level resume's can't disturb
  // debugger suspended threads. For now, don't give JDI its breakpoint
  // so we can run faster in the common case. When breakpoints no
  // longer exact a per/bytecode cost this code should get removed!
  //

  if (!UseFastBreakpoints) {
    const char *class_name  = (m == NULL) ? "NULL" : m->klass_name()->as_C_string();
    const char *method_name = (m == NULL) ? "NULL" : m->name()->as_C_string();
    
    if ((class_name != NULL) && (strcmp(class_name,"java/lang/Thread") == 0)) {
      if ((method_name != NULL) && (strcmp(method_name,"resume") == 0)) {
	warning("Ignoring breakpoint on java.lang.Thread.resume");
	JVMDI_RETURN(JVMDI_ERROR_NONE);
      }
    }
  }

  //
  // END HACK ALERT (4/8/99)
  //
  /////////////////////////////////////////////////////////////////////////////

  JvmdiBreakpoints& jvmdi_breakpoints = JvmdiCurrentBreakpoints::get_jvmdi_breakpoints();
  if (jvmdi_breakpoints.set(bp) == JVMDI_ERROR_DUPLICATE)
	JVMDI_RETURN(JVMDI_ERROR_DUPLICATE);

  if (TraceJVMDICalls) {
    jvmdi_breakpoints.print();
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, ClearBreakpoint, (jclass clazz, jmethodID method, 
                                          jlocation location))
  klassOop k = JvmdiInternal::get_klassOop(clazz);
  NULL_CHECK(k, JVMDI_ERROR_INVALID_CLASS);
  methodOop m = JvmdiInternal::get_methodOop(method, k);
  NULL_CHECK(m, JVMDI_ERROR_INVALID_METHODID);
  if (location < 0) {	// simple invalid location check first
    JVMDI_RETURN(JVMDI_ERROR_INVALID_LOCATION);
  }

  // verify that the breakpoint is not past the end of the method
  if (location >= (jlocation) m->code_size()) {
    JVMDI_RETURN(JVMDI_ERROR_INVALID_LOCATION);
  }

  JvmdiBreakpoint bp(clazz, method, location);
  JVMDI_TRACE_ARG2("breakpoint", bp);

  JvmdiBreakpoints& jvmdi_breakpoints = JvmdiCurrentBreakpoints::get_jvmdi_breakpoints();
  if (jvmdi_breakpoints.clear(bp) == JVMDI_ERROR_NOT_FOUND)
	JVMDI_RETURN(JVMDI_ERROR_NOT_FOUND);

  if (TraceJVMDICalls) {
    jvmdi_breakpoints.print();
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END

JVMDI_ENTER(jvmdiError, ClearAllBreakpoints, (void))

  JvmdiBreakpoints& jvmdi_breakpoints = JvmdiCurrentBreakpoints::get_jvmdi_breakpoints();
  jvmdi_breakpoints.clearall();
  if (TraceJVMDICalls) {
    jvmdi_breakpoints.print();
  }

  JVMDI_RETURN(JVMDI_ERROR_NONE);
JVMDI_END


//
// Watched Fields
//

static jvmdiError field_watch_common(jclass clazz, jfieldID field,
jint watch_type, bool value) {
  // Check arguments
  oop mirror = JNIHandles::resolve_external_guard(clazz);
  if (mirror == NULL) return JVMDI_ERROR_INVALID_CLASS;
  if (field == NULL) return JVMDI_ERROR_INVALID_FIELDID;
  if (mirror->klass() != SystemDictionary::class_klass() ||
      java_lang_Class::is_primitive(mirror)) {
    return JVMDI_ERROR_INVALID_CLASS;
  }
  klassOop k = java_lang_Class::as_klassOop(mirror);
  if (k == NULL) return JVMDI_ERROR_INVALID_CLASS;

  ResourceMark rm;
  fieldDescriptor fd;
  // use get_field_descriptor to validate field and get current access flags
  if (!JvmdiInternal::get_field_descriptor(k, field, &fd)) {
    return JVMDI_ERROR_INVALID_FIELDID;
  }
  switch (watch_type) {
  case JVMDI_EVENT_FIELD_ACCESS:
    if (value) {
      // make sure we haven't set this watch before
      if (fd.is_field_access_watched()) return JVMDI_ERROR_DUPLICATE;
    } else {
      // make sure we have a watch to clear
      if (!fd.is_field_access_watched()) return JVMDI_ERROR_NOT_FOUND;
    }
    fd.set_is_field_access_watched(value);
    break;

  case JVMDI_EVENT_FIELD_MODIFICATION:
    if (value) {
      // make sure we haven't set this watch before
      if (fd.is_field_modification_watched()) return JVMDI_ERROR_DUPLICATE;
    } else {
      // make sure we have a watch to clear
      if (!fd.is_field_modification_watched()) return JVMDI_ERROR_NOT_FOUND;
    }
    fd.set_is_field_modification_watched(value);
    break;

  default:
    assert(false, "invalid watch_type");
    return JVMDI_ERROR_INTERNAL;
  }

  // update the access_flags for the field in the klass
  instanceKlass* ik = instanceKlass::cast(k);
  typeArrayOop fields = ik->fields();
  fields->ushort_at_put(fd.index(), (jushort)fd.access_flags().as_short());

  return (JVMDI_ERROR_NONE);
}


JVMDI_ENTER(jvmdiError, SetFieldAccessWatch, (jclass clazz, 
                                              jfieldID field))
  jvmdiError result = field_watch_common(clazz, field,
    JVMDI_EVENT_FIELD_ACCESS, true);

  if (result == JVMDI_ERROR_NONE) jvmdi::inc_field_access_count();

  JVMDI_RETURN(result);
JVMDI_END

JVMDI_ENTER(jvmdiError, ClearFieldAccessWatch, (jclass clazz, 
                                                jfieldID field))
  jvmdiError result = field_watch_common(clazz, field,
    JVMDI_EVENT_FIELD_ACCESS, false);

  if (result == JVMDI_ERROR_NONE) jvmdi::dec_field_access_count();

  JVMDI_RETURN(result);
JVMDI_END

JVMDI_ENTER(jvmdiError, SetFieldModificationWatch, (jclass clazz, 
                                                    jfieldID field))
  jvmdiError result = field_watch_common(clazz, field,
    JVMDI_EVENT_FIELD_MODIFICATION, true);

  if (result == JVMDI_ERROR_NONE) jvmdi::inc_field_modification_count();

  JVMDI_RETURN(result);
JVMDI_END

JVMDI_ENTER(jvmdiError, ClearFieldModificationWatch, (jclass clazz, 
                                                      jfieldID field))
  jvmdiError result = field_watch_common(clazz, field,
    JVMDI_EVENT_FIELD_MODIFICATION, false);

  if (result == JVMDI_ERROR_NONE) jvmdi::dec_field_modification_count();

  JVMDI_RETURN(result);
JVMDI_END


//////////////////////////////////////////////////////////////////////////////

//
// External Utility functions.
//

bool jvmdi::is_valid_event_type(jint event_type) {
  if ((event_type < 0) || (event_type > JVMDI_MAX_EVENT_TYPE_VAL)) {
    return false;
  }
  return true;
}

// State so VM knows whether it should post events or not.
bool              jvmdi::_bytecode_stepping       = false;
bool              jvmdi::_enabled                 = false;
int               jvmdi::_field_access_count      = 0;
int               jvmdi::_field_modification_count= 0;
bool              jvmdi::_method_entry_on         = false;
//
// Since JVMDI_EVENT_FRAME_POP is enabled by default, the method_exit_on
// flag should also be on by default. However, we only care about frame
// pops if NotifyFramePop() is called.
//
bool              jvmdi::_method_exit_on          = false;
int               jvmdi::_notice_safepoints_depth = 0;

void jvmdi::initialize() {
 JvmdiCurrentBreakpoints::initialize();
 JvmdiEventDispatcher::initialize();
}

void jvmdi::destroy() {
  disable();
  JvmdiCurrentBreakpoints::destroy();
  JvmdiEventDispatcher::destroy();
}

void jvmdi::enable() {
  jvmdi::initialize();
  _enabled = true;
}

void jvmdi::disable() { 
// XXX - If we do this, then trigger an "invalid method ordering length"
// assertion. This needs to be fixed.
//
//  _enabled = false; 
}

void jvmdi::set_bytecode_stepping(bool on) {
  // nothing to do or JVM/DI is not enabled
  if (on == _bytecode_stepping || !enabled()) return;

  _bytecode_stepping = on;

  //
  // notice_safepoints() and ignore_safepoints() support the
  // concept of nested uses of the "notice safepoints" table.
  // The other user of these methods is the regular safepoint
  // logic. However, we should always be the first notice_safepoints()
  // call and the last ignore_safepoints() call.
  // 
  if (on) {
    assert(notice_safepoints_depth() == 0, "should not be nested before call");
    AbstractInterpreter::notice_safepoints();    
  } else {
    AbstractInterpreter::ignore_safepoints();    
    assert(notice_safepoints_depth() == 0, "should not be nested after call");
  }
}


// Private function to see if we can turn off the method_entry and method_exit events
static void recompute_method_entry_and_exit_on() {
  // lock Threads_lock
  MutexLocker mu(Threads_lock);
  JavaThread *java_thread = Threads::first();
  bool result = false;
  while (java_thread != NULL) {
    JvmdiThreadState *thread_state = java_thread->jvmdi_thread_state();
    if (thread_state != NULL) {
      // we still keep method_entry and method_exit on if METHOD_ENTRY, METHOD_EXIT or FRAME_POP event is enabled
      // either globally or in any threads, or if any thread is running in interpreted-only mode (and we therefore
      // need to keep track of the stack depth)
      result = (JvmdiEventDispatcher::is_enabled(java_thread, JVMDI_EVENT_METHOD_ENTRY) ||
                JvmdiEventDispatcher::is_enabled(java_thread, JVMDI_EVENT_FRAME_POP)    ||
                JvmdiEventDispatcher::is_enabled(java_thread, JVMDI_EVENT_METHOD_EXIT)  ||
                java_thread->jvmdi_thread_state()->is_interp_only_mode());
      if (result == true) break; // we just need to know if one thread has event enabled
    }    
    java_thread = java_thread->next();
  }
  jvmdi::set_method_entry_on(result);
  jvmdi::set_method_exit_on(result);
}


// Private function to interpret changes to JVMDI_EVENT_FRAME_POP,
// JVMDI_EVENT_METHOD_ENTRY, and/or JVMDI_EVENT_METHOD_EXIT handling.
// The method_entry_on flag is used to enable or disable handling for
// the JVMDI_EVENT_METHOD_ENTRY event. Similarly the method_exit_on
// flag is used for JVMDI_EVENT_FRAME_POP or JVMDI_EVENT_METHOD_EXIT
// events. If the appropriate event is enabled globally or for any
// thread, then the corresponding flag is set to true. If the event
// is disabled globally *and* for all the threads, then the flag is
// set to false.
//
static void set_method_event_flags(JavaThread *thread, jint event_type, bool enabled) {
  assert(event_type == JVMDI_EVENT_METHOD_ENTRY ||
	 event_type == JVMDI_EVENT_METHOD_EXIT ||
	 event_type == JVMDI_EVENT_FRAME_POP, "Expected METHOD_ENTRY, METHOD_EXIT or FRAME_POP event");

  bool result = false;

  if (enabled) {
    // we enabled an event so just get to setting the flags
    result = enabled;
  } else {
    // We have disabled the event either globally or for the target
    // thread. Disabling at the global level is really a misnomer
    // because it really means disabling the event for all threads
    // don't explicitly have the event enabled. If another thread
    // has the event enabled, then we can't turn the related flag
    // off.
    recompute_method_entry_and_exit_on();
    return;
  } // unlock Threads_lock

  // we use no synchronization here, so it is possible that we don't notify events
  // for sometime even though they are enabled
  jvmdi::set_method_entry_on(result);
  jvmdi::set_method_exit_on(result);
}

////////////////////////////////////////////////////////////////////////////////////////////////


//
// JVM/DI field access management
//

void jvmdi::set_dynamic_enable_watchpoints(int event_type, bool enabled)
{
  VM_ChangeFieldWatch op(event_type, enabled);
  VMThread::execute(&op);

  // method_entry_on and method_exit_on are enabled when field watchpoints
  // are set in fullspeed debugging (which is done in enable_interp_only_mode())
  // When all field watchpoints are cleared, unset method_entry_on and
  // method_exit_on if necessary to eliminate unnecessary overhead.
  if (_field_access_count == 0 && _field_modification_count == 0) {
    recompute_method_entry_and_exit_on();
  }
}

void jvmdi::dec_field_access_count() {
  set_dynamic_enable_watchpoints(JVMDI_EVENT_FIELD_ACCESS, false);
}


// interpreter generator needs the address of the counter
address jvmdi::get_field_access_count_addr() {
  // We don't grab JVMDI_field_access_lock because we don't want to
  // serialize field access between all threads. This means that a
  // thread on another processor can see the wrong count value and
  // may either miss making a needed call into post_field_access()
  // or will make an unneeded call into post_field_access(). We pay
  // this price to avoid slowing down the VM when we aren't watching
  // field accesses.
  // Other access/mutation safe by virtue of being in VM state.
  return (address)(&_field_access_count);
}


void jvmdi::inc_field_access_count() {
  set_dynamic_enable_watchpoints(JVMDI_EVENT_FIELD_ACCESS, true);
}


//
// JVM/DI field modification management
//

void jvmdi::dec_field_modification_count() {
  set_dynamic_enable_watchpoints(JVMDI_EVENT_FIELD_MODIFICATION, false);
}


// interpreter generator needs the address of the counter
address jvmdi::get_field_modification_count_addr() {
  // We don't grab JVMDI_field_modification_lock because we don't
  // want to serialize field modification between all threads. This
  // means that a thread on another processor can see the wrong
  // count value and may either miss making a needed call into
  // post_field_modification() or will make an unneeded call into
  // post_field_modification(). We pay this price to avoid slowing
  // down the VM when we aren't watching field modifications.
  // Other access/mutation safe by virtue of being in VM state.
  return (address)(&_field_modification_count);
}


void jvmdi::inc_field_modification_count() {
  set_dynamic_enable_watchpoints(JVMDI_EVENT_FIELD_MODIFICATION, true);
}


//
// JVM/DI single step management
//
void jvmdi::at_single_stepping_point(JavaThread *thread, methodOop method, address location) {
  assert(jvmdi::is_bytecode_stepping(), "must be bytecode stepping");
  // update information about current location and post a step event
  JvmdiThreadState *state = thread->jvmdi_thread_state();
  if (!state->hide_single_stepping()) {
    state->compare_and_set_current_location(method, location, JVMDI_EVENT_SINGLE_STEP);
    if (!state->single_stepping_posted()) {
      jvmdi::post_single_step_event(thread, method, location);
    }
  }
  // If we are using slow breakpoints and we are at the right spot, then
  // post a breakpoint.
  if (!UseFastBreakpoints && !state->breakpoint_posted() &&
	JvmdiCurrentBreakpoints::is_breakpoint(location)) {
    jvmdi::post_breakpoint_event(thread, method, location);
  }
}


void jvmdi::expose_single_stepping(JavaThread *thread) {
  JvmdiThreadState *state = thread->jvmdi_thread_state();
  assert(state, "missing JVM/DI thread state");
  state->clear_hide_single_stepping();
}


bool jvmdi::hide_single_stepping(JavaThread *thread) {
  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_SINGLE_STEP)) { 
    JvmdiThreadState *state = thread->jvmdi_thread_state();
    assert(state, "missing JVM/DI thread state");
    state->set_hide_single_stepping();
    return true; 
  } else {
    return false; 
  }
}

//
// JVMDI events that the VM posts to the debugger
//

void jvmdi::post_vm_initialized_event() {
  JvmdiInternal::jvmdi_init();

  { ResourceMark rm;
    JvmdiEventVm e(JVMDI_EVENT_VM_INIT);
    JvmdiInternal::post_event(e);
  }

  JvmdiInternal::jvmdi_init_done();
}

void jvmdi::post_vm_death_event() {

  // Post only if this event is enabled.
  if (JvmdiEventDispatcher::is_enabled(JVMDI_EVENT_VM_DEATH)) { 
    ResourceMark rm;
    JvmdiEventVm e(JVMDI_EVENT_VM_DEATH);
    JvmdiInternal::post_event(e);
  }

  JvmdiInternal::jvmdi_death();
}

void jvmdi::post_class_load_event(JavaThread *thread, klassOop klass) {
  { 
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    KlassHandle  klass_handle(thread, klass);
  
    if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_CLASS_LOAD)) { 
      JvmdiPostEventMark wm(thread);
      JvmdiEventClass e(JVMDI_EVENT_CLASS_LOAD,thread_handle,klass_handle);
      JvmdiInternal::post_event(e);
    }
  }
}

void jvmdi::post_class_prepare_event(JavaThread *thread, klassOop klass) {
  { 
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    KlassHandle  klass_handle(thread, klass);
  
    if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_CLASS_PREPARE)) { 
      JvmdiPostEventMark wm(thread);
      JvmdiEventClass e(JVMDI_EVENT_CLASS_PREPARE,thread_handle,klass_handle);
      JvmdiInternal::post_event(e);
    }
  }
}

void jvmdi::post_class_unload_event(klassOop klass) {
  if (JvmdiEventDispatcher::is_enabled(JVMDI_EVENT_CLASS_UNLOAD)) {
    Thread *thread = Thread::current();
    assert(thread->is_VM_thread(), "wrong thread");

    // get JavaThread for whom we are proxy
    JavaThread *jthread =
        (JavaThread *)((VMThread *)thread)->vm_operation()->calling_thread();

    HandleMark  hm;
    Handle      thread_handle(thread, jthread->threadObj());
    KlassHandle klass_handle(thread, klass);

    JvmdiEventClassUnload e(thread_handle, klass_handle);
    JvmdiInternal::post_proxy_event(e, jthread);
  }
}


void jvmdi::post_thread_start_event(JavaThread *thread) {
  // If we are running in fullspeed mode and method entry or exit
  // events are enabled globally or a watchpoint is active, we need 
  // to switch this thread to interpreted mode. 
  // Race conditions are avoided since the thread state is_thread_in_vm.
  // See VM_ChangeMethodEntryOrExit and VM_ChangeFieldWatch.
  if (FullSpeedJVMDI) {
    assert(thread->thread_state() == _thread_in_vm, "must be in vm state");
    // NOTE: the VM_Deoptimize operation will not take place since
    // we have no Java frames yet.
    // NOTE 2: we handle these separately for simplicity
    if (JvmdiEventDispatcher::is_globally_enabled(JVMDI_EVENT_METHOD_ENTRY)) {
      thread->jvmdi_thread_state()->enable_interp_only_mode();
    }
    if (JvmdiEventDispatcher::is_globally_enabled(JVMDI_EVENT_METHOD_EXIT)) {
      thread->jvmdi_thread_state()->enable_interp_only_mode();
    }
    if (*((int *)get_field_access_count_addr()) > 0) {
      thread->jvmdi_thread_state()->enable_interp_only_mode();
    }
    if (*((int *)get_field_modification_count_addr()) > 0) {
      thread->jvmdi_thread_state()->enable_interp_only_mode();
    }
  }

  // Do not post thread start event for hidden java thread.
  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_THREAD_START) &&
      !thread->is_hidden_from_external_view()) {
    HandleMark   hm;
    Handle       thread_handle(thread, thread->threadObj());
    JvmdiEventThreadChange e(JVMDI_EVENT_THREAD_START, thread_handle);
    JvmdiInternal::post_event(e);
  }
}

void jvmdi::post_thread_end_event(JavaThread *thread) {
  // Do not post thread end event for hidden java thread.
  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_THREAD_END) &&
      !thread->is_hidden_from_external_view()) {
    HandleMark   hm;
    Handle       thread_handle(thread, thread->threadObj());
    JvmdiEventThreadChange e(JVMDI_EVENT_THREAD_END, thread_handle);
    JvmdiInternal::post_event(e);
  }
}

void jvmdi::post_method_entry_event(JavaThread *thread, methodOop method, frame current_frame) {
  JvmdiThreadState* state = thread->jvmdi_thread_state();
  if (state->is_interp_only_mode()) {
    state->incr_cur_stack_depth();
  }

  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_METHOD_ENTRY)) {
    
    JvmdiPostEventMark wm(thread);
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    methodHandle method_handle(thread, method);
    KlassHandle  klass_handle(thread, method->method_holder());
 
    JvmdiEventFrame e(JVMDI_EVENT_METHOD_ENTRY, thread_handle, klass_handle, method_handle, wm.current_frame());
    JvmdiInternal::post_event(e);
  }
}

void jvmdi::post_method_exit_event(JavaThread *thread, methodOop method, frame current_frame) {
  HandleMark   hm(thread);
  Handle       thread_handle(thread, thread->threadObj());
  methodHandle method_handle(thread, method);
  KlassHandle  klass_handle(thread, method->method_holder());

  bool method_exit_enabled = JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_METHOD_EXIT);
  JvmdiThreadState *state = thread->jvmdi_thread_state();

  if (method_exit_enabled) {
    // post method exit event only when a method terminates not by throwing an exception
    // i.e. if an exception is thrown, it's already caught by the current method
    if (!state->is_exception_detected() || state->is_exception_caught()) {
      JvmdiPostEventMark wm(thread);
      JvmdiEventFrame e(JVMDI_EVENT_METHOD_EXIT, thread_handle, klass_handle, method_handle, wm.current_frame());
      JvmdiInternal::post_event(e);
    }
  }

  if (state->is_interp_only_mode()) {
    int cur_frame_number = state->cur_stack_depth() - 1;
    state->decr_cur_stack_depth();
    if (state->is_frame_pop(cur_frame_number)) {
      // we have a NotifyFramePop entry for this frame
      if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_FRAME_POP)) {
        // we also need to issue a frame pop event for this frame
        JvmdiPostEventMark wm(thread);
        JvmdiEventFrame e(JVMDI_EVENT_FRAME_POP, thread_handle, klass_handle, method_handle, wm.current_frame());
        JvmdiInternal::post_event(e);
      }
      // remove the frame's entry
      JvmdiFramePop fp(cur_frame_number);
      JvmdiFramePops* fps = thread->jvmdi_thread_state()->get_jvmdi_frame_pops();
      fps->clear(fp);
      // recompute method entry and exit flags if no more frame pops
      if (fps->length() == 0) {
        recompute_method_entry_and_exit_on();
      }
    }
  }
}

void jvmdi::post_breakpoint_event(JavaThread *thread, methodOop method, address location) {
  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_BREAKPOINT)) {

    JvmdiPostEventMark wm(thread);
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    methodHandle method_handle(thread, method);
    KlassHandle  klass_handle(thread, method->method_holder());

    JvmdiEventBreakpoint e(thread_handle,klass_handle,method_handle,location);
    JvmdiInternal::post_event(e);      

    JvmdiThreadState *state = thread->jvmdi_thread_state();
    state->set_breakpoint_posted();
  }
}

// Todo: inline this for optimization
void jvmdi::post_single_step_event(JavaThread *thread, methodOop method, address location) {
  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_SINGLE_STEP)) {
    JvmdiPostEventMark wm(thread);
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    methodHandle method_handle(thread, method);
    KlassHandle  klass_handle(thread, method->method_holder());

    JvmdiEventSingleStep e(thread_handle,klass_handle,method_handle,location);
    JvmdiInternal::post_event(e);

    JvmdiThreadState *state = thread->jvmdi_thread_state();
    state->set_single_stepping_posted();
  }
}

void jvmdi::post_exception_throw_event(JavaThread *thread, methodOop method, address location, oop exception) {
  JvmdiThreadState *state = thread->jvmdi_thread_state();
  
  if (!state->is_exception_detected()) {
    state->set_exception_detected();

    if ((JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_EXCEPTION)) &&
	(exception != NULL)) {

      ResourceMark rm;
      JvmdiPostEventMark wm(thread);
      HandleMark   hm(thread);
      Handle       thread_handle(thread, thread->threadObj());
      methodHandle method_handle(thread, method);
      KlassHandle  klass_handle(thread, method->method_holder());
      Handle       exception_handle(thread, exception);
      methodHandle catch_method_handle(thread, methodOop(NULL));
      address      catch_location = NULL;
      KlassHandle  catch_klass_handle;
    
      // It's okay to clear these exceptions here because we duplicate
      // this lookup in InterpreterRuntime::exception_handler_for_exception.
      EXCEPTION_MARK;

      bool should_repeat;
      vframeStream st(thread);
      assert(!st.at_end(), "cannot be at end");
      methodOop current_method = NULL;
      int current_bci = -1;
      do {
	current_method = st.method();
	current_bci = st.bci();
	do {
	  should_repeat = false;
          KlassHandle eh_klass(thread, exception_handle()->klass());
	  current_bci = current_method->fast_exception_handler_bci_for(
                                          eh_klass, current_bci, true, THREAD);
	  if (HAS_PENDING_EXCEPTION) {
	    exception_handle = KlassHandle(thread, PENDING_EXCEPTION);
	    CLEAR_PENDING_EXCEPTION;
	    should_repeat = true;
	  }
	} while (should_repeat && (current_bci != -1));
	st.next();
      } while ((current_bci < 0) && (!st.at_end()));

      if (current_bci < 0) {
	catch_method_handle = methodHandle();
	catch_location = NULL;
      } else {
	catch_method_handle = methodHandle(thread, current_method);
	catch_location = current_method->code_base() + current_bci;
      }
      
      catch_klass_handle = (catch_method_handle.is_null() ? KlassHandle() :
                    KlassHandle(thread, catch_method_handle->method_holder()));
        
      JvmdiEventException e(thread_handle, klass_handle, method_handle, location, exception_handle,
			    catch_klass_handle, catch_method_handle, catch_location);
      JvmdiInternal::post_event(e);
    }
  }
}

void jvmdi::notice_unwind_due_to_exception(JavaThread *thread, methodOop method, address location, oop exception, bool in_handler_frame) {
  JvmdiThreadState *state = thread->jvmdi_thread_state();
  
  if (state->is_exception_detected()) {
    
    if (state->is_interp_only_mode()) {
      JvmdiFramePops* fps = state->get_jvmdi_frame_pops();
      int cur_frame_number = state->cur_stack_depth() - 1;
      JvmdiFramePop fp(cur_frame_number);
      if (fps->length() > 0) {
        // location is NULL when an exception is forwarded to its caller; 
        // As specified in the JVMDI spec, no frame pop event or
        // method exit event will be generated if a method terminates
        // by throwing an exception.
        if (location == NULL) {
          assert(!state->is_exception_caught(), "exception must not be caught yet.");
          state->decr_cur_stack_depth();
          if (state->is_frame_pop(cur_frame_number)) {
            // clear framepop of the current frame
            fps->clear(fp);
          }
          return;
        }
      }
          
      // clear pending framepops upto this frame.
      fps->clear_to(fp);
    }

    if (!in_handler_frame) {
      //This is an intermediate stack frame unwind.  Do not report it.
      return;
    }
    assert(location != NULL, "must be a known location");
    // Update cur_stack_depth - the frames above the current frame
    // have been unwound due to this exception:
    state->set_cur_stack_depth(count_thread_num_frames(thread));
    assert(!state->is_exception_caught(), "exception must not be caught yet.");
    state->set_exception_caught();

    if ((JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_EXCEPTION_CATCH)) &&
	(exception != NULL)) {
      JvmdiPostEventMark wm(thread);
      HandleMark   hm(thread);
      Handle       thread_handle(thread, thread->threadObj());
      methodHandle method_handle(thread, method);
      KlassHandle  klass_handle(thread, method->method_holder());
      Handle       exception_handle(thread, exception);
      
      JvmdiEventExceptionCatch e(thread_handle, klass_handle, method_handle, location, exception_handle);
      JvmdiInternal::post_event(e);
    }
  }
}

oop jvmdi::jni_GetField_probe(
  JavaThread *thread,
  jobject jobj,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static
) {
  if (*((int *)get_field_access_count_addr()) > 0 && thread->has_last_Java_frame()) {
    // At least one field access watch is set so we have more work
    // to do. This wrapper is used by entry points that allow us
    // to create handles in post_field_access_by_jni().
    post_field_access_by_jni(thread, obj, klass, fieldID, is_static);
    // post_event() can block so refetch oop if we were passed a jobj
    if (jobj != NULL) return JNIHandles::resolve_non_null(jobj);
  }
  return obj;
}

oop jvmdi::jni_GetField_probe_nh(
  JavaThread *thread,
  jobject jobj,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static
) {
  if (*((int *)get_field_access_count_addr()) > 0 && thread->has_last_Java_frame()) {
    // At least one field access watch is set so we have more work
    // to do. This wrapper is used by "quick" entry points that don't
    // allow us to create handles in post_field_access_by_jni(). We
    // override that with a ResetNoHandleMark.
    ResetNoHandleMark rnhm;
    post_field_access_by_jni(thread, obj, klass, fieldID, is_static);
    // post_event() can block so refetch oop if we were passed a jobj
    if (jobj != NULL) return JNIHandles::resolve_non_null(jobj);
  }
  return obj;
}

void jvmdi::post_field_access_by_jni(
  JavaThread *thread,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static
) {
  // We must be called with a Java context in order to provide reasonable
  // values for the klazz, method, and location fields. The callers of this
  // function don't make the call unless there is a Java context.
  assert(thread->has_last_Java_frame(), "must be called with a Java context");

  ResourceMark rm;
  fieldDescriptor fd;
  // if get_field_descriptor finds fieldID to be invalid, then we just bail
  if (!JvmdiInternal::get_field_descriptor(klass, fieldID, &fd)) return;
  // field accesses are not watched so bail
  if (!fd.is_field_access_watched()) return;

  HandleMark hm(thread);
  KlassHandle h_klass(thread, klass);
  Handle h_obj;
  if (!is_static) {
    // non-static field accessors have an object, but we need a handle
    assert(obj != NULL, "non-static needs an object");
    h_obj = Handle(thread, obj);
  }
  post_field_access_event(thread,
                          thread->last_frame().interpreter_frame_method(),
                          thread->last_frame().interpreter_frame_bcp(),
                          h_klass, h_obj, fieldID);
}

void jvmdi::post_field_access_event(JavaThread *thread, methodOop method,
  address location, KlassHandle field_klass, Handle object, jfieldID field) {

  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_FIELD_ACCESS)) {
    ResourceMark rm;
    JvmdiPostEventMark wm(thread);
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    methodHandle method_handle(thread, method);
    KlassHandle  klass_handle(thread, method->method_holder());

    JvmdiEventFieldAccess e(thread_handle, klass_handle, method_handle,
      location, field_klass, object, field);
    JvmdiInternal::post_event(e);
  }
}

oop jvmdi::jni_SetField_probe(
  JavaThread *thread,
  jobject jobj,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static,
  char sig_type,
  jvalue *value
) {
  if (*((int *)get_field_modification_count_addr()) > 0 && thread->has_last_Java_frame()) {
    // At least one field modification watch is set so we have more work
    // to do. This wrapper is used by entry points that allow us
    // to create handles in post_field_modification_by_jni().
    post_field_modification_by_jni(thread, obj, klass, fieldID, is_static, sig_type, value);
    // post_event() can block so refetch oop if we were passed a jobj
    if (jobj != NULL) return JNIHandles::resolve_non_null(jobj);
  }
  return obj;
}

oop jvmdi::jni_SetField_probe_nh(
  JavaThread *thread,
  jobject jobj,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static,
  char sig_type,
  jvalue *value
) {
  if (*((int *)get_field_modification_count_addr()) > 0 && thread->has_last_Java_frame()) {
    // At least one field modification watch is set so we have more work
    // to do. This wrapper is used by "quick" entry points that don't
    // allow us to create handles in post_field_modification_by_jni(). We
    // override that with a ResetNoHandleMark.
    ResetNoHandleMark rnhm;
    post_field_modification_by_jni(thread, obj, klass, fieldID, is_static, sig_type, value);
    // post_event() can block so refetch oop if we were passed a jobj
    if (jobj != NULL) return JNIHandles::resolve_non_null(jobj);
  }
  return obj;
}

void jvmdi::post_field_modification_by_jni(
  JavaThread *thread,
  oop obj,
  klassOop klass,
  jfieldID fieldID,
  bool is_static,
  char sig_type,
  jvalue *value
) {
  // We must be called with a Java context in order to provide reasonable
  // values for the klazz, method, and location fields. The callers of this
  // function don't make the call unless there is a Java context.
  assert(thread->has_last_Java_frame(), "must be called with Java context");

  ResourceMark rm;
  fieldDescriptor fd;
  // if get_field_descriptor finds fieldID to be invalid, then we just bail
  if (!JvmdiInternal::get_field_descriptor(klass, fieldID, &fd)) return;
  // field modifications are not watched so bail
  if (!fd.is_field_modification_watched()) return;

  HandleMark hm(thread);

  Handle h_obj;
  if (!is_static) {
    // non-static field accessors have an object, but we need a handle
    assert(obj != NULL, "non-static needs an object");
    h_obj = Handle(thread, obj);
  }
  KlassHandle h_klass(thread, klass);
  post_field_modification_event(thread, 
                                thread->last_frame().interpreter_frame_method(),
                                thread->last_frame().interpreter_frame_bcp(),
                                h_klass, h_obj, fieldID, sig_type, value);
}

void jvmdi::post_field_modification_event(JavaThread *thread, methodOop method,
  address location, KlassHandle field_klass, Handle object, jfieldID field,
  char sig_type, jvalue *value) {

  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_FIELD_MODIFICATION)) {
    ResourceMark rm;
    JvmdiPostEventMark wm(thread);
    HandleMark   hm(thread);
    Handle       thread_handle(thread, thread->threadObj());
    methodHandle method_handle(thread, method);
    KlassHandle  klass_handle(thread, method->method_holder());

    JvmdiEventFieldModification e(thread_handle, klass_handle, method_handle,
      location, field_klass, object, field, sig_type, value);
    JvmdiInternal::post_event(e);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void jvmdi::initialize_thread(JavaThread *thread) {
  thread->set_jvmdi_thread_state(new JvmdiThreadState(thread));
}


void jvmdi::cleanup_thread(JavaThread* thread) {
  assert(JavaThread::current() == thread, "thread is not current");

  if (JvmdiEventDispatcher::is_enabled(thread, JVMDI_EVENT_SINGLE_STEP)) {
    //
    // Turn off single stepping for this thread. If this is the last thread
    // that was single stepping, then bytecode stepping is turned off by
    // set_enabled().
    //
    JvmdiEventDispatcher::set_enabled(thread, JVMDI_EVENT_SINGLE_STEP, false);
  }

  if (thread->jvmdi_thread_state()) {
    delete thread->jvmdi_thread_state();
    thread->set_jvmdi_thread_state(NULL);
  }
}

void jvmdi::oops_do(OopClosure* f) {
  JvmdiCurrentBreakpoints::oops_do(f);
}

void jvmdi::gc_epilogue() {
  JvmdiCurrentBreakpoints::gc_epilogue();
}

JVMDI_Interface_1* jvmdi::GetInterface_1(JavaVM *interfaces_vm) {
  JVMDI_Interface_1 *jvmdi_interface = JvmdiInternal::get_jvmdi_interface();

  if (jvmdi_interface->SetEventHook == NULL) {
    JvmdiInternal::set_vm(interfaces_vm);

    // Pick up interfaces local to other files
    JvmdiInternal::set_info_hooks(jvmdi_interface);
    JvmdiInternal::set_info2_hooks(jvmdi_interface);
#ifdef HOTSWAP
    JvmdiInternal::set_hotswap_hooks(jvmdi_interface);
#endif HOTSWAP

    jvmdi_interface->SetEventHook = SetEventHook;
    jvmdi_interface->SetEventNotificationMode = SetEventNotificationMode;
    jvmdi_interface->GetThreadStatus = GetThreadStatus;
    jvmdi_interface->GetAllThreads = GetAllThreads;
    jvmdi_interface->SuspendThread = hSuspendThread;
    jvmdi_interface->ResumeThread = hResumeThread;
    jvmdi_interface->StopThread = StopThread;
    jvmdi_interface->InterruptThread = InterruptThread;
    jvmdi_interface->GetThreadInfo = GetThreadInfo;
    jvmdi_interface->GetOwnedMonitorInfo = GetOwnedMonitorInfo;
    jvmdi_interface->GetCurrentContendedMonitor = GetCurrentContendedMonitor;
    jvmdi_interface->RunDebugThread = RunDebugThread;
    jvmdi_interface->CreateRawMonitor = CreateRawMonitor;
    jvmdi_interface->DestroyRawMonitor = DestroyRawMonitor;
    jvmdi_interface->RawMonitorEnter = RawMonitorEnter;
    jvmdi_interface->RawMonitorExit = RawMonitorExit;
    jvmdi_interface->RawMonitorWait = RawMonitorWait;
    jvmdi_interface->RawMonitorNotify = RawMonitorNotify;
    jvmdi_interface->RawMonitorNotifyAll = RawMonitorNotifyAll;
    jvmdi_interface->GetTopThreadGroups = GetTopThreadGroups;
    jvmdi_interface->GetThreadGroupInfo = GetThreadGroupInfo;
    jvmdi_interface->GetThreadGroupChildren = GetThreadGroupChildren;
    jvmdi_interface->GetFrameCount = GetFrameCount;
    jvmdi_interface->GetCurrentFrame = GetCurrentFrame;
    jvmdi_interface->GetCallerFrame = GetCallerFrame;
    jvmdi_interface->GetFrameLocation = GetFrameLocation;
    jvmdi_interface->NotifyFramePop = NotifyFramePop;
    jvmdi_interface->GetLocalObject = GetLocalObject;
    jvmdi_interface->GetLocalInt = GetLocalInt;
    jvmdi_interface->GetLocalLong = GetLocalLong;
    jvmdi_interface->GetLocalFloat = GetLocalFloat;
    jvmdi_interface->GetLocalDouble = GetLocalDouble;
    jvmdi_interface->SetLocalObject = SetLocalObject;
    jvmdi_interface->SetLocalInt = SetLocalInt;
    jvmdi_interface->SetLocalLong = SetLocalLong;
    jvmdi_interface->SetLocalFloat = SetLocalFloat;
    jvmdi_interface->SetLocalDouble = SetLocalDouble;
    jvmdi_interface->SetBreakpoint = SetBreakpoint;
    jvmdi_interface->ClearBreakpoint = ClearBreakpoint;
    jvmdi_interface->ClearAllBreakpoints = ClearAllBreakpoints;
    jvmdi_interface->SetFieldAccessWatch = SetFieldAccessWatch;
    jvmdi_interface->ClearFieldAccessWatch = ClearFieldAccessWatch;
    jvmdi_interface->SetFieldModificationWatch = SetFieldModificationWatch;
    jvmdi_interface->ClearFieldModificationWatch = ClearFieldModificationWatch;
    jvmdi_interface->SetAllocationHooks = SetAllocationHooks;
    jvmdi_interface->Allocate = Allocate;
    jvmdi_interface->Deallocate = Deallocate;
    if (UseSuspendResumeThreadLists) {
      jvmdi_interface->SuspendThreadList = SuspendThreadList;
      jvmdi_interface->ResumeThreadList = ResumeThreadList;
    } else {
      jvmdi_interface->SuspendThreadList = NULL;
      jvmdi_interface->ResumeThreadList = NULL;
    }
  }

  return jvmdi_interface;
}
