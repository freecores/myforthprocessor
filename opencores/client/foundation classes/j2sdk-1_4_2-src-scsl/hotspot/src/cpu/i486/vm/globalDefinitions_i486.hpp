#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)globalDefinitions_i486.hpp	1.5 03/01/23 10:54:31 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// On i486 we may align all objects on double word boundaries, in which case the base
// for double/long arrays are allocated aligned as well.
// We could align double/long fields, currently not done.
#define ALIGN_DOUBLE_FIELDS   0
#define ALIGN_DOUBLE_ELEMENTS ALIGN_ALL_OBJECTS
#define ALIGN_LONG_FIELDS     0
#define ALIGN_LONG_ELEMENTS   ALIGN_ALL_OBJECTS


