/*
 * @(#)FontFileFilter.java	1.1 02/10/01
 * 
 * <PRE>
 * This class filters TrueType font files from other file 
 * found in the font path. 
 *
 * </PRE>
 *
 * @author Ilya Bagrak
 */

package com.sun.java2d.fontchecker;

import java.io.*; 
import sun.awt.font.*; 

public class FontFileFilter implements java.io.FileFilter, FontCheckerConstants {
    
    /**
     * Boolean flag indicating whether this filter filters out 
     * non-TrueType fonts. 
     */ 
    private boolean checkNonTTF; 
    
    public FontFileFilter() {
	this(false); 
    }
    
    public FontFileFilter(boolean checkNonTTF) {
	super(); 
	this.checkNonTTF = checkNonTTF; 
    }
    
    /**
     * Checks whether a file is accepted by this filter. 
     * <BR>
     * This method checks whehter a file is accepted by this filter. 
     * This filter is made to accept all the file whose extension is 
     * either .ttf or .TTF. These files are assumed to be TrueType fonts. 
     * <BR><BR>
     * @return returns a boolean value indicating whether or not a file is 
     * accepted
     */ 
    public boolean accept(File pathname) {
	
	String name = pathname.getName(); 
	return (name.endsWith(".ttf") || 
		name.endsWith(".TTF") ||
		name.endsWith(".ttc") ||
		name.endsWith(".TTC"))  ||
	    (name.endsWith( ".ps") ||
	     name.endsWith( ".PS") ||
	     name.endsWith(".pfb") ||
	     name.endsWith(".PFB") ||
	     name.endsWith(".pfa") ||
	     name.endsWith(".PFA") && 
	     checkNonTTF == true);
    }
    
    public static int getFontType(String filename) {
	if (filename.endsWith(".ttf") || 
	    filename.endsWith(".TTF") ||
	    filename.endsWith(".ttc") ||
	    filename.endsWith(".TTC"))
	    return NativeFontWrapper.FONTFORMAT_TRUETYPE; 
	else if (filename.endsWith( ".ps") ||
		 filename.endsWith( ".PS") ||
		 filename.endsWith(".pfb") ||
		 filename.endsWith(".PFB") ||
		 filename.endsWith(".pfa") ||
		 filename.endsWith(".PFA"))
	    return NativeFontWrapper.FONTFORMAT_TYPE1; 
	else
	    return NativeFontWrapper.FONTFORMAT_NONE; 
    }



}
