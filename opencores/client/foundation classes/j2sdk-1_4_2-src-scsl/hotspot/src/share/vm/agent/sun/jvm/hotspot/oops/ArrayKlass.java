/*
 * @(#)ArrayKlass.java	1.7 03/01/23 11:41:43
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

// ArrayKlass is the abstract class for all array classes

public class ArrayKlass extends Klass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type          = db.lookupType("arrayKlass");
    dimension          = new CIntField(type.getCIntegerField("_dimension"), Oop.getHeaderSize());
    higherDimension    = new OopField(type.getOopField("_higher_dimension"), Oop.getHeaderSize());
    lowerDimension     = new OopField(type.getOopField("_lower_dimension"), Oop.getHeaderSize());
    vtableLen          = new CIntField(type.getCIntegerField("_vtable_len"), Oop.getHeaderSize());
    allocSize          = new CIntField(type.getCIntegerField("_alloc_size"), Oop.getHeaderSize());
    arrayHeaderInBytes = new CIntField(type.getCIntegerField("_array_header_in_bytes"), Oop.getHeaderSize());
  }

  ArrayKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static CIntField dimension;
  private static OopField  higherDimension;
  private static OopField  lowerDimension;
  private static CIntField vtableLen;
  private static CIntField allocSize;
  private static CIntField arrayHeaderInBytes;

  public long  getDimension()       { return         dimension.getValue(this); }
  public Klass getHigherDimension() { return (Klass) higherDimension.getValue(this); }
  public Klass getLowerDimension()  { return (Klass) lowerDimension.getValue(this); }
  public long  getVtableLen()       { return         vtableLen.getValue(this); }
  public long  getAllocSize()       { return         allocSize.getValue(this); }
  public long  getArrayHeaderInBytes() { return arrayHeaderInBytes.getValue(this); }

  public void printValueOn(PrintStream tty) {
    tty.print("ArrayKlass");
  }

  public long getObjectSize() {
    return alignObjectSize(InstanceKlass.getHeaderSize() + getVtableLen() * getHeap().getOopSize());
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doCInt(dimension, true);
      visitor.doOop(higherDimension, true);
      visitor.doOop(lowerDimension, true);
      visitor.doCInt(vtableLen, true);
      visitor.doCInt(allocSize, true);
      visitor.doCInt(arrayHeaderInBytes, true);
    }
  }

  public String signature() { //fixme jjh:  will this be the component type?
      return "[L" + super.signature() + ";" ;
  }
  
}
