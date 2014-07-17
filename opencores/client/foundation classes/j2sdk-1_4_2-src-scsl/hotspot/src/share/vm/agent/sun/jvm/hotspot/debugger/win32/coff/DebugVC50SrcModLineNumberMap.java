/*
 * @(#)DebugVC50SrcModLineNumberMap.java	1.3 03/01/23 11:34:26
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Supplies line number/address mapping for a given source file. */

public interface DebugVC50SrcModLineNumberMap {
  /** Segment index for this table. */
  public int getSegment();

  /** Number of source line pairs. */
  public int getNumSourceLinePairs();

  /** Get the <i>i</i>th (i..getNumSourceLinePairs() - 1) offset
      within the code segment of the start of the line in the parallel
      line number array. */
  public int getCodeOffset(int i);

  /** Get the <i>i</i>th (i..getNumSourceLinePairs() - 1) line
      number. */
  public int getLineNumber(int i);
}
