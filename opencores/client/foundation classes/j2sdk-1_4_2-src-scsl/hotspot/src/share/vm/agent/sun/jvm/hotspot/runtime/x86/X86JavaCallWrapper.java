/*
 * @(#)X86JavaCallWrapper.java	1.4 03/01/23 11:46:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime.x86;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;

public class X86JavaCallWrapper extends JavaCallWrapper {
  private static AddressField lastJavaFPField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("JavaFrameAnchor");
    
    lastJavaFPField  = type.getAddressField("_last_Java_fp");
  }

  public X86JavaCallWrapper(Address addr) {
    super(addr);
  }

  public Address getLastJavaFP() {
    return lastJavaFPField.getValue(addr.addOffsetTo(anchorField.getOffset()));
  }
}
