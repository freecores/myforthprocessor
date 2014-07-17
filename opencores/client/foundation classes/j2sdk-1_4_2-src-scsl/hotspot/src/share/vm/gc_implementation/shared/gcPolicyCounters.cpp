#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcPolicyCounters.cpp	1.2 03/01/23 12:03:35 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcPolicyCounters.cpp.incl"

GCPolicyCounters::GCPolicyCounters(const char* name_space, const char* name,
                                   int collectors, int generations) {

  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    // construct the name space for this object from the given
    // containing name space and the 'policy' child name space
    //
    _name_space = PerfDataManager::name_space(name_space, "policy");

    char* cname = PerfDataManager::counter_name(_name_space, "name");
    PerfDataManager::create_string_constant(cname, name, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "collectors");
    PerfDataManager::create_constant(cname,  PerfData::U_None,
                                                   collectors, CHECK);

    cname = PerfDataManager::counter_name(_name_space, "generations");
    PerfDataManager::create_constant(cname,  PerfData::U_None,
                                                   generations, CHECK);
  }
}
