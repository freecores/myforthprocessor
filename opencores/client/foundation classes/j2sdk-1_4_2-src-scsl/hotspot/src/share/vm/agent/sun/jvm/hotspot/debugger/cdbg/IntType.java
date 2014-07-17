/*
 * @(#)IntType.java	1.4 03/01/23 11:27:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface IntType extends Type {
  /** Returns size in bytes of this type */
  public int getIntSize();

  /** Indicates whether this type is unsigned */
  public boolean isUnsigned();
}
