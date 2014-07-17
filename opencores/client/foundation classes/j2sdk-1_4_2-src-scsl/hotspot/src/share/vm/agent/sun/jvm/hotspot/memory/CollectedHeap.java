/*
 * @(#)CollectedHeap.java	1.5 03/01/23 11:40:27
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

public abstract class CollectedHeap extends VMObject {
  private static long         reservedFieldOffset;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("CollectedHeap");
    
    reservedFieldOffset = type.getField("_reserved").getOffset();
  }

  public CollectedHeap(Address addr) {
    super(addr);
  }

  /** Returns the lowest address of the heap. */
  public Address start() {
    return reservedRegion().start();
  }

  public abstract long capacity();
  public abstract long used();

  public MemRegion reservedRegion() {
    return new MemRegion(addr.addOffsetTo(reservedFieldOffset));
  }

  public abstract boolean isIn(Address a);

  public boolean isInReserved(Address a) {
    return reservedRegion().contains(a);
  }

  public void print() { printOn(System.out); }
  public abstract void printOn(PrintStream tty);
}
