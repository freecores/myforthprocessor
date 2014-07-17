/*
 * @(#)JNIid.java	1.4 03/01/23 11:45:02
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;

public class JNIid extends VMObject {
  private static sun.jvm.hotspot.types.OopField holder;
  private static AddressField next;
  private static CIntegerField offset;
  private static sun.jvm.hotspot.types.OopField resolvedMethod;
  private static sun.jvm.hotspot.types.OopField resolvedReceiver;

  private ObjectHeap heap;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("JNIid");
    
    holder = type.getOopField("_holder");
    next = type.getAddressField("_next");
    offset = type.getCIntegerField("_offset");
    resolvedMethod = type.getOopField("_resolved_method");
    resolvedReceiver = type.getOopField("_resolved_receiver");
  }

  public JNIid(Address addr, ObjectHeap heap) {
    super(addr);
    this.heap = heap;
  }

  public JNIid next() {
    Address nextAddr = next.getValue(addr);
    if (nextAddr == null) {
      return null;
    }
    return new JNIid(nextAddr, heap);
  }

  public Klass     holder()           { return (Klass) heap.newOop(holder.getValue(addr)); }
  public int       offset()           { return (int) offset.getValue(addr); }
  public Method    method() {
    return (Method) ((InstanceKlass) holder()).getMethods().getObjAt(offset());
  }
  public Method    resolvedMethod()   { return (Method) heap.newOop(resolvedMethod.getValue(addr)); }
  public Klass     resolvedReceiver() { return (Klass) heap.newOop(resolvedReceiver.getValue(addr)); }
}
