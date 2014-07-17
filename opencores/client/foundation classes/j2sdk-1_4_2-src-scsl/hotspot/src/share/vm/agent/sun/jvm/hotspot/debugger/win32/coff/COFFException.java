/*
 * @(#)COFFException.java	1.3 03/01/23 11:32:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Generic exception class for all exceptions which occur in this
    package. Since there is no mechanism built into this library for
    recovering from errors, the best clients can do is display the
    error string. */

public class COFFException extends RuntimeException {
  public COFFException() {
    super();
  }

  public COFFException(String message) {
    super(message);
  }
}
