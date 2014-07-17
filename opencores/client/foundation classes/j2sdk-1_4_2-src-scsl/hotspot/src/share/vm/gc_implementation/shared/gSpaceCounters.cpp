#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gSpaceCounters.cpp	1.3 03/01/23 12:03:30 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gSpaceCounters.cpp.incl"

GSpaceCounters::GSpaceCounters(const char* name_space, const char* name,
                               int ordinal, size_t max_size,
                               Generation* g, bool sampled) :
   _gen(g) {
   
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
                                   _gen->capacity(), CHECK);

    cname = PerfDataManager::counter_name(_name_space, "used");
    if (sampled) {
      _used = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                               new GenerationUsedHelper(_gen),
                                               CHECK);
    }
    else {
      _used = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                               (jlong)0, CHECK);
    }
  }
}
