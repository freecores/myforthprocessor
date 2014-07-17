#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)threadLS_linux_i486.cpp	1.4 03/01/23 11:09:07 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_threadLS_linux_i486.cpp.incl"

void ThreadLocalStorage::generate_code_for_get_thread() {
    // nothing we can do here for user-level thread
}
