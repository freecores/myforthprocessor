/*
 * @(#)GenerationFactory.java	1.3 03/01/23 11:40:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** Factory containing a VirtualConstructor suitable for instantiating
    wrapper objects for all types of generations */

public class GenerationFactory {
  private static VirtualConstructor ctor;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    ctor = new VirtualConstructor(db);

    ctor.addMapping("CompactingPermGenGen", CompactingPermGenGen.class);
    ctor.addMapping("DefNewGeneration", DefNewGeneration.class);
    ctor.addMapping("TenuredGeneration", TenuredGeneration.class);
  }

  public static Generation newObject(Address addr) {
    return (Generation) ctor.instantiateWrapperFor(addr);
  }
}
