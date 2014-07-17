/*
 * @(#)TenuredGeneration.java	1.5 03/01/23 11:41:27
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

public class TenuredGeneration extends OneContigSpaceCardGeneration {
  public TenuredGeneration(Address addr) {
    super(addr);
  }

  public GenerationName kind() {
    return GenerationName.MARK_SWEEP_COMPACT;
  }

  public String name() {
    return "tenured generation";
  }
}
