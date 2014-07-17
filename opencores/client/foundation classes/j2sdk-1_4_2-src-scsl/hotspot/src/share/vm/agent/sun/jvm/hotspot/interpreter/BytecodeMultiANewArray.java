/*
 * @(#)BytecodeMultiANewArray.java	1.2 03/01/23 11:36:57
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class BytecodeMultiANewArray extends BytecodeWithKlass {
  BytecodeMultiANewArray(Method method, int bci)  {
    super(method, bci);
  }

  public Klass getKlass() {
    return super.getKlass();
  }

  public int getDimension() {
    return 0xFF & javaByteAt(2);
  }

  public void verify() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isValid(), "check multianewarray");
    }
  }

  public boolean isValid() {
    return javaCode() == Bytecodes._multianewarray;
  }

  public static BytecodeMultiANewArray at(Method method, int bci) {
    BytecodeMultiANewArray b = new BytecodeMultiANewArray(method, bci);
    if (Assert.ASSERTS_ENABLED) {
      b.verify();
    }
    return b;
  }

  /** Like at, but returns null if the BCI is not at multianewarray  */
  public static BytecodeMultiANewArray atCheck(Method method, int bci) {
    BytecodeMultiANewArray b = new BytecodeMultiANewArray(method, bci);
    return (b.isValid() ? b : null);
  }

  public static BytecodeMultiANewArray at(BytecodeStream bcs) {
    return new BytecodeMultiANewArray(bcs.method(), bcs.bci());
  }

  public String toString() {
    StringBuffer buf = new StringBuffer();
    buf.append(super.toString());
    buf.append(spaces);
    buf.append(new Integer(getDimension()).toString());
    return buf.toString();
  }
}
