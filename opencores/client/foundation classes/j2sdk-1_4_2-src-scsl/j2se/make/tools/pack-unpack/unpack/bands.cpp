/*
 * @(#)bands.cpp	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// -*- C++ -*-
// Small program for unpacking specially compressed Java packages.
// John R. Rose

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "defines.h"
#include "bytes.h"
#include "utils.h"
#include "coding.h"
#include "bands.h"

#include "constants.h"
#include "zip.h"

void band::init() {
  c = coding::findBySpec((int)c);
}

void band::setIndexByTag(byte tag) {
  setIndex(getCPIndex(tag));
}

void band::readFrom(byte* &rpVar) {
  rp = rpVar;
  byte*   rp0 = rp;
  NOT_PRODUCT(rpBase = rp);
  NOT_PRODUCT(coding* c0  = c);
  decideCoding();
#ifndef PRODUCT
  if (c0 != c)
    printcr(3,"band %s has irregular coding %s", name, c->string());
#endif
  rpVar = rp;
  // update rpVar with the position at the end of the band
  c->skipArrayFrom(rpVar, length);
#ifndef PRODUCT
  rpLimit = rpVar;
  printcr(3,"readFrom %s at %p [%d values, %d bytes, cp=%d]",
	  name, rp, length, rpVar-rp0, rp-rp0);
#endif
  // must have already read from previous band:
  assert(this == &package_magic ||
	 this == &fake_package_attr_count ||
	 prevBand()->rpLimit != null);
}

void band::decideCoding() {
  if (length == 0)  return;
  if (!c->isSigned)  return;
  byte* rp0 = rp;
  int cval = getValue();
  if (cval >= 0) {
    rp = rp0;  // Do not consume this value.
  } else if (cval != -1) {
    int id = -1 - cval;
    // Set a non-default coding:
    c = coding::findByIndex(id);
  }
  dbase[0] = 0;  // reset, in case getValue changed something
}

int band::getValue() {
  assert(!isDone);
  return c->readFrom(rp, dbase);
}

// @@@ get rid of?
int* band::getValueArray() {
  assert(!isDone);
  bytes buf;
  buf.malloc(length * sizeof(int));
  int* values = (int*)buf.ptr;
  c->readArrayFrom(this->rp, this->dbase, this->length, values);
  return values;
}

entry* band::getRef() {
  assert(!isDone);
  assert(ix != null);
  int n = c->readFrom(rp, dbase);
  if (n == 0)
    return NULL;
  else
    return ix[n-1];
}

entry* band::getRefUsing(entry** ix2) {
  assert(!isDone);
  assert(ix == null);
  int n = c->readFrom(rp, dbase);
  if (n == 0)
    return NULL;
  else
    return ix2[n-1];
}

#ifndef PRODUCT
bool band::assertDone() {
  assert(!isDone);
  isDone = true;
  if (rpLimit == null) {
    rpLimit = rp;  // client did setInputFrom, not readFrom
    return true;
  } else {
    if (rp != rpLimit)
      fprintf(errstrm, "%s: rp off by %d\n", name, (int)( rp - rpLimit ));
    return rp == rpLimit;
  }
}
#endif


#define INDEX(tag) ((entry**)((tag)*2+1))

band all_bands[] = {
  BAND_INIT("package_magic", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("package_header", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("cp_Utf8_prefix", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("cp_Utf8_suffix", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("cp_Utf8_chars", CODING_SPEC(3,128,0,0), null),
  BAND_INIT("cp_Utf8_big_length", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("cp_Utf8_big_chars", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("cp_Int", CODING_SPEC(5,64,0,1), null),
  BAND_INIT("cp_Float", CODING_SPEC(5,64,0,1), null),
  BAND_INIT("cp_Long_hi", CODING_SPEC(5,64,0,1), null),
  BAND_INIT("cp_Long_lo", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("cp_Double_hi", CODING_SPEC(5,64,0,1), null),
  BAND_INIT("cp_Double_lo", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("cp_String", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_Utf8)),
  BAND_INIT("cp_Class", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_Utf8)),
  BAND_INIT("cp_Signature_form", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Utf8)),
  BAND_INIT("cp_Signature_classes", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_Class)),
  BAND_INIT("cp_Descr_name", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_Utf8)),
  BAND_INIT("cp_Descr_type", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Signature)),
  BAND_INIT("cp_Field_class", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("cp_Field_desc", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_NameandType)),
  BAND_INIT("cp_Method_class", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("cp_Method_desc", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_NameandType)),
  BAND_INIT("cp_Imethod_class", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("cp_Imethod_desc", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_NameandType)),
  BAND_INIT("class_flags", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("class_this", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("class_super", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("class_interface_count", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("class_interface", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("ic_this_class", CODING_SPEC(5,64,0,1), INDEX(CONSTANT_Class)),
  BAND_INIT("ic_flags", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("ic_outer_class", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("ic_name", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Utf8)),
  BAND_INIT("class_field_count", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("class_method_count", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("field_flags", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("method_flags", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("field_descr", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_NameandType)),
  BAND_INIT("method_descr", CODING_SPEC(5,64,2,1), INDEX(CONSTANT_NameandType)),
  BAND_INIT("field_constant_value", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("method_exception_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("method_exceptions", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Class)),
  BAND_INIT("code_headers", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("code_max_stack", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("code_max_locals", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("code_handler_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("code_handler_start", CODING_SPEC(5,4,0,0), null),
  BAND_INIT("code_handler_end", CODING_SPEC(5,4,0,0), null),
  BAND_INIT("code_handler_catch", CODING_SPEC(5,4,0,0), null),
  BAND_INIT("code_handler_class", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Class)),
  BAND_INIT("code_attr_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("method_attr_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("field_attr_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("class_attr_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("code_attr_name", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Utf8)),
  BAND_INIT("method_attr_name", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Utf8)),
  BAND_INIT("field_attr_name", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Utf8)),
  BAND_INIT("class_attr_name", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Utf8)),
  BAND_INIT("package_attr_name", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Utf8)),
  BAND_INIT("code_attr_size", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("method_attr_size", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("field_attr_size", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("class_attr_size", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("package_attr_size", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("code_attr_bits", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("method_attr_bits", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("field_attr_bits", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("class_attr_bits", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("package_attr_bits", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("bc_codes", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("bc_case_count", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_case_value", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("bc_byte", CODING_SPEC(1,256,0,0), null),
  BAND_INIT("bc_short", CODING_SPEC(5,64,1,1), null),
  BAND_INIT("bc_local", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_label", CODING_SPEC(5,4,2,0), null),
  BAND_INIT("bc_intref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Integer)),
  BAND_INIT("bc_floatref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Float)),
  BAND_INIT("bc_longref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Long)),
  BAND_INIT("bc_doubleref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Double)),
  BAND_INIT("bc_stringref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_String)),
  BAND_INIT("bc_classref", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Class)),
  BAND_INIT("bc_fieldref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_Fieldref)),
  BAND_INIT("bc_methodref", CODING_SPEC(5,64,0,0), INDEX(CONSTANT_Methodref)),
  BAND_INIT("bc_imethodref", CODING_SPEC(5,64,1,1), INDEX(CONSTANT_InterfaceMethodref)),
  BAND_INIT("bc_thismethod", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_supermethod", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_thisfield", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_superfield", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("bc_initref", CODING_SPEC(5,64,0,0), null),
  BAND_INIT("fake_package_attr_count", CODING_SPEC(4,256,0,0), null),
  {0}
};

void band::initStatics() {
  for (band* scan =  &all_bands[0]; scan->c; scan++) {
    scan->init();
  }
}

void band::initIndexes() {
  for (band* scan =  &all_bands[0]; scan->c; scan++) {
    void* ix = scan->ix;
    if ((void*)(int)ix == ix && ((int)ix & 1) != 0) {
      uint tag = ((int)ix - 1) / 2;  // Cf. #define INDEX(tag) above
      assert(tag < CONSTANT_Limit);  // May fail on very odd platforms!
      scan->ix = getCPIndex(tag);
    }
  }
}


void attr_bands::expectClient(void* client) {
  clients.add(client);
  count->expectMoreLength(1);
  // expected # of names and sizes is computed later (from counts)
}
void attr_bands::readCountsFrom(byte* &rpVar) {
  count->readFrom(rpVar);
  band snapshot = *count;
  int na = 0, nc = count->length;
  for (int i = 0; i < nc; i++)
    na += count->getUnsigned();
  *count = snapshot;  // will replay one more time
  name->expectLength(na);
  size->expectLength(na);
}
void attr_bands::readNamesFrom(byte* &rpVar) {
  name->readFrom(rpVar);
}
void attr_bands::readSizesFrom(byte* &rpVar) {
  size->readFrom(rpVar);
}
void attr_bands::setBitsInputFrom(byte* &rpVar) {
  bits->setInputFrom(rpVar);
}
void attr_bands::done(byte* &rpVar) {
  count->done();
  name->done();
  size->done();
  bits->done();
  rpVar = bits->rp;  // copy out final read position
}


#define fake_attr_count fake_package_attr_count
attr_bands all_attr_bands[ATTR_BAND_LIMIT] = {
 { &code_attr_count,   &code_attr_name,    &code_attr_size,    &code_attr_bits },
 { &method_attr_count, &method_attr_name,  &method_attr_size,  &method_attr_bits },
 { &field_attr_count,  &field_attr_name,   &field_attr_size,   &field_attr_bits },
 { &class_attr_count,  &class_attr_name,   &class_attr_size,   &class_attr_bits },
 { &fake_attr_count,   &package_attr_name, &package_attr_size, &package_attr_bits }
};

