#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)icache.cpp	1.11 03/01/23 12:22:56 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_icache.cpp.incl"

//%note icache_1

void AbstractICache::invalidate_word(u_char* addr) { 
  ICache::invalidate_range(addr, BytesPerWord);
}

void AbstractICache::invalidate_range(u_char* addr, int nBytes) { 
  for (u_char* p = addr;  p  <  addr + nBytes;  p += BytesPerWord)
    ICache::invalidate_word(p);
}

void icache_init() {
  ICache::initialize();
}
