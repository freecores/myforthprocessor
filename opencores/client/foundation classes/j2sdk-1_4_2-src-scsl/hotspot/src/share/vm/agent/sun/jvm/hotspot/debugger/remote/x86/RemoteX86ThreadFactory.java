/*
 * @(#)RemoteX86ThreadFactory.java	1.3 03/01/23 11:31:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.remote.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.remote.*;

public class RemoteX86ThreadFactory implements RemoteThreadFactory {
  private RemoteDebuggerClient debugger;
  
  public RemoteX86ThreadFactory(RemoteDebuggerClient debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new RemoteX86Thread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new RemoteX86Thread(debugger, id);
  }
}
