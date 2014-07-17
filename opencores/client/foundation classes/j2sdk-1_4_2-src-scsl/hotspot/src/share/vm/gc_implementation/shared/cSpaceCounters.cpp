#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)cSpaceCounters.cpp	1.2 03/01/23 12:03:21 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_cSpaceCounters.cpp.incl"

CSpaceCounters::CSpaceCounters(const char* name_space, const char* name,
                               int ordinal, size_t max_size,
                               ContiguousSpace* s) :
   _space(s) {
   
  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;
 

    const char* cns = PerfDataManager::counter_name(name_space, "space");
    _name_space = PerfDataManager::name_space(cns, ordinal);

    const char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "size");
    PerfDataManager::create_constant(cname, PerfData::U_Bytes,
                                     (jlong)max_size, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "capacity");
    _capacity = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                   _space->capacity(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "used");
    _used = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                    new ContiguousSpaceUsedHelper(_space),
                                    CHECK);
  }
}
