#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)jvmdi_impl.hpp	1.54 03/01/23 12:20:22 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Forward Declarations
//

class JvmdiFramePops;
class JvmdiMonitor;
class JvmdiEvent;
class JvmdiBreakpoint;

#ifdef ASSERT

//
// JVMDITraceWrapper
//
// Wrapper to trace JVMDI functions
// Used only for debugging
// 

class JvmdiTraceWrapper : public StackObj {
public:
    JvmdiTraceWrapper(const char *str);
    JvmdiTraceWrapper(const char *parm, JvmdiMonitor *monitor);
    JvmdiTraceWrapper(const char *parm, jlong l);
    JvmdiTraceWrapper(const char *parm, const char *str);
    JvmdiTraceWrapper(const char *parm, JavaThread *thr);
    JvmdiTraceWrapper(const char *parm, int event_type, bool enabled);
    JvmdiTraceWrapper(const char *parm, oop obj);
    JvmdiTraceWrapper(const char *parm, methodOop m, jint bci);
    JvmdiTraceWrapper(const char *parm, JvmdiBreakpoint& bp);
    JvmdiTraceWrapper(const char *parm, JvmdiEvent& event);
};

// Macros for using the JvmdiTraceWrapper
#define JVMDI_TRACE_ARG2(X,Y)   if (TraceJVMDICalls) { JvmdiTraceWrapper _ignore(X,Y); }
#define JVMDI_TRACE_ARG3(X,Y,Z) if (TraceJVMDICalls) { JvmdiTraceWrapper _ignore(X,Y,Z); }

#else

  #define JVMDI_TRACE_ARG2(X,Y)
  #define JVMDI_TRACE_ARG3(X,Y,Z)  
  #define JVMDI_TRACE_EVENT(X) 

#endif


//
// class JvmdiEventEnabled
// Used internally by   : JvmdiEventDispatcher, JvmdiThreadState
// Used by JVMDI methods: not directly.
// Note: It's a helper class.
//
// A boolean array indexed by event_type, used as an internal
// data structure to track what JVMDI event types are enabled 
// both globally and on a per thread basis.
//

class JvmdiEventEnabled : public CHeapObj {
private:
  bool _event_enabled[JVMDI_MAX_EVENT_TYPE_VAL+1];

public:
  JvmdiEventEnabled(bool init_val);
  bool is_enabled(jint event_type);
  void set_enabled(jint event_type, bool enabled);
};


//
// class JvmdiEventDispatcher
// Used internally by   : No one, it's a top level class.
// Used by JVMDI methods: SetEventNotificationMode, post_*_event
//
// JVMDI events can be enabled (or disabled) either globally (for all threads)
// or for a single thread. The JvmdiEventDispatcher is a top level
// class that manages state describing what JVMDI event types are enabled both
// locally and globally.
//

class JvmdiEventDispatcher : public AllStatic {
private:
  // global array indexed by (jint) event_type  
  static JvmdiEventEnabled _global_event_enabled;

  static void JvmdiEventDispatcher::reset_thread_location(JavaThread *thread, jint event, bool enabled);
  friend class VM_ChangeMethodEntryOrExit;
  friend class VM_ChangeFieldWatch;
public:
  static void initialize();
  static void destroy();

  // events that can ONLY be enabled/disabled globally (can't toggle on individual threads).
  static inline bool is_global_event(jint event_type) {
    return ((event_type == JVMDI_EVENT_VM_INIT)      ||
	    (event_type == JVMDI_EVENT_VM_DEATH)     ||
	    (event_type == JVMDI_EVENT_THREAD_START) ||
	    (event_type == JVMDI_EVENT_CLASS_UNLOAD)); 
  }

  // returns true if the event is enabled globally
  static bool is_globally_enabled(jint event_type);

  // Use (thread == NULL) to enable/disable an event globally.
  // Use (thread != NULL) to enable/disable an event for a particular thread.
  // thread is ignored for events that can only be specified globally
  static void set_enabled(JavaThread *thread, jint event_type, bool enabled);

  // return true if event is enabled globally or for any thread
  static bool is_enabled(jint event_type);
  
  // returns true if event is enabled for thread.
  // events are enabled for a thread if they are enabled locally OR globally.
  // thread is ignored for events that can only be specified globally
  static bool is_enabled(JavaThread *thread,  jint event_type);

};



// 
// class GrowableCache, GrowableElement
// Used by              : JvmdiBreakpointCache, JvmdiFramePopCache
// Used by JVMDI methods: none directly.
//
// GrowableCache is a permanent CHeap growable array of <GrowableElement *>
//
// In addition, the GrowableCache maintains a NULL terminated cache array of type address
// that's created from the element array using the function: 
//     address GrowableElement::getCacheValue(). 
//
// Whenever the GrowableArray changes size, the cache array gets recomputed into a new C_HEAP allocated
// block of memory. Additionally, every time the cache changes its position in memory, the
//    void (*_listener_fun)(void *this_obj, address* cache) 
// gets called with the cache's new address. This gives the user of the GrowableCache a callback
// to update its pointer to the address cache.
//

class GrowableElement : public CHeapObj {
public:
  virtual address getCacheValue()          =0;
  virtual bool equals(GrowableElement* e)  =0;
  virtual bool lessThan(GrowableElement *e)=0;
  virtual GrowableElement *clone()         =0;
  virtual void oops_do(OopClosure* f)      =0;
};

class GrowableCache VALUE_OBJ_CLASS_SPEC {

private:
  // Object pointer passed into cache & listener functions.
  void *_this_obj;

  // Array of elements in the collection
  GrowableArray<GrowableElement *> *_elements;

  // Parallel array of cached values
  address *_cache;

  // Listener for changes to the _cache field.
  // Called whenever the _cache field has it's value changed  
  // (but NOT when cached elements are recomputed).
  void (*_listener_fun)(void *, address*);

  static bool equals(void *, GrowableElement *);

  // recache all elements after size change, notify listener
  void recache();
  
public:  
   GrowableCache();
   ~GrowableCache();

  void initialize(void *this_obj, void listener_fun(void *, address*) );

