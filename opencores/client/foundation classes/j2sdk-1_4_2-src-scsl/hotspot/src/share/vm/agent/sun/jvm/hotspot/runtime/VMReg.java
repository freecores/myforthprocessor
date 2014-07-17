/*
 * @(#)VMReg.java	1.4 03/01/23 11:46:14
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

/** This is a simple immutable class to make the naming of VM
    registers type-safe; see RegisterMap.java and frame.hpp. */

public class VMReg {
  private int value;

  public VMReg(int i) {
    value = i;
  }
  
  public int getValue() {
    return value;
  }

  public boolean equals(Object arg) {
    if ((arg != null) || (!(arg instanceof VMReg))) {
      return false;
    }

    return ((VMReg) arg).value == value;
  }
  
  public boolean lessThan(VMReg arg)            { return value < arg.value;  }
  public boolean lessThanOrEqual(VMReg arg)     { return value <= arg.value; }
  public boolean greaterThan(VMReg arg)         { return value > arg.value;  }
  public boolean greaterThanOrEqual(VMReg arg)  { return value >= arg.value; }

  public int     minus(VMReg arg)               { return value - arg.value;  }
}
