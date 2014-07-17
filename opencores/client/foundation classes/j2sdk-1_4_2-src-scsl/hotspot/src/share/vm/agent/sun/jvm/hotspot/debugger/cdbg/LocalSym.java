/*
 * @(#)LocalSym.java	1.3 03/01/23 11:27:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

/** Models a local variable in a scope. The meaning of the frame
    offset is platform-dependent, and is typically added to the base
    of frame pointer. */

public interface LocalSym extends Sym {
  /** Name of the local variable */
  public String getName();

  /** Type of the local variable */
  public Type getType();

  /** Offset, in bytes, in the frame of the local variable */
  public long getFrameOffset();
}
