/*
 * @(#)X11Renderer.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.Composite;
import java.awt.Polygon;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.PathIterator;
import java.awt.geom.IllegalPathStateException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.GraphicsPrimitive;
import sun.java2d.pipe.Region;
import sun.java2d.pipe.PixelDrawPipe;
import sun.java2d.pipe.PixelFillPipe;
import sun.java2d.pipe.ShapeDrawPipe;
import sun.java2d.pipe.SpanIterator;
import sun.java2d.pipe.ShapeSpanIterator;
import sun.java2d.pipe.LoopPipe;

public class X11Renderer implements
    PixelDrawPipe,
    PixelFillPipe,
    ShapeDrawPipe
{
    native void doDrawLine(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int x1, int y1, int x2, int y2);

    public void drawLine(SunGraphics2D sg2d,
			 int x1, int y1, int x2, int y2)
    {
	int transx = sg2d.transX;
	int transy = sg2d.transY;
	doDrawLine(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   x1+transx, y1+transy, x2+transx, y2+transy);
    }

    native void doDrawRect(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int x, int y, int w, int h);

    public void drawRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doDrawRect(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doDrawRoundRect(SurfaceData sData,
				Region clip, Composite comp, int pixel,
				int x, int y, int w, int h,
				int arcW, int arcH);

    public void drawRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	doDrawRoundRect(sg2d.surfaceData,
                        sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
			x+sg2d.transX, y+sg2d.transY, width, height,
			arcWidth, arcHeight);
    }

    native void doDrawOval(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int x, int y, int w, int h);

    public void drawOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doDrawOval(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doDrawArc(SurfaceData sData,
			  Region clip, Composite comp, int pixel,
			  int x, int y, int w, int h,
			  int angleStart, int angleExtent);

    public void drawArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	doDrawArc(sg2d.surfaceData,
                  sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		  x+sg2d.transX, y+sg2d.transY, width, height,
		  startAngle, arcAngle);
    }

    native void doDrawPoly(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int transx, int transy,
			   int[] xpoints, int[] ypoints,
			   int npoints, boolean isclosed);

    public void drawPolyline(SunGraphics2D sg2d,
			     int xpoints[], int ypoints[],
			     int npoints)
    {
	doDrawPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints, false);
    }

    public void drawPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	doDrawPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints, true);
    }

    native void doFillRect(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int x, int y, int w, int h);

    public void fillRect(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doFillRect(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doFillRoundRect(SurfaceData sData,
				Region clip, Composite comp, int pixel,
				int x, int y, int w, int h,
				int arcW, int arcH);

    public void fillRoundRect(SunGraphics2D sg2d,
			      int x, int y, int width, int height,
			      int arcWidth, int arcHeight)
    {
	doFillRoundRect(sg2d.surfaceData,
                        sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
			x+sg2d.transX, y+sg2d.transY, width, height,
			arcWidth, arcHeight);
    }

    native void doFillOval(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int x, int y, int w, int h);

    public void fillOval(SunGraphics2D sg2d,
			 int x, int y, int width, int height)
    {
	doFillOval(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   x+sg2d.transX, y+sg2d.transY, width, height);
    }

    native void doFillArc(SurfaceData sData,
			  Region clip, Composite comp, int pixel,
			  int x, int y, int w, int h,
			  int angleStart, int angleExtent);

    public void fillArc(SunGraphics2D sg2d,
			int x, int y, int width, int height,
			int startAngle, int arcAngle)
    {
	doFillArc(sg2d.surfaceData,
                  sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		  x+sg2d.transX, y+sg2d.transY, width, height,
		  startAngle, arcAngle);
    }

    native void doFillPoly(SurfaceData sData,
			   Region clip, Composite comp, int pixel,
			   int transx, int transy,
			   int[] xpoints, int[] ypoints,
			   int npoints);

    public void fillPolygon(SunGraphics2D sg2d,
			    int xpoints[], int ypoints[],
			    int npoints)
    {
	doFillPoly(sg2d.surfaceData,
                   sg2d.getCompClip(), sg2d.composite, sg2d.pixel,
		   sg2d.transX, sg2d.transY, xpoints, ypoints, npoints);
    }

    public void draw(SunGraphics2D sg2d, Shape s) {
	if (sg2d.strokeState == sg2d.STROKE_THIN) {
	    AffineTransform at;
	    Polygon p;
	    if (sg2d.transformState < sg2d.TRANSFORM_TRANSLATESCALE) {
		if (s instanceof Polygon) {
		    p = (Polygon) s;
		    drawPolygon(sg2d, p.xpoints, p.ypoints, p.npoints);
		    return;
		}
		at = null;
	    } else {
		at = sg2d.transform;
	    }
	    PathIterator pi = s.getPathIterator(at, 0.5f);
	    p = new Polygon();
	    float coords[] = new float[2];
	    while (!pi.isDone()) {
		switch (pi.currentSegment(coords)) {
		case PathIterator.SEG_MOVETO:
		    if (p.npoints > 1) {
			drawPolyline(sg2d, p.xpoints, p.ypoints, p.npoints);
		    }
		    p.reset();
		    p.addPoint((int) coords[0], (int) coords[1]);
		    break;
		case PathIterator.SEG_LINETO:
		    if (p.npoints == 0) {
			throw new IllegalPathStateException
			    ("missing initial moveto in path definition");
		    }
		    p.addPoint((int) coords[0], (int) coords[1]);
		    break;
		case PathIterator.SEG_CLOSE:
		    if (p.npoints > 0) {
			p.addPoint(p.xpoints[0], p.ypoints[0]);
		    }
		    break;
		default:
		    throw new IllegalPathStateException("path not flattened");
		}
		pi.next();
	    }
	    if (p.npoints > 1) {
		drawPolyline(sg2d, p.xpoints, p.ypoints, p.npoints);
	    }
	} else if (sg2d.strokeState < sg2d.STROKE_CUSTOM) {
	    // REMIND: X11 can handle uniform scaled wide lines
	    // and dashed lines itself if we set the appropriate
	    // XGC attributes (TBD).
	    ShapeSpanIterator si = LoopPipe.getStrokeSpans(sg2d, s);
	    try {
		devFillSpans(sg2d.surfaceData, si, si.getNativeIterator(),
			     sg2d.composite, 0, 0, sg2d.pixel);
	    } finally {
		si.dispose();
	    }
	} else {
	    fill(sg2d, sg2d.stroke.createStrokedShape(s));
	}
    }

    native void devFillSpans(SurfaceData sData,
			     SpanIterator si, long iterator, Composite comp,
			     int transx, int transy, int pixel);

    public void fill(SunGraphics2D sg2d, Shape s) {
	AffineTransform at;
	int transx, transy;

	if (sg2d.transformState < sg2d.TRANSFORM_TRANSLATESCALE) {
	    if (s instanceof Polygon) {
		Polygon p = (Polygon) s;
		fillPolygon(sg2d, p.xpoints, p.ypoints, p.npoints);
		return;
	    }
	    // Transform (translation) will be done by devFillSpans
	    at = null;
	    transx = sg2d.transX;
	    transy = sg2d.transY;
	} else {
	    // Transform will be done by the PathIterator
	    at = sg2d.transform;
	    transx = transy = 0;
	}
	ShapeSpanIterator ssi = new ShapeSpanIterator(sg2d, false);
	try {
	    // Subtract transx/y from the SSI clip to match the
	    // (potentially untranslated) geometry fed to it
	    Region clip = sg2d.getCompClip();
	    ssi.setOutputAreaXYXY(clip.getLoX() - transx,
				  clip.getLoY() - transy,
				  clip.getHiX() - transx,
				  clip.getHiY() - transy);
	    ssi.appendPath(s.getPathIterator(at));
	    devFillSpans(sg2d.surfaceData,
			 ssi, ssi.getNativeIterator(),
			 sg2d.composite, transx, transy, sg2d.pixel);
	} finally {
	    ssi.dispose();
	}
    }

    native void devCopyArea(SurfaceData sData,
			    int srcx, int srcy,
			    int dstx, int dsty,
			    int w, int h);

    public X11Renderer traceWrap() {
	return new Tracer();
    }

    public static class Tracer extends X11Renderer {
	void doDrawLine(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int x1, int y1, int x2, int y2)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawLine");
	    super.doDrawLine(sData, clip, comp, pixel, x1, y1, x2, y2);
	}
	void doDrawRect(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawRect");
	    super.doDrawRect(sData, clip, comp, pixel, x, y, w, h);
	}
	void doDrawRoundRect(SurfaceData sData,
			     Region clip, Composite comp, int pixel,
			     int x, int y, int w, int h,
			     int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawRoundRect");
	    super.doDrawRoundRect(sData, clip, comp, pixel,
                                  x, y, w, h, arcW, arcH);
	}
	void doDrawOval(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawOval");
	    super.doDrawOval(sData, clip, comp, pixel, x, y, w, h);
	}
	void doDrawArc(SurfaceData sData,
		       Region clip, Composite comp, int pixel,
		       int x, int y, int w, int h,
		       int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawArc");
	    super.doDrawArc(sData, clip, comp, pixel, x, y, w, h,
			    angleStart, angleExtent);
	}
	void doDrawPoly(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int transx, int transy,
			int[] xpoints, int[] ypoints,
			int npoints, boolean isclosed)
	{
	    GraphicsPrimitive.tracePrimitive("X11DrawPoly");
	    super.doDrawPoly(sData, clip, comp, pixel, transx, transy,
			     xpoints, ypoints, npoints, isclosed);
	}
	void doFillRect(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillRect");
	    super.doFillRect(sData, clip, comp, pixel, x, y, w, h);
	}
	void doFillRoundRect(SurfaceData sData,
			     Region clip, Composite comp, int pixel,
			     int x, int y, int w, int h,
			     int arcW, int arcH)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillRoundRect");
	    super.doFillRoundRect(sData, clip, comp, pixel,
                                  x, y, w, h, arcW, arcH);
	}
	void doFillOval(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int x, int y, int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillOval");
	    super.doFillOval(sData, clip, comp, pixel, x, y, w, h);
	}
	void doFillArc(SurfaceData sData,
		       Region clip, Composite comp, int pixel,
		       int x, int y, int w, int h,
		       int angleStart, int angleExtent)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillArc");
	    super.doFillArc(sData, clip, comp, pixel, x, y, w, h,
			    angleStart, angleExtent);
	}
	void doFillPoly(SurfaceData sData,
			Region clip, Composite comp, int pixel,
			int transx, int transy,
			int[] xpoints, int[] ypoints,
			int npoints)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillPoly");
	    super.doFillPoly(sData, clip, comp, pixel, transx, transy,
			     xpoints, ypoints, npoints);
	}
	void devFillSpans(SurfaceData sData,
			  SpanIterator si, long iterator,
			  Composite comp, int transx, int transy, int pixel)
	{
	    GraphicsPrimitive.tracePrimitive("X11FillSpans");
	    super.devFillSpans(sData, si, iterator,
                               comp, transx, transy, pixel);
	}
	void devCopyArea(SurfaceData sData,
			 int srcx, int srcy,
			 int dstx, int dsty,
			 int w, int h)
	{
	    GraphicsPrimitive.tracePrimitive("X11CopyArea");
	    super.devCopyArea(sData, srcx, srcy, dstx, dsty, w, h);
	}
    }
}
