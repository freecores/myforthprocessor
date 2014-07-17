/*
 * @(#)WrongTypeException.java	1.3 03/01/23 11:47:48
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.types;

public class WrongTypeException extends RuntimeException {
  public WrongTypeException() {
    super();
  }
  
  public WrongTypeException(String detail) {
    super(detail);
  }
}
