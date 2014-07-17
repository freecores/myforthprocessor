/*
 * @(#)DebugVC50SSOffsetMap32.java	1.3 03/01/23 11:33:42
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstOffsetMap32" subsection in Visual C++ 5.0 debug
    information. This table provides a mapping from logical to
    physical offsets. This mapping is applied between the logical to
    physical mapping described by the seg map table. FIXME: this table
    is being elided for now unless it proves to be necessary to finish
    the implementation. */
public interface DebugVC50SSOffsetMap32 extends DebugVC50Subsection {
}
