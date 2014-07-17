#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ageTable.cpp	1.24 03/01/23 12:03:16 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/* Copyright 1992 Sun Microsystems, Inc. and Stanford University.
   See the LICENSE file for license information. */

# include "incls/_precompiled.incl"
# include "incls/_ageTable.cpp.incl"

#define PERF_AT   PERF_GC ".agetable"

ageTable::ageTable(bool global) {

  clear();

  if (UsePerfData && global) {

    const char* sym_name = PERF_AT ".bytes.";
    int sym_length = (int)strlen(sym_name) + 2 + 1; // add space for age characters
                                             // and null terminator

    ResourceMark rm;
    EXCEPTION_MARK;

    char *sym = NEW_RESOURCE_ARRAY(char, sym_length);

    for(int age = 0; age < table_size; age ++) {
      jio_snprintf(sym, sym_length, "%s%2.2d", sym_name, age);
      _perf_sizes[age] =
            PerfDataManager::create_variable(sym, PerfData::U_Bytes, CHECK);
    }

    PerfDataManager::create_constant(PERF_AT ".size",
                                     PerfData::U_None, table_size, CHECK);

    PerfDataManager::create_constant(PERF_AT ".mtt", PerfData::U_None,
                                     MaxTenuringThreshold, CHECK);
    _perf_tenuring_threshold =
        PerfDataManager::create_variable(PERF_AT ".tt", PerfData::U_None,
                                     MaxTenuringThreshold, CHECK);
    _perf_desired_survivor_size =
        PerfDataManager::create_variable(PERF_AT ".dss", PerfData::U_Bytes,
                                     CHECK);
  }
}

void ageTable::clear() {
  Memory::set_words((HeapWord*)&sizes[0], table_size, 0);
}

void ageTable::merge(ageTable* subTable) {
  for (int i = 0; i < table_size; i++) {
    sizes[i]+= subTable->sizes[i];
  }
}

int ageTable::compute_tenuring_threshold(size_t survivor_capacity) {
  size_t desired_survivor_size = (((double) survivor_capacity)*TargetSurvivorRatio)/100;
  size_t total = 0;
  int age = 1;
  assert(sizes[0] == 0, "no objects with age zero should be recorded");
  while (age < table_size) {
    total += sizes[age];
    // check if including objects of age 'age' made us pass the desired
    // size, if so 'age' is the new threshold
    if (total > desired_survivor_size) break;
    age++;
  }
  int result = age < MaxTenuringThreshold ? age : MaxTenuringThreshold;

  if (PrintTenuringDistribution || UsePerfData) {

    if (PrintTenuringDistribution) {
      tty->cr();
      tty->print_cr("Desired survivor size %ld bytes, new threshold %d (max %d)",
        desired_survivor_size*oopSize, result, MaxTenuringThreshold);
    }

    total = 0;
    age = 1;
    while (age < table_size) {
      total += sizes[age];
      if (sizes[age] > 0) {
        if (PrintTenuringDistribution) {
          tty->print_cr("- age %3d: %10ld bytes, %10ld total", age, sizes[age]*oopSize, total*oopSize);
        }
      }
      if (UsePerfData) {
        _perf_sizes[age]->set_value(sizes[age]*oopSize);
      }
      age++;
    }
    if (UsePerfData) {
      _perf_tenuring_threshold->set_value(result);
      _perf_desired_survivor_size->set_value(desired_survivor_size*oopSize);
    }
  }

  return result;
}
