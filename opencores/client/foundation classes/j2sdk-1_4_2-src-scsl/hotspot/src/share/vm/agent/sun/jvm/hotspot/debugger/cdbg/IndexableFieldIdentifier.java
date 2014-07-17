/*
 * @(#)IndexableFieldIdentifier.java	1.3 03/01/23 11:27:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Identifier for indices of arrays */

public interface IndexableFieldIdentifier extends FieldIdentifier {
  public int getIndex();
}
