/*
 * @(#)ConstantPoolCacheEntry.java	1.3 03/01/23 11:42:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class ConstantPoolCacheEntry {
  private static long          size;
  private static long          baseOffset;
  private static CIntegerField indices;
  private static sun.jvm.hotspot.types.OopField f1;
  private static CIntegerField f2;
  private static CIntegerField flags;

  private OopHandle cp;
  private long      offset;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type      = db.lookupType("ConstantPoolCacheEntry");
    size = type.getSize();

    indices = type.getCIntegerField("_indices");
    f1      = type.getOopField     ("_f1");
    f2      = type.getCIntegerField("_f2");
    flags   = type.getCIntegerField("_flags");

    type = db.lookupType("constantPoolCacheOopDesc");
    baseOffset = type.getSize();
  }

  ConstantPoolCacheEntry(OopHandle cp, int index) {
    this.cp = cp;
    offset  = baseOffset + index * size;
  }

  public int getConstantPoolIndex() {
    return (int) (getIndices() & 0xFFFF);
  }

  private long getIndices() {
    return cp.getCIntegerAt(indices.getOffset() + offset, indices.getSize(), indices.isUnsigned());
  }
}
