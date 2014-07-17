/*
 * @(#)GenerationIsInClosure.java	1.3 03/01/23 11:40:53
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

/** Should only be used once */

class GenerationIsInClosure implements SpaceClosure {
  private Address p;
  private Space sp;

  GenerationIsInClosure(Address p) {
    this.p = p;
  }

  public void doSpace(Space s) {
    if (s.contains(p)) {
      sp = s;
    }
  }
  
  Space space() {
    return sp;
  }
}
