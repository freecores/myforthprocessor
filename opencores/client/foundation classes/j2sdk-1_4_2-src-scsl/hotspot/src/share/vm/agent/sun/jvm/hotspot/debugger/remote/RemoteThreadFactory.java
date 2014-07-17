/*
 * @(#)RemoteThreadFactory.java	1.3 03/01/23 11:31:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.remote;

import sun.jvm.hotspot.debugger.*;

/** An interface used only internally by the ProcDebugger to be able to
    create platform-specific Thread objects */

public interface RemoteThreadFactory {
  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr);
  public ThreadProxy createThreadWrapper(long id);
}
