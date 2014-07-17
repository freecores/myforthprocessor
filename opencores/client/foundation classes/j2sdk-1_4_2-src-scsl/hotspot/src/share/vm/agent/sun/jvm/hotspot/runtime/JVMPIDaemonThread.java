/*
 * @(#)JVMPIDaemonThread.java	1.6 03/01/23 11:45:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class JVMPIDaemonThread extends JavaThread {
  public JVMPIDaemonThread(Address addr) {
    super(addr);
  }

  public boolean isJavaThread() { return false; }

  public boolean isJVMPIDaemonThread() { return true; }
}
