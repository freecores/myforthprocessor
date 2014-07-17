/*
 * @(#)WindbgThreadContext.java	1.2 03/01/23 11:35:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.windbg;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.x86.*;

class WindbgThreadContext extends X86ThreadContext {
  private WindbgDebugger debugger;

  public WindbgThreadContext(WindbgDebugger debugger) {
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
