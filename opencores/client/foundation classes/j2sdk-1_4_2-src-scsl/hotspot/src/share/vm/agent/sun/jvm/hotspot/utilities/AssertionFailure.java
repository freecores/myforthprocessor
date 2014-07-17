/*
 * @(#)AssertionFailure.java	1.3 03/01/23 11:51:05
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

public class AssertionFailure extends RuntimeException {
  public AssertionFailure() {
    super();
  }

  public AssertionFailure(String message) {
    super(message);
  }
}
