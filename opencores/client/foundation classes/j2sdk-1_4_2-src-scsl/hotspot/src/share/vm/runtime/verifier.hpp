#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)verifier.hpp	1.10 03/01/23 12:26:29 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Interface to the external verifier

class Verifier : AllStatic {
 private:
  static bool should_verify_for(oop class_loader);

 public:
  // relax certain verifier checks to enable some broken 1.1 apps to run on 1.2.
  static bool relax_verify_for(oop class_loader);

  // Verifies the class file format
  static void verify_class_format(symbolHandle class_name, Handle class_loader, ClassFileStream* st, TRAPS);

  // Verifies the byte codes of the class
  static void verify_byte_codes(instanceKlassHandle klass, TRAPS);
};
