/*
 * @(#)DebugVC50SubsectionDirectory.java	1.3 03/01/23 11:34:30
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the subsection directory portion of Visual C++ 5.0 debug
    information. */

public interface DebugVC50SubsectionDirectory {
  public short getHeaderLength();
  public short getEntryLength();
  public int   getNumEntries();
  
  // lfoNextDir and flags fields have been elided as they are unused
  // according to the documentation

  /** Returns subsection at (zero-based) index i. */
  public DebugVC50Subsection getSubsection(int i);
}
