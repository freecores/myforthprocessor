/*
 * @(#)MethodDataKlass.java	1.4 03/01/23 11:43:20
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

// The MethodDataKlass is the klass of a MethodData oop

public class MethodDataKlass extends Klass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type  = db.lookupType("methodDataKlass");
    headerSize = type.getSize() + Oop.getHeaderSize();
  }

  MethodDataKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static long headerSize;

  public long getObjectSize() { return alignObjectSize(headerSize); }

  public void printValueOn(PrintStream tty) {
    tty.print("MethodDataKlass");
  }
}
