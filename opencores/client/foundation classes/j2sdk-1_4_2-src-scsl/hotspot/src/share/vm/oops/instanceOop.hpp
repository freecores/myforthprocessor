#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)instanceOop.hpp	1.8 03/01/23 12:12:42 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An instanceOop is an instance of a Java Class
// Evaluating "new HashTable()" will create an instanceOop.

class instanceOopDesc : public oopDesc {
 public:
  static int header_size() { return sizeof(instanceOopDesc)/HeapWordSize; }
};


