/*
 * @(#)BytecodeRet.java	1.2 03/01/23 11:37:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class BytecodeRet extends BytecodeWideable {
  BytecodeRet(Method method, int bci) {
    super(method, bci);
  }

  public void verify() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isValid(), "check ret");
    }
  }

  public boolean isValid() {
    return javaCode() == Bytecodes._ret;
  }

  public static BytecodeRet at(Method method, int bci) {
    BytecodeRet b = new BytecodeRet(method, bci);
    if (Assert.ASSERTS_ENABLED) {
      b.verify();
    }
    return b;
  }

  /** Like at, but returns null if the BCI is not at ret  */
  public static BytecodeRet atCheck(Method method, int bci) {
    BytecodeRet b = new BytecodeRet(method, bci);
    return (b.isValid() ? b : null);
  }

  public static BytecodeRet at(BytecodeStream bcs) {
    return new BytecodeRet(bcs.method(), bcs.bci());
  }

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append("ret");
    buf.append(spaces);
    buf.append('#');
    buf.append(new Integer(getLocalVarIndex()).toString());
    return buf.toString();
  }
}
