/*
 * @(#)ObjArray.java	1.6 03/01/23 11:43:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

// An ObjArray is an array containing oops

public class ObjArray extends Array {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type   = db.lookupType("objArrayOopDesc");
    elementSize = db.getOopSize();
    baseOffset  = type.getSize();
  }

  ObjArray(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isObjArray()          { return true; }

  // Tells the offset to the first array element
  private static long baseOffset;
  private static long elementSize;

  public Oop getObjAt(long index) {
    long offset = baseOffset + (index * elementSize); 
    return getHeap().newOop(getHandle().getOopHandleAt(offset));
  }

  public void printValueOn(PrintStream tty) {
    tty.print("ObjArray");
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    int length = (int) getLength();
    for (int index = 0; index < length; index++) {
      long offset = baseOffset + (index * elementSize);
      visitor.doOop(new OopField(new IndexableFieldIdentifier(index), offset, false), false);
    }
  }
}
