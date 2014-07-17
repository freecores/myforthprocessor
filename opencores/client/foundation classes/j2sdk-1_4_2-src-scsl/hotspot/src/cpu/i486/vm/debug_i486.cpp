#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)debug_i486.cpp	1.8 03/01/23 10:54:02 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_debug_i486.cpp.incl"

void pd_ps(frame f) {}

// This function is used to add platform specific info
// to the error reporting code.

void pd_obfuscate_location(char *buf,int buflen) {}
