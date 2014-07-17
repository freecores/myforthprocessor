/*
 * @(#)ContiguousSpace.java	1.3 03/01/23 11:40:39
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import java.io.*;
import java.util.*;

import sun.jvm.hotspot.debugger.*;
import sun.jvm.hotspot.runtime.*;
import sun.jvm.hotspot.types.*;

public class ContiguousSpace extends CompactibleSpace {
  private static AddressField topField;

  static {
    VM.registerVMInitializedObserver(new Observer() {
        public void update(Observable o, Object data) {
          initialize(VM.getVM().getTypeDataBase());
        }
      });
  }

  private static synchronized void initialize(TypeDataBase db) {
    Type type = db.lookupType("ContiguousSpace");
    
    topField = type.getAddressField("_top");
  }
  
  public ContiguousSpace(Address addr) {
    super(addr);
  }

  public Address top() {
    return topField.getValue(addr);
  }
  
  /** In bytes */
  public long capacity() {
    return end().minus(bottom());
  }

  /** In bytes */
  public long used() {
    return top().minus(bottom());
  }

  /** In bytes */
  public long free() {
    return end().minus(top());
  }

  /** In a contiguous space we have a more obvious bound on what parts
      contain objects. */
  public MemRegion usedRegion() {
    return new MemRegion(bottom(), top());
  }

  /** Testers */
  public boolean contains(Address p) {
    return (bottom().lessThanOrEqual(p) && top().greaterThan(p));
  }

  public void printOn(PrintStream tty) {
    tty.println(" [" + bottom() + "," +
                top() + "," + end() + ")");
  }
}
