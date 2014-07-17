/*
 * @(#)DebugSwitch.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.pack;

/**
 * Variable bit of source, depending on whether it's a product or debug build.
 * @author John Rose
 * @version 1.3, 01/23/03
 */
interface DebugSwitch {
    public final boolean debug = false;  // production version
}
