#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)c1_LIROptimizer_sparc.hpp	1.9 03/01/23 11:00:00 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

 private:
  bool _pending_delay;

  void pd_init() { _pending_delay = false; }

  LIR_Op* select_delay(CodeEmitInfo* info);
  void delayed_emit(LIR_Op* op);

  void emit_delay(LIR_OpDelay* op);
