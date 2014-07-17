/*
 * @(#)SystemDictionaryEntry.java	1.3 03/01/23 11:41:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class SystemDictionaryEntry extends VMObject {
  private static AddressField nextField;
  private static sun.jvm.hotspot.types.OopField klassField;
  private static sun.jvm.hotspot.types.OopField loaderField;
  private static AddressField pdSetField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("SystemDictionaryEntry");
    
    nextField   = type.getAddressField("_next");
    klassField  = type.getOopField("_klass");
    loaderField = type.getOopField("_loader");
    pdSetField  = type.getAddressField("_pd_set");
  }

  public SystemDictionaryEntry(Address addr) {
    super(addr);
  }

  public SystemDictionaryEntry next() {
    Address nextAddr = nextField.getValue(addr);
    if (nextAddr == null) return null;
    return (SystemDictionaryEntry) VMObjectFactory.newObject(SystemDictionaryEntry.class, nextAddr);
  }

  public Oop klass() {
    return VM.getVM().getObjectHeap().newOop(klassField.getValue(addr));
  }

  public Oop loader() {
    return VM.getVM().getObjectHeap().newOop(loaderField.getValue(addr));
  }

  public ProtectionDomainEntry protectionDomainSet() {
    return (ProtectionDomainEntry) VMObjectFactory.newObject(ProtectionDomainEntry.class, pdSetField.getValue(addr));
  }

  public boolean isValidProtectionDomain(Oop protectionDomain) {
    return (protectionDomain == null)
      ? true
      : containsProtectionDomain(protectionDomain);
  }

  public boolean containsProtectionDomain(Oop protectionDomain) {
    ObjectHeap heap = VM.getVM().getObjectHeap();

    if (heap.equal(((InstanceKlass) klass()).getProtectionDomain(), protectionDomain))return true;
    for (ProtectionDomainEntry current = protectionDomainSet(); current != null; current = current.next()) {
      if (heap.equal(current.protectionDomain(), protectionDomain)) return true;
    }
    return false;
  }
}
