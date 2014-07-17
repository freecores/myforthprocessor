/*
 * @(#)BytecodeWideable.java	1.3 03/01/23 11:37:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;

public abstract class BytecodeWideable extends Bytecode {
  BytecodeWideable(Method method, int bci) {
    super(method, bci);
  }

  public boolean isWide() {
    int prevBci = bci() - 1;
    return (prevBci > -1 && method.getBytecodeOrBPAt(prevBci) == Bytecodes._wide);
  }

  // the local variable index
  public int getLocalVarIndex() {
    return (isWide()) ? (int) (0xFFFF & javaShortAt(1))
            : (int) (0xFF & javaByteAt(1));
  }
}
