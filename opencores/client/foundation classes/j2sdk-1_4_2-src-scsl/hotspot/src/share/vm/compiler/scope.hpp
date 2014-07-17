#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)scope.hpp	1.27 03/01/23 12:01:42 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The class Scope is the abstract class for compilation scopes.
// It contains an interface that  is independent of the compiler 
// implementation

class ScopeClosure;

enum { IllegalBCI = -1 };

class Scope: public ResourceObj {
 protected:
  methodHandle            _method;	    // the method of the scope
  GrowableArray<Scope*>*  _subscopes;       // list of inlined subscopes (sorted by bci)
  Scope*const             _caller;          // calling scope (NULL if not inlined)
  const int               _callerBCI;       // bci (in caller) of call to this scope 
  int                     _num_bytecodes;   // total bytecodes (including subscopes)
  float                   _frequency;       // estimated execution probability
 public:
  Scope(methodHandle method, Scope* caller = NULL, int callerBCI = IllegalBCI);

  // type testers
  virtual bool is_compilation_scope() const { return false; }

  // Public accessors
  methodHandle method() const               { return _method; }
  void      set_method( methodHandle m )    { _method = m; }
  Scope*    caller() const                  { return _caller; }
  int       callerBCI() const               { return _callerBCI; }
  bool      is_inlined() const              { return _caller != NULL; }
  int       size() const                    { return _num_bytecodes; }
  void      set_frequency( float f )        { _frequency = f; }
  float     frequency() const               { return _frequency; }
  const Scope*    root() const;                   // root of caller, or self if not inlined
  int       nof_subscopes() const;          // number of inlined subscopes
  Scope*    subscope_at(int i) const;       // returns ith subscope (must exist)
  Scope*    subscope_at_bci(int bci) const; // returns subscope for call at bci (NULL if none, error if >1 at this bci)
  int       nof_subscopes_at(int bci) const;// number of inlined subscopes at bci
  Scope*    subscope_at_bci(int bci, int which) const;
  void      add_subscope(Scope* callee);    // add new subscope 

  // Returns all scopes in the inlining tree
  GrowableArray<Scope*>* all_scopes(GrowableArray<Scope*>* result = NULL);

  int       inlining_depth();

  // iteration (will recursively iterate over subscopes)
  void      iterate(ScopeClosure* c);       // iterate in bytecode order

  void         print()                 NOT_PRODUCT({ print(0); }) PRODUCT_RETURN;
  virtual void print(int indent) = 0;                         // print this scope (indented n levels)
  virtual void print_tree()            PRODUCT_RETURN;        // print this scope and all subscopes
  void print_codes( int indent ) const; // Print bytecodes, with inlining

 protected:
  int index_of_subscope_at_bci(int bci) const;
}; 



