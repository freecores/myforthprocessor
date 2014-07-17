/*
 * @(#)InstanceKlassKlass.java	1.4 03/01/23 11:42:54
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

// An InstanceKlassKlass is the klass of an InstanceKlass.
// There only exist one instance Universe::instanceKlassKlassObj()

public class InstanceKlassKlass extends KlassKlass {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }
  
  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type  = db.lookupType("instanceKlassKlass");
    headerSize = type.getSize() + Oop.getHeaderSize();
  }

  InstanceKlassKlass(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  private static long headerSize;

  public long getObjectSize() { return alignObjectSize(headerSize); }

  public void printValueOn(PrintStream tty) {
    tty.print("InstanceKlassKlass");
  }
}
