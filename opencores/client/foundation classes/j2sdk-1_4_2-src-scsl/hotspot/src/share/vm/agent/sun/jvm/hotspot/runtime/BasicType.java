/*
 * @(#)BasicType.java	1.5 03/01/23 11:44:32
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

/** Encapsulates the BasicType enum in globalDefinitions.hpp in the
    VM. */

public class BasicType {
  private static int tBoolean  = 4;
  private static int tChar     = 5;
  private static int tFloat    = 6;
  private static int tDouble   = 7;
  private static int tByte     = 8;
  private static int tShort    = 9;
  private static int tInt      = 10;
  private static int tLong     = 11;
  private static int tObject   = 12;
  private static int tArray    = 13;
  private static int tVoid     = 14;
  private static int tAddress  = 15;
  private static int tConflict = 16;
  private static int tIllegal  = 99;

  public static int getTBoolean() {
    return tBoolean;
  }

  public static int getTChar() {
    return tChar;
  }

  public static int getTFloat() {
    return tFloat;
  }

  public static int getTDouble() {
    return tDouble;
  }

  public static int getTByte() {
    return tByte;
  }

  public static int getTShort() {
    return tShort;
  }

  public static int getTInt() {
    return tInt;
  }

  public static int getTLong() {
    return tLong;
  }

  public static int getTObject() {
    return tObject;
  }

  public static int getTArray() {
    return tArray;
  }

  public static int getTVoid() {
    return tVoid;
  }

  public static int getTAddress() {
    return tAddress;
  }

  /** For stack value type with conflicting contents */
  public static int getTConflict() {
    return tConflict;
  }

  public static int getTIllegal() {
    return tIllegal;
  }

  public static int charToType(char c) {
    switch( c ) {
    case 'B': return tByte;
    case 'C': return tChar;
    case 'D': return tDouble;
    case 'F': return tFloat;
    case 'I': return tInt;
    case 'J': return tLong;
    case 'S': return tShort;
    case 'Z': return tBoolean;
    case 'V': return tVoid;
    case 'L': return tObject;
    case '[': return tArray;
    }
    return tIllegal;
  }
}
