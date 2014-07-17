#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_sparc.hpp	1.6 03/01/23 11:01:06 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// On SPARC we align all objects on double word boundaries. 
// All double/long fields have to be aligned.
// Elements in double/long arrays must be aligned as well.
#define ALIGN_DOUBLE_FIELDS   ALIGN_ALL_OBJECTS
#define ALIGN_DOUBLE_ELEMENTS ALIGN_ALL_OBJECTS
#define ALIGN_LONG_FIELDS     ALIGN_ALL_OBJECTS
#define ALIGN_LONG_ELEMENTS   ALIGN_ALL_OBJECTS

#ifdef _GNU_SOURCE
// Need 8 byte alignment for gcc
#define ARENA_AMALLOC_ALIGNMENT 8
#endif

#define BytesPerInstWord        4       // Size of Sparc Instructions
