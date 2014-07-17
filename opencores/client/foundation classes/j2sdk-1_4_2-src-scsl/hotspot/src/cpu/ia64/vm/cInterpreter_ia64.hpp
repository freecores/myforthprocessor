#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cInterpreter_ia64.hpp	1.6 03/01/23 10:56:48 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Platform specific for C++ based Interpreter
#define LOTS_OF_REGS    /* Lets interpreter use plenty of registers */

private:
    // These are values used to find interpreter frames when it calls out to the
    // vm or blocks itself for a safepoint. These identify the frame that the
    // frame manager has setup NOT the frame for InterpretMethod. As far as
    // the vm/gc world is concerend InterpretMethod is invisible. (No live
    // oops are ever present in InterpretMethod locals when we block).
    // 
    address    _last_Java_pc;             // return address to frame manager
    intptr_t*  _last_Java_fp;             // link to frame managers bsp

    // save the bottom of the stack after frame manager setup. For ease of restoration after return 
    // from recursive interpreter call
    intptr_t*        _frame_bottom;       // saved bottom of frame manager frame
    interpreterState _self_link;          // points to self
    intptr_t         _native_lresult;     // save result of native calls that might return handle/longs
    double           _native_fresult[3];  // save result of native calls that might return floats.  needs
                                          // 16 bytes at 16-byte alignment, so pad out in case only 8-byte aligned.

public:
    address last_Java_pc(void)            { return _last_Java_pc; }
    intptr_t* last_Java_fp(void)          { return _last_Java_fp; }

    static void pd_layout_interpreterState(interpreterState istate, address last_Java_pc, intptr_t* last_Java_fp);

#define SET_LAST_JAVA_FRAME()                                                        \
	THREAD->frame_anchor()->set_last_Java_fp((intptr_t*) istate->_last_Java_fp); \
	THREAD->frame_anchor()->set_last_Java_pc(istate->_last_Java_pc);             \
	THREAD->frame_anchor()->set_flushed_windows(false);                          \
	THREAD->frame_anchor()->set_last_Java_sp((intptr_t*)istate->_stack);

#define RESET_LAST_JAVA_FRAME() THREAD->frame_anchor()->clear();
 
    // gdb refuses to display object so we need this
    void print();
