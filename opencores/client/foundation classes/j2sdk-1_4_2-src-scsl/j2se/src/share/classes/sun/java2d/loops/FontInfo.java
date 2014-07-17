/*
 * @(#)FontInfo.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.loops;

import java.awt.Font;
import java.awt.geom.AffineTransform;

/*
 * A FontInfo object holds all calculated or derived data needed
 * to handle rendering operations based on a particular set of
 * Graphics2D rendering attributes.
 */
public class FontInfo {
    public Font font;
    public double[] devTx;
    public double[] glyphTx;
    public int pixelHeight;
    public float originX;
    public float originY;
    public boolean doFractEnable;
    public boolean doAntiAlias;
    public int rotate;

    public String mtx(double[] matrix) {
    	return ("["+
		matrix[0]+", "+
		matrix[1]+", "+
		matrix[2]+", "+
		matrix[3]+
		"]");
    }

    public String toString() {
	return ("FontInfo["+
		"font="+font+", "+
		"devTx="+mtx(devTx)+", "+
		"glyphTx="+mtx(glyphTx)+", "+
		"pixelHeight="+pixelHeight+", "+
		"origin=("+originX+","+originY+"), "+
		"FractEnable="+doFractEnable+", "+
		"AntiAlias="+doAntiAlias+", "+
		"rotate="+rotate+
		"]");
    }
}
