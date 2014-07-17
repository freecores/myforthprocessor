/*
 * @(#)ELFException.java	1.4 03/01/23 11:30:11
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.posix.elf;

/** Generic exception class for all exceptions which occur in this
    package. Since there is no mechanism built into this library for
    recovering from errors, the best clients can do is display the
    error string. */

public class ELFException extends RuntimeException {
  public ELFException() {
    super();
  }

  public ELFException(String message) {
    super(message);
  }
}
