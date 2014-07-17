/*
 * @(#)DebugVC50SSMPC.java	1.3 03/01/23 11:33:33
 *
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package sun.jvm.hotspot.debugger.win32.coff;

/** Models the "sstMPC" subsection in Visual C++ 5.0 debug
    information. This subsection is only used for Pcode programs and
    therefore the contents are not exposed. */

public interface DebugVC50SSMPC extends DebugVC50Subsection {
}
