#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciScope.hpp	1.6 03/01/23 11:58:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class CompilationScope;

// ciScope
//
// This class represents a tree of potentially inlineable
// methods.
class ciScope : public ResourceObj {
private:
  friend class CompileBroker;
  
  jobject                    _method_handle;
  ciMethod*                  _method;
  GrowableArray<ciScope*>*   _subscopes;
  ciScope*                   _caller;
  int                        _caller_bci;

  // Helper method for subscope_at_bci
  int index_of_subscope_at_bci(int bci) const;

  ciScope(CompilationScope* scope);
  ciScope(ciScope* caller, CompilationScope* scope);

  jobject   method_handle() { return _method_handle; }

public:

  ciMethod* method();
  ciScope*  subscope_at_bci(int bci) const;
  ciScope*  caller()               { return _caller; }
  int       caller_bci()           { return _caller_bci; }

  int       number_of_subscopes()  { return _subscopes->length(); }
  ciScope*  subscope_at(int index) { return _subscopes->at(index); }

  void      print_codes( int indent );
  void      print();
};

