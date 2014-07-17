/*
 * @(#)DebugVC50SSSrcLnSeg.java	1.3 03/01/23 11:33:58
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstSrcLnSeg" subsection in Visual C++ 5.0 debug
    information. This subsection has been replaced by the sstSrcModule
    table and therefore currently has an empty implementation. */

public interface DebugVC50SSSrcLnSeg extends DebugVC50Subsection {
}
