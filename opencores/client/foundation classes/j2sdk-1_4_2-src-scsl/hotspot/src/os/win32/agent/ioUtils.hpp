#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ioUtils.hpp	1.5 03/01/23 11:07:02 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#ifndef _IO_UTILS_
#define _IO_UTILS_

bool scanInt(char** data, int* num);
bool scanUnsignedLong(char** data, unsigned long* num);
bool scanAddress(char** data, unsigned long* addr);

// Binary utils (for poke)
bool scanAndSkipBinEscapeChar(char** data);
bool scanBinUnsignedLong(char** data, unsigned long* num);

#endif  // #defined _IO_UTILS_
