/*
 * @(#)UnmappedAddressException.java	1.3 03/01/23 11:26:12
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class UnmappedAddressException extends AddressException {
  public UnmappedAddressException(long addr) {
    super(addr);
  }

  public UnmappedAddressException(String detail, long addr) {
    super(detail, addr);
  }
}
