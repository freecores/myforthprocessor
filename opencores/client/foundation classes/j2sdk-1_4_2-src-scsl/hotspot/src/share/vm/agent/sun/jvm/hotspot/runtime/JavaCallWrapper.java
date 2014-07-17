/*
 * @(#)JavaCallWrapper.java	1.6 03/01/23 11:45:07
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** <P> A JavaCallWrapper is constructed before each JavaCall and
    destroyed after the call.  Its purpose is to allocate/deallocate a
    new handle block and to save/restore the last Java fp/sp. A
    pointer to the JavaCallWrapper is stored on the stack. </P>

    <P> NOTE: this port is very minimal and is only designed to get
    frame traversal working. FIXME: we will have to add platform-
    specific stuff later and therefore a factory for these
    objects. </P> */

public class JavaCallWrapper extends VMObject {
  protected static AddressField  anchorField;
  private static AddressField  lastJavaSPField;
  private static AddressField  notAtCallIDField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("JavaCallWrapper");
    Type anchorType = db.lookupType("JavaFrameAnchor");
    
    anchorField      = type.getAddressField("_anchor");
    lastJavaSPField  = anchorType.getAddressField("_last_Java_sp");
    notAtCallIDField = anchorType.getAddressField("_not_at_call_id");
  }

  public JavaCallWrapper(Address addr) {
    super(addr);
  }

  public Address getLastJavaSP() {
    return lastJavaSPField.getValue(addr.addOffsetTo(anchorField.getOffset()));
  }

  public Address getNotAtCallID() {
    return notAtCallIDField.getValue(addr.addOffsetTo(anchorField.getOffset()));
  }
}
