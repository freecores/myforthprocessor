/*
 * @(#)Oop.java	1.12 03/01/23 11:43:45
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

// Oop represents the superclass for all types of
// objects in the HotSpot object heap.

public class Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type  = db.lookupType("oopDesc");
    mark       = new CIntField(type.getCIntegerField("_mark"), 0);
    klass      = new OopField(type.getOopField("_klass"), 0);
    headerSize = type.getSize();
  }

  private OopHandle  handle;
  private ObjectHeap heap;

  public Oop(OopHandle handle, ObjectHeap heap) {
    this.handle = handle;
    this.heap   = heap;
  }

  ObjectHeap getHeap()   { return heap; }

  /** Should not be used or needed by most clients outside this
      package; is needed, however, by {@link
      sun.jvm.hotspot.utilities.MarkBits}. */
  public OopHandle getHandle() { return handle; }

  private static long headerSize;
  public  static long getHeaderSize() { return headerSize; }

  private static CIntField mark;
  private static OopField  klass;

  // Accessors for declared fields
  public Mark  getMark()   { return new Mark(getHandle()); }
  public Klass getKlass()  { return (Klass) klass.getValue(this); }

  // Returns the byte size of this object
  public long getObjectSize() {
    return getKlass().getSizeHelper() * getHeap().getOopSize();
  }

  // Type test operations
  public boolean isInstance()          { return false; }
  public boolean isInstanceRef()       { return false; }
  public boolean isArray()             { return false; }
  public boolean isObjArray()          { return false; }
  public boolean isTypeArray()         { return false; }
  public boolean isSymbol()            { return false; }
  public boolean isKlass()             { return false; }
  public boolean isThread()            { return false; }
  public boolean isMethod()            { return false; }
  public boolean isMethodData()        { return false; }
  public boolean isConstantPool()      { return false; }
  public boolean isConstantPoolCache() { return false; }
  public boolean isCompiledICHolder()  { return false; }

  // Aligns the object size 
  static long alignObjectSize(long size) {
    if (VM.getVM().getAlignAllObjects()) {
      return VM.getVM().alignUp(size, 8);
    } else {
      return size;
    }
  }

  // If the target machine aligns longs, then pad out certain offsets too.
  static long alignObjectOffset(long offset) {
    if ((VM.getVM().getAlignDoubleFields() || VM.getVM().getAlignLongFields())
        && VM.getVM().getAlignAllObjects()) {
      // FIXME: getAddressSize() is probably not guaranteed to be
      // equivalent to sizeof(long); see globalDefinitions.hpp
      return VM.getVM().alignUp(offset, 8);
    } else {
      return offset;
    }
  }

  public boolean equals(Object obj) {
    if (obj != null && (obj instanceof Oop)) {
      return getHandle().equals(((Oop) obj).getHandle());
    }
    return false;
 }

  public int hashCode() { return getHandle().hashCode(); }

  /** Identity hash in the target VM */
  public long identityHash() {
    Mark mark = getMark();
    return (mark.isUnlocked() && (!mark.hasNoHash())) ? (int) mark.hash() : slowIdentityHash();
  }

  public long slowIdentityHash() {
    return VM.getVM().getObjectSynchronizer().identityHashValueFor(this);
  }

  public void iterate(OopVisitor visitor, boolean doVMFields) {
    visitor.setObj(this);
    visitor.prologue();
    iterateFields(visitor, doVMFields);
    visitor.epilogue();
  }
 
  void iterateFields(OopVisitor visitor, boolean doVMFields) {
    if (doVMFields) {
      visitor.doCInt(mark, true);
      visitor.doOop(klass, true);
    }
  }

  public void print()      { printOn(System.out); }
  public void printValue() { printValueOn(System.out); }
  public void printRaw()   { printRawOn(System.out); }

  public static void printOopValueOn(Oop obj, PrintStream tty) {
    if (obj == null) {
      tty.print("null");
    } else {
      obj.printValueOn(tty);
      tty.print("@" + obj.getHandle());
    }
  }

  public static void printOopAddressOn(Oop obj, PrintStream tty) {
    if (obj == null) {
      tty.print("null");
    } else {
      tty.print(obj.getHandle().toString());
    }
  }

  public void printOn(PrintStream tty) {
    OopPrinter printer = new OopPrinter(tty);
    iterate(printer, true);
  }

  public void printValueOn(PrintStream tty) {
    try {
      tty.print("Oop for " + getKlass().getName().asString() + " @" + getHandle());
    } catch (java.lang.NullPointerException e) {
      tty.print("Oop");
    }
  }

  public void printRawOn(PrintStream tty) {
    tty.print("Dumping raw memory for ");
    printValueOn(tty);
    tty.println();
    long size = getObjectSize() * 4;
    for (long i = 0; i < size; i += 4) {
      long memVal = getHandle().getCIntegerAt(i, 4, true);
      tty.println(Long.toHexString(memVal));
    }
  }

  public boolean verify() { return true;}

  // Package-private routine to speed up ObjectHeap.newOop
  static OopHandle getKlassForOopHandle(OopHandle handle) {
    if (handle == null) {
      return null;
    }
    return handle.getOopHandleAt(klass.getOffset());
  }
};
