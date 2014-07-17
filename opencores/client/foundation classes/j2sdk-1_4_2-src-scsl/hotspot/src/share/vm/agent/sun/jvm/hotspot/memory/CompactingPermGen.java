/*
 * @(#)CompactingPermGen.java	1.3 03/01/23 11:40:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** A PermGen implemented with a contiguous space. */

public class CompactingPermGen extends PermGen {
  // The "generation" view.
  private static AddressField genField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("CompactingPermGen");
    
    genField = type.getAddressField("_gen");
  }

  public CompactingPermGen(Address addr) {
    super(addr);
  }
  
  public Generation asGen() {
    return GenerationFactory.newObject(genField.getValue(addr));
  }
}
