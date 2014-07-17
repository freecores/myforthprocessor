/*
 * @(#)MemberFunctionType.java	1.3 03/01/23 11:27:23
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface MemberFunctionType extends FunctionType {
  /** Containing class of this member function */
  public Type getContainingClass();

  /** Type of this pointer */
  public Type getThisType();

  /** Logical this adjustor for the method. Whenever a class element
      is referenced via the this pointer, thisadjust will be added to
      the resultant offset before referencing the element. */
  public long getThisAdjust();
}
