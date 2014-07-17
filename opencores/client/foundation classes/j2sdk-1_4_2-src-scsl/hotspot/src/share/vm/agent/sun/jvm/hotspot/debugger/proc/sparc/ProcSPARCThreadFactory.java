/*
 * @(#)ProcSPARCThreadFactory.java	1.4 03/01/23 11:30:53
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.proc.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.proc.*;

public class ProcSPARCThreadFactory implements ProcThreadFactory {
  private ProcDebugger debugger;
  
  public ProcSPARCThreadFactory(ProcDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new ProcSPARCThread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new ProcSPARCThread(debugger, id);
  }
}
