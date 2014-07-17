/*
 * @(#)ArrayData.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

import sun.tools.java.*;

public final
class ArrayData {
    Type type;
    int nargs;

    public ArrayData(Type type, int nargs) {
	this.type = type;
	this.nargs = nargs;
    }
}
