/*
 * @(#)DbxThreadFactory.java	1.4 03/01/23 11:29:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.dbx;

import sun.jvm.hotspot.debugger.*;

/** An interface used only internally by the DbxDebugger to be able to
    create platform-specific Thread objects */

public interface DbxThreadFactory {
  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr);
  public ThreadProxy createThreadWrapper(long id);
}
