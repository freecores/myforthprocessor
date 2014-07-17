/*
 * @(#)FunctionType.java	1.4 03/01/23 11:27:03
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface FunctionType extends Type {
  public Type getReturnType();
  public int  getNumArguments();
  public Type getArgumentType(int i);
}
