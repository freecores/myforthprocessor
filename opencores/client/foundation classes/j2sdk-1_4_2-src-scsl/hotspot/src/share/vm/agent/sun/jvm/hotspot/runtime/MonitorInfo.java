/*
 * @(#)MonitorInfo.java	1.3 03/01/23 11:45:22
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import sun.jvm.hotspot.debugger.*;

public class MonitorInfo {
  private OopHandle owner;
  private BasicLock lock;

  public MonitorInfo(OopHandle owner, BasicLock lock) {
    this.owner = owner;
    this.lock  = lock;
  }

  public OopHandle owner() { return owner; }
  public BasicLock lock()  { return lock; }
}
