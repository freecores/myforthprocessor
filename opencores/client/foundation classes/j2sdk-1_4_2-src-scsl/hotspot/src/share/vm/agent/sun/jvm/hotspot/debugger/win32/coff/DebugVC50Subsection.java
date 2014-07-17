/*
 * @(#)DebugVC50Subsection.java	1.4 03/01/23 11:34:28
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Base class for subsections in Visual C++ 5.0 debug information. */

public interface DebugVC50Subsection {
  /** Returns type of this subsection; see {@link
      sun.jvm.hotspot.debugger.win32.coff.DebugVC50SubsectionTypes}. */
  public short getSubsectionType();

  /** Returns module index associated with this subsection. This
      number is 1 based and zero is never a valid index. The index
      0xffff is reserved for tables that are not associated with a
      specific module. These tables include sstLibraries,
      sstGlobalSym, sstGlobalPub and sstGlobalTypes. */
  public short getSubsectionModuleIndex();

  /** Number of bytes in subsection. */
  public int getSubsectionSize();
}
