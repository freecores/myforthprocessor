/*
 * @(#)C2IAdapter.java	1.4 03/01/23 11:23:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class C2IAdapter extends CodeBlob {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("C2IAdapter");

    // FIXME: add any needed fields
  }

  public C2IAdapter(Address addr) {
    super(addr);
  }
  
  public boolean isC2IAdapter() {
    return true;
  }
}
