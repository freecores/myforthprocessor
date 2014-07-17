/*
 * @(#)AddressException.java	1.3 03/01/23 11:24:59
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class AddressException extends RuntimeException {
  private long addr;

  public AddressException(long addr) {
    this.addr = addr;
  }

  public AddressException(String detail, long addr) {
    super(detail);
    this.addr = addr;
  }

  public long getAddress() {
    return addr;
  }
}
