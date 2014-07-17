/*
 * @(#)AccessControl.java	1.3 03/01/23 11:26:14
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.cdbg;

public interface AccessControl {
  public static final int NO_PROTECTION = 0;
  public static final int PRIVATE       = 1;
  public static final int PROTECTED     = 2;
  public static final int PUBLIC        = 3;
}
