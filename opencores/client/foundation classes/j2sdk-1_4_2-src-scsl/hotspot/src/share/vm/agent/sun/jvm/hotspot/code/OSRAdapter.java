/*
 * @(#)OSRAdapter.java	1.6 03/01/23 11:24:17
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class OSRAdapter extends CodeBlob {

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("OSRAdapter");

    // FIXME: add any needed fields
  }

  public OSRAdapter(Address addr) {
    super(addr);
  }
  
  public boolean isOSRAdapter() {
    return true;
  }
}
