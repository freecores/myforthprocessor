#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vframe_hp.hpp	1.43 03/01/23 12:26:47 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A deoptimizedVFrame is represented by a frame and an offset
// into the packed array of frames.

class deoptimizedVFrame: public javaVFrame {
 public:
  // JVM state
  methodOop                    method()         const;
  int                          bci()            const;
  StackValueCollection*        locals()         const;
  StackValueCollection*        expressions()    const;
  GrowableArray<MonitorInfo*>* monitors()       const;

  void set_locals(StackValueCollection* values) const;

  // Virtuals defined in vframe
  bool is_deoptimized_frame() const { return true; }
  vframe* sender() const;
  bool is_top() const;

  // Casting
  static deoptimizedVFrame* cast(vframe* vf) {
    assert(vf == NULL || vf->is_deoptimized_frame(), "must be deoptimized frame");
    return (deoptimizedVFrame*) vf;
  }

 public:
  // Constructor
  deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread);
  deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread, int index);
  deoptimizedVFrame(vframeArray* array);
  deoptimizedVFrame(vframeArray* array, int index);
  deoptimizedVFrame(const frame* fr, const RegisterMap* reg_map, vframeArray* array, int index);
 private:
  deoptimizedVFrame(const frame* fr, vframeArray* array, int index);

 private:
  int            _index;
  vframeArray* _array;

  int            index() const { return _index; }
  vframeArray* array() const { return _array; }  

 public:
  // Returns the on stack word size for this frame
  // callee_parameters is the number of callee locals residing inside this frame
  int on_stack_size(int callee_parameters, int callee_locals, int popframe_extra_stack_expression_els, bool is_top_frame) const;

  int raw_bci() const;  

  // Unpacks the vframe to skeletal interpreter frame
  void unpack_vframe_on_stack(int callee_parameters, 
			      int callee_locals,
			      frame* caller,
			      frame* interpreter_frame,
			      bool is_top_frame,
			      int exec_mode);
  // Returns sender in a vframe array.
  deoptimizedVFrame *deoptimized_sender_or_null() const;
};

class compiledVFrame: public javaVFrame {
 public:
  // JVM state
  methodOop                    method()         const;
  int                          bci()            const;
  StackValueCollection*        locals()         const;
  StackValueCollection*        expressions()    const;
  GrowableArray<MonitorInfo*>* monitors()       const;

  void set_locals(StackValueCollection* values) const;

  // Virtuals defined in vframe
  bool is_compiled_frame() const { return true; }
  vframe* sender() const;
  bool is_top() const;

  // Casting
  static compiledVFrame* cast(vframe* vf) {
    assert(vf == NULL || vf->is_compiled_frame(), "must be compiled frame");
    return (compiledVFrame*) vf;
  }

 public:
  // Constructors  
  compiledVFrame(const frame* fr, const RegisterMap* reg_map, JavaThread* thread, ScopeDesc* scope);

  // Returns the active nmethod
  nmethod*  code() const;

  // Returns the scopeDesc
  ScopeDesc* scope() const { return _scope; }

  // Returns SynchronizationEntryBCI or bci() (used for synchronization)
  int raw_bci() const;

 protected:
  ScopeDesc* _scope;

  //StackValue resolve(ScopeValue* sv) const;
  BasicLock* resolve_monitor_lock(Location location) const;
  StackValue *create_stack_value(ScopeValue *sv) const;
  void write_stack_value(ScopeValue *scope_val, StackValue *stack_val) const;

#ifndef PRODUCT
 public:
  void verify() const;
#endif
};

