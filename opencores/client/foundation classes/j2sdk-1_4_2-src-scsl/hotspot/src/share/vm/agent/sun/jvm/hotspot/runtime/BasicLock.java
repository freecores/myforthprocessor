/*
 * @(#)BasicLock.java	1.3 03/01/23 11:44:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;

public class BasicLock extends VMObject {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type  = db.lookupType("BasicLock");
    displacedHeaderField = type.getCIntegerField("_displaced_header");
  }

  private static CIntegerField displacedHeaderField;

  public BasicLock(Address addr) {
    super(addr);
  }

  public Mark displacedHeader() {
    return new Mark(addr.addOffsetTo(displacedHeaderField.getOffset()));
  }
}
