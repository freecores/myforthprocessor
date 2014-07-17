/*
 * "@(#)LinuxThreadContext.java	1.2 03/01/23 11:30:03"
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.linux;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;

class LinuxThreadContext extends X86ThreadContext {
  private LinuxDebugger debugger;

  public LinuxThreadContext(LinuxDebugger debugger) {
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
