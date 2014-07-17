#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)markOop.cpp	1.21 03/01/23 12:13:15 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_markOop.cpp.incl"


void markOopDesc::print_on(outputStream* st) const {
  if (is_locked()) {
    st->print("locked(0x%lx)->", value());
    markOop(*(markOop*)value())->print_on(st);
  } else {
    assert(is_unlocked(), "just checking");
    st->print("mark(");
    st->print("hash %#lx,", hash());
    st->print("age %d)", age());
  }
}
