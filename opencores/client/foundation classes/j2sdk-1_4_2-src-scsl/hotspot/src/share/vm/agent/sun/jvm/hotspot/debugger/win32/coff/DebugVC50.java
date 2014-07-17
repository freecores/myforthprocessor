/*
 * @(#)DebugVC50.java	1.3 03/01/23 11:33:10
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models debug information in Visual C++ 5.0 format. */

public interface DebugVC50 {
  public int getSubsectionDirectoryOffset();

  public DebugVC50SubsectionDirectory getSubsectionDirectory();
}
