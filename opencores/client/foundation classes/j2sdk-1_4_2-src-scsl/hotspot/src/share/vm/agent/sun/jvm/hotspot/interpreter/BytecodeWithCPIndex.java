/*
 * @(#)BytecodeWithCPIndex.java	1.3 03/01/23 11:37:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;

// any bytecode with constant pool index

public abstract class BytecodeWithCPIndex extends Bytecode {
  BytecodeWithCPIndex(Method method, int bci) {
    super(method, bci);
  }

  // the constant pool index for this bytecode
  public int index() { return 0x00FF & javaShortAt(1); }

  protected int indexForFieldOrMethod() {
     ConstantPoolCache cpCache = method().getConstants().getCache();
     // get ConstantPool index from ConstantPoolCacheIndex at given bci
     int cpCacheIndex = index();
     if (cpCache == null) {
        return cpCacheIndex;
     } else {
        // change byte-ordering and go via cache
        return cpCache.getEntryAt((int) VM.getVM().getBytes().swapShort((short) cpCacheIndex)).getConstantPoolIndex();
     }
  }
}
