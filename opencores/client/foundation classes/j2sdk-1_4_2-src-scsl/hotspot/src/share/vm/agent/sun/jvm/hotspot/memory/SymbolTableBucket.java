/*
 * @(#)SymbolTableBucket.java	1.3 03/01/23 11:41:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.util.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.oops.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.utilities.*;

public class SymbolTableBucket extends VMObject {
  private static sun.jvm.hotspot.types.OopField entryField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("symbolTableBucket");
    
    entryField = type.getOopField("_entry");
  }

  public SymbolTableBucket(Address addr) {
    super(addr);
  }

  public Symbol entry() {
    return (Symbol) VM.getVM().getObjectHeap().newOop(entryField.getValue(addr));
  }

  public Symbol lookup(byte[] utf8Chars) {
    for (Symbol symbol = entry(); symbol != null; symbol = symbol.getNext()) {
      if (symbol.equals(utf8Chars)) {
        return symbol;
      }
    }
    return null;
  }
}
