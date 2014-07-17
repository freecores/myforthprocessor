#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciNullObject.cpp	1.4 03/01/23 11:57:39 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciNullObject.cpp.incl"

// ciNullObject
//
// This class represents a null reference.  It can be used
// as a class loader or as the null constant.

// ------------------------------------------------------------------
// ciNullObject::print_impl
//
// Implementation of the print method.
void ciNullObject::print_impl() {
  ciObject::print_impl();
  tty->print(" unique");
}

// ------------------------------------------------------------------
// ciNullObject::make
//
// Get the distinguished instance of this class.
ciNullObject* ciNullObject::make() {
  return CURRENT_ENV->_null_object_instance->as_null_object();
}

