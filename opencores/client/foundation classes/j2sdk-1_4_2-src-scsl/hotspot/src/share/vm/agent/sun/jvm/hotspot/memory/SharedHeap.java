/*
 * @(#)SharedHeap.java	1.2 03/01/23 11:41:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public abstract class SharedHeap extends CollectedHeap {
  private static AddressField permGenField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("SharedHeap");
    
    permGenField        = type.getAddressField("_perm_gen");
  }

  public SharedHeap(Address addr) {
    super(addr);
  }

  /** These functions return the "permanent" generation, in which
      reflective objects are allocated and stored.  Two versions, the
      second of which returns the view of the perm gen as a
      generation. (FIXME: this distinction is strange and seems
      unnecessary, and should be cleaned up.) */
  public PermGen perm() {
    return (PermGen) VMObjectFactory.newObject(CompactingPermGen.class, permGenField.getValue(addr));
  }

  public Generation permGen() {
    return perm().asGen();
  }
}
