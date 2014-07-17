#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vmNullPointerExc.hpp	1.6 03/01/23 12:26:58 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class is needed for better location of problems
// when a NULL pointer exception in the VM happens.
// (It used to report stop in COmpilerRuntime)

class VmNullPointerException: public AllStatic {
 public:
  static void stop_vm(char* message);
};

