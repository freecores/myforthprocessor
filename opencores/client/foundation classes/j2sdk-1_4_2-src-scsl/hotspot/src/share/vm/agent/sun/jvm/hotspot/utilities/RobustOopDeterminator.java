/*
 * @(#)RobustOopDeterminator.java	1.5 03/01/23 11:52:11
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.memory.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

/** This class determines to the best of its ability, and in a
    reasonably robust fashion, whether a given pointer is an intact
    oop or not. It does this by checking the integrity of the
    metaclass hierarchy. This is only intended for use in the
    debugging system. It may provide more resilience to unexpected VM
    states than the ObjectHeap code. */

public class RobustOopDeterminator {
  private static OopField klassField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static void initialize(TypeDataBase db) {
    Type type = db.lookupType("oopDesc");

    klassField = type.getOopField("_klass");
  }

  public static boolean oopLooksValid(OopHandle oop) {
    if (oop == null) {
      return false;
    }
    if (!VM.getVM().getUniverse().isIn(oop)) {
      return false;
    }
    try {
      for (int i = 0; i < 4; ++i) {
        OopHandle next = klassField.getValue(oop);
        if (next == null) {
          return false;
        }
        if (next.equals(oop)) {
          return true;
        }
        oop = next;
      }
      return false;
    }
    catch (AddressException e) {
      return false;
    }
  }
}
