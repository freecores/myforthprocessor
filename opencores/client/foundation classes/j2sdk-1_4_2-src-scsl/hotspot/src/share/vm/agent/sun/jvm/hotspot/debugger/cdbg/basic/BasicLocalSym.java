/*
 * @(#)BasicLocalSym.java	1.5 03/01/23 11:28:41
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic;

import sun.jvm.hotspot.debugger.cdbg.*;

public class BasicLocalSym extends BasicSym implements LocalSym {
  private Type   type;
  private long   frameOffset;

  public BasicLocalSym(String name, Type type, long frameOffset) {
    super(name);
    this.type = type;
    this.frameOffset = frameOffset;
  }

  public LocalSym asLocal()        { return this; }

  public Type     getType()        { return type; }
  public long     getFrameOffset() { return frameOffset; }

  public void resolve(BasicCDebugInfoDataBase db, ResolveListener listener) {
    type = db.resolveType(this, type, listener, "resolving type of local");
  }
}
