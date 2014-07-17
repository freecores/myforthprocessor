/*
 * @(#)BytecodeLoadStore.java	1.2 03/01/23 11:36:52
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;

public abstract class BytecodeLoadStore extends BytecodeWideable {
  BytecodeLoadStore(Method method, int bci) {
    super(method, bci);
  }

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append(getJavaBytecodeName());
    buf.append(spaces);
    buf.append('#');
    buf.append(new Integer(getLocalVarIndex()).toString());
    return buf.toString();
  }
}