  // number of elements in the collection
  int length();
  // get the value of the index element in the collection
  GrowableElement* at(int index);
  // find the index of the element, -1 if it doesn't exist
  int find(GrowableElement* e);
  // append a copy of the element to the end of the collection, notify listener
  void append(GrowableElement* e);  
  // insert a copy of the element using lessthan(), notify listener
  void insert(GrowableElement* e);  
  // remove the element at index, notify listener
  void remove (int index);
  // clear out all elements and release all heap space, notify listener
  void clear();
  // apply f to every element and update the cache
  void oops_do(OopClosure* f);
  void gc_epilogue();
};

//
// class JvmdiBreakpointCache
// Used by              : JvmdiBreakpoints
// Used by JVMDI methods: none directly.
// Note   : typesafe wrapper for GrowableCache of JvmdiBreakpoint
//

class JvmdiBreakpointCache : public CHeapObj {

private:
  GrowableCache _cache;
  
public:  
  JvmdiBreakpointCache()  {}
  ~JvmdiBreakpointCache() {}

  void initialize(void *this_obj, void listener_fun(void *, address*) ) {
    _cache.initialize(this_obj,listener_fun);
  }

  int length()                          { return _cache.length(); }
  JvmdiBreakpoint& at(int index)        { return (JvmdiBreakpoint&) *(_cache.at(index)); }
  int find(JvmdiBreakpoint& e)          { return _cache.find((GrowableElement *) &e); }
  void append(JvmdiBreakpoint& e)       { _cache.append((GrowableElement *) &e); }
  void remove (int index)               { _cache.remove(index); }
  void clear()                          { _cache.clear(); }
  void oops_do(OopClosure* f)           { _cache.oops_do(f); }
  void gc_epilogue()                    { _cache.gc_epilogue(); }
};


//
// class JvmdiObj
// Used by              : JvmdiMonitor
// Used by JVMDI methods: none directly.
//
// Overrides new & delete to use JvmdiInternal::jvmdiMalloc and JvmdiInternal::jvmdiFree
//

class JvmdiObj
{
public:
  void *operator new(size_t size);
  void  operator delete(void *p);
};


// 
// class JvmdiMonitor
// Used by JVMDI methods: All RawMonitor methods (CreateRawMonitor, EnterRawMonitor, etc.)
//
// Wrapper for ObjectMonitor class that saves the Monitor's name
//

class JvmdiMonitor : public JvmdiObj {
private:
  int           _magic;
  char *        _name;
  ObjectMonitor _monitor;

public:
  JvmdiMonitor(const char *name, const int magic);
  ~JvmdiMonitor();
  int            magic()   { return _magic;  }
  ObjectMonitor& monitor() { return _monitor;  }
  void print(outputStream& out)       { out.print(_name); }
};


//
// class JvmdiBreakpoint
// Used by              : JvmdiBreakpoints
// Used by JVMDI methods: SetBreakpoint, ClearBreakpoint, ClearAllBreakpoints
// Note: Extends GrowableElement for use in a GrowableCache
//
// A JvmdiBreakpoint describes a location (class, method, bci) to break at.
//

typedef void (methodOopDesc::*method_action)(int _bci);

class JvmdiBreakpoint : public GrowableElement {
private:   
  methodOop 		_method;
  int       		_bci;
  Bytecodes::Code 	_orig_bytecode;

public: 
  JvmdiBreakpoint();
  JvmdiBreakpoint(jclass clazz, jmethodID method, jlocation location);
  bool equals(JvmdiBreakpoint& bp);
  bool lessThan(JvmdiBreakpoint &bp);
  void copy(JvmdiBreakpoint& bp);
  bool is_valid();
  address getBcp();
  void each_method_version_do(method_action meth_act);
  void set();
  void clear();
  void print();

#ifdef HOTSWAP
  methodOop method() { return _method; }
#endif HOTSWAP

  // GrowableElement implementation
  address getCacheValue()         { return getBcp(); }
  bool lessThan(GrowableElement* e) { Unimplemented(); return false; }
  bool equals(GrowableElement* e) { return equals((JvmdiBreakpoint&) *e); }
  void oops_do(OopClosure* f)     { f->do_oop((oop *) &_method); }
  GrowableElement *clone()        {
    JvmdiBreakpoint *bp = new JvmdiBreakpoint();
    bp->copy(*this);
    return bp;
  }
};

//
// class VM_ChangeBreakpoints
// Used by              : JvmdiBreakpoints
// Used by JVMDI methods: none directly.
// Note: A Helper class.
//
// VM_ChangeBreakpoints implements a VM_Operation for ALL modifications to the JvmdiBreakpoints class.
//

class VM_ChangeBreakpoints : public VM_Operation {
private:
  JvmdiBreakpoints* _breakpoints;
  int               _operation;
  JvmdiBreakpoint*  _bp;
  
public:
  enum { SET_BREAKPOINT=0, CLEAR_BREAKPOINT=1, CLEAR_ALL_BREAKPOINT=2 };

  VM_ChangeBreakpoints(JvmdiBreakpoints* breakpoints, int operation) {
    _breakpoints = breakpoints;
    _bp = NULL;
    _operation = operation;
    assert(breakpoints != NULL, "breakpoints != NULL");
    assert(operation == CLEAR_ALL_BREAKPOINT, "unknown breakpoint operation");
  }
  VM_ChangeBreakpoints(JvmdiBreakpoints* breakpoints, int operation, JvmdiBreakpoint *bp) {
    _breakpoints = breakpoints;
    _bp = bp;
    _operation = operation;
    assert(breakpoints != NULL, "breakpoints != NULL");
    assert(bp != NULL, "bp != NULL");
    assert(operation == SET_BREAKPOINT || operation == CLEAR_BREAKPOINT , "unknown breakpoint operation");
  }    
  void doit();
  const char* name() const                       { return "change breakpoints"; }  
};
 

//
// class JvmdiBreakpoints 
// Used by              : JvmdiCurrentBreakpoints
// Used by JVMDI methods: none directly
// Note: A Helper class
//
// JvmdiBreakpoints is a GrowableCache of JvmdiBreakpoint.
// All changes to the GrowableCache occur at a safepoint using VM_ChangeBreakpoints.
//
// Because _bps is only modified at safepoints, its possible to always use the
// cached byte code pointers from _bps without doing any synchronization (see JvmdiCurrentBreakpoints).
//
// It would be possible to make JvmdiBreakpoints a static class, but I've made it
// CHeap allocated to emphasize its similarity to JvmdiFramePops. 
//

class JvmdiBreakpoints : public CHeapObj {
private:

  JvmdiBreakpointCache _bps;
  
