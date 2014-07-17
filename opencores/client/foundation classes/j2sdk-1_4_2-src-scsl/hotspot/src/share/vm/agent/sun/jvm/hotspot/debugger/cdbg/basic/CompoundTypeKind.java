/*
 * @(#)CompoundTypeKind.java	1.3 03/01/23 11:29:02
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic;

/** Type-safe enum for discriminating between classes, structs and
    unions, which are all represented as compound types */

public class CompoundTypeKind {
  public static final CompoundTypeKind CLASS  = new CompoundTypeKind();
  public static final CompoundTypeKind STRUCT = new CompoundTypeKind();
  public static final CompoundTypeKind UNION  = new CompoundTypeKind();

  private CompoundTypeKind() {}
}
