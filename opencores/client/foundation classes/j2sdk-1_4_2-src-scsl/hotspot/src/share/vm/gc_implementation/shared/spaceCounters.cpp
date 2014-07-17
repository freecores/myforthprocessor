#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)spaceCounters.cpp	1.2 03/01/23 12:04:12 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_spaceCounters.cpp.incl"

SpaceCounters::SpaceCounters(const char* name_space, const char* name,
                             int ordinal, size_t max_size, MutableSpace* m) :
   _object_space(m) {
   
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
                                   _object_space->capacity_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "used");
    _used = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                    new MutableSpaceUsedHelper(_object_space),
                                    CHECK);
  }
}
