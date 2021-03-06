/*
 * @(#)ObjectSynchronizer.java	1.5 03/01/23 11:45:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;

import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.utilities.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class ObjectSynchronizer {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type;
    try {
      type = db.lookupType("ObjectSynchronizer");
      AddressField blockListField;
      blockListField = type.getAddressField("gBlockList");
      gBlockListAddr = blockListField.getValue();
      blockSize = db.lookupIntConstant("ObjectSynchronizer::BLOCK_SIZE").intValue();
    } catch (RuntimeException e) { }
    type = db.lookupType("ObjectMonitor");
    objectMonitorTypeSize = type.getSize();
  }

  public long identityHashValueFor(Oop obj) {
    Mark mark = obj.getMark();
    if (mark.isUnlocked()) {
      // FIXME: can not generate marks in debugging system
      return mark.hash();
    } else if (mark.hasMonitor()) {
      ObjectMonitor monitor = mark.monitor();
      Mark temp = monitor.header();
      return temp.hash();
    } else {
      if (Assert.ASSERTS_ENABLED) {
        Assert.that(VM.getVM().isDebugging(), "Can not access displaced header otherwise");
      }
      if (mark.hasDisplacedMarkHelper()) {
        Mark temp = mark.displacedMarkHelper();
        return temp.hash();
      }
      // FIXME: can not do anything else here in debugging system
      return 0;
    }
  }

  public static Iterator objectMonitorIterator() {
    if (gBlockListAddr != null) {
      return new ObjectMonitorIterator();
    } else {
      return null;
    }
  }

  private static class ObjectMonitorIterator implements Iterator {

    // JVMDI/JVMPI raw monitors are not pointed by gBlockList
    // and are not included by this Iterator. May add them later.

    ObjectMonitorIterator() {
      blockAddr = gBlockListAddr;
      index = blockSize - 1;
      block = new ObjectMonitor(blockAddr);
    }

    public boolean hasNext() {
      return (index > 0 || block.queue() != null);
    }

    public Object next() {
      Address addr;
      if (index > 0) {
        addr = blockAddr.addOffsetTo(index*objectMonitorTypeSize);
      } else {
        blockAddr = block.queue();
        index = blockSize - 1;
        addr = blockAddr.addOffsetTo(index*objectMonitorTypeSize);
      }
      index --;
      return new ObjectMonitor(addr);
    }

    public void remove() {
      throw new UnsupportedOperationException();
    }

    private ObjectMonitor block;
    private int index;
    private Address blockAddr;
  }

  private static Address gBlockListAddr;
  private static int blockSize;
  private static long objectMonitorTypeSize;

}
