/*
 * @(#)GenerationSpec.java	1.3 03/01/23 11:40:57
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class GenerationSpec extends VMObject {
  private static CIntegerField gnField;
  private static CIntegerField initSizeField;
  private static CIntegerField maxSizeField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("GenerationSpec");
    
    gnField       = type.getCIntegerField("gn");
    initSizeField = type.getCIntegerField("initSize");
    maxSizeField  = type.getCIntegerField("maxSize");
  }

  public GenerationSpec(Address addr) {
    super(addr);
  }

  public GenerationName name() {
    return GenerationName.nameForEnum((int) gnField.getValue(addr));
  }

  public long initSize() {
    return initSizeField.getValue(addr);
  }

  public long maxSize() {
    return maxSizeField.getValue(addr);
  }
}
