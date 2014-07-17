#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)inlining.hpp	1.23 03/01/23 12:01:24 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//-------------------------------------------------------------
// The inlining policy decides what to inline or not inline; it is shared (in part) by
// the compiler and the recompiler.
class InliningPolicy : public AllStatic {
 public:
  static const char* shouldInline   (methodHandle callee, float frequency, int cnt);
  // positive filter: should send be inlined?  returns NULL (--> yes) or rejection msg 
  static const char* shouldNotInline(methodHandle callee, Scope*const caller);
  // negative filter: should send NOT be inlined?  returns NULL (--> inline) or rejection msg 

  // the following variables hold values computed by the last inlining decision
  // they are used for performance debugging only (print better messages)
  static const char* msg;            // reason for not inlining
  static int   cost;                 // estimated inlining cost (method size)

  // keep track of cumulative cost (top method and all inlined ones)
 private:
  static int _cumulCost;             // cumulative cost
 public:
  static void init_cost(int _cost)   { _cumulCost = _cost; }
  static int  cumulCost()            { return _cumulCost; }
  static void inc_inline_cost()      { _cumulCost += cost; }
};


// Actual inliner -- builds scope tree at the start of compilation.

class InliningClosure : public ScopeClosure {
 public:
   InliningClosure(Scope* subscope) : ScopeClosure(subscope) {}
 protected:
  void pre_invoke_interface_by_index     (int intfc_index, symbolHandle name, symbolHandle signature, int nofArgs);
  void pre_invoke_nonvirtual_by_index    (int klass_index, symbolHandle name, symbolHandle signature);
  void pre_invoke_static_by_index        (int klass_index, symbolHandle name, symbolHandle signature);
  void pre_invoke_virtual_klass_by_index (int klass_index, symbolHandle name, symbolHandle signature);

  bool should_enter_scope(Scope* s, int bci);
  ScopeClosure* clone_for_subiteration(Scope* subscope);
 private:
  void resolve_call(int idx, symbolHandle name, symbolHandle signature);
  void resolve_direct_call(int idx, symbolHandle name, symbolHandle signature, bool is_special);
  KlassHandle get_klass_or_NULL(int klass_index) const;
  const char* try_to_inline( methodHandle m );
  void printInline(methodHandle method, const char* msg) PRODUCT_RETURN;
};

