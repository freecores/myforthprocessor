#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)Dispatcher.hpp	1.4 03/01/23 11:06:18 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _DISPATCHER_
#define _DISPATCHER_

#include "Handler.hpp"

/** This class understands the commands supported by the system and
    calls the appropriate handler routines. */

class Dispatcher {
public:
  static void dispatch(char* cmd, Handler* handler);
};

#endif  // #defined _DISPATCHER_
