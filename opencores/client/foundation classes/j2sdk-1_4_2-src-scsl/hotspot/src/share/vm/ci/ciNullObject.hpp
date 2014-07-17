#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciNullObject.hpp	1.5 03/01/23 11:57:41 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciNullObject
//
// This class represents a null reference in the VM.
class ciNullObject : public ciObject {
  CI_PACKAGE_ACCESS

private:
  ciNullObject() : ciObject() {}

  const char* type_string() { return "ciNullObject"; }

  void print_impl();

public:
  // Is this ciObject a Java Language Object?  That is,
  // is the ciObject an instance or an array
  bool is_java_object() { return true; }

  // What kind of ciObject is this?
  bool is_null_object() const { return true; }
  bool is_classless() const   { return true; }

  // Get the distinguished instance of this klass.
  static ciNullObject* make();
};


