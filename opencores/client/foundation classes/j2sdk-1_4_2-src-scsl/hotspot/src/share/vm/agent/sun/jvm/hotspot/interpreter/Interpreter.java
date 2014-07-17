/*
 * @(#)Interpreter.java	1.5 03/01/23 11:37:34
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.interpreter;

import java.util.*;
import sun.jvm.hotspot.code.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class Interpreter {
  private static AddressField codeField;
  
  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("AbstractInterpreter");
    
    codeField     = type.getAddressField("_code");
  }

  public Interpreter() {
  }

  public StubQueue getCode() {
    Address code = codeField.getValue();
    if (code == null) return null;
    return new StubQueue(code, InterpreterCodelet.class);
  }

  public boolean contains(Address pc) {
    return getCode().contains(pc);
  }

  /** Debugging/printing */
  public InterpreterCodelet getCodeletContaining(Address pc) {
    return (InterpreterCodelet) getCode().getStubContaining(pc);
  }
}
