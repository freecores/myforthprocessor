/*
 * @(#)CharToByteBig5_HKSCS.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.io;

public class CharToByteBig5_HKSCS extends CharToByteHKSCS {
    CharToByteBig5 cbBig5 = new CharToByteBig5();

    public String getCharacterEncoding() {
        return "Big5_HKSCS";
    }

    protected int getNative(char ch) {
	int r = super.getNative(ch);
	return (r != 0) ? r : cbBig5.getNative(ch);
    }
}
