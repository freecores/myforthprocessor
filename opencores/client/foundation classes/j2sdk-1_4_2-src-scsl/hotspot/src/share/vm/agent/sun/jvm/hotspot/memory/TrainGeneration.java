/*
 * @(#)TrainGeneration.java	1.6 03/01/23 11:41:34
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import sun.jvm.hotspot.debugger.*;

public class TrainGeneration extends CardGeneration {
  public TrainGeneration(Address addr) {
    super(addr);
  }

  public long capacity()                { throw new RuntimeException("not yet implemented"); }
  public long used()                    { throw new RuntimeException("not yet implemented"); }
  public long free()                    { throw new RuntimeException("not yet implemented"); }
  public long contiguousAvailable()     { throw new RuntimeException("not yet implemented"); }

  public boolean contains(Address p)    { throw new RuntimeException("not yet implemented"); }

  public Train firstTrain()             { throw new RuntimeException("not yet implemented"); }
  public Train lastTrain()              { throw new RuntimeException("not yet implemented"); }
  // FIXME: is this necessary?
  public Train specialNonOopTrain()     { throw new RuntimeException("not yet implemented"); }

  public String name()                  { throw new RuntimeException("not yet implemented"); }

  public void spaceIterate(SpaceClosure blk, boolean usedOnly) { throw new RuntimeException("not yet implemented"); }

  public void printOn(PrintStream tty) {
    throw new RuntimeException("not yet implemented");
  }
}
