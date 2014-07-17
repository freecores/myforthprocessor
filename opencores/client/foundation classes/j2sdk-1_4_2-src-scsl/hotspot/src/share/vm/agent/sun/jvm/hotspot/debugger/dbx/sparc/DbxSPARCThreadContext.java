/*
 * @(#)DbxSPARCThreadContext.java	1.4 03/01/23 11:29:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.dbx.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.sparc.*;
import sun.jvm.hotspot.debugger.dbx.*;

public class DbxSPARCThreadContext extends SPARCThreadContext {
  private DbxDebugger debugger;

  public DbxSPARCThreadContext(DbxDebugger debugger) {
    super();
    this.debugger = debugger;
  }

  public void setRegisterAsAddress(int index, Address value) {
    setRegister(index, debugger.getAddressValue(value));
  }

  public Address getRegisterAsAddress(int index) {
    return debugger.newAddress(getRegister(index));
  }
}
