#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)perfData.cpp	1.7 03/01/23 12:24:13 JVM"
#endif
/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_perfData.cpp.incl"

PerfDataList*	PerfDataManager::_all = NULL;
PerfDataList*	PerfDataManager::_sampled = NULL;
PerfDataList*	PerfDataManager::_constants = NULL;

PerfData::PerfData(const char* name, Units u, Variability v) :
          _name(NULL), _u(u), _v(v), _valuep(NULL), _on_c_heap(false) {

  _name = NEW_C_HEAP_ARRAY(char, strlen(name) + 1);
  assert(_name != NULL && strlen(name) != 0, "invalid name");
  strcpy(_name, name);
}

PerfData::~PerfData() {
  if (_name != NULL) {
    FREE_C_HEAP_ARRAY(char, _name);
  }
  if (is_on_c_heap()) {
    FREE_C_HEAP_ARRAY(PerfDataEntry, _pdep);
  }
}

void PerfData::create_entry(BasicType dtype, size_t dsize, size_t vlen) {

  size_t dlen = vlen==0 ? 1 : vlen;

  size_t namelen = strlen(name()) + 1;  // include null terminator
  size_t size = sizeof(PerfDataEntry) + namelen;
  size_t pad_length = ((size % dsize) == 0) ? 0 : dsize - (size % dsize);
  size += pad_length;
  size_t data_start = size;
  size += (dsize * dlen);

  // align size to assure allocation in units of 8 bytes
  int align = sizeof(jlong) - 1;
  size = ((size + align) & ~align);
  char* psmp = PerfMemory::alloc(size);

  if (psmp == NULL) {
    // out of PerfMemory memory resources. allocate on the C heap
    // to avoid vm termination.
    psmp = NEW_C_HEAP_ARRAY(char, size);
    _on_c_heap = true;
  }

  // compute the addresses for the name and data
  char* cname = psmp + sizeof(PerfDataEntry);

  // data is in the last dsize*dlen bytes of the entry
  void* valuep = (void*) (psmp + data_start);

  assert(is_on_c_heap() || PerfMemory::contains(cname), "just checking");
  assert(is_on_c_heap() || PerfMemory::contains((char*)valuep), "just checking");

  // copy the name, including null terminator, into PerfData memory
  strcpy(cname, name());

  // set the header values in PerfData memory
  PerfDataEntry* pdep = (PerfDataEntry*)psmp;
  pdep->entry_length = (jint)size;
  pdep->name_length = (jint)namelen;
  pdep->vector_length = (jint)vlen;
  pdep->data_type = dtype;

  pdep->data_units = units();
  pdep->data_variability = variability();

  if (PerfTraceDataCreation) {
    jio_fprintf(stderr, "name = %s, dtype = %d, variability = %d,"
                        " units = %d, dsize = %d, vlen = %d,"
                        " pad_length = %d, size = %d, on_c_heap = %s,"
                        " address = " INTPTR_FORMAT ","
                        " data address = " INTPTR_FORMAT "\n",
                        cname, dtype, variability(),
                        units(), dsize, vlen,
                        pad_length, size, is_on_c_heap() ? "TRUE":"FALSE",
                        psmp, valuep);
  }

  // record the start of the entry and the location of the data field.
  _pdep = pdep;
  _valuep = valuep;
  
  // mark the PerfData memory region as having been updated.
  PerfMemory::mark_updated();
}

int PerfData::compare_by_name(PerfData* p) {

  if (p == NULL)
    return 1;

  return strcmp(name(), p->name());
}

PerfLong::PerfLong(const char* namep, Units u, Variability v) : PerfData(namep, u, v) {

  create_entry(T_LONG, sizeof(jlong));
}

int PerfLong::format(char* buffer, int length) {
  return jio_snprintf(buffer, length,"%lld", *(jlong*)_valuep);
}

PerfLongVariant::PerfLongVariant(const char* namep, Units u, Variability v, jlong* sampled) : PerfLong(namep, u, v), _sampled(sampled), _sample_helper(NULL) {

  sample();
}

PerfLongVariant::PerfLongVariant(const char* namep, Units u, Variability v, PerfLongSampleHelper* sample_helper) : PerfLong(namep, u, v), _sampled(NULL), _sample_helper(sample_helper) {

  sample();
}

void PerfLongVariant::sample() {

  assert(_sample_helper != NULL || _sampled != NULL, "unexpected state");

  if (_sample_helper != NULL) {
    *(jlong*)_valuep = _sample_helper->take_sample();
  }
  else if (_sampled != NULL) {
    *(jlong*)_valuep = *_sampled;
  }
}

