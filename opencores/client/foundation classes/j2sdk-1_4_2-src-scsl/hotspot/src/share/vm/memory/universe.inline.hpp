#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)universe.inline.hpp	1.39 03/01/23 12:11:08 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Check whether an element of a typeArrayOop with the given type must be
// aligned 0 mod 8.  The typeArrayOop itself must be aligned at least this
// strongly.

// This code is more convoluted than it needs be because MSVC 5.0 doesn't
// inline the clean version. It's time-critical (part of e.g. array
// allocation).

#ifdef ASSERT
inline bool Universe::element_type_should_be_aligned(BasicType type) {  // clean version
  if ((ALIGN_DOUBLE_ELEMENTS && type == T_DOUBLE) ||
      (ALIGN_LONG_ELEMENTS   && type == T_LONG)) {
    assert(ALIGN_ALL_OBJECTS, "aligned element must be inside aligned array object");
    return true;
  } else {
    return false;
  }
}
#else
inline bool Universe::element_type_should_be_aligned(BasicType type) {  // clean version
  return
#if ALIGN_DOUBLE_ELEMENTS
    type == T_DOUBLE ||
#endif
#if ALIGN_LONG_ELEMENTS
    type == T_LONG
#else
    false
#endif
    ;
}
#endif


// Check whether an object field (static/non-static) of the given type must be aligned 0 mod 8.

inline bool Universe::field_type_should_be_aligned(BasicType type) {
  if ((ALIGN_DOUBLE_FIELDS && type == T_DOUBLE) ||
      (ALIGN_LONG_FIELDS   && type == T_LONG)) {
    assert(ALIGN_ALL_OBJECTS, "aligned field must be inside aligned object");
    return true;
  } else {
    return false;
  }
}



