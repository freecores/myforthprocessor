/*
 * @(#)coding.cpp	1.4 03/01/23
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

#include "constants.h"
#include "zip.h"

extern coding basic_codings[];

#define CODING_PRIVATE \
  int spec_ = this->spec; \
  int B = CODING_B(spec_); \
  int H = CODING_H(spec_); \
  int L = 256 - H; \
  int S = CODING_S(spec_); \
  int D = CODING_D(spec_)

#define SAT32(longval) ( \
  INT_MAX_VALUE < (longval) ? INT_MAX_VALUE : \
  INT_MIN_VALUE > (longval) ? INT_MIN_VALUE : \
  (int)(longval) )

#define IS_NEG_CODE(S, codeVal) \
  ( (((int)(codeVal)+1) & ((1<<S)-1)) == 0 )

#define DECODE_SIGN_S1(ux) \
  ( ((uint)(ux) >> 1) ^ -((ux) & 1) )

static jlong decode_sign(int S, jlong ux) {
  assert(S > 0);
  jlong sigbits = (ux >> S);
  if (IS_NEG_CODE(S, ux))
    return ~sigbits;
  else
    return ux - sigbits;
}

coding* coding::init() {
  if (umax > 0)  return this;  // already done
  assert(spec != 0);  // sanity

  // fill in derived fields
  CODING_PRIVATE;

  // first compute the range of the coding, in 64 bits
  jlong range = 0;
  assert(!(B == 1 && H != 256));  // no byte subranges, please
  jlong H_i = 1;
  for (int i = 0; i < B; i++) {
    range += H_i;
    H_i *= H;
  }
  range *= L;
  range += H_i;
  assert(range > 0);  // no useless codings, please

  // now, compute min and max
  this->umax = SAT32(range-1);
  this->umin = (range >= ((jlong)1 << 32)) ? INT_MIN_VALUE : 0;
  if (S == 0) {
    this->max = this->umax;
    this->min = this->umin;
  } else {
    int Smask = (1<<S)-1;
    jlong maxPosCode = range-1;
    if (IS_NEG_CODE(S, maxPosCode))  --maxPosCode;
    jlong maxL = decode_sign(S, maxPosCode);
    assert(maxL > 0);
    this->max = SAT32(maxL);
    jlong maxNegCode = (range & ~Smask)-1;
    jlong minL = decode_sign(S, maxNegCode);
    assert(minL < 0);  // no useless signed codings, please
    this->min = SAT32(minL);
  }

  assert(!(isFullRange | isSubrange | isSubrange)); // init
  if (min < 0)
    this->isSigned = true;
  if (max < INT_MAX_VALUE && range <= INT_MAX_VALUE)
    this->isSubrange = true;
  if (max == INT_MAX_VALUE && min == INT_MIN_VALUE)
    this->isFullRange = true;

  assert(this == findBySpec(spec));

  return this;
}

coding* coding::findBySpec(int spec) {
  for (coding* scan = &basic_codings[0]; ; scan++) {
    if (scan->spec == spec)
      return scan->init();
    assert(scan->spec != 0);  // do not run off end of array
  }
}

int coding::readFrom(byte* &rpVar, int* dbase) {
  int value[1];
  readArrayFrom(rpVar, dbase, 1, value);
  return value[0];
}

void coding::readArrayFrom(byte* &rpVar, int* dbase, int length, int* values) {
  CODING_PRIVATE;
  byte* rp = rpVar;
  if (values == null) {
    // Just skip; do not decode.
    if (B == 1 || H == 256) {
      rp += length * B;
    } else {
      for (int n = 0; n < length; n++) {
	for (int i = 0; i < B; i++) {
	  int b_i = *rp++ & 0xFF;
	  if (b_i < L)  break;
	}
      }
    }
  } else {
    // Decode signed values into the given array:
    for (int n = 0; n < length; n++) {
      julong sum = 0;  // S>=2 requires a few more bits than 32
      int   H_i = 1;  // radix never overflows, though
      for (int i = 0; i < B; i++) {
	int b_i = *rp++ & 0xFF;
	sum += b_i * (julong)H_i;
	assert(b_i < L || i+1 == B || H_i*H/H == H_i);  // no radix overflow
	H_i *= H;
	if (b_i < L)  break;
      }
      if (S == 0) {
	values[n] = (int)sum;
      } else if (S == 1) {
	values[n] = DECODE_SIGN_S1((int)sum);
	assert(values[n] == decode_sign(S, sum));
      } else {
	values[n] = decode_sign(S, sum);  // requires 64-bit sum
      }
    }
    // Handle delta-coding:
    int range = 0;
    if (isSubrange) {
      range = umax+1;
      assert(range > 0);
    }
    for (int dstep = 0; dstep < D; dstep++) {
      int acc = dbase[dstep];
      for (int n = 0; n < length; n++) {
	int del = values[n];
	acc += del;
	if (range > 0) {
	  NOT_PRODUCT(int acc0 = acc);
	  acc %= range;
	  if (acc < 0)  acc += range;
	  assert(canRepresentUnsigned(acc));
	  assert(!(canRepresentUnsigned(acc0) && acc != acc0));
	}
	values[n] = acc;
      }
      dbase[dstep] = acc;
    }
  }
  rpVar = rp;  // update scan pointer of caller
}

coding basic_codings[] = {
  // This one is not a usable irregular coding, but is used by cp_Utf8_chars.
  CODING_INIT(3,128,0,0),

  // Fixed-length codings:
  CODING_INIT(1,256,0,0),
  CODING_INIT(1,256,1,0),
  CODING_INIT(1,256,0,1),
  CODING_INIT(1,256,1,1),
  CODING_INIT(2,256,0,0),
  CODING_INIT(2,256,1,0),
  CODING_INIT(2,256,0,1),
  CODING_INIT(2,256,1,1),
  CODING_INIT(3,256,0,0),
  CODING_INIT(3,256,1,0),
  CODING_INIT(3,256,0,1),
  CODING_INIT(3,256,1,1),
  CODING_INIT(4,256,0,0),
  CODING_INIT(4,256,1,0),
  CODING_INIT(4,256,0,1),
  CODING_INIT(4,256,1,1),

  // Full-range variable-length codings:
  CODING_INIT(5,  4,0,0),
  CODING_INIT(5,  4,1,0),
  CODING_INIT(5,  4,2,0),
  CODING_INIT(5, 16,0,0),
  CODING_INIT(5, 16,1,0),
  CODING_INIT(5, 16,2,0),
  CODING_INIT(5, 32,0,0),
  CODING_INIT(5, 32,1,0),
  CODING_INIT(5, 32,2,0),
  CODING_INIT(5, 64,0,0),
  CODING_INIT(5, 64,1,0),
  CODING_INIT(5, 64,2,0),
  CODING_INIT(5,128,0,0),
  CODING_INIT(5,128,1,0),
  CODING_INIT(5,128,2,0),

  CODING_INIT(5,  4,0,1),
  CODING_INIT(5,  4,1,1),
  CODING_INIT(5,  4,2,1),
  CODING_INIT(5, 16,0,1),
  CODING_INIT(5, 16,1,1),
  CODING_INIT(5, 16,2,1),
  CODING_INIT(5, 32,0,1),
  CODING_INIT(5, 32,1,1),
  CODING_INIT(5, 32,2,1),
  CODING_INIT(5, 64,0,1),
  CODING_INIT(5, 64,1,1),
  CODING_INIT(5, 64,2,1),
  CODING_INIT(5,128,0,1),
  CODING_INIT(5,128,1,1),
  CODING_INIT(5,128,2,1),

  // Variable length subrange codings:
  CODING_INIT(2,192,0,0),
  CODING_INIT(2,224,0,0),
  CODING_INIT(2,240,0,0),
  CODING_INIT(2,248,0,0),
  CODING_INIT(2,252,0,0),

  CODING_INIT(2,  8,0,1),
  CODING_INIT(2,  8,1,1),
  CODING_INIT(2, 16,0,1),
  CODING_INIT(2, 16,1,1),
  CODING_INIT(2, 32,0,1),
  CODING_INIT(2, 32,1,1),
  CODING_INIT(2, 64,0,1),
  CODING_INIT(2, 64,1,1),
  CODING_INIT(2,128,0,1),
  CODING_INIT(2,128,1,1),
  CODING_INIT(2,192,0,1),
  CODING_INIT(2,192,1,1),
  CODING_INIT(2,224,0,1),
  CODING_INIT(2,224,1,1),
  CODING_INIT(2,240,0,1),
  CODING_INIT(2,240,1,1),
  CODING_INIT(2,248,0,1),
  CODING_INIT(2,248,1,1),

  CODING_INIT(3,192,0,0),
  CODING_INIT(3,224,0,0),
  CODING_INIT(3,240,0,0),
  CODING_INIT(3,248,0,0),
  CODING_INIT(3,252,0,0),

  CODING_INIT(3,  8,0,1),
  CODING_INIT(3,  8,1,1),
  CODING_INIT(3, 16,0,1),
  CODING_INIT(3, 16,1,1),
  CODING_INIT(3, 32,0,1),
  CODING_INIT(3, 32,1,1),
  CODING_INIT(3, 64,0,1),
  CODING_INIT(3, 64,1,1),
  CODING_INIT(3,128,0,1),
  CODING_INIT(3,128,1,1),
  CODING_INIT(3,192,0,1),
  CODING_INIT(3,192,1,1),
  CODING_INIT(3,224,0,1),
  CODING_INIT(3,224,1,1),
  CODING_INIT(3,240,0,1),
  CODING_INIT(3,240,1,1),
  CODING_INIT(3,248,0,1),
  CODING_INIT(3,248,1,1),

  CODING_INIT(4,192,0,0),
  CODING_INIT(4,224,0,0),
  CODING_INIT(4,240,0,0),
  CODING_INIT(4,248,0,0),
  CODING_INIT(4,252,0,0),

  CODING_INIT(4,  8,0,1),
  CODING_INIT(4,  8,1,1),
  CODING_INIT(4, 16,0,1),
  CODING_INIT(4, 16,1,1),
  CODING_INIT(4, 32,0,1),
  CODING_INIT(4, 32,1,1),
  CODING_INIT(4, 64,0,1),
  CODING_INIT(4, 64,1,1),
  CODING_INIT(4,128,0,1),
  CODING_INIT(4,128,1,1),
  CODING_INIT(4,192,0,1),
  CODING_INIT(4,192,1,1),
  CODING_INIT(4,224,0,1),
  CODING_INIT(4,224,1,1),
  CODING_INIT(4,240,0,1),
  CODING_INIT(4,240,1,1),
  CODING_INIT(4,248,0,1),
  CODING_INIT(4,248,1,1),

  0
};

coding* coding::findByIndex(int irregularCodingIndex) {
  assert(irregularCodingIndex > 0);
  coding* c = &basic_codings[irregularCodingIndex];
  assert((char*)c - (char*)basic_codings < sizeof(basic_codings));
  return c->init();
}

#ifndef PRODUCT
const char* coding::string() {
  CODING_PRIVATE;
  bytes buf;
  buf.malloc(100);
  char maxS[20], minS[20];
  sprintf(maxS, "%d", max);
  sprintf(minS, "%d", min);
  if (max == INT_MAX_VALUE)  strcpy(maxS, "max");
  if (min == INT_MIN_VALUE)  strcpy(minS, "min");
  sprintf((char*)buf.ptr, "(%d,%d,%d,%d) L=%d r=[%s,%s]",
	  B,H,S,D,L,minS,maxS);
  return (const char*) buf.ptr;
}
#endif