  // These should only be used by VM_ChangeBreakpoints
  // to insure they only occur at safepoints.
  // Todo: add checks for safepoint
  friend class VM_ChangeBreakpoints;
  void set_at_safepoint(JvmdiBreakpoint& bp);
  void clear_at_safepoint(JvmdiBreakpoint& bp);
  void clearall_at_safepoint();

  static void do_element(GrowableElement *e);

public:  
  JvmdiBreakpoints(void listener_fun(void *, address *));
  ~JvmdiBreakpoints(); 

  int length();
  void oops_do(OopClosure* f);
  void gc_epilogue();
  void print();

  int  set(JvmdiBreakpoint& bp);
  int  clear(JvmdiBreakpoint& bp);
#ifdef HOTSWAP
  void  clearall_in_class_at_safepoint(klassOop klass);
#endif HOTSWAP
  void clearall(); 
};


//
// class JvmdiCurrentBreakpoints
// Used by              : JvmdiInternal, InterpreterRuntime::at_safepoint
// Used by JVMDI methods: SetBreakpoint, ClearBreakpoint, ClearAllBreakpoints
//
// A static wrapper class for the JvmdiBreakpoints that provides:
// 1. a fast inlined function to check if a byte code pointer is a breakpoint (is_breakpoint).
// 2. a function for lazily creating the JvmdiBreakpoints class (this is not strictly necessary,
//    but I'm copying the code from JvmdiThreadState which needs to lazily initialize 
//    JvmdiFramePops).
// 3. An oops_do entry point for GC'ing the breakpoint array.
//

class JvmdiCurrentBreakpoints : public AllStatic {

private:

  // Current breakpoints, lazily initialized by get_jvmdi_breakpoints();
  static JvmdiBreakpoints *_jvmdi_breakpoints;

  // NULL terminated cache of byte-code pointers corresponding to current breakpoints.
  // Updated only at safepoints (with listener_fun) when the cache is moved. 
  // It exists only to make is_breakpoint fast.
  static address          *_breakpoint_list;
  static inline void set_breakpoint_list(address *breakpoint_list) { _breakpoint_list = breakpoint_list; }
  static inline address *get_breakpoint_list()                     { return _breakpoint_list; }

  // Listener for the GrowableCache in _jvmdi_breakpoints, updates _breakpoint_list.
  static void listener_fun(void *this_obj, address *cache);

public:
  static void initialize();
  static void destroy();

  // lazily create _jvmdi_breakpoints and _breakpoint_list
  static JvmdiBreakpoints& get_jvmdi_breakpoints();

  // quickly test whether the bcp matches a cached breakpoint in the list
  static inline bool is_breakpoint(address bcp);

  static void oops_do(OopClosure* f);
  static void gc_epilogue();
};

// quickly test whether the bcp matches a cached breakpoint in the list
bool JvmdiCurrentBreakpoints::is_breakpoint(address bcp) {
    address *bps = get_breakpoint_list();
    if (bps == NULL) return false;
    for ( ; (*bps) != NULL; bps++) {
      if ((*bps) == bcp) return true;
    }
    return false;
}


//
// class JvmdiFrame
// Used by              : JvmdiThreadState.
// Used by JVMDI methods: none directly.
// 
// Wrapper for hotspot vframe object
//
// Usage outside JvmdiThreadState is extremely discouraged because of synchronization issues
//

class JvmdiFrame : public CHeapObj {
private:
  int           _magic;
  JavaThread *  _thread;
  // Depth of 0 indicates the topmost frame on the stack
  int           _depth;
  jframeID      _frameID;

public:
  JvmdiFrame(JavaThread *thread, int depth, const int magic, jframeID fid)   {
    assert(thread != NULL, "thread != NULL");
    assert(magic != 0,     "magic != 0");
    assert(depth >= 0,     "depth >= 0");

    _magic  = magic;
    _thread = thread;
    _depth  = depth;
    _frameID = fid;
  }
  ~JvmdiFrame()            { _magic = 0;  _thread = NULL; }
  JavaThread *get_thread() { return _thread;     }
  int get_depth()          { return _depth;      }
  jframeID get_jframeID()  { return _frameID;    }
  int magic()              { return _magic;      }
  vframe *get_vframe();

  bool verify();

  jvmdiError get_local(jint n, oop     *o);
  jvmdiError get_local(jint n, jint    *i);
  jvmdiError get_local(jint n, jlong   *l);
  jvmdiError get_local(jint n, jfloat  *f);
  jvmdiError get_local(jint n, jdouble *d);

  jvmdiError set_local(jint n, Handle *o);
  jvmdiError set_local(jint n, jint    i);
  jvmdiError set_local(jint n, jlong   l);
  jvmdiError set_local(jint n, jfloat  f);
  jvmdiError set_local(jint n, jdouble d);
};


// The get/set local operations must only be done by the VM thread
// because the interpreter version needs to access oop maps, which can
// only safely be done by the VM thread
class VM_GetOrSetLocal : public VM_Operation {
private:
  JvmdiFrame* _f;
  int         _index;
  BasicType   _type;
  jvalue      _value;
  Handle*     _obj;        // For setting objects
  oop         _oop_result; // For getting objects
  bool        _set;

  jvmdiError  _result;

  bool check_index(StackValueCollection* c, int i);

public:
  // Constructor for getter
  VM_GetOrSetLocal(JvmdiFrame *f, int index, BasicType type);

  // Constructor for non-object setter
  VM_GetOrSetLocal(JvmdiFrame *f, int index, BasicType type, jvalue value);

  // Constructor for object setter
  VM_GetOrSetLocal(JvmdiFrame *f, int index, Handle* value);

  jvalue value()      { return _value; }
  oop    oop_value()  { return _oop_result; }
  jvmdiError result() { return _result; }

  void doit();
  bool allow_nested_vm_operations() const;
  const char* name() const                       { return "get/set locals"; }
};


//
// class JvmdiFramePop
// Used by              : JvmdiFramePops
// Used by JVMDI methods: none directly.
//
// Wrapper class for FramePop, used in the JvmdiFramePops class.
//

class JvmdiFramePop VALUE_OBJ_CLASS_SPEC {
 private:  
  // Frame number counting from BOTTOM (oldest) frame; bottom frame ==
  // #0
  int _frame_number;
 public:
  JvmdiFramePop() {}
  JvmdiFramePop(int frame_number) {
    assert(frame_number >= 0, "invalid frame number");
    _frame_number = frame_number;
  }

