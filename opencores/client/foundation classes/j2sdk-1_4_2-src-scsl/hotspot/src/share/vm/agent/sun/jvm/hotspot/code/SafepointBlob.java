/*
 * @(#)SafepointBlob.java	1.4 03/01/23 11:24:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** SafepointBlob: handles illegal_instruction exceptions during a safepoint */

public class SafepointBlob extends SingletonBlob {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("SafepointBlob");

    // FIXME: add any needed fields
  }

  public SafepointBlob(Address addr) {
    super(addr);
  }
  
  public boolean isSafepointStub() {
    return true;
  }
}
