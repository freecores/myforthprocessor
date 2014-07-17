/*
 * @(#)DebugVC50SSSymbols.java	1.3 03/01/23 11:34:11
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstSymbols" subsection in Visual C++ 5.0 debug
    information. Public symbols from these subsections are moved to
    the sstGlobalSym subsection during packing, and remaining symbols
    are moved to the sstAlignSym subsection. For this reason this
    subsection contains no accessors. */

public interface DebugVC50SSSymbols extends DebugVC50Subsection {
}
