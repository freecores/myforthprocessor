#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)collectorCounters.cpp	1.2 03/01/23 12:03:25 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_collectorCounters.cpp.incl"

CollectorCounters::CollectorCounters(const char* name_space,
                                     const char* name, int ordinal) {

  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    char* cns = PerfDataManager::counter_name(name_space, "collector");
    _name_space = PerfDataManager::name_space(cns, ordinal);

    char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "invocations");
    _invocations =
        PerfDataManager::create_counter(cname, PerfData::U_Events, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "time");
    _time = PerfDataManager::create_counter(cname, PerfData::U_Ticks, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "last_entry");
    _last_entry_time =
            PerfDataManager::create_variable(cname, PerfData::U_Ticks, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "last_exit");
    _last_exit_time =
            PerfDataManager::create_variable(cname, PerfData::U_Ticks, CHECK);
  }
}
