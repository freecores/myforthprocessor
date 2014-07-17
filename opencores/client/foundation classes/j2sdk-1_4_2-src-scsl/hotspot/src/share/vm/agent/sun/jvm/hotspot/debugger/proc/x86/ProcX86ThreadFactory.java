/*
 * @(#)ProcX86ThreadFactory.java	1.4 03/01/23 11:31:01
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.proc.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.proc.*;

public class ProcX86ThreadFactory implements ProcThreadFactory {
  private ProcDebugger debugger;
  
  public ProcX86ThreadFactory(ProcDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new ProcX86Thread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new ProcX86Thread(debugger, id);
  }
}
