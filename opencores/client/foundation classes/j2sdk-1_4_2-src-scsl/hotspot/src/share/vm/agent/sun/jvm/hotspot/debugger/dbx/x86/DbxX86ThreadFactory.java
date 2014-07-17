/*
 * @(#)DbxX86ThreadFactory.java	1.4 03/01/23 11:29:40
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.dbx.x86;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.dbx.*;

public class DbxX86ThreadFactory implements DbxThreadFactory {
  private DbxDebugger debugger;
  
  public DbxX86ThreadFactory(DbxDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new DbxX86Thread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new DbxX86Thread(debugger, id);
  }
}
