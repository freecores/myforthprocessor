/*
 * @(#)BasicIndexableFieldIdentifier.java	1.3 03/01/23 11:28:31
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic;

import sun.jvm.hotspot.debugger.cdbg.*;

public class BasicIndexableFieldIdentifier implements IndexableFieldIdentifier {
  private Type type;
  private int  index;

  public BasicIndexableFieldIdentifier(Type type, int index) {
    this.type = type;
    this.index = index;
  }

  public Type    getType()  { return type; }
  public int     getIndex() { return index; }
  public String  toString() { return Integer.toString(getIndex()); }
}
