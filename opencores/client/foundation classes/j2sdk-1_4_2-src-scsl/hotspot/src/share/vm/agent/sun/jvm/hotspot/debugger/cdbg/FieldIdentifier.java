/*
 * @(#)FieldIdentifier.java	1.3 03/01/23 11:26:54
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Abstraction over named fields and indices of arrays. Call
    toString() on a FieldIdentifier to get a printable name for the
    field. */

public interface FieldIdentifier {
  public Type getType();
  public String toString();
}
