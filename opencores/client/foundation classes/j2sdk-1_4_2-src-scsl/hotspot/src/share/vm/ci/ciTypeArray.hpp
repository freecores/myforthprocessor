#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciTypeArray.hpp	1.5 03/01/23 11:58:40 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciTypeArray
//
// This class represents a typeArrayOop in the HotSpot virtual
// machine.
class ciTypeArray : public ciArray {
  CI_PACKAGE_ACCESS

protected:
  ciTypeArray(typeArrayHandle h_t) : ciArray(h_t) {}

  ciTypeArray(ciKlass* klass, int len) : ciArray(klass, len) {}

  typeArrayOop get_typeArrayOop() {
    return (typeArrayOop)get_oop();
  }

  const char* type_string() { return "ciTypeArray"; }

public:
  // What kind of ciObject is this?
  bool is_type_array() { return true; }
};
