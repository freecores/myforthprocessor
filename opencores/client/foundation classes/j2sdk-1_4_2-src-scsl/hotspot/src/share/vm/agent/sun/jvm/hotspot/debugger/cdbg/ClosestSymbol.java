/*
 * @(#)ClosestSymbol.java	1.3 03/01/23 11:26:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Models the closest symbol to a given program counter: name and
    offset. */

public class ClosestSymbol {
  private String name;
  private long   offset;

  public ClosestSymbol(String name, long offset) {
    this.name = name;
    this.offset = offset;
  }

  public String getName() {
    return name;
  }

  public long getOffset() {
    return offset;
  }

  public String offsetAsHex() {
    return "0x" + Long.toHexString(offset);
  }
}
