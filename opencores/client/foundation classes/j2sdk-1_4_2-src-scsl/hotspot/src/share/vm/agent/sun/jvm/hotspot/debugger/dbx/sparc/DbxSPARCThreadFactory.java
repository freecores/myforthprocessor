/*
 * @(#)DbxSPARCThreadFactory.java	1.4 03/01/23 11:29:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.dbx.sparc;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.dbx.*;

public class DbxSPARCThreadFactory implements DbxThreadFactory {
  private DbxDebugger debugger;
  
  public DbxSPARCThreadFactory(DbxDebugger debugger) {
    this.debugger = debugger;
  }

  public ThreadProxy createThreadWrapper(Address threadIdentifierAddr) {
    return new DbxSPARCThread(debugger, threadIdentifierAddr);
  }

  public ThreadProxy createThreadWrapper(long id) {
    return new DbxSPARCThread(debugger, id);
  }
}
