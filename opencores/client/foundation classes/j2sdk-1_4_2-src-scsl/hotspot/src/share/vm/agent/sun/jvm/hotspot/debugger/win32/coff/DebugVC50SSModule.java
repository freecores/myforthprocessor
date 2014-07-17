/*
 * @(#)DebugVC50SSModule.java	1.3 03/01/23 11:33:36
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "module" subsection in Visual C++ 5.0 debug
    information. (Some of the descriptions are taken directly from
    Microsoft's documentation and are copyrighted by Microsoft.) */

public interface DebugVC50SSModule extends DebugVC50Subsection {
  public short getOverlayNumber();
  /** Index into sstLibraries subsection if this module was linked
      from a library. */
  public short getLibrariesIndex();

  /** Count of the number of code segments this module contributes to.
      This is the length of the SegInfo array, below. */
  public short getNumCodeSegments();

  /** Debugging style for this module. Currently only "CV" is defined.
      A module can have only one debugging style. If a module contains
      debugging information in an unrecognized style, the information
      will be discarded. */
  public short getDebuggingStyle();

  /** Fetch description of segment to which this module contributes
      code (0..getNumCodeSegments - 1) */
  public DebugVC50SegInfo getSegInfo(int i);
      
  /** Name of the module */
  public String getName();
}
