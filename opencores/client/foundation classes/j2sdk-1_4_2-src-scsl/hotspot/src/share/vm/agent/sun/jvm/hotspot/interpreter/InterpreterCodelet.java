/*
 * @(#)InterpreterCodelet.java	1.3 03/01/23 11:37:37
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;
import sun.jvm.hotspot.utilities.*;

/** An InterpreterCodelet is a piece of interpreter code. All
    interpreter code is generated into little codelets which contain
    extra information for safepoint, debugging and printing purposes. */

public class InterpreterCodelet extends Stub {
  private static long          instanceSize;
  private static CIntegerField sizeField;             // the size in bytes
  private static AddressField  descriptionField;      // a description of the codelet, for debugging & printing
  private static JIntField     isSafepointSafeField;  // true if thread cannot escape interpreter during a safepoint, false otherwise
  private static CIntegerField bytecodeField;         // associated bytecode if any

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("InterpreterCodelet");
    
    sizeField            = type.getCIntegerField("_size");
    descriptionField     = type.getAddressField("_description");
    isSafepointSafeField = type.getJIntField("_is_safepoint_safe");
    bytecodeField        = type.getCIntegerField("_bytecode");
    
    instanceSize = type.getSize();
  }

  public InterpreterCodelet(Address addr) {
    super(addr);
  }

  public long getSize() {
    return sizeField.getValue(addr);
  }

  public Address codeBegin() {
    return addr.addOffsetTo(instanceSize);
  }

  public Address codeEnd() {
    return addr.addOffsetTo(getSize());
  }

  public long codeSize() {
    return codeEnd().minus(codeBegin());
  }

  public String getDescription() {
    return CStringUtilities.getString(descriptionField.getValue(addr));
  }

  public boolean isSafepointSafe() {
    return (isSafepointSafeField.getValue(addr) != 0);
  }

  public void verify() {
    // FIXME: add verification of bytecode/safepoint_safe state
  }
  
  public void printOn(PrintStream tty) {
    String desc = getDescription();
    if (desc != null) {
      tty.print(desc);
    }
    // FIXME: add printing of bytecode
    tty.println(" [" + codeBegin() + ", " + codeEnd() + ")  " +
                codeSize() + " bytes  " + (isSafepointSafe() ? "" : "not ") +
                "safepoint safe");
    // FIXME: add disassembly
  }
}
