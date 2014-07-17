/*
 * @(#)CarSpace.java	1.5 03/01/23 11:40:16
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.types.*;

public class CarSpace extends OffsetTableContigSpace {
  public CarSpace(Address addr) {
    super(addr);
  }

  public static long carSize()                  { throw new RuntimeException("not yet implemented"); }
  public static long carSizeInOops()            { throw new RuntimeException("not yet implemented"); }

  public CarSpace nextCar()                     { throw new RuntimeException("not yet implemented"); }
  //  public CarRememberedSet rememberedSet()   { throw new RuntimeException("not yet implemented"); }
  public CarTableDesc desc()                    { throw new RuntimeException("not yet implemented"); }
  public long blocks()                          { throw new RuntimeException("not yet implemented"); }
  public boolean marked()                       { throw new RuntimeException("not yet implemented"); }
  public Train train()                          { throw new RuntimeException("not yet implemented"); }
  public long carNumber()                       { throw new RuntimeException("not yet implemented"); }
  public long trainNumber()                     { throw new RuntimeException("not yet implemented"); }
}