  int frame_number() { return _frame_number; }
  int above_on_stack(JvmdiFramePop& other) { return _frame_number > other._frame_number; }
  void print() PRODUCT_RETURN;
};


//
// class VM_ChangeFramePops
// Used by              : JvmdiFramePops
// Used by JVMDI methods: none directly.
// Note: A Helper class.
//
// VM_ChangeFramePops implements a VM_Operation for adding a FramePop to the JvmdiFramePops class.
//

class VM_ChangeFramePops : public VM_Operation {
 private:
  JvmdiFramePops* _frame_pops;
  JvmdiFramePop   _fp;

 public:
  VM_ChangeFramePops(JvmdiFramePops* frame_pops, JvmdiFramePop fp) {
    _frame_pops = frame_pops;
    _fp = fp;
    assert(frame_pops != NULL, "frame_pops != NULL");
  }
  void doit();
  bool allow_nested_vm_operations() const;
  const char* name() const                       { return "change frame pops"; }  
};


//
// class JvmdiFramePops
// Used by              : JvmdiThreadState
// Used by JVMDI methods: none directly.
//
// A collection of JvmdiFramePop.
// It records what frames on a threads stack should post frame_pop events when they're exited.
//

class JvmdiFramePops : public CHeapObj {
 private: 
  GrowableArray<int> _pops;
  JvmdiThreadState   *_threadstate;

  // should only be used by VM_ChangeFramePops
  // to insure they only occur at safepoints.
  // Todo: add checks for safepoint
  friend class VM_ChangeFramePops;
  void set_at_safepoint(JvmdiFramePop& fp);
  JvmdiThreadState* threadstate() { return _threadstate; }
  
 public:
  JvmdiFramePops(JvmdiThreadState *threadstate);

  // Are frame pops enabled?
  bool enabled() const;

  // clears the stored framepop.
  // should only be called by thread that owns the frame.
  void clear(JvmdiFramePop& fp);

  // clears all framepops above and including this one on the stack.
  // should only be called by thread that owns the frame.
  void clear_to(JvmdiFramePop& fp);

  bool contains(JvmdiFramePop& fp) { return _pops.contains(fp.frame_number()); }

  int length() { return _pops.length(); }
  void set(JvmdiFramePop& fp);
  void print() PRODUCT_RETURN;
};


//
// class JvmdiThreads
// Used by              : JvmdiInternal
// Used by JVMDI methods: GetCurrentFrame, GetCallerFrame, GetFrameLocation, NotifyFramePop, GetLocal*, SetLocal*
//
// Convenience routines for suspending and resuming threads.
// JvmdiThreads provides MT-safe methods for creating, destroying and examining
// JvmdiThreads and JvmdiFrames.
// 
// All attempts by JVMDI to suspend and resume threads must go through the 
// JvmdiThreads interface, allowing the class to precisely determine 
// which jframeID's are valid (correspond to an actual suspended frame) and which are stale.
//
// Threads are suspended with JVM_Suspend, insuring that the thread gets stopped at a safepoint
//
// All public methods are synchronized by the Threads_lock to guarantee: 
//  1. the set of valid jframeID's remains constant during the method's execution.
//  2. the set of JVMDI suspended threads remains constant during the method's execution.
//
// methods return true when successful and false when given an invalid jframeID 
// or an unwalkable JavaThread.
//
class JvmdiThreads : public AllStatic {

 private:
  static jframeID _last_jframeID;

  // get a JvmdiFrame from a jframeID
  static JvmdiFrame *get_frame(jframeID fid);

  static JavaThread *get_thread(jframeID fid);

  // Check to see whether we can operate on this thread
  static bool check_thread(JavaThread* thread);

 public:

  // suspend the thread, taking it to a safepoint
  static bool suspend(JavaThread *java_thread);
  // resume the thread
  static bool resume(JavaThread *java_thread);
  // is a thread suspended?
  static bool suspended(JavaThread *java_thread);

  // return the jframeID of the current Java frame in the thread.
  static jframeID current_frame(JavaThread *java_thread);
  // create the next jframeID
  static jframeID next_jframeID();
  // return the number of frames in the thread.
  static int          nFrames(JavaThread *java_thread);

  static bool is_opaque(jframeID fid, bool return_true_for_native_frames);

  // get the parent's jframeID
  static jframeID get_caller(jframeID child);
  // get the current klass, method and location for a jframeID
  static bool get_frame_location(jframeID frame, jclass *klass, jmethodID *method, jlocation *location);

  // add a frame pop notification to the thread for the jframeID
  static bool notify_frame_pop(jframeID fid);

  // get a local value from frame slot n
  static jvmdiError get_local(jframeID fid, jint n, Handle  *h);
  static jvmdiError get_local(jframeID fid, jint n, jint    *i);
  static jvmdiError get_local(jframeID fid, jint n, jlong   *l);
  static jvmdiError get_local(jframeID fid, jint n, jfloat  *f);
  static jvmdiError get_local(jframeID fid, jint n, jdouble *d);

  // set a local value in frame slot n
  static jvmdiError set_local(jframeID fid, jint n, Handle *h);
  static jvmdiError set_local(jframeID fid, jint n, jint    i);
  static jvmdiError set_local(jframeID fid, jint n, jlong   l);
  static jvmdiError set_local(jframeID fid, jint n, jfloat  f);
  static jvmdiError set_local(jframeID fid, jint n, jdouble d);

  static void print();
  static void print_frame_pops(jframeID frame);

};


