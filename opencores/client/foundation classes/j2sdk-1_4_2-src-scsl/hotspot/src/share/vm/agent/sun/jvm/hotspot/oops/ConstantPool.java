/*
 * @(#)ConstantPool.java	1.9 03/01/23 11:42:11
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

public class ConstantPool extends Array {
  // Used for debugging this code
  private static final boolean DEBUG = false;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type   = db.lookupType("constantPoolOopDesc");
    tags        = new OopField(type.getOopField("_tags"), 0);
    cache       = new OopField(type.getOopField("_cache"), 0);
    poolHolder  = new OopField(type.getOopField("_pool_holder"), 0);
    headerSize  = type.getSize();
    elementSize = db.getOopSize();
  }

  ConstantPool(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isConstantPool()      { return true; }

  private static OopField tags;
  private static OopField cache;
  private static OopField poolHolder;

  private static long headerSize;
  private static long elementSize; 

  public TypeArray         getTags()       { return (TypeArray)         tags.getValue(this); }
  public ConstantPoolCache getCache()      { return (ConstantPoolCache) cache.getValue(this); }
  public Klass             getPoolHolder() { return (Klass)             poolHolder.getValue(this); }

  private long indexOffset(long index) {
    return (index * elementSize) + headerSize;
  }

  public ConstantTag getTagAt(long index) {
    return new ConstantTag(getTags().getByteAt((int) index));
  }

  public Oop getObjAt(long index){
    return getHeap().newOop(getHandle().getOopHandleAt(indexOffset(index)));
  }

  public Symbol getSymbolAt(long index) {
    return (Symbol) getObjAt(index);
  }

  public int getIntAt(long index){
    return getHandle().getJIntAt(indexOffset(index));
  }

  public float getFloatAt(long index){
    return getHandle().getJFloatAt(indexOffset(index));
  }

  public long getLongAt(long index) {
    int oneHalf = getHandle().getJIntAt(indexOffset(index + 1));
    int otherHalf   = getHandle().getJIntAt(indexOffset(index));
    // buildLongFromIntsPD accepts higher address value, lower address value
    // in that order.
    return VM.getVM().buildLongFromIntsPD(oneHalf, otherHalf);
  }

  public double getDoubleAt(long index) {
    return Double.longBitsToDouble(getLongAt(index));
  }

  public int getFieldOrMethodAt(int which) {
    if (DEBUG) {
      System.err.print("ConstantPool.getFieldOrMethodAt(" + which + "): new index = ");
    }
    int i = -1;
    ConstantPoolCache cache = getCache();
    if (cache == null) {
      i = which;
    } else {
      // change byte-ordering and go via cache
      i = cache.getEntryAt(VM.getVM().getBytes().swapShort((short) which)).getConstantPoolIndex();
    }
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getTagAt(i).isFieldOrMethod(), "Corrupted constant pool");
    }
    if (DEBUG) {
      System.err.println(i);
    }
    int res = getIntAt(i);
    if (DEBUG) {
      System.err.println("ConstantPool.getFieldOrMethodAt(" + i + "): result = " + res);
    }
    return res;
  }

  public int getNameAndTypeAt(int which) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getTagAt(which).isNameAndType(), "Corrupted constant pool");
    }
    int i = getIntAt(which);
    if (DEBUG) {
      System.err.println("ConstantPool.getNameAndTypeAt(" + which + "): result = " + i);
    }
    return i;
  }

  public Symbol getNameRefAt(int which) {
    int refIndex = getNameAndTypeAt(getNameAndTypeRefIndexAt(which));
    int nameIndex = extractLowShortFromInt(refIndex);
    return getSymbolAt(nameIndex);
  }

  public Symbol getSignatureRefAt(int which) {
    int refIndex = getNameAndTypeAt(getNameAndTypeRefIndexAt(which));
    int sigIndex = extractHighShortFromInt(refIndex);
    return getSymbolAt(sigIndex);
  }

  // returns null, if not resolved.
  public Klass getKlassRefAt(int which) {
    if( ! getTagAt(which).isKlass()) return null;
    return (Klass) getObjAt(which);
  }

  // returns null, if not resolved.
  public InstanceKlass getFieldOrMethodKlassRefAt(int which) {
    int refIndex = getFieldOrMethodAt(which);
    int klassIndex = extractLowShortFromInt(refIndex);
    return (InstanceKlass) getKlassRefAt(klassIndex);
  }

  // returns null, if not resolved.
  public Method getMethodRefAt(int which) {
    InstanceKlass klass = getFieldOrMethodKlassRefAt(which);
    if (klass == null) return null;
    Symbol name = getNameRefAt(which);
    Symbol sig  = getSignatureRefAt(which);
    return klass.findMethod(name, sig);
  }

  // returns null, if not resolved.
  public Field getFieldRefAt(int which) {
    InstanceKlass klass = getFieldOrMethodKlassRefAt(which);
    if (klass == null) return null;
    Symbol name = getNameRefAt(which);
    Symbol sig  = getSignatureRefAt(which);
    return klass.findField(name, sig);
  }

  public int getNameAndTypeRefIndexAt(int index) {
    int refIndex = getFieldOrMethodAt(index);
    if (DEBUG) {
      System.err.println("ConstantPool.getNameAndTypeRefIndexAt(" + index + "): refIndex = " + refIndex);
    }
    int i = extractHighShortFromInt(refIndex);
    if (DEBUG) {
      System.err.println("ConstantPool.getNameAndTypeRefIndexAt(" + index + "): result = " + i);
    }
    return i;
  }

  /** Lookup for entries consisting of (name_index, signature_index) */
  public int getNameRefIndexAt(int index) {
    int refIndex = getNameAndTypeAt(index);
    if (DEBUG) {
      System.err.println("ConstantPool.getNameRefIndexAt(" + index + "): refIndex = " + refIndex);
    }
    int i = extractLowShortFromInt(refIndex);
    if (DEBUG) {
      System.err.println("ConstantPool.getNameRefIndexAt(" + index + "): result = " + i);
    }
    return i;
  }

  /** Lookup for entries consisting of (name_index, signature_index) */
  public int getSignatureRefIndexAt(int index) {
    int refIndex = getNameAndTypeAt(index);
    if (DEBUG) {
      System.err.println("ConstantPool.getSignatureRefIndexAt(" + index + "): refIndex = " + refIndex);
    }
    int i = extractHighShortFromInt(refIndex);
    if (DEBUG) {
      System.err.println("ConstantPool.getSignatureRefIndexAt(" + index + "): result = " + i);
    }
    return i;
  }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(tags, true);
      visitor.doOop(cache, true);
      visitor.doOop(poolHolder, true);
    }
    /*
    int length = getLength();
    for (int index = 0; index < length; index++) {
      long offset = baseOffset + (index + typeDataBase.getOopSize());
      visitor.doOop(new IndexableField(index, offset, false), getObjAt(index));
    }
    */
  }

  public void printValueOn(PrintStream tty) {
    tty.print("ConstantPool for " + getPoolHolder().getName().asString());
  }

  public long getObjectSize() {
    return alignObjectSize(headerSize + (getLength() * elementSize)); 
  }

  //----------------------------------------------------------------------
  // Internals only below this point
  //

  private static int extractHighShortFromInt(int val) {
    return (val >> 16) & 0xFFFF;
  }

  private static int extractLowShortFromInt(int val) {
    return val & 0xFFFF;
  }
}
