/*
 * @(#)UnalignedAddressException.java	1.3 03/01/23 11:26:09
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class UnalignedAddressException extends AddressException {
  public UnalignedAddressException(long addr) {
    super(addr);
  }

  public UnalignedAddressException(String detail, long addr) {
    super(detail, addr);
  }
}
