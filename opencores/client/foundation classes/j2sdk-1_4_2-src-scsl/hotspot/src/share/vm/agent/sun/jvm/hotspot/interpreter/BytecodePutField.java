/*
 * @(#)BytecodePutField.java	1.2 03/01/23 11:37:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;

public class BytecodePutField extends BytecodeGetPut {
  BytecodePutField(Method method, int bci) {
    super(method, bci);
  }

  public boolean isStatic() {
    return false;
  }

  public void verify() {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(isValid(), "check putfield");
    }
  }

  public boolean isValid() {
    return javaCode() == Bytecodes._putfield;
  }

  public static BytecodePutField at(Method method, int bci) {
    BytecodePutField b = new BytecodePutField(method, bci);
    if (Assert.ASSERTS_ENABLED) {
      b.verify();
    }
    return b;
  }

  /** Like at, but returns null if the BCI is not at putfield  */
  public static BytecodePutField atCheck(Method method, int bci) {
    BytecodePutField b = new BytecodePutField(method, bci);
    return (b.isValid() ? b : null);
  }

  public static BytecodePutField at(BytecodeStream bcs) {
    return new BytecodePutField(bcs.method(), bcs.bci());
  }
}
