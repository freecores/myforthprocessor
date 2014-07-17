/*
 * @(#)SymbolTable.java	1.4 03/01/23 11:41:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

public class SymbolTable {
  private static int symbolTableSize;
  private static long symbolTableBucketSize;
  private static AddressField bucketsField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("SymbolTable");
    
    bucketsField = type.getAddressField("_buckets");
    symbolTableSize = db.lookupIntConstant("SymbolTable::symbol_table_size").intValue();
    symbolTableBucketSize = db.lookupType("symbolTableBucket").getSize();
  }

  /** Clone of VM's "temporary" probe routine, as the SA currently
      does not support mutation so lookup() would have no effect
      anyway. Returns null if the given string is not in the symbol
      table. */
  public Symbol probe(String name) {
    try {
      return probe(name.getBytes("UTF-8"));
    } catch (UnsupportedEncodingException e) {
      return null;
    }
  }

  /** Clone of VM's "temporary" probe routine, as the SA currently
      does not support mutation so lookup() would have no effect
      anyway. Returns null if the given string is not in the symbol
      table. */
  public Symbol probe(byte[] name) {
    int hashValue = hash(name);
    SymbolTableBucket bucket = bucketFor(hashValue);
    return bucket.lookup(name);
  }

  //----------------------------------------------------------------------
  // Internals only below this point
  //

  private SymbolTableBucket bucketFor(int i) {
    Address base = bucketsField.getValue();
    Address addr = base.addOffsetTo(i * symbolTableBucketSize);
    return (SymbolTableBucket) VMObjectFactory.newObject(SymbolTableBucket.class, addr);
  }

  private static int hash(byte[] s) {
    int h = 0;
    for (int i = 0; i < s.length; i++) {
      h = 31*h + (s[i] & 0xFF);
    }
    // FIXME: conversion to 64 bits is the only way to emulate this effect in Java!
    long l = h & 0xFFFFFFFFL;
    l = l % (long) symbolTableSize;
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(l>=0 && l < symbolTableSize, "Illegal hash value " + l);
    }
    return (int) l;
  }
}
