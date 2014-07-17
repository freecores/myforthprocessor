#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)memprofiler.hpp	1.10 03/01/23 12:23:41 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Prints periodic memory usage trace of HotSpot VM

class MemProfilerTask;

class MemProfiler : AllStatic {
 friend class MemProfilerTask;
 private:
  static MemProfilerTask* _task;
  static FILE* _log_fp;
  // Do trace (callback from MemProfilerTask and from disengage)
  static void do_trace()      PRODUCT_RETURN;
 public:
  // Start/stop the profiler
  static void engage()        PRODUCT_RETURN;
  static void disengage()     PRODUCT_RETURN;
  // Tester
  static bool is_active()     PRODUCT_RETURN0;
};
