/*
 * @(#)UnsupportedPlatformException.java	1.3 03/01/23 11:52:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.utilities;

public class UnsupportedPlatformException extends RuntimeException {
  public UnsupportedPlatformException() {
    super();
  }

  public UnsupportedPlatformException(String message) {
    super(message);
  }
}
