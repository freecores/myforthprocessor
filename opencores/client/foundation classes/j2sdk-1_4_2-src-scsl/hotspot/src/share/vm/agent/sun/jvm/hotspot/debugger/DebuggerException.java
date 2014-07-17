/*
 * @(#)DebuggerException.java	1.3 03/01/23 11:25:09
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger;

public class DebuggerException extends RuntimeException {
  public DebuggerException() {
    super();
  }

  public DebuggerException(String message) {
    super(message);
  }
}
