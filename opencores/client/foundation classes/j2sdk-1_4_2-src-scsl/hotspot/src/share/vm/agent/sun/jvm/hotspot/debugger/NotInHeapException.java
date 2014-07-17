/*
 * @(#)NotInHeapException.java	1.3 03/01/23 11:25:38
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class NotInHeapException extends AddressException {
  public NotInHeapException(long addr) {
    super(addr);
  }

  public NotInHeapException(String detail, long addr) {
    super(detail, addr);
  }
}
