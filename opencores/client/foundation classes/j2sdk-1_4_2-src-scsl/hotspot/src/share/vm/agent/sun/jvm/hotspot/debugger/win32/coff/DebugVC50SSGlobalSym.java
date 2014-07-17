/*
 * @(#)DebugVC50SSGlobalSym.java	1.3 03/01/23 11:33:25
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstGlobalSym" subsection in Visual C++ 5.0 debug
    information. This class provides access to the symbols via
    iterators; it does not instantiate objects to represent symbols
    because of the expected high volume of symbols. The caller is
    expected to traverse this table and convert the platform-dependent
    symbols into a platform-independent format at run time. */

public interface DebugVC50SSGlobalSym extends DebugVC50SSSymbolBase {
}