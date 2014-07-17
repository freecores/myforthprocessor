/*
 * @(#)BytecodeJmp.java	1.2 03/01/23 11:36:39
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public abstract class BytecodeJmp extends Bytecode {
  BytecodeJmp(Method method, int bci) {
    super(method, bci);
  }

  public abstract int getTargetBCI();

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append(getJavaBytecodeName());
    buf.append(spaces);
    buf.append(new Integer(getTargetBCI()).toString());
    return buf.toString();
  }
}
