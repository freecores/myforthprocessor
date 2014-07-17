/*
 * @(#)CompactingPermGenGen.java	1.3 03/01/23 11:40:35
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;

import sun.jvm.hotspot.debugger.*;

/** This is the "generation" view of a CompactingPermGen. */

public class CompactingPermGenGen extends OneContigSpaceCardGeneration {
  // NEEDS_CLEANUP
  public CompactingPermGenGen(Address addr) {
    super(addr);
  }

  public String name() {
    return "compacting permanent generation";
  }

  public void printOn(PrintStream tty) {
    tty.print("  perm");
    theSpace().printOn(tty);
  }
}
