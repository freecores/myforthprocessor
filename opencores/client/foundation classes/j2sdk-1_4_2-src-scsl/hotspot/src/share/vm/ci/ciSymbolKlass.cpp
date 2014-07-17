#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciSymbolKlass.cpp	1.3 03/01/23 11:58:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciSymbolKlass.cpp.incl"

// ciSymbolKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is a symbolKlass.

// ------------------------------------------------------------------
// ciSymbolKlass::instance
//
// Return the distinguished instance of this class
ciSymbolKlass* ciSymbolKlass::make() {
  return CURRENT_ENV->_symbol_klass_instance;
}
