/*
 * @(#)TenuredSpace.java	1.5 03/01/23 11:41:29
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

/** No additional functionality for now */

public class TenuredSpace extends OffsetTableContigSpace {
  public TenuredSpace(Address addr) {
    super(addr);
  }
}
