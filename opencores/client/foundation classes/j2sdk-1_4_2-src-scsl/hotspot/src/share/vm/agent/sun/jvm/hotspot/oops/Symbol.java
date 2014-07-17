/*
 * @(#)Symbol.java	1.6 03/01/23 11:44:03
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

// A Symbol is a canonicalized string.
// All Symbols reside in global symbolTable.

public class Symbol extends Oop {
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) throws WrongTypeException {
    Type type  = db.lookupType("symbolOopDesc");
    next       = new OopField(type.getOopField("_next"), 0);
    length     = new CIntField(type.getCIntegerField("_length"), 0);
    baseOffset = type.getField("_body").getOffset();
  }

  // Format:
  //   [header]
  //   [klass ]
  //   [next  ] next symbol in the symbolTable
  //   [length] byte size of uft8 string
  //   ..body..

  Symbol(OopHandle handle, ObjectHeap heap) {
    super(handle, heap);
  }

  public boolean isSymbol()            { return true; }

  private static long baseOffset; // tells where the array part starts

  // Fields
  private static OopField  next;
  private static CIntField length;
  
  // Accessors for declared fields
  public Symbol getNext()   { return (Symbol) next.getValue(this); }
  public long   getLength() { return          length.getValue(this); }

  public byte getByteAt(long index) {
    return getHandle().getJByteAt(baseOffset + index);
  }

  public boolean equals(byte[] utf8Chars) {
    int l = (int) getLength();
    if (l != utf8Chars.length) return false;
    while (l-- > 0) {
      if (utf8Chars[l] != getByteAt(l)) return false;
    }
    if (Assert.ASSERTS_ENABLED) {
      Assert.that(l == -1, "we should be at the beginning");
    }
    return true;
  }

  public byte[] asByteArray() {
    int length = (int) getLength();
    byte [] result = new byte [length];
    for (int index = 0; index < length; index++) {
      result[index] = getByteAt(index);
    }
    return result;
  }

  public String asString() {
    // Decode the byte array and return the string.
    try {
      return new String(asByteArray(), "UTF-8");
    } catch(Exception e) {
      return null;
    } 
  }

  public boolean startsWith(String str) {
    int l = str.length();
    if (l > getLength()) {
      return false;
    }
    for (int i = 0; i < l; i++) {
      if ((getByteAt(i) & 0xFF) != str.charAt(i)) {
        return false;
      }
    }
    return true;
  }

  public void printValueOn(PrintStream tty) {
    tty.print("#" + asString());
  }

  public long getObjectSize() {
    return alignObjectSize(baseOffset + getLength());
  }

  void iterateFields(OopVisitor visitor, boolean doVMFields) {
    super.iterateFields(visitor, doVMFields);
    if (doVMFields) {
      visitor.doOop(next, true);
      visitor.doCInt(length, true);
      int length = (int) getLength();
      for (int index = 0; index < length; index++) {
        visitor.doByte(new ByteField(new IndexableFieldIdentifier(index), baseOffset + index, false), true);
      }
    }
  }

  /** Note: this comparison is used for vtable sorting only; it
      doesn't matter what order it defines, as long as it is a total,
      time-invariant order Since symbolOops are in permSpace, their
      relative order in memory never changes, so use address
      comparison for speed. */
  public int fastCompare(Symbol other) {
    return (int) getHandle().minus(other.getHandle());
  }
}
