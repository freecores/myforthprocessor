/*
 * @(#)ObjArrayKlassKlass.java	1.4 03/01/23 11:43:35
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

// An ObjArrayKlassKlass is klass for ObjArrayKlass'
// We only have one

public class ObjArrayKlassKlass extends ArrayKlassKlass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type = db.lookupType("objArrayKlassKlass");
    headerSize         = type.getSize() + Oop.getHeaderSize();
  }

  ObjArrayKlassKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static long headerSize;

  public long getObjectSize() { return alignObjectSize(headerSize); }

  public void printValueOn(PrintStream tty) {
    tty.print("ObjArrayKlassKlass");
  }
}
