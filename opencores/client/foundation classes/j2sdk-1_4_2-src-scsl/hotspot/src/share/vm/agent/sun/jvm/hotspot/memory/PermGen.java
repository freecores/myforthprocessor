/*
 * @(#)PermGen.java	1.3 03/01/23 11:41:08
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;

/** All heaps contains a "permanent generation," containing permanent
    (reflective) objects.  This is like a regular generation in some
    ways, but unlike one in others, and so is split apart. (FIXME:
    this distinction is confusing and seems unnecessary.) */

public abstract class PermGen extends VMObject {
  // NEEDS_CLEANUP
  public PermGen(Address addr) {
    super(addr);
  }

  public abstract Generation asGen();
}
