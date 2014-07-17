/*
 * @(#)Bits.java	1.4 03/01/23 11:51:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

/** Bit manipulation routines */

public class Bits {
  public static final int AllBits = ~0;
  public static final int NoBits  = 0;
  public static final int OneBit  = 1;

  public static final int BitsPerByte =  8;
  public static final int BitsPerInt  = 32;

  public static int setBits(int x, int m) {
    return x | m;
  }

  public static int clearBits(int x, int m) {
    return x & ~m;
  }

  public static int nthBit(int n) {
    return (n > 32) ? 0 : (OneBit << n);
  }

  public static int setNthBit(int x, int n) {
    return setBits(x, nthBit(n));
  }

  public static int clearNthBit(int x, int n) {
    return clearBits(x, nthBit(n));
  }

  public static boolean isSetNthBit(int word, int n) {
    return maskBits(word, nthBit(n)) != NoBits;
  }

  public static int rightNBits(int n) {
    return (nthBit(n) - 1);
  }

  public static int maskBits(int x, int m) {
    return x & m;
  }

  public static long maskBitsLong(long x, long m) {
    return x & m;
  }

  /** Returns integer round-up to the nearest multiple of s (s must be
      a power of two) */
  public static int roundTo(int x, int s) {
    int m = s - 1;
    return maskBits(x + m, ~m);
  }
}
