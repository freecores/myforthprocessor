/*
 * @(#)DebugVC50SSAlignSym.java	1.4 03/01/23 11:33:18
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstAlignSym" subsection in Visual C++ 5.0 debug
    information. This subsection apparently contains non-global
    symbols left over from packing into the sstGlobalSym
    subsection. Until we understand the contents of the sstGlobalSym
    subsection, this subsection will contain no accessors. */

public interface DebugVC50SSAlignSym extends DebugVC50Subsection {
  public DebugVC50SymbolIterator getSymbolIterator();
}
