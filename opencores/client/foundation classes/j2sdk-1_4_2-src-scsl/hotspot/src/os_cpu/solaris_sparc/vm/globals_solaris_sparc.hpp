#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_solaris_sparc.hpp	1.11 03/03/19 09:28:37 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//

// Allow extra space in DEBUG builds for asserts.
define_pd_global(uintx, StackReguardSlack,       512 debug_only(* 4));
define_pd_global(uintx, JVMInvokeMethodSlack,    12288);
define_pd_global(intx, CompilerThreadStackSize,  0);
define_pd_global(bool, UseDefaultStackSize, false);
