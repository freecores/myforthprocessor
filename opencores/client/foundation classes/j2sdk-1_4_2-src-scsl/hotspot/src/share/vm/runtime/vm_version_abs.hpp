#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)vm_version_abs.hpp	1.14 03/01/23 12:27:20 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// VM_Version provides information about the VM.

class Abstract_VM_Version: AllStatic {
 protected:
  friend class VMStructs;
  static bool _supports_cx8;  // set by machine-dependent initialization
  static const char* _s_vm_release;
  static const char* _s_internal_vm_info_string;
 public:
  // Name
  static const char* vm_name();
  // Vendor
  static const char* vm_vendor();
  // VM version information string printed by launcher (java -version)
  static const char* vm_info_string();
  static const char* vm_release();
  // Internal version providing additional build information
  static const char* internal_vm_info_string();

  // does HW support an 8-byte compare-exchange operation?
  static bool supports_cx8()  {return _supports_cx8;}
};

