/*
 * @(#)Universe.java	1.10 03/01/23 11:41:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;

public class Universe {
  private static AddressField collectedHeapField;
  private static VirtualConstructor heapConstructor;
  private static sun.jvm.hotspot.types.OopField mainThreadGroupField;
  private static sun.jvm.hotspot.types.OopField systemThreadGroupField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("Universe");
    
    collectedHeapField = type.getAddressField("_collectedHeap");

    heapConstructor = new VirtualConstructor(db);
    heapConstructor.addMapping("GenCollectedHeap", GenCollectedHeap.class);
    
    mainThreadGroupField   = type.getOopField("_main_thread_group");
    systemThreadGroupField = type.getOopField("_system_thread_group");
  }

  public Universe() {
  }

  public CollectedHeap heap() {
    return (CollectedHeap) heapConstructor.instantiateWrapperFor(collectedHeapField.getValue());
  }

  /** Returns "TRUE" iff "p" points into the allocated area of the heap. */
  public boolean isIn(Address p) {
    return heap().isIn(p);
  }

  /** Returns "TRUE" iff "p" points into the reserved area of the heap. */
  public boolean isInReserved(Address p) {
    return heap().isInReserved(p);
  }

  public Oop mainThreadGroup() {
    return VM.getVM().getObjectHeap().newOop(mainThreadGroupField.getValue());
  }

  public Oop systemThreadGroup() {
    return VM.getVM().getObjectHeap().newOop(systemThreadGroupField.getValue());
  }

  public void print() { printOn(System.out); }
  public void printOn(PrintStream tty) {
    heap().printOn(tty);
  }
}
