/*
 * @(#)ArrayType.java	1.4 03/01/23 11:26:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface ArrayType extends Type {
  public Type getElementType();
  public int  getLength();
}
