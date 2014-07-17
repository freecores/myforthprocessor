/*
 * @(#)Win32ThreadContext.java	1.3 03/01/23 11:32:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;

class Win32ThreadContext extends X86ThreadContext {
  private Win32Debugger debugger;

  public Win32ThreadContext(Win32Debugger debugger) {
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
