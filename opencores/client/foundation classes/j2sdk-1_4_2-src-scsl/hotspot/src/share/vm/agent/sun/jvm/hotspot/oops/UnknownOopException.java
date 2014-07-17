/*
 * @(#)UnknownOopException.java	1.3 03/01/23 11:44:20
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

public class UnknownOopException extends RuntimeException {
  public UnknownOopException() {
    super();
  }
  
  public UnknownOopException(String detail) {
    super(detail);
  }
}