//
// class JvmdiThreadState
// Used by              : JvmdiInternal, JvmdiThreads
// Used by JVMDI methods: post_*_event,
//                        GetCurrentFrame, GetCallerFrame, GetFrameLocation, NotifyFramePop, GetLocal*, SetLocal*
//
// The Jvmdi state for each thread:
// 1. Local table of enabled events.
// 2. Cache of pending frame_pop_events, created by NotifyFramePop
//    and lazily initialized.
// 3: Location of last executed instruction, used to filter out duplicate
//    events due to instruction rewriting.
// 4. Cache of JvmdiFrames returned to the outside world during stack
//    walking.
//
// JVMDI calls cannot usually examine the frames in the stack of a running JavaThread, 
// except under the following scenarios:
//
// 1. The thread has been suspended by the JVMDI hSuspendThread call.
// 2. The thread has called (and is still inside) the JVMDI event hook function.
// 3. The thread is in a native method and examination is occurring from within that native method.
// 
// In these cases the frames of the thread are examinable and modifiable using JVMDI calls.
// Threads which JVMDI can examine I'll call 'walkable.'
//
// JVMDI refers to a frame by a 32-bit jframeID value and gives the implementor full choice about what
// it represents. jframeID's are valid only as long as the thread they refer to remains walkable.
// Once a thread leaves the walkable state all its jframeID's become stale.
//
// The calls in JVMDI (GetFrameLocation) introduce another wrinkle, by requiring that a jframeID 
// map directly to its thread.
//
// This leads to the following implementation possibilities:
//
// 1. The simplest implementation of jframeID would be the frame's SP. However, this leaves no way to
// detect stale jframeID's or (for hotspot) easily map back to the appropriate thread.
//
// 2. Another simple implementation would use a pointer to a JvmdiFrame object, which could be marked stale,
// but would need to be collected at some point (this is the approach used by classic). Once the JvmdiFrame object
// was collected you would lose the ability to detect a pointer to it as stale. By dereferencing a stale pointer
// you might introduce bugs that show up much later and are difficult to detect.
//
// 3. A more complex alternative (which may not be a good idea?!?) can detect stale jframeID's
// which in the presence of buggy JVMDI user code might help licensee's (and our JBUG people) more. 
//
// 4. A combination of all of the above with some changes, of course. 
// A sequence number is used as the jframeID. This makes the
// jframeID unique for a particular frame. The JvmdiFrame
// object is used as a wrapper around the vframe.
// It also contains a "magic" value to make it
// easier to detect stale JvmdiFrames.
//
// Currently, we use (4).
//
// NOTE: this is pretty complicated and could probably be further simplified.
// NEEDS_CLEANUP
class JvmdiThreadState : public CHeapObj {
 private:
  JavaThread        *_thread;
  JvmdiEventEnabled *_local_event_enabled;
  bool              _exception_detected;
  bool              _exception_caught;
  JNIid*            _current_method_id;
  int               _current_bci;
  bool		    _breakpoint_posted;
  bool		    _single_stepping_posted;
  bool              _hide_single_stepping;
  bool              _pending_step_for_popframe;
  int               _hide_level;

  // Used by interpreter in fullspeed mode for single-stepping and
  // frame pop support
  int               _interp_only_mode;
  // This is only valid when is_interp_only_mode() returns true
  int               _cur_stack_depth;
  
  //
  // Frame routines
  //

  // JvmdiFrames that have been for this thread
  GrowableArray<JvmdiFrame *> *_frames;

  // helper method to find JvmdiFrame that contains fp
  static bool equalJFrameID(void* fid, JvmdiFrame *jvmdi_frame);

  // assure the the stack is of the specified depth (and _frames is filled-in)
  bool assure_depth(int depth);

  // convert a frame depth to a jframeID (no_more_frames_jframeID() if not that deep)
  jframeID depth_to_jframeID(int depth);

 public:
  JvmdiThreadState(JavaThread *thread);
  ~JvmdiThreadState();

  // used by JvmdiEventDispatcher  
  bool is_enabled(jint event_type) const;           // is event_type enabled for this thread? 
  void set_enabled(jint event_type, bool enabled);  // enable/disable event_type for this thread   
  // Used by the interpreter for fullspeed debugging support
  static ByteSize interp_only_mode_offset() { return byte_offset_of(JvmdiThreadState, _interp_only_mode); }
  bool is_interp_only_mode()                { return (_interp_only_mode != 0); }
  void enable_interp_only_mode();
  void disable_interp_only_mode();
  // Current stack depth is only valid when is_interp_only_mode() returns true.
  // Returns the number of Java activations on the stack.
  int cur_stack_depth()                     { assert(is_interp_only_mode(),
                                                     "cur_stack_depth only valid when is_interp_only_mode() is true");
                                              return _cur_stack_depth; }
  void set_cur_stack_depth(int depth)       { _cur_stack_depth = depth; }
  void incr_cur_stack_depth()               { assert(is_interp_only_mode(),
                                                     "incr_cur_stack_depth only valid when is_interp_only_mode() is true");
                                              ++_cur_stack_depth; }
  void decr_cur_stack_depth()               { assert(is_interp_only_mode(),
                                                     "decr_cur_stack_depth only valid when is_interp_only_mode() is true");
                                              --_cur_stack_depth;
                                              assert(_cur_stack_depth >= 0, "incr/decr_cur_stack_depth mismatch"); }

  inline JavaThread *get_thread()      { return _thread;              }
  inline bool is_exception_detected()  { return _exception_detected;  }
  inline bool is_exception_caught()    { return _exception_caught;  }
  inline void set_exception_detected() { _exception_detected = true; 
                                         _exception_caught = false; }
  inline void set_exception_caught()   { _exception_caught = true;
                                         _exception_detected = false; }
  inline void set_current_location(JNIid* method_id, int bci) { _current_method_id = method_id; 
                                                                _current_bci  = bci;}
  inline void get_current_location(JNIid** method_id, int* bci) { *method_id = _current_method_id; 
                                                                  *bci = _current_bci;}

  inline void set_breakpoint_posted()  { _breakpoint_posted = true; }
  inline void set_single_stepping_posted() {
    _single_stepping_posted = true;
  }
  inline bool breakpoint_posted() { return _breakpoint_posted; }
  inline bool single_stepping_posted() { 
    return _single_stepping_posted;
  }

  inline void clear_hide_single_stepping() {
    if (_hide_level > 0) {
      _hide_level--;
    } else {
      assert(_hide_single_stepping, "hide_single_stepping is out of phase");
      _hide_single_stepping = false;
    }
  }
  inline bool hide_single_stepping() { return _hide_single_stepping; }
  inline void set_hide_single_stepping() {
    if (_hide_single_stepping) {
      _hide_level++;
    } else {
      assert(_hide_level == 0, "hide_level is out of phase");
      _hide_single_stepping = true;
    }
  }

  // If the thread is in the given method at the given 
  // location just return.  Otherwise, reset the current location
  // and reset _breakpoint_posted and _single_stepping_posted.
  // _breakpoint_posted and _single_stepping_posted are only cleared
  // here.
  void compare_and_set_current_location(methodOop method, address location, jint event);

