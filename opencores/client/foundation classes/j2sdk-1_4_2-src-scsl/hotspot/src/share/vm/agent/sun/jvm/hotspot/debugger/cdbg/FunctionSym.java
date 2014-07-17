/*
 * @(#)FunctionSym.java	1.4 03/01/23 11:27:00
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** A FunctionSym is an extension of a BlockSym which contains
    additional information such as the type (signature) of the
    function. */

public interface FunctionSym extends BlockSym {
  /** Type (a FunctionType, unless it could not be properly resolved)
      of the procedure */
  public Type getType();

  /** Indicates whether this function is global or module-local (i.e.,
      static) */
  public boolean isModuleLocal();

  /** Should provide a reasonable visual representation of the
      signature of this symbol. */
  public String toString();
}
