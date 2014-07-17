/*
 * @(#)BitType.java	1.3 03/01/23 11:26:21
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface BitType extends IntType {
  /** Size in bits of this type */
  public int getSizeInBits();

  /** Offset from the least-significant bit (LSB) of the LSB of this
      type */
  public int getOffset();
}
