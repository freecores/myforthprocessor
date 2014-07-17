#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CacheLocals_i486.cpp	1.42 03/01/23 10:52:30 JVM"
#endif
// 
// Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_CacheLocals_i486.cpp.incl"


//------------------caching locals---------------------

LocalMapping* LIR_LocalCaching::preferred_locals(const ciMethod* method) {
  return new LocalMapping(ir()->local_name_to_offset_map(), new RInfoCollection());
}
