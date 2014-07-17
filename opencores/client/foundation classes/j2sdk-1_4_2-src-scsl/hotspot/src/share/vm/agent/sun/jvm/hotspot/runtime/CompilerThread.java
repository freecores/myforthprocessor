/*
 * @(#)CompilerThread.java	1.8 03/01/23 11:44:42
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.io.*;
import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class CompilerThread extends JavaThread {
  public CompilerThread(Address addr) {
    super(addr);
  }

  public boolean isJavaThread() { return false; }

  public boolean isCompilerThread() { return true; }

}
