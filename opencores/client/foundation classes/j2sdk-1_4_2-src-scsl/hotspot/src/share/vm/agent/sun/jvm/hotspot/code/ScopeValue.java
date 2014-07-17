/*
 * @(#)ScopeValue.java	1.5 03/01/23 11:24:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;

import sun.jvm.hotspot.utilities.*;

/** <P> Classes used for serializing debugging information. These
    abstractions are introducted to provide symmetric read and write
    operations. </P>

    <P>
    <UL>
    <LI> ScopeValue: describes the value of a variable/expression in a scope
    <UL>
    <LI> LocationValue: describes a value in a given location (in frame or register)
    <LI> ConstantValue: describes a constant
    </UL>
    </UL>
    </P> */

public abstract class ScopeValue {
  // Package private enumeration values (FIXME: read from target VM)
  static final int LOCATION_CODE        = 0;
  static final int CONSTANT_INT_CODE    = 1;
  static final int CONSTANT_OOP_CODE    = 2;
  static final int CONSTANT_LONG_CODE   = 3;
  static final int CONSTANT_DOUBLE_CODE = 4;

  public boolean isLocation()       { return false; }
  public boolean isConstantInt()    { return false; }
  public boolean isConstantDouble() { return false; }
  public boolean isConstantLong()   { return false; }
  public boolean isConstantOop()    { return false; }

  public static ScopeValue readFrom(DebugInfoReadStream stream) {
    switch (stream.readInt()) {
    case LOCATION_CODE:
      return new LocationValue(stream);
    case CONSTANT_INT_CODE:
      return new ConstantIntValue(stream);
    case CONSTANT_OOP_CODE:
      return new ConstantOopReadValue(stream);
    case CONSTANT_LONG_CODE:
      return new ConstantLongValue(stream);
    case CONSTANT_DOUBLE_CODE:
      return new ConstantDoubleValue(stream);
    default:
      Assert.that(false, "should not reach here");
      return null;
    }
  }

  public abstract void printOn(PrintStream tty);
}