  // Step pending flag is set when PopFrame is called and it is cleared
  // when step for the Pop Frame is completed.
  // This logic is used to distinguish b/w step for pop frame and repeat step.
  void set_pending_step_for_popframe() { _pending_step_for_popframe = true; }
  void clear_pending_step_for_popframe() { _pending_step_for_popframe = false; }
  bool is_pending_step_for_popframe() { return _pending_step_for_popframe; }
    
            
  // Todo: get rid of this!  
 private:
  bool _debuggable;
 public:
  // Should the thread be enumerated by jvmdiInternal::GetAllThreads?
  bool is_debuggable()                 { return _debuggable; }
  // If a thread cannot be suspended (has no valid last_java_frame) then it gets marked !debuggable
  void set_debuggable(bool debuggable) { _debuggable = debuggable; }

 private:
  bool _in_native_code;  // User's most recent activation is native code

 public:
  // Touched only by the current thread
  void set_in_native_code();
  void clear_in_native_code();
  bool is_in_native_code();

  // This should be used when a thread becomes not walkable
  // Used by both the current thread as well as suspend/resumption
  // (but not both at the same time)
  void clear_cached_frames();

  bool may_be_walked();

 private:
  // Class used to store pending framepops.
  // lazily initialized by get_jvmdi_frame_pops();
  JvmdiFramePops *_jvmdi_frame_pops;

 public:

  // lazily initialize _jvmdi_frame_pops and _frame_pop_list
  JvmdiFramePops* get_jvmdi_frame_pops();  

  // quickly test whether we should deliver a frame pop event on return from sp
  inline bool is_frame_pop(int cur_stack_depth);

#ifndef PRODUCT
 private:
  GrowableArray<JvmdiMonitor*> *_monitors;
  int _jvmdi_count;

 public:
  void enter_jvmdi_monitor(JvmdiMonitor *jvmdi_monitor) {
    assert(jvmdi_monitor != NULL, "jvmdi_monitor != NULL");
    assert(JavaThread::current() == _thread, "Only this thread can change _monitors");
    get_monitors()->append(jvmdi_monitor);
  }
  void exit_jvmdi_monitor(JvmdiMonitor *jvmdi_monitor) {
    assert(jvmdi_monitor != NULL, "jvmdi_monitor != NULL");
    assert(JavaThread::current() == _thread, "Only this thread can change _monitors");
    assert(get_monitors()->find(jvmdi_monitor) != -1, "Unmatched Monitor Exit");
    get_monitors()->remove(jvmdi_monitor);
  }

  GrowableArray<JvmdiMonitor*> *get_monitors() {
    if (_monitors == NULL) {
      _monitors = new (ResourceObj::C_HEAP) GrowableArray<JvmdiMonitor*>(5,true);
    }
    return _monitors;
  }

  int get_jvmdi_count()           { return _jvmdi_count; }
  void set_jvmdi_count(int count) { _jvmdi_count = count; }
#endif

  void print_jvmdi_monitors() {
#ifndef PRODUCT
    int len = get_monitors()->length();
    for (int i=0; i<len; i++) {
      tty->print("%d: ", i);
      JvmdiMonitor *jvmdi_monitor = get_monitors()->at(i);
      jvmdi_monitor->print(*tty);
      tty->print_cr("");
    }
#endif
  }

  //
  // Frame routines
  //

 private:
  bool verify_jvmdi_frame(JvmdiFrame *jvmdi_frame);

 public:

  //  true when the thread was suspended with a pointer to the last Java frame.
  bool has_last_frame()                     { return _thread->has_last_Java_frame(); }

  // convert between JvmdiFrame and jframeID (verify frames in debug build)
  JvmdiFrame *from_jframeID(jframeID fid);
  jframeID to_jframeID(JvmdiFrame *jvmdi_frame);
  static jframeID invalid_jframeID();
  static jframeID no_more_frames_jframeID();
  static bool is_error_jframeID(jframeID fid);

  // Methods for accessing and traversing frames
  int nFrames();
  jframeID current_frame();
  jframeID get_caller(jframeID fid);
  bool is_opaque(jframeID fid, bool return_true_for_native_frames);
  bool get_frame_location(jframeID fid, methodHandle *method_handle, jint *bci);
  void pop_top_frame();
};

bool JvmdiThreadState::is_frame_pop(int cur_frame_number) {
  if (!is_interp_only_mode()) return false;
  JvmdiFramePop fp(cur_frame_number);
  return get_jvmdi_frame_pops()->contains(fp);
}

//
// class DebuggerThread
// Used by: JvmdiInternal
//
// JavaThread used to wrap a thread started by the debugger dll
// using the Jvmdi method RunDebugThread.
//
class DebuggerThread : public JavaThread {

  JVMDI_StartFunction _start_fn;
  void *_start_arg;
  static void start_function_wrapper(JavaThread *thread, TRAPS);

 public:
  DebuggerThread(JVMDI_StartFunction start_fn, void *start_arg);
};


//
// class JvmdiInternal
//
// Internal helper class for jvmdi. Contains:
// 1. Jvmdi interface implementations.
// 2. Helper methods and state.
//

class JvmdiInternal : AllStatic {

  friend class jvmdi;
  friend class JvmdiObj;

 private:
  static bool              _jvmdi_initialized;
  static JVMDI_Interface_1 _jvmdi_interface;
  static JVMDI_EventHook   _event_hook;
  static JavaVM*           _vm;

  // Hooks for Allocate/DeAllocate functions, if they've been set.
  // never use these directly, always call jvmdiMalloc/Free.
  static JVMDI_AllocHook          _ahook;
  static JVMDI_DeallocHook        _dhook;

  // wrapper functions for posting events
  static void post_event(JvmdiEvent& e);
  static void post_proxy_event(JvmdiEventProxy& e, JavaThread *jthread);

  // helper methods for creating arrays of global JNI Handles from local Handles
  static jobject *new_jobjectArray(int length, Handle *handles);

  // extra methods so we can have "C" linkage static functions in separate files
  // and yet still get the JVMDI_Interface_1 initialized in a single call.
  static void set_info_hooks(JVMDI_Interface_1 *jvmdi_interface);
  static void set_info2_hooks(JVMDI_Interface_1 *jvmdi_interface);
#ifdef HOTSWAP
  static void set_hotswap_hooks(JVMDI_Interface_1 *jvmdi_interface);
#endif HOTSWAP

public:

