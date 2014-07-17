/*
 * @(#)ConstantPoolCache.java	1.6 03/01/23 11:42:13
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
import sun.jvm.hotspot.utilities.*;

// A ConstantPool is an array containing class constants
// as described in the class file

public class ConstantPoolCache extends Array {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type      = db.lookupType("constantPoolCacheOopDesc");
    constants      = new OopField(type.getOopField("_constant_pool"), 0);
    baseOffset     = type.getSize();

    Type elType    = db.lookupType("ConstantPoolCacheEntry");
    elementSize    = elType.getSize();
  }

  ConstantPoolCache(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isConstantPoolCache() { return true; }

  private static OopField constants;

  private static long baseOffset;
  private static long elementSize;

  public ConstantPool getConstants() { return (ConstantPool) constants.getValue(this); }

  public long getObjectSize() {
    return alignObjectSize(baseOffset + getLength() * elementSize); 
  }

  public ConstantPoolCacheEntry getEntryAt(int i) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(0 <= i && i < getLength(), "index out of bounds");    
    }
    return new ConstantPoolCacheEntry(getHandle(), i);
  }

  public int getIntAt(int entry, int fld) {
    //alignObjectSize ?
    long offset = baseOffset + /*alignObjectSize*/entry * elementSize + fld* getHeap().getIntSize(); 
    return (int) getHandle().getCIntegerAt(offset, getHeap().getIntSize(), true );
  }


  public void printValueOn(PrintStream tty) {
    tty.print("ConstantPoolCache for " + getConstants().getPoolHolder().getName().asString());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(constants, true);
    }
  }
};
