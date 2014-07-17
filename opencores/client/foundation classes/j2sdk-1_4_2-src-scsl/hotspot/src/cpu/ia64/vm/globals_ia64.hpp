#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globals_ia64.hpp	1.9 03/01/23 10:57:24 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//
// Sets the default values for platform dependent flags used by the runtime system.
// (see globals.hpp)
// These are just guesses at this point for ia64
//

define_pd_global(bool,  UseCallBackInInterpreter, false); // Not needed for c++ based interpreter
define_pd_global(bool,  ConvertSleepToYield,      true);
define_pd_global(bool,  ShareVtableStubs,         true);
define_pd_global(bool,  CountInterpCalls,         true);
define_pd_global(bool,  EnforceStrictFP,          true);

define_pd_global(bool, ImplicitNullChecks,          false);  // Generate code for implicit null checks
define_pd_global(bool, UncommonNullCast,            false);  // Uncommon-trap NULLs past to check cast

define_pd_global(intx,  CodeEntryAlignment,       32); 

define_pd_global(uintx, TLABSize,              16 * K);
define_pd_global(uintx, NewSize,               (2048 * K) + (2 * (64 * K)));
// define_pd_global(intx,  SurvivorRatio,         32);  // Design center runs on 1.3.1
// sparc didn't define these
// define_pd_global(uintx, MaxNewSize,               2560*K); 

define_pd_global(intx,  InlineFrequencyCount,     100);
define_pd_global(intx,  PreInflateSpin,		  10);
define_pd_global(intx, PrefetchCopyIntervalInBytes, -1);
define_pd_global(intx, PrefetchScanIntervalInBytes, -1);
define_pd_global(intx, PrefetchFieldsAhead,         -1);

