/*
 * @(#)MutationException.java	1.3 03/01/23 11:43:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.oops;

public class MutationException extends RuntimeException {
  public MutationException() {
    super();
  }
  
  public MutationException(String detail) {
    super(detail);
  }
}
