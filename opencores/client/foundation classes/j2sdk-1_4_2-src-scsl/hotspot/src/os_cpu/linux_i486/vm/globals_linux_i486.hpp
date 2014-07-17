#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_linux_i486.hpp	1.16 03/04/23 14:36:04 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
//
define_pd_global(bool, DontYieldALot,            false);
define_pd_global(intx, ThreadStackSize,		 0); // 0 => use system default
define_pd_global(intx, VMThreadStackSize,	 512);
define_pd_global(intx, CompilerThreadStackSize,  0);
define_pd_global(intx, SurvivorRatio,            8);

// Allow extra space in DEBUG builds for asserts.
define_pd_global(uintx, StackReguardSlack,       512 DEBUG_ONLY(* 4));
define_pd_global(uintx, JVMInvokeMethodSlack,    8192);
define_pd_global(bool, UseDefaultStackSize,      false);
