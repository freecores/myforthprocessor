#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)resourceArea.cpp	1.46 03/01/23 12:09:53 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_resourceArea.cpp.incl"

//------------------------------ResourceMark-----------------------------------
debug_only(int ResourceArea::_warned;)      // to suppress multiple warnings

// The following routines are declared in allocation.hpp and used everywhere:

// Allocation in thread-local resource area 
extern char* resource_allocate_bytes(size_t size) {
  return Thread::current()->resource_area()->allocate_bytes(size);
}

extern char* resource_reallocate_bytes( char *old, size_t old_size, size_t new_size){
  return (char*)Thread::current()->resource_area()->Arealloc(old, old_size, new_size);
}

extern void resource_free_bytes( char *old, size_t size ) {
  Thread::current()->resource_area()->Afree(old, size);
}


//-------------------------------------------------------------------------------
// Non-product code
#ifndef PRODUCT

void ResourceMark::free_malloced_objects() {
  Arena::free_malloced_objects(_chunk, _hwm, _max, _area->_hwm);
}

#endif
