/*
 * @(#)SharedInfo.java	1.4 03/01/23 11:37:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import java.util.*;

import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class SharedInfo {
  // FIXME: this is an OptoReg over in the VM, but it's mixed in use
  // and in comparisons with VMRegs (see, for example,
  // OopMapValue::stack_offset(). Since we don't currently need
  // OptoReg here in the SA, we call this a VMReg.

  private static VMReg stack0;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    // NOTE: we can not lookup SharedInfo::stack0 as an integer
    // constant because it's set up during the initialization process
    // of the VM, not at compile time.
    Type type = db.lookupType("SharedInfo");
    CIntegerField stack0Field = type.getCIntegerField("stack0");
    int stack0Val = (int) stack0Field.getValue();
    stack0 = new VMReg(stack0Val);
  }

  public static VMReg getStack0() {
    return stack0;
  }
}