  // Convienence wrappers for calls to Allocate/DeAllocate
  // use for all memory that could escape outside the interface.
  static jbyte *jvmdiMalloc(jlong size);
  static void   jvmdiFree(jbyte *mem);
  
  static jvmdiError AllocateInternal(jlong size, jbyte** memPtr);

  static jvmdiError DeallocateInternal(jbyte* mem);

  static void  set_allocation_hooks(JVMDI_AllocHook ahook, JVMDI_DeallocHook dhook);

  // helper methods for creating arrays of global JNI Handles from local Handles
  static jthread *new_jthreadArray(int length, Handle *handles);
  static jthreadGroup *new_jthreadGroupArray(int length, Handle *handles);

  // return a valid string no matter what state the threadObj is in
  static const char *safe_get_thread_name(oop threadObj);

  // get a field descriptor for the specified class and field
  static bool get_field_descriptor(klassOop k, jfieldID field, fieldDescriptor* fd);
  // convert from JNIHandle to JavaThread *
  static JavaThread *get_JavaThread(jthread jni_thread);  
  // convert from jclass to klassOop
  static klassOop get_klassOop(jclass clazz);
  // convert from jmethodID to methodOop
  static methodOop get_methodOop(jmethodID method, klassOop k);
  // convert to a jni jmethodID from a non-null methodOop
  static jmethodID get_jni_method_non_null(methodOop method);
  // convert to a jni jclass from a non-null klassOop
  static jclass get_jni_class_non_null(klassOop k);

  static jobject get_jni_object(oop o);
  static oop get_Oop(jobject obj);

  // Call to create and destroy internal data structures
  static void jvmdi_init();
  static void jvmdi_init_done();
  static void jvmdi_death();
  
  static void default_event_hook(JNIEnv *env, JVMDI_Event *event);  

  static JVMDI_Interface_1* get_jvmdi_interface()   { return &_jvmdi_interface; }
  static JVMDI_EventHook get_event_hook()           { return _event_hook; }
  static void set_event_hook(JVMDI_EventHook hook)  { _event_hook = hook; }
  static JavaVM* get_vm()                           { return _vm; }
  static void set_vm(JavaVM* vm)                    { _vm = vm; }
  static bool is_jvmdi_initialized()                { return _jvmdi_initialized; }
  static bool is_thread_fully_suspended(JavaThread *thread, bool wait_for_suspend);
};

