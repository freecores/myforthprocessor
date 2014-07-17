/*
 * @(#)ContigPermSpace.java	1.3 03/01/23 11:40:37
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.memory;

import sun.jvm.hotspot.debugger.*;

/** No additional functionality for now */

public class ContigPermSpace extends OffsetTableContigSpace {
  public ContigPermSpace(Address addr) {
    super(addr);
  }
}
