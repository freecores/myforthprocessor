/*
 * @(#)bytes.cpp	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"
#include "bytes.h"
#include "utils.h"


bool bytes::inBounds(const void* p) {
  return p >= ptr && p < limit();
}

void bytes::malloc(int len_) {
  len = len_;
  ptr = NEW(byte, len_+1);  // add trailing zero byte always
}

void bytes::realloc(int len_) {
  if (ptr == null) {
    malloc(len_);
    return;
  }
  ptr = (byte*)::realloc(ptr, len_+1);
  if (ptr == null)  unpack_abort("ERROR: Internal Error");
  if (len < len_)  memset(ptr+len, 0, len_-len);
  ptr[len_] = 0;
  len = len_;
}

void bytes::free() {
  ::free(ptr);
  len = 0;
  ptr = 0;
}

int bytes::indexOf(byte c) {
  byte* p = (byte*) memchr(ptr, c, len);
  return (p == 0) ? -1 : p - ptr;
}

int bytes::lastIndexOf(byte c) {
  for (byte* cp = limit(); cp > ptr; ) {
    if (*--cp == c)  return cp - ptr;
  }
  return -1;
}

byte* bytes::writeTo(byte* bp) {
  memcpy(bp, ptr, len);
  return bp+len;
}

int bytes::compareTo(bytes& other) {
  int l1 = len;
  int l2 = other.len;
  int cmp = memcmp(ptr, other.ptr, (l1 < l2) ? l1 : l2);
  return (cmp != 0) ? cmp : l1 - l2;
}

void bytes::saveFrom(const void* ptr_, int len_) {
  malloc(len_);
  copyFrom(ptr_, len_);
}

void bytes::copyFrom(const void* ptr_, int len_, int offset) {
  assert(len_ == 0 || inBounds(ptr + offset));
  assert(len_ == 0 || inBounds(ptr + offset+len_-1));
  memcpy(ptr+offset, ptr_, len_);
}


#ifndef PRODUCT
const char* bytes::string() {
  if (len == 0)  return "";
  if (ptr[len] == 0 && strlen((char*)ptr) == len)  return (const char*) ptr;
  bytes junk;
  junk.saveFrom(*this);
  return (char*) junk.ptr;
}
#endif


void ptrlist::add(void* p) {
  if (b.len == len * sizeof(void*)) {
    b.realloc((len == 0 ? 1000 : len*2) * sizeof(void*));
  }
  base()[len++] = p;
}

bool ptrlist::contains(void* p) {
  for (int i = 0; i < len; i++) {
    if (get(i) == p)  return true;
  }
  return false;
}

void ptrlist::free() {
  b.free();
  len = 0;
}


extern "C"
void test(bytes& b1, bytes& b2) {
  b2 = b1.slice(0, 10);
}
