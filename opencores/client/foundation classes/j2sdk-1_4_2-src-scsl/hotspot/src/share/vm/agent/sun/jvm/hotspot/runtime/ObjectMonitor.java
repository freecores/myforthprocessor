/*
 * @(#)ObjectMonitor.java	1.4 03/01/23 11:45:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;

public class ObjectMonitor extends VMObject {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    heap = VM.getVM().getObjectHeap();
    Type type  = db.lookupType("ObjectMonitor");
    sun.jvm.hotspot.types.Field f = type.getField("_header");
    headerFieldOffset = f.getOffset();
    f = type.getField("_object");
    objectFieldOffset = f.getOffset();
    f = type.getField("_owner");
    ownerFieldOffset = f.getOffset();
    f = type.getField("_queue");
    queueFieldOffset = f.getOffset();
    countField  = type.getCIntegerField("_count");
    waitersField = type.getCIntegerField("_waiters");
    recursionsField = type.getCIntegerField("_recursions");
  }

  public ObjectMonitor(Address addr) {
    super(addr);
  }

  public Mark header() {
    return new Mark(addr.addOffsetTo(headerFieldOffset));
  }

  // FIXME
  //  void      set_header(markOop hdr);

  // FIXME: must implement and delegate to platform-dependent implementation
  //  public boolean isBusy();
  public boolean isEntered(sun.jvm.hotspot.runtime.Thread current) {
    Address o = owner();
    if (current.threadObjectAddress().equals(o) ||
        current.isLockOwned(o)) {
      return true;
    }
    return false;
  }

  public Address owner() { return addr.getAddressAt(ownerFieldOffset); }
  // FIXME
  //  void      set_owner(void* owner);

  public long    waiters() { return waitersField.getValue(addr); }
  // FIXME: need to expose a class for this to be useful
  public Address queue()   { return addr.getAddressAt(queueFieldOffset); }
  // FIXME
  //  void      set_queue(void* owner);

  public long count() { return countField.getValue(addr); }
  // FIXME
  //  void      set_count(intptr_t count);

  public long recursions() { return recursionsField.getValue(addr); }

  public OopHandle object() {
    return addr.getOopHandleAt(objectFieldOffset);
  }
  // FIXME
  //  void*     object_addr();
  //  void      set_object(void* obj);

  // The following four either aren't expressed as typed fields in
  // vmStructs.cpp because they aren't strongly typed in the VM, or
  // would confuse the SA's type system.
  private static ObjectHeap    heap;
  private static long          headerFieldOffset;
  private static long          objectFieldOffset;
  private static long          ownerFieldOffset;
  private static long          queueFieldOffset;
  private static CIntegerField countField;
  private static CIntegerField waitersField;
  private static CIntegerField recursionsField;
  // FIXME: expose platform-dependent stuff
}
