/*
 * @(#)CarTableDesc.java	1.3 03/01/23 11:40:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

public class CarTableDesc {
  public long trainNumber()                     { throw new RuntimeException("not yet implemented"); }
  public long carNumber()                       { throw new RuntimeException("not yet implemented"); }
  public Train train()                          { throw new RuntimeException("not yet implemented"); }
  public Space space()                          { throw new RuntimeException("not yet implemented"); }

  /** Testers */
  public boolean isNew()                        { throw new RuntimeException("not yet implemented"); }
  public boolean isSpecialNonOop()              { throw new RuntimeException("not yet implemented"); }
  public boolean isTrain()                      { throw new RuntimeException("not yet implemented"); }
  public boolean isTenured()                    { throw new RuntimeException("not yet implemented"); }
  public boolean isPerm()                       { throw new RuntimeException("not yet implemented"); }
  
  public boolean equals(Object obj)             { throw new RuntimeException("not yet implemented"); }
  public boolean lessThan(CarTableDesc arg)     { throw new RuntimeException("not yet implemented"); }
  
  public boolean initialize(long trainNumber, long carNumber, Train train, Space sp, boolean shouldScavenge) {
    throw new RuntimeException("not yet implemented");
  }
}
