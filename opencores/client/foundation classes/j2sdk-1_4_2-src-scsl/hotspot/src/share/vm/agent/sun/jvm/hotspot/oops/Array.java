/*
 * @(#)Array.java	1.7 03/01/23 11:41:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

// Array is an abstract superclass for TypeArray and ObjArray

public class Array extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  Array(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static void initialize(TypeDataBase db) throws WrongTypeException {
    Type type   = db.lookupType("arrayOopDesc");
    length      = new CIntField(type.getCIntegerField("_length"), 0);
    headerSize  = type.getSize();
  }

  // Size of the arrayOopDesc
  private static long headerSize;

  // Fields
  private static CIntField length;

  // Accessors for declared fields
  public long getLength() { return length.getValue(this); }

  public long getObjectSize() {
    ArrayKlass klass = (ArrayKlass) getKlass();
    // We have to fetch the length of the array, shift (multiply) it
    // appropriately, up to wordSize, add the header, and align to
    // object size.
    long s = getLength() << (-1 - klass.getSizeHelper());
    s += klass.getArrayHeaderInBytes();

    // This code could be simplified, but by keeping array_header_in_bytes
    // in units of bytes and doing it this way we can round up just once,
    // skipping the intermediate round to wordSize.
    long wordSize = VM.getVM().getAddressSize();
    if (VM.getVM().getAlignAllObjects()) {
      s = ((s + (wordSize*2-1)) & ~(wordSize*2-1));
    } else {
      s = (s + (wordSize-1));
    }
    return s;
  }

  public boolean isArray()             { return true; }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doCInt(length, true);
    }
  }
}
