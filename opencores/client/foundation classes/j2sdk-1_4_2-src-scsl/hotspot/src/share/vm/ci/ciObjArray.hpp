#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciObjArray.hpp	1.5 03/01/23 11:57:43 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciObjArray
//
// This class represents a ObjArrayOop in the HotSpot virtual
// machine.
class ciObjArray : public ciArray {
  CI_PACKAGE_ACCESS

protected:
  ciObjArray(objArrayHandle h_o) : ciArray(h_o) {}

  ciObjArray(ciKlass* klass, int len) : ciArray(klass, len) {}

  objArrayOop get_objArrayOop() {
    return (objArrayOop)get_oop();
  }

  const char* type_string() { return "ciObjArray"; }

public:
  // What kind of ciObject is this?
  bool is_obj_array() { return true; }
};
