/*
 * @(#)CarTable.java	1.3 03/01/23 11:40:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.oops.*;

public class CarTable {
  public static long trainNumberFor(Oop p)      { throw new RuntimeException("not yet implemented"); }
  public static long carNumberFor(Oop p)        { throw new RuntimeException("not yet implemented"); }
  public static Train trainFor(Oop p)           { throw new RuntimeException("not yet implemented"); }
  public static Space spaceFor(Oop p)           { throw new RuntimeException("not yet implemented"); }
  public static CarTableDesc descFor(Oop p)     { throw new RuntimeException("not yet implemented"); }
  public static boolean shouldScavenge(Oop p)   { throw new RuntimeException("not yet implemented"); }
}
