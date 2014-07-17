/*
 * @(#)GlyphListPipe.java	1.6 03/03/20
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.java2d.pipe;

import sun.java2d.SurfaceData;
import java.awt.font.GlyphVector;
import sun.java2d.SunGraphics2D;
import sun.awt.font.GlyphCacheOverFlowException;
import sun.awt.font.GlyphList;

import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.Font;
import sun.java2d.loops.FontInfo;

/**
 * A delegate pipe of SG2D for drawing text.
 */

public abstract class GlyphListPipe implements TextPipe {

    public void drawString(SunGraphics2D sg2d, String s,
			   double x, double y)
    {
	FontInfo info = sg2d.fontInfo;
        if (info.pixelHeight > OutlineTextRenderer.THRESHHOLD) {
            SurfaceData.outlineTextRenderer.drawString(sg2d, s, x, y);
            return;
        }
	if (sg2d.transformState > sg2d.TRANSFORM_TRANSLATEONLY) {
	    double origin[] = {x, y};
	    sg2d.transform.transform(origin, 0, origin, 0, 1);
	    x = origin[0] + info.originX;
	    y = origin[1] + info.originY;
	} else {
	    x += info.originX + (double) sg2d.transX;
	    y += info.originY + (double) sg2d.transY;
	}
	GlyphList gl = GlyphList.getInstance();
	try {
	    gl.setFromString(info, s, (float) x, (float) y);
	    drawGlyphList(sg2d, gl);
	}
        catch (GlyphCacheOverFlowException e) {
	    SurfaceData.outlineTextRenderer.drawString(sg2d, s, x, y);
	} finally {
	    gl.dispose();
	}
    }

    public void drawChars(SunGraphics2D sg2d,
			  char data[], int offset, int length,
			  int ix, int iy)
    {
	FontInfo info = sg2d.fontInfo;
	float x, y;
        if (info.pixelHeight > OutlineTextRenderer.THRESHHOLD) {
            SurfaceData.outlineTextRenderer.drawChars(
                                        sg2d, data, offset, length, ix, iy);
            return;
        }
	if (sg2d.transformState > sg2d.TRANSFORM_TRANSLATEONLY) {
	    double origin[] = {ix, iy};
	    sg2d.transform.transform(origin, 0, origin, 0, 1);
	    x = (float) origin[0] + info.originX;
	    y = (float) origin[1] + info.originY;
	} else {
	    x = ix + info.originX + sg2d.transX;
	    y = iy + info.originY + sg2d.transY;
	}
	GlyphList gl = GlyphList.getInstance();
	try {
	    gl.setFromChars(info, data, offset, length, x, y);
	    drawGlyphList(sg2d, gl);
	}
        catch (GlyphCacheOverFlowException e) {
	    SurfaceData.outlineTextRenderer.drawChars(
                                        sg2d, data, offset, length, ix, iy);
	} finally {
	    gl.dispose();
	}
    }

    public void drawBytes(SunGraphics2D sg2d,
			  byte data[], int offset, int length,
			  int ix, int iy)
    {
	FontInfo info = sg2d.fontInfo;
	float x, y;
        if (info.pixelHeight > OutlineTextRenderer.THRESHHOLD) {
            SurfaceData.outlineTextRenderer.drawBytes(
                                        sg2d, data, offset, length, ix, iy);
            return;
        }
	if (sg2d.transformState > sg2d.TRANSFORM_TRANSLATEONLY) {
	    double origin[] = {ix, iy};
	    sg2d.transform.transform(origin, 0, origin, 0, 1);
	    x = (float) origin[0] + info.originX;
	    y = (float) origin[1] + info.originY;
	} else {
	    x = ix + info.originX + sg2d.transX;
	    y = iy + info.originY + sg2d.transY;
	}
	GlyphList gl = GlyphList.getInstance();
	try {
	    gl.setFromBytes(info, data, offset, length, x, y);
	    drawGlyphList(sg2d, gl);
	}
	catch (GlyphCacheOverFlowException e) {
	    SurfaceData.outlineTextRenderer.drawBytes(
                                        sg2d, data, offset, length, ix, iy);
	} finally {
	    gl.dispose();
	}
    }

    public void drawGlyphVector(SunGraphics2D sg2d, GlyphVector g,
				float x, float y)
    {
        Font font = g.getFont();
	FontInfo info = sg2d.lastGlyphVectorFontInfo;
	info = sg2d.checkFontInfo(info, font);
        if (info.pixelHeight >  OutlineTextRenderer.THRESHHOLD) {
	    SurfaceData.outlineTextRenderer.drawGlyphVector(
					sg2d, g, x, y);
	    return;
	}
	sg2d.lastGlyphVectorFontInfo = info;
	if (sg2d.transformState > sg2d.TRANSFORM_TRANSLATEONLY) {
	    double origin[] = {x, y};
	    sg2d.transform.transform(origin, 0, origin, 0, 1);
	    x = (float) origin[0] + info.originX;
	    y = (float) origin[1] + info.originY;
	} else {
	    x += info.originX + sg2d.transX;
	    y += info.originY + sg2d.transY;
	}
	GlyphList gl = GlyphList.getInstance();
	try {
	    gl.setFromGlyphVector(info, g, x, y);
	    drawGlyphList(sg2d, gl);
	}
	catch (GlyphCacheOverFlowException e) {
	    SurfaceData.outlineTextRenderer.drawGlyphVector(sg2d, g, x, y);
	} finally {
	    gl.dispose();
	}
    }

    protected abstract void drawGlyphList(SunGraphics2D sg2d, GlyphList gl);
}
