/*
 * @(#)Sym.java	1.4 03/01/23 11:27:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Provides a superinterface for all symbol types */

public interface Sym {
  /** Name of this symbol */
  public String      getName();

  /** Returns getName() unless a subclass can return something more
      appropriate */
  public String      toString();

  public BlockSym    asBlock();
  public FunctionSym asFunction();
  public GlobalSym   asGlobal();
  public LocalSym    asLocal();

  public boolean     isBlock();
  public boolean     isFunction();
  public boolean     isGlobal();
  public boolean     isLocal();
}
