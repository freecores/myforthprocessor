#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)helloWorld.cpp	1.3 03/01/23 11:04:22 JVM"
#endif
/* 
 * 
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include <stdio.h>
#include <inttypes.h>

extern "C" {
  const char* helloWorldString = "Hello, world!";
  // Do not change these values without changing TestDebugger.java as well
  // FIXME: should make these jbyte, jshort, etc...
  volatile int8_t  testByte     = 132;
  volatile int16_t testShort    = 27890;
  volatile int32_t testInt      = 1020304050;
  volatile int64_t testLong     = 102030405060708090LL;
  volatile float   testFloat    = 35.4F;
  volatile double  testDouble   = 1.23456789;

  volatile int helloWorldTrigger = 0;
}

int
main(int, char**) {
  while (1) {
    while (helloWorldTrigger == 0) {
    }

    fprintf(stderr, "%s\n", helloWorldString);
    fprintf(stderr, "testByte=%d\n", testByte);
    fprintf(stderr, "testShort=%d\n", testShort);
    fprintf(stderr, "testInt=%d\n", testInt);
    fprintf(stderr, "testLong=%d\n", testLong);
    fprintf(stderr, "testFloat=%d\n", testFloat);
    fprintf(stderr, "testDouble=%d\n", testDouble);

    while (helloWorldTrigger != 0) {
    }
  }
}
