/*
 * @(#)ObjArrayKlass.java	1.5 03/01/23 11:43:32
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

// ObjArrayKlass is the klass for ObjArrays

public class ObjArrayKlass extends ArrayKlass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type = db.lookupType("objArrayKlass");
    elementKlass = new OopField(type.getOopField("_element_klass"), Oop.getHeaderSize());
    bottomKlass  = new OopField(type.getOopField("_bottom_klass"), Oop.getHeaderSize());
  }

  ObjArrayKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static OopField elementKlass;
  private static OopField bottomKlass;

  public Klass getElementKlass() { return (Klass) elementKlass.getValue(this); }
  public Klass getBottomKlass()  { return (Klass) bottomKlass.getValue(this); }

  public void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(elementKlass, true);
      visitor.doOop(bottomKlass, true);
    }
  }

  public Klass arrayKlassImpl(boolean orNull, int n) {
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(getDimension() <= n, "check order of chain");
    }
    int dimension = (int) getDimension();
    if (dimension == n) {
      return this;
    }
    ObjArrayKlass ak = (ObjArrayKlass) getHigherDimension();
    if (ak == null) {
      if (orNull) return null;
      // FIXME: would need to change in reflective system to actually
      // allocate klass
      throw new RuntimeException("Can not allocate array klasses in debugging system");
    }
    
    if (orNull) {
      return ak.arrayKlassOrNull(n);
    }
    return ak.arrayKlass(n);
  }

  public Klass arrayKlassImpl(boolean orNull) {
    return arrayKlassImpl(orNull, (int) (getDimension() + 1));
  }

  public void printValueOn(PrintStream tty) {
    tty.print("ObjArrayKlass for ");
    getElementKlass().printValueOn(tty);
  }
};
