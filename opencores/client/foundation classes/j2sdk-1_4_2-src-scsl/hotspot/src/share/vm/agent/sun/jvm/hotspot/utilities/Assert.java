/*
 * @(#)Assert.java	1.4 03/01/23 11:51:03
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

public class Assert {
  public static final boolean ASSERTS_ENABLED = true;

  public static void that(boolean test, String message) {
    if (!test) {
      throw new AssertionFailure(message);
    }
  }
}
