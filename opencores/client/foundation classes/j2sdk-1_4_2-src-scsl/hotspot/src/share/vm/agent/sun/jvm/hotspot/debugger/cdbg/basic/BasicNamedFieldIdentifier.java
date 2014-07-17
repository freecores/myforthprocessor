/*
 * @(#)BasicNamedFieldIdentifier.java	1.3 03/01/23 11:28:46
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic;

import sun.jvm.hotspot.debugger.cdbg.*;

public class BasicNamedFieldIdentifier implements NamedFieldIdentifier {
  private Field field;

  public BasicNamedFieldIdentifier(Field field) {
    this.field = field;
  }

  public String getName()  { return field.getName(); }
  public Type getType()    { return field.getType(); }
  public String toString() { return getName(); }
}
