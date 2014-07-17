/*
 * @(#)FontDescriptor.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import java.awt.GraphicsEnvironment;
import sun.io.CharToByteConverter;
import sun.io.CharToByteUnicode;

public class FontDescriptor implements Cloneable {

    static {
        NativeLibLoader.loadLibraries();
        initIDs();
    }

    String nativeName;
    public CharToByteConverter fontCharset;
    int[] exclusionRanges;

    public FontDescriptor(String nativeName, CharToByteConverter fontCharset,
			  int[] exclusionRanges){

	this.nativeName = nativeName;
	this.fontCharset = fontCharset;
	this.exclusionRanges = exclusionRanges;
	this.useUnicode = false;
    }

    /**
     * Return true if the character is exclusion character.
     */
    public boolean isExcluded(char ch){
	for (int i = 0; i < exclusionRanges.length; i++){

	    int lo = (exclusionRanges[i] >> 16) & 0xffff;
	    int up = (exclusionRanges[i]) & 0xffff ;

	    if (ch >= lo && ch <= up){
		return true;
	    }
	}
	return false;
    }

    public String toString() {
	return super.toString() + " [" + nativeName + "|" + fontCharset + "]";
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();


    public CharToByteConverter unicodeCharset;
    boolean useUnicode;
   
    public boolean useUnicode() {
        if (useUnicode && unicodeCharset == null) {
            this.unicodeCharset = new CharToByteUnicode(false);
 	}
        return useUnicode;
    }    
}

