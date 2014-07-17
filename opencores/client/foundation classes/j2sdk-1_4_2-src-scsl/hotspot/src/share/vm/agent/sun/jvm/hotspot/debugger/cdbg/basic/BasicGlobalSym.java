/*
 * @(#)BasicGlobalSym.java	1.5 03/01/23 11:28:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg.basic;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.debugger.cdbg.*;

public class BasicGlobalSym extends BasicSym implements GlobalSym {
  private Type    type;
  private Address addr;
  private boolean isModuleLocal;

  public BasicGlobalSym(String name, Type type, Address addr, boolean isModuleLocal) {
    super(name);
    this.type = type;
    this.addr = addr;
    this.isModuleLocal = isModuleLocal;
  }

  public GlobalSym asGlobal()      { return this; }

  public Type      getType()       { return type; }
  public Address   getAddress()    { return addr; }
  public boolean   isModuleLocal() { return isModuleLocal; }

  public void resolve(BasicCDebugInfoDataBase db, ResolveListener listener) {
    type = db.resolveType(this, type, listener, "resolving type of global");
  }
}
