/*
 * @(#)X11CustomCursor.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import sun.awt.CustomCursor;
import java.awt.*;
import java.awt.image.*;
import sun.awt.image.ImageRepresentation;

/**
 * A class to encapsulate a custom image-based cursor.  
 *
 * @see Component#setCursor
 * @version 	1.17 01/23/03
 * @author 	Thomas Ball
 */
public class X11CustomCursor extends CustomCursor {

    public X11CustomCursor(Image cursor, Point hotSpot, String name) 
            throws IndexOutOfBoundsException {
        super(cursor, hotSpot, name);
    }

    /**
     * Returns the supported cursor size
     */
    public static Dimension getBestCursorSize(
        int preferredWidth, int preferredHeight) {

        // Fix for bug 4212593 The Toolkit.createCustomCursor does not 
        //                     check absence of the image of cursor 
        // We use XQueryBestCursor which accepts unsigned ints to obtain
        // the largest cursor size that could be dislpayed 
        Dimension d = new Dimension(Math.abs(preferredWidth), Math.abs(preferredHeight));

        queryBestCursor(d);
        return d;
    }

    private static native void queryBestCursor(Dimension d);

    protected void createNativeCursor(Image im, int[] pixels, int width, int height,
                                      int xHotSpot, int yHotSpot) {
	
	class CCount implements Comparable {
	    int color;
	    int count;
	    
	    public CCount(int cl, int ct) {
		color = cl;
		count = ct;
	    }
	    
	    public int compareTo(Object cc) {
	        return ((CCount)cc).count - count;
	    }
        }

	int tmp[] = new int[pixels.length];
	for (int i=0; i<pixels.length; i++) {
	    if ((pixels[i] & 0xff000000) == 0) {
		tmp[i] = -1;
	    } else { 
		tmp[i] = pixels[i] & 0x00ffffff;
	    }
	}
	java.util.Arrays.sort(tmp);
	
	int fc = 0x000000;
	int bc = 0xffffff;
	CCount cols[] = new CCount[pixels.length];
	
	int is = 0;
	int numColors = 0;
	while ( is < pixels.length ) {
	    if (tmp[is] != -1) { 
		cols[numColors++] = new CCount(tmp[is], 1);
		break; 
	    }
	    is ++;
	}
	
	for (int i = is+1; i < pixels.length; i++) {
	    if (tmp[i] != cols[numColors-1].color) {
		cols[numColors++] = new CCount(tmp[i], 1);
	    } else {
	        cols[numColors-1].count ++;
	    }
	}
	java.util.Arrays.sort(cols, 0, numColors);
	
	if (numColors > 0) fc = cols[0].color;
	int fcr = (fc >> 16) & 0x000000ff;
	int fcg = (fc >>  8) & 0x000000ff;
	int fcb = (fc >>  0) & 0x000000ff;
	
	int rdis = 0;
	int gdis = 0;
	int bdis = 0;
	for (int j = 1; j < numColors; j++) {
	    int rr = (cols[j].color >> 16) & 0x000000ff;
	    int gg = (cols[j].color >>  8) & 0x000000ff;
	    int bb = (cols[j].color >>  0) & 0x000000ff;
	    rdis = rdis + cols[j].count * rr;
	    gdis = gdis + cols[j].count * gg;
	    bdis = bdis + cols[j].count * bb;
        }
	int rest = pixels.length - ((numColors > 0) ? cols[0].count : 0);
    // 4653170 Avoid divide / zero exception
    if (rest > 0) {
        rdis = rdis / rest - fcr;
        gdis = gdis / rest - fcg;
        bdis = bdis / rest - fcb;
    }
	rdis = (rdis*rdis + gdis*gdis + bdis*bdis) / 2;
	// System.out.println(" rdis is "+ rdis);
	
	for (int j = 1; j < numColors; j++) {
	    int rr = (cols[j].color >> 16) & 0x000000ff;
	    int gg = (cols[j].color >>  8) & 0x000000ff;
	    int bb = (cols[j].color >>  0) & 0x000000ff;
	    
	    if ( (rr-fcr)*(rr-fcr) + (gg-fcg)*(gg-fcg) + (bb-fcb)*(bb-fcb) 
		 >= rdis )  {
	        bc = cols[j].color;
		break;
            }
	}
	int bcr = (bc >> 16) & 0x000000ff;
	int bcg = (bc >>  8) & 0x000000ff;
	int bcb = (bc >>  0) & 0x000000ff;
	
	
	// On Solaris 2.5.x, the above code for cursor of any size runs fine
        // but on Solaris 2.6, the width of a cursor has to be 8 divisible, 
        //   otherwise, the cursor could be displayed as garbaged.
  	// To work around the 2.6 problem, the following code pads any cursor 
	//   with a transparent area to make a new cursor of width 8 multiples.
	// --- Bug 4148455
	int wNByte = (width + 7)/8;
	int tNByte = wNByte * height;
        byte[] xorMask = new byte[tNByte];
        byte[] andMask = new byte[tNByte];
	
	for (int i = 0; i < width; i++) {
            int omask = 1 << (i % 8);
	    for (int j = 0; j < height; j++) {
	        int ip = j*width + i;
		int ibyte = j*wNByte + i/8;
		
                if ((pixels[ip] & 0xff000000) != 0) {
                    andMask[ibyte] |= omask;
                }
		
		int pr = (pixels[ip] >> 16) & 0x000000ff;
		int pg = (pixels[ip] >>  8) & 0x000000ff;
		int pb = (pixels[ip] >>  0) & 0x000000ff;
		if ( (pr-fcr)*(pr-fcr) + (pg-fcg)*(pg-fcg) + (pb-fcb)*(pb-fcb) 
	          <= (pr-bcr)*(pr-bcr) + (pg-bcg)*(pg-bcg) + (pb-bcb)*(pb-bcb) ) {
		    // show foreground color
                    xorMask[ibyte] |= omask;
	        } 
	    }
	}
	
        createCursor(xorMask, andMask, 8*wNByte, height, fc, bc, xHotSpot, yHotSpot);
    }

    private native void createCursor(byte[] xorMask, byte[] andMask, 
                                     int width, int height, 
                                     int fcolor, int bcolor, 
                                     int xHotSpot, int yHotSpot);

    static {
        cacheInit();
    }

    private native static void cacheInit();
}