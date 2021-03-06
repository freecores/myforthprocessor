#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stackValue.cpp	1.19 03/01/23 12:25:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_stackValue.cpp.incl"

#ifndef PRODUCT

void StackValue::print_on(outputStream* st) const {
  switch(_type) {
    case T_INT:
      st->print("%d (int) %f (float) %x (hex)",  *(int *)&_i, *(float *)&_i,  *(int *)&_i);
      break;    

    case T_OBJECT:
     _o()->print_value_on(st);
      st->print(" <" INTPTR_FORMAT ">", _o());
     break;

    case T_CONFLICT:
     st->print("conflict"); 
     break;

    default:
     ShouldNotReachHere();
  }
}

#endif

