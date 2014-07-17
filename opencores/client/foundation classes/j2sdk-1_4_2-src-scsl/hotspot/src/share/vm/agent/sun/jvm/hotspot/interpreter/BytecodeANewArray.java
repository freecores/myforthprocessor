/*
 * @(#)BytecodeANewArray.java	1.2 03/01/23 11:36:00
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class BytecodeANewArray extends BytecodeWithKlass {
  BytecodeANewArray(Method method, int bci) {
    super(method, bci);
  }

  public Klass getKlass() {
    return super.getKlass();
  }

  public void verify() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isValid(), "check anewarray");
    }
  }

  public boolean isValid() {
    return javaCode() == Bytecodes._anewarray;
  }

  public static BytecodeANewArray at(Method method, int bci) {
    BytecodeANewArray b = new BytecodeANewArray(method, bci);
    if (Assert.ASSERTS_ENABLED) {
      b.verify();
    }
    return b;
  }

  /** Like at, but returns null if the BCI is not at anewarray  */
  public static BytecodeANewArray atCheck(Method method, int bci) {
    BytecodeANewArray b = new BytecodeANewArray(method, bci);
    return (b.isValid() ? b : null);
  }

  public static BytecodeANewArray at(BytecodeStream bcs) {
    return new BytecodeANewArray(bcs.method(), bcs.bci());
  }
}
