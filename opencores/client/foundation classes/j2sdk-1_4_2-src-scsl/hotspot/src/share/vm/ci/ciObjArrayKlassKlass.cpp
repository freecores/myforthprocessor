#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciObjArrayKlassKlass.cpp	1.3 03/01/23 11:57:50 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciObjArrayKlassKlass.cpp.incl"

// ciObjArrayKlassKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an arrayKlassKlass.

// ------------------------------------------------------------------
// ciObjArrayKlassKlass::instance
//
// Return the distinguished instance of this class
ciObjArrayKlassKlass* ciObjArrayKlassKlass::make() {
  return CURRENT_ENV->_obj_array_klass_klass_instance;
}
