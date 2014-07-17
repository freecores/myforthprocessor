/*
 * @(#)StackValueCollection.java	1.3 03/01/23 11:45:51
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.runtime;

import java.util.*;

import sun.jvm.hotspot.debugger.*;

public class StackValueCollection {
  private List list;

  public StackValueCollection()           { list = new ArrayList(); }
  public StackValueCollection(int length) { list = new ArrayList(length); }

  public void add(StackValue val) { list.add(val); }
  public int  size()              { return list.size(); }
  public boolean isEmpty()        { return (size() == 0); }
  public StackValue get(int i)    { return (StackValue) list.get(i); }

  // Get typed locals/expressions
  // FIXME: must figure out whether word swapping is necessary on x86
  public int       intAt(int slot)       { return (int) get(slot).getInteger(); }
  public long      longAt(int slot)      { return VM.getVM().buildLongFromIntsPD((int) get(slot).getInteger(),
                                                                                 (int) get(slot+1).getInteger()); }
  public OopHandle oopHandleAt(int slot) { return get(slot).getObject(); }
  public float     floatAt(int slot)     { return Float.intBitsToFloat(intAt(slot)); }
  public double    doubleAt(int slot)    { return Double.longBitsToDouble(longAt(slot)); }
}
