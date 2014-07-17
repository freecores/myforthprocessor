/*
 * @(#)GlyphList.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.font;

import java.awt.Font;
import java.awt.font.GlyphVector;
import java.awt.font.FontRenderContext;
import sun.java2d.loops.FontInfo;

/*
 * This class represents a list of actual renderable glyphs.
 * It can be constructed from a number of text sources, representing
 * the various ways in which a programmer can ask a Graphics2D object
 * to render some text.  Once constructed, it provides a way of iterating
 * through the device metrics and graybits of the individual glyphs that
 * need to be rendered to the screen.
 *
 * Note that this class holds pointers to native data which must be
 * disposed.  It is not marked as finalizable since it is intended
 * to be very lightweight and finalization is a comparitively expensive
 * procedure.  The caller must specifically use try{} finally{} to
 * manually ensure that the object is disposed after use, otherwise
 * native data structures might be leaked.
 *
 * Here is a code sample for using this class:
 *
 * public void drawString(String str, FontInfo info, float x, float y) {
 *     GlyphList gl = GlyphList.getInstance();
 *     try {
 *         gl.setFromString(info, str, x, y);
 *         int strbounds[] = gl.getBounds();
 *         int numglyphs = gl.getNumGlyphs();
 *         for (int i = 0; i < numglyphs; i++) {
 *             gl.setGlyphIndex(i);
 *             int metrics[] = gl.getMetrics();
 *             byte bits[] = gl.getGrayBits();
 *             int glyphx = metrics[0];
 *             int glyphy = metrics[1];
 *             int glyphw = metrics[2];
 *             int glyphh = metrics[3];
 *             int off = 0;
 *             for (int j = 0; j < glyphh; j++) {
 *                 for (int i = 0; i < glyphw; i++) {
 *                     int dx = glyphx + i;
 *                     int dy = glyphy + j;
 *                     int alpha = bits[off++];
 *                     drawPixel(alpha, dx, dy);
 *                 }
 *             }
 *         }
 *     } finally {
 *         gl.dispose();
 *     }
 * }
 */
public final class GlyphList {
    private static final int MINGRAYLENGTH = 1024;
    private static final int MAXGRAYLENGTH = 8192;

    long pData;
    int glyphindex;
    int metrics[];
    byte graybits[];

    private static native void initIDs();

    static {
	initIDs();
    }

    private static GlyphList saved;

    private GlyphList() {
    }

    public long getPeer() {
	return pData;
    }

    public static GlyphList getInstance() {
	synchronized(NativeFontWrapper.class) {
	    GlyphList gl = saved;
	    if (gl == null) {
		gl = new GlyphList();
	    } else {
		saved = null;
	    }
	    return gl;
	}
    }

    public void setFromString(FontInfo info, String str,
			      float x, float y)
    {
	synchronized(NativeFontWrapper.class) {
	    setupStringData(str,
			    info.font, x, y,
			    info.glyphTx,
			    info.doAntiAlias,
			    info.doFractEnable);
	    glyphindex = -1;
	}
    }

    public void setFromChars(FontInfo info, char[] chars, int off, int len,
			     float x, float y)
    {
	synchronized(NativeFontWrapper.class) {
	    setupCharData(chars, off, len,
			  info.font, x, y,
			  info.glyphTx,
			  info.doAntiAlias,
			  info.doFractEnable);
	    glyphindex = -1;
	}
    }

    public void setFromBytes(FontInfo info, byte[] bytes, int off, int len,
			     float x, float y)
    {
	synchronized(NativeFontWrapper.class) {
	    setupByteData(bytes, off, len,
			  info.font, x, y,
			  info.glyphTx,
			  info.doAntiAlias,
			  info.doFractEnable);
	    glyphindex = -1;
	}
    }

    public void setFromGlyphVector(FontInfo info, GlyphVector gv,
				   float x, float y)
    {
	double[] ftx = null;
	FontRenderContext frc = gv.getFontRenderContext();
	boolean aa = frc.isAntiAliased();
	boolean fm = frc.usesFractionalMetrics();
	try {
	    StandardGlyphVector sgv = (StandardGlyphVector)gv;
	    ftx = sgv.fontTX;
	}
	catch (ClassCastException e) {
	    ftx = new double[6];
	    double scale = info.font.getSize2D();
	    if (info.font.isTransformed()) {
		info.font.getTransform().getMatrix(ftx);
		for (int i = 0; i < ftx.length; ++i) {
		    ftx[i] *= scale;
		}
	    } else {
		ftx[0] = ftx[3] = scale;
	    }
	}
	synchronized(NativeFontWrapper.class) {
	    setupGlyphData(gv,
			   info.font, x, y,
			   ftx,
			   info.devTx,
			   aa,
			   fm);
	    glyphindex = -1;
	}
    }

    public int[] getBounds() {
	if (glyphindex >= 0) {
	    throw new InternalError("calling getBounds after setGlyphIndex");
	}
	if (metrics == null) {
	    metrics = new int[4];
	}
	synchronized(NativeFontWrapper.class) {
	    fillBounds(metrics);
	}
	return metrics;
    }

    public void setGlyphIndex(int index) {
	glyphindex = index;
	if (metrics == null) {
	    metrics = new int[4];
	}
	synchronized(NativeFontWrapper.class) {
	    fillMetrics(index, metrics);
	}
    }

    public int[] getMetrics() {
	return metrics;
    }

    public byte[] getGrayBits() {
	int len = metrics[2] * metrics[3];
	if (graybits == null) {
	    graybits = new byte[Math.max(len, MINGRAYLENGTH)];
	} else {
	    if (len > graybits.length) {
		graybits = new byte[len];
	    }
	}
	synchronized(NativeFontWrapper.class) {
	    fillGrayBits(glyphindex, graybits);
	}

	return graybits;
    }

    public void dispose() {
	synchronized(NativeFontWrapper.class) {
	    discardData();
	    if (graybits != null && graybits.length > MAXGRAYLENGTH) {
		graybits = null;
	    }
	    saved = this;
	}
    }

    private native void setupStringData(String str,
					Font font, float x, float y,
					double[] glyphTx,
					boolean doAA, boolean doFract);

    private native void setupCharData(char[] chars, int off, int len,
				      Font font, float x, float y,
				      double[] glyphTx,
				      boolean doAA, boolean doFract);

    private native void setupByteData(byte[] bytes, int off, int len,
				      Font font, float x, float y,
				      double[] glyphTx,
				      boolean doAA, boolean doFract);

    private native void setupGlyphData(GlyphVector gv,
				       Font font, float x, float y,
				       double[] glyphTX, double[] devTX,
				       boolean doAA, boolean doFract);

    public native int getNumGlyphs();

    private native void fillBounds(int[] metrics);

    private native void fillMetrics(int index, int[] metrics);

    private native void fillGrayBits(int index, byte[] graybits);

    private native void discardData();
}
