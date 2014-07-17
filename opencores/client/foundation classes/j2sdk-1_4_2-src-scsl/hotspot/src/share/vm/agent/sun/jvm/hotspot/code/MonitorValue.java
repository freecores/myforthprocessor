/*
 * @(#)MonitorValue.java	1.3 03/01/23 11:24:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.code;

import java.io.*;

public class MonitorValue {
  private ScopeValue owner;
  private Location   basicLock;

  // FIXME: not useful yet
  //  MonitorValue(ScopeValue* owner, Location basic_lock);

  public MonitorValue(DebugInfoReadStream stream) {
    basicLock = new Location(stream);
    owner     = ScopeValue.readFrom(stream);
  }

  public ScopeValue owner()     { return owner; }
  public Location   basicLock() { return basicLock; }

  // FIXME: not yet implementable
  //  void write_on(DebugInfoWriteStream* stream);

  public void printOn(PrintStream tty) {
    tty.print("monitor{");
    owner().printOn(tty);
    tty.print(",");
    basicLock().printOn(tty);
    tty.print("}");
  }
}