PerfByteArray::PerfByteArray(const char* namep, jint length, Units u, Variability v) : PerfData(namep, u, v), _length(length) {

  create_entry(T_BYTE, sizeof(jbyte), (size_t)_length);
}

void PerfString::set_string(const char* s2) {

  // copy n bytes of the string, assuring the null string is
  // copied if s2 == NULL.
  strncpy((char *)_valuep, s2 == NULL ? "" : s2, _length);

  // assure the string is null terminated when strlen(s2) >= _length
  ((char*)_valuep)[_length-1] = '\0';
}

int PerfString::format(char* buffer, int length) {
  return jio_snprintf(buffer, length, "%s", (char*)_valuep);
}

PerfStringConstant::PerfStringConstant(const char* namep, const char* initial_value) : PerfString(namep, initial_value == NULL ? 1 : MIN2(
                          (jint)(strlen((char*)initial_value)+1),
                          (jint)(PerfMaxStringConstLength+1)),
                 V_Constant, initial_value) {

  if (PrintMiscellaneous && Verbose) {
    if (is_valid() && initial_value != NULL &&
        ((jint)strlen(initial_value) > (jint)PerfMaxStringConstLength)) {

      warning("Truncating PerfStringConstant: name = %s,"
              " length = " INT32_FORMAT ","
              " PerfMaxStringConstLength = " INT32_FORMAT "\n",
              namep,
              (jint)strlen(initial_value),
              (jint)PerfMaxStringConstLength);
    }
  }

}






void PerfDataManager::destroy() {

  if (_all == NULL)
    // destroy already called, or initialization never happened
    return;

  for (int index = 0; index < _all->length(); index++) {
    PerfData* p = _all->at(index);
    delete p;
  }

  delete(_all);
  delete(_sampled);
  delete(_constants);

  _all = NULL;
  _sampled = NULL;
  _constants = NULL;
}

void PerfDataManager::add_item(PerfData* p, bool sampled) {

  MutexLocker ml(PerfDataManager_lock);

  if (_all == NULL) {
    _all = new PerfDataList(100);
  }

  assert(!_all->contains(p->name()), "duplicate name added");

  // add to the list of all perf data items
  _all->append(p);

  if (p->variability() == PerfData::V_Constant) {
    if (_constants == NULL) {
      _constants = new PerfDataList(25);
    }
    _constants->append(p);
    return;
  }
    
  if (sampled) {
    if (_sampled == NULL) {
      _sampled = new PerfDataList(25);
    }
    _sampled->append(p);
  }
}

PerfDataList* PerfDataManager::all() {

  MutexLocker ml(PerfDataManager_lock);

  if (_all == NULL)
    return NULL;

  PerfDataList* clone = _all->clone();
  return clone;
}

PerfDataList* PerfDataManager::sampled() {

  MutexLocker ml(PerfDataManager_lock);

  if (_sampled == NULL)
    return NULL;

  PerfDataList* clone = _sampled->clone();
  return clone;
}

PerfDataList* PerfDataManager::constants() {

  MutexLocker ml(PerfDataManager_lock);

  if (_constants == NULL)
    return NULL;

  PerfDataList* clone = _constants->clone();
  return clone;
}

char* PerfDataManager::counter_name(const char* parent, const char* name) {

   size_t len = strlen(parent) + strlen(name) + 2;
   char* result = NEW_RESOURCE_ARRAY(char, len);
   sprintf(result, "%s.%s", parent, name);
   return result;
}

char* PerfDataManager::counter_name(const char* parent, unsigned int name) {
   char intbuf[40];
   jio_snprintf(intbuf, 40, UINT32_FORMAT, name);
   return counter_name(parent, intbuf);
}

char* PerfDataManager::name_space(const char* parent, const char* child) {
   size_t len = strlen(parent) + strlen(child) + 2;
   char* result = NEW_C_HEAP_ARRAY(char, len);
   sprintf(result, "%s.%s", parent, child);
   return result;
}

char* PerfDataManager::name_space(const char* parent, unsigned int child) {
   char intbuf[40];
   jio_snprintf(intbuf, 40, UINT32_FORMAT, child);
   return name_space(parent, intbuf);
}

PerfStringConstant* PerfDataManager::create_string_constant(const char* name, const char* s, TRAPS) {

  PerfStringConstant* p = new PerfStringConstant(name, s);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, false);

  return p;
}

