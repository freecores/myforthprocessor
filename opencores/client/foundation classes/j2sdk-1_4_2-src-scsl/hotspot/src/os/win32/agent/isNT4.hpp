#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)isNT4.hpp	1.4 03/01/23 11:07:07 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _ISNT4_H_
#define _ISNT4_H_

// We need to special-case the Windows NT 4.0 implementations of some
// of the debugging routines because the Tool Help API is not
// available on this platform.

bool isNT4();

#endif  // #defined _ISNT4_H_
