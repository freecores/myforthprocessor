/*
 * @(#)DebugVC50SSPublicSym.java	1.3 03/01/23 11:33:50
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstPublicSym" subsection in Visual C++ 5.0 debug
    information. These subsections from various modules are compacted
    into the sstGlobalSym table and for this reason this subsection
    currently has no accessors. */

public interface DebugVC50SSPublicSym extends DebugVC50Subsection {
}
