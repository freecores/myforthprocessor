/*
 * @(#)coding.h	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
 
#define INT_MAX_VALUE ((int)0x7FFFFFFF)
#define INT_MIN_VALUE ((int)0x80000000)

#define CODING_SPEC(B, H, S, D) ((B)<<20|(H)<<8|(S)<<4|(D)<<0)
#define CODING_B(x) ((x)>>20 & 0xF)
#define CODING_H(x) ((x)>>8  & 0xFFF)
#define CODING_S(x) ((x)>>4  & 0xF)
#define CODING_D(x) ((x)>>0  & 0xF)

#define long do_not_use_C_long_types_use_jlong_or_int

struct coding {
  int  spec;  // B,H,S,D

  // Handy values derived from the spec:
  int  min, max;
  int  umin, umax;
  char isSigned, isSubrange, isFullRange;

  coding* init();  // returns self

  static coding* findBySpec(int spec);
  static coding* findByIndex(int irregularCodingIndex);

  bool canRepresent(int x)         { return (x >= min  && x <= max);  }
  bool canRepresentUnsigned(int x) { return (x >= umin && x <= umax); }

  int readFrom(byte* &rpVar, int* dbase);
  void readArrayFrom(byte* &rpVar, int* dbase, int length, int* values);
  void skipArrayFrom(byte* &rpVar, int length) {
    readArrayFrom(rpVar, (int*)NULL, length, (int*)NULL);
  }

#ifndef PRODUCT
  const char* string();
#endif
};

#define CODING_INIT(B, H, S, D) \
  { CODING_SPEC(B, H, S, D) }