PerfLongConstant* PerfDataManager::create_long_constant(const char* name, PerfData::Units u, jlong val, TRAPS) {

  PerfLongConstant* p = new PerfLongConstant(name, u, val);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, false);

  return p;
}

PerfStringVariable* PerfDataManager::create_string_variable(const char* name, const char* s, jint max_length, TRAPS) {

  if (max_length == 0 && s != NULL) max_length = (jint)strlen(s);

  assert(max_length != 0, "PerfStringVariable with length 0");

  PerfStringVariable* p = new PerfStringVariable(name, s, max_length);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, false);

  return p;
}

PerfLongVariable* PerfDataManager::create_long_variable(const char* name, PerfData::Units u, jlong ival, TRAPS) {

  PerfLongVariable* p = new PerfLongVariable(name, u, ival);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, false);

  return p;
}

PerfLongVariable* PerfDataManager::create_long_variable(const char* name, PerfData::Units u, jlong* sp, TRAPS) {

  PerfLongVariable* p = new PerfLongVariable(name, u, sp);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, true);

  return p;
}

PerfLongVariable* PerfDataManager::create_long_variable(const char* name, PerfData::Units u, PerfSampleHelper* sh, TRAPS) {

  PerfLongVariable* p = new PerfLongVariable(name, u, sh);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, true);

  return p;
}

PerfLongCounter* PerfDataManager::create_long_counter(const char* name, PerfData::Units u, jlong ival, TRAPS) {

  PerfLongCounter* p = new PerfLongCounter(name, u, ival);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, false);

  return p;
}

PerfLongCounter* PerfDataManager::create_long_counter(const char* name, PerfData::Units u, jlong* sp, TRAPS) {

  PerfLongCounter* p = new PerfLongCounter(name, u, sp);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, true);

  return p;
}

PerfLongCounter* PerfDataManager::create_long_counter(const char* name, PerfData::Units u, PerfSampleHelper* sh, TRAPS) {

  PerfLongCounter* p = new PerfLongCounter(name, u, sh);

  if (!p->is_valid()) {
    // allocation of native resources failed. 
    delete p;
    THROW_0(vmSymbols::java_lang_OutOfMemoryError());
  }

  add_item(p, true);

  return p;
}

PerfDataList::PerfDataList(int length) {

  _set = new(ResourceObj::C_HEAP) PerfDataArray(length, true);
}

PerfDataList::PerfDataList(PerfDataList* p) {

  _set = new(ResourceObj::C_HEAP) PerfDataArray(p->length(), true);

  _set->appendAll(p->get_impl());
}

PerfDataList::~PerfDataList() {

  FreeHeap(_set);

}

bool PerfDataList::by_name(void* name, PerfData* pd) {

  if (pd == NULL) return -1;

  return strcmp((const char*)name, pd->name()) == 0;
}

PerfData* PerfDataList::find_by_name(const char* name) {

  int i = _set->find((void*)name, PerfDataList::by_name);

  if (i >= 0 && i <= _set->length())
    return _set->at(i);
  else
    return NULL;
}

PerfDataList* PerfDataList::find_by_regex(const char* regex) {
#if 0

  PerfDataArray* matches = new PerfDataList(100);

  for (int i = 0; i < set->length(); i++) {
    PerfData* p = _set->at(i);
    char* name = p->name();
    if (os::match(regex, name))
      matches->add(p);
  }
  return matches;
#endif
  return NULL;
}

int PerfDataList::compare_name_ascending(PerfData** p1, PerfData** p2) {
  assert(p1 != NULL && *p1 != NULL, "compare function passed null pointer");
  assert(p2 != NULL && *p2 != NULL, "compare function passed null pointer");
  return (*p1)->compare_by_name(*p2);
}

int PerfDataList::compare_name_descending(PerfData** p1, PerfData** p2) {
  assert(p1 != NULL && *p1 != NULL, "compare function passed null pointer");
  assert(p2 != NULL && *p2 != NULL, "compare function passed null pointer");
  return (*p2)->compare_by_name(*p1);
}

void PerfDataList::sort_by_name(SortOrder s) {

  switch(s) {
  case ASCENDING:
    _set->sort(PerfDataList::compare_name_ascending);
    break;
  case DESCENDING:
    _set->sort(PerfDataList::compare_name_descending);
    break;
  }
}

PerfDataList* PerfDataList::clone() {

  PerfDataList* copy = new PerfDataList(this);

  assert(copy != NULL, "just checking");

  return copy;
}
