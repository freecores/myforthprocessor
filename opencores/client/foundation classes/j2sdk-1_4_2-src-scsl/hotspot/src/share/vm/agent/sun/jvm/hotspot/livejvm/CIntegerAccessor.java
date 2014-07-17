/*
 * @(#)CIntegerAccessor.java	1.2 03/01/23 11:40:02
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.livejvm;

import sun.jvm.hotspot.debugger.*;

class CIntegerAccessor {
  private Address addr;
  private long numBytes;
  private boolean isUnsigned;
  
  CIntegerAccessor(Address addr, long numBytes, boolean isUnsigned) {
    this.addr = addr;
    this.numBytes = numBytes;
    this.isUnsigned = isUnsigned;
  }

  long getValue() {
    return addr.getCIntegerAt(0, numBytes, isUnsigned);
  }
  
  void setValue(long value) {
    addr.setCIntegerAt(0, numBytes, value);
  }
}
