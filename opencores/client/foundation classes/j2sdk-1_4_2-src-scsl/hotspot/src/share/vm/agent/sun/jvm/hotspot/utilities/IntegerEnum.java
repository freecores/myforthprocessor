/*
 * @(#)IntegerEnum.java	1.4 03/01/23 11:51:23
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

/** A class enabling simple construction of type-safe integer
    enumerations */

public class IntegerEnum {
  private int value;

  /** Constructor is protected because it will only be used by the
      subclass */
  protected IntegerEnum(int value) {
    this.value = value;
  }

  public int getValue() {
    return value;
  }
}
