#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)generationCounters.cpp	1.2 03/01/23 12:03:45 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_generationCounters.cpp.incl"


GenerationCounters::GenerationCounters(const char* name_space,
                                       const char* name,
                                       int ordinal, int spaces,
                                       VirtualSpace* v):
                    _virtual_space(v) {

  if (UsePerfData) {

    EXCEPTION_MARK;
    ResourceMark rm;

    const char* cns = PerfDataManager::counter_name(name_space, "generation");
    _name_space = PerfDataManager::name_space(cns, ordinal);

    const char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "spaces");
    PerfDataManager::create_constant(cname, PerfData::U_None, spaces, CHECK);

    char* cap_ns = PerfDataManager::counter_name(_name_space, "capacity");
    cname = PerfDataManager::counter_name(cap_ns, "min");
    PerfDataManager::create_constant(cname, PerfData::U_Bytes,
                                     _virtual_space->committed_size(), CHECK);

    cname = PerfDataManager::counter_name(cap_ns, "max");
    PerfDataManager::create_constant(cname, PerfData::U_Bytes,
                                     _virtual_space->reserved_size(), CHECK);

    cname = PerfDataManager::counter_name(cap_ns, "current");
    _current_size = PerfDataManager::create_variable(cname, PerfData::U_Bytes,
                                     _virtual_space->committed_size(), CHECK);
  }
}