// These aren't part of JvmdiInternal because you don't seem able to declare a
// static function in a class and have it have "C" linkage. Since these pointers
// to these functions ultimately get assigned to a C defined struct and you can't
// cast away the linkage (silently) they must be outside of the class as much as
// we'd like them internal.
//
extern "C" {
 // Functions exported from the VM to the debugger.
  static jvmdiError JNICALL SetEventHook(JVMDI_EventHook hook);
  static jvmdiError JNICALL SetEventNotificationMode(jint mode, jint eventType, jthread jni_thread, ...);
  static jvmdiError JNICALL GetThreadStatus(jthread jni_thread, jint *threadStatusPtr, jint *suspendStatusPtr);
  static jvmdiError JNICALL GetAllThreads(jint *threadsCountPtr, jthread **threadsPtr);
  // Need to avoid collision with SuspendThread and ResumeThread defined
  // within Visual C++ environment
  static jvmdiError JNICALL hSuspendThread(jthread jni_thread);
  static jvmdiError JNICALL hResumeThread(jthread jni_thread);
  static jvmdiError JNICALL StopThread(jthread jni_thread, jobject exception);
  static jvmdiError JNICALL InterruptThread(jthread jni_thread);
  static jvmdiError JNICALL GetThreadInfo(jthread jni_thread, JVMDI_thread_info *infoPtr);
  static jvmdiError JNICALL GetOwnedMonitorInfo(jthread jni_thread, JVMDI_owned_monitor_info *infoPtr);
  static jvmdiError JNICALL GetCurrentContendedMonitor(jthread jni_thread, jobject *monitor);
  static jvmdiError JNICALL RunDebugThread(jthread jni_thread, JVMDI_StartFunction proc, void *arg, int priority);
  static jvmdiError JNICALL GetTopThreadGroups(jint *groupCountPtr, jthreadGroup **groupsPtr);
  static jvmdiError JNICALL GetThreadGroupInfo(jthreadGroup group, JVMDI_thread_group_info *infoPtr);
  static jvmdiError JNICALL GetThreadGroupChildren(jthreadGroup group, jint *threadCountPtr, jthread **threadsPtr, jint *groupCountPtr, jthreadGroup **groupsPtr);
  static jvmdiError JNICALL GetFrameCount(jthread jni_thread, jint *countPtr);
  static jvmdiError JNICALL GetCurrentFrame(jthread jni_thread, jframeID *framePtr);
  static jvmdiError JNICALL GetCallerFrame(jframeID called, jframeID *framePtr);
  static jvmdiError JNICALL GetFrameLocation(jframeID frame, jclass *classPtr, jmethodID *methodPtr, jlocation *locationPtr);
  static jvmdiError JNICALL NotifyFramePop(jframeID frame);
  static jvmdiError JNICALL GetLocalObject(jframeID frame, jint slot, jobject *valuePtr);
  static jvmdiError JNICALL GetLocalInt(jframeID frame, jint slot, jint *valuePtr);
  static jvmdiError JNICALL GetLocalLong(jframeID frame, jint slot, jlong *valuePtr);
  static jvmdiError JNICALL GetLocalFloat(jframeID frame, jint slot, jfloat *valuePtr);
  static jvmdiError JNICALL GetLocalDouble(jframeID frame, jint slot, jdouble *valuePtr);
  static jvmdiError JNICALL SetLocalObject(jframeID frame, jint slot, jobject value);
  static jvmdiError JNICALL SetLocalInt(jframeID frame, jint slot, jint value);
  static jvmdiError JNICALL SetLocalLong(jframeID frame, jint slot, jlong value);
  static jvmdiError JNICALL SetLocalFloat(jframeID frame, jint slot, jfloat value);
  static jvmdiError JNICALL SetLocalDouble(jframeID frame, jint slot, jdouble value);
  static jvmdiError JNICALL CreateRawMonitor(char *name, JVMDI_RawMonitor *monitorPtr);
  static jvmdiError JNICALL DestroyRawMonitor(JVMDI_RawMonitor monitor);
  static jvmdiError JNICALL RawMonitorEnter(JVMDI_RawMonitor monitor);
  static jvmdiError JNICALL RawMonitorExit(JVMDI_RawMonitor monitor);
  static jvmdiError JNICALL RawMonitorWait(JVMDI_RawMonitor monitor, jlong millis);
  static jvmdiError JNICALL RawMonitorNotify(JVMDI_RawMonitor monitor);
  static jvmdiError JNICALL RawMonitorNotifyAll(JVMDI_RawMonitor monitor);
  static jvmdiError JNICALL SetBreakpoint(jclass clazz, jmethodID method, jlocation location);
  static jvmdiError JNICALL ClearBreakpoint(jclass clazz, jmethodID method, jlocation location);
  static jvmdiError JNICALL ClearAllBreakpoints(void);
  static jvmdiError JNICALL SetFieldAccessWatch(jclass clazz, jfieldID field);
  static jvmdiError JNICALL ClearFieldAccessWatch(jclass clazz, jfieldID field);
  static jvmdiError JNICALL SetFieldModificationWatch(jclass clazz, jfieldID field);
  static jvmdiError JNICALL ClearFieldModificationWatch(jclass clazz, jfieldID field);
  static jvmdiError JNICALL SetAllocationHooks(JVMDI_AllocHook ahook, JVMDI_DeallocHook dhook);
  static jvmdiError JNICALL Allocate(jlong size, jbyte** memPtr);
  static jvmdiError JNICALL Deallocate(jbyte* mem);
  static jvmdiError JNICALL GetClassSignature(jclass clazz, char **sigPtr);
  static jvmdiError JNICALL GetClassStatus(jclass clazz, jint *statusPtr);
  static jvmdiError JNICALL GetSourceFileName(jclass clazz, char **sourceNamePtr);
  static jvmdiError JNICALL GetSourceDebugExtension(jclass clazz, char **sourceDebugExtensionPtr);
  static jvmdiError JNICALL GetClassModifiers(jclass clazz, jint *modifiersPtr);
  static jvmdiError JNICALL GetClassMethods(jclass clazz, jint *methodCountPtr, jmethodID **methodsPtr);
  static jvmdiError JNICALL GetClassFields(jclass clazz, jint *fieldCountPtr, jfieldID **fieldsPtr);
  static jvmdiError JNICALL GetImplementedInterfaces(jclass clazz, jint *interfaceCountPtr, jclass **interfacesPtr);
  static jvmdiError JNICALL IsInterface(jclass clazz, jboolean *isInterfacePtr);
  static jvmdiError JNICALL IsArrayClass(jclass clazz, jboolean *isArrayClassPtr);
  static jvmdiError JNICALL GetClassLoader(jclass clazz, jobject *classloaderPtr);
  static jvmdiError JNICALL GetObjectHashCode(jobject object, jint *hashCodePtr);
  static jvmdiError JNICALL XGetMonitorInfo(jobject object, JVMDI_monitor_info *infoPtr);
  static jvmdiError JNICALL GetFieldName(jclass clazz, jfieldID field, char **namePtr, char **signaturePtr);
  static jvmdiError JNICALL GetFieldDeclaringClass(jclass clazz, jfieldID field, jclass *declaringClassPtr);
  static jvmdiError JNICALL GetFieldModifiers(jclass clazz, jfieldID field, jint *modifiersPtr);
  static jvmdiError JNICALL IsFieldSynthetic(jclass clazz, jfieldID field, jboolean *isSyntheticPtr);
  static jvmdiError JNICALL GetMethodName(jclass clazz, jmethodID method, char **namePtr, char **signaturePtr);
  static jvmdiError JNICALL GetMethodDeclaringClass(jclass clazz, jmethodID method, jclass *declaringClassPtr);
  static jvmdiError JNICALL GetMethodModifiers(jclass clazz, jmethodID method, jint *modifiersPtr);
  static jvmdiError JNICALL GetMaxStack(jclass clazz, jmethodID method, jint *maxPtr);
  static jvmdiError JNICALL GetMaxLocals(jclass clazz, jmethodID method, jint *maxPtr);
  static jvmdiError JNICALL GetArgumentsSize(jclass clazz, jmethodID method, jint *sizePtr);
  static jvmdiError JNICALL GetLineNumberTable(jclass clazz, jmethodID method, jint *entryCountPtr, JVMDI_line_number_entry **tablePtr);
  static jvmdiError JNICALL GetMethodLocation(jclass clazz, jmethodID method, jlocation *startLocationPtr, jlocation *endLocationPtr);
  static jvmdiError JNICALL GetLocalVariableTable(jclass clazz, jmethodID method, jint *entryCountPtr, JVMDI_local_variable_entry **tablePtr);
  static jvmdiError JNICALL GetExceptionHandlerTable(jclass clazz, jmethodID method, jint *entryCountPtr, JVMDI_exception_handler_entry **tablePtr);
  static jvmdiError JNICALL GetThrownExceptions(jclass clazz, jmethodID method, jint *exceptionCountPtr, jclass **exceptionsPtr);
  static jvmdiError JNICALL GetBytecodes(jclass clazz, jmethodID method, jint *bytecodeCountPtr, jbyte **bytecodesPtr);
  static jvmdiError JNICALL IsMethodNative(jclass clazz, jmethodID method, jboolean *isNativePtr);
  static jvmdiError JNICALL IsMethodSynthetic(jclass clazz, jmethodID method, jboolean *isSyntheticPtr);
  static jvmdiError JNICALL GetLoadedClasses(jint *classCountPtr, jclass **classesPtr);
  static jvmdiError JNICALL GetClassLoaderClasses(jobject initiatingLoader, jint *classesCountPtr, jclass **classesPtr);
  static jvmdiError JNICALL GetVersionNumber(jint *versionPtr);
  static jvmdiError JNICALL GetCapabilities(JVMDI_capabilities *capabilitiesPtr);
}


// Utility macro that checks for NULL pointers:
#define NULL_CHECK(X, Y) if ((X) == NULL) { JVMDI_RETURN(Y); } 

// Variant on the CHECK macro for returning JVMDI_ERROR_INTERNAL
#define  JVMDI_CHECK  THREAD); if (HAS_PENDING_EXCEPTION) { CLEAR_PENDING_EXCEPTION; return JVMDI_ERROR_INTERNAL; }; (0
