/*
 * @(#)DebuggerThread.java	1.5 03/01/23 11:44:47
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.io.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

/** FIXME: should be in ../prims dir if that directory existed; for
    now keep it in runtime dir */

public class DebuggerThread extends JavaThread {
  public DebuggerThread(Address addr) {
    super(addr);
  }

  public boolean isJavaThread() { return false; }

  public boolean isDebuggerThread() { return true; }

}
