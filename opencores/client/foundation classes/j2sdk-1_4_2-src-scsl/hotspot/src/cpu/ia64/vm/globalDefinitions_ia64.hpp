#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_ia64.hpp	1.5 03/02/11 17:26:09 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// On ia64 we align all objects on double word boundaries.
// All double/long fields have to be aligned.
// Elements in double/long arrays must be aligned as well.
#define ALIGN_DOUBLE_FIELDS   ALIGN_ALL_OBJECTS
#define ALIGN_DOUBLE_ELEMENTS ALIGN_ALL_OBJECTS
#define ALIGN_LONG_FIELDS     ALIGN_ALL_OBJECTS
#define ALIGN_LONG_ELEMENTS   ALIGN_ALL_OBJECTS

// From CVM
#define VM_ENDIANNESS VM_LITTLE_ENDIAN
#define VM_DOUBLE_ENDIANNESS VM_LITTLE_ENDIAN

#define BytesPerInstWord 16 

// Kludge to fix bug in gcc for ia64 in converting float to double for denorms
extern double ia64_double_zero;
