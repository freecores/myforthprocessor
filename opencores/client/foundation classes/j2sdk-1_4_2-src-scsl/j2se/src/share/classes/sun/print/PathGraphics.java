/*
 * @(#)PathGraphics.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.print;

import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Paint;
import java.awt.Polygon;
import java.awt.Shape;

import java.text.AttributedCharacterIterator;

import java.awt.font.GlyphVector;
import java.awt.font.TextLayout;

import java.awt.geom.AffineTransform;
import java.awt.geom.Arc2D;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.RoundRectangle2D;
import java.awt.geom.PathIterator;

import java.awt.print.PageFormat;
import java.awt.print.Printable;
import java.awt.print.PrinterException;
import java.awt.print.PrinterGraphics;
import java.awt.print.PrinterJob;

public abstract class PathGraphics extends ProxyGraphics2D {

    private Printable mPainter;
    private PageFormat mPageFormat;
    private int mPageIndex;
    private boolean mCanRedraw;

    protected PathGraphics(Graphics2D graphics, PrinterJob printerJob,
                           Printable painter, PageFormat pageFormat,
                           int pageIndex, boolean canRedraw) {
	super(graphics, printerJob);

        mPainter = painter;
        mPageFormat = pageFormat;
        mPageIndex = pageIndex;
        mCanRedraw = canRedraw;
    }

    /**
     * Return the Printable instance responsible for drawing
     * into this Graphics.
     */
    protected Printable getPrintable() {
        return mPainter;
    }

    /**
     * Return the PageFormat associated with this page of
     * Graphics.
     */
    protected PageFormat getPageFormat() {
        return mPageFormat;
    }

    /**
     * Return the page index associated with this Graphics.
     */
    protected int getPageIndex() {
        return mPageIndex;
    }

    /**
     * Return true if we are allowed to ask the application
     * to redraw portions of the page. In general, with the
     * PrinterJob API, the application can be asked to do a
     * redraw. When PrinterJob is emulating PrintJob then we
     * can not.
     */
    public boolean canDoRedraws() {
        return mCanRedraw;
    }

     /** 
      * Redraw a rectanglular area using a proxy graphics
      */
    public abstract void redrawRegion(Rectangle2D region,
                                  double scaleX, double scaleY,
                                  Rectangle2D srcRect, AffineTransform xform) 

                    throws PrinterException ;

    /** 
     * Draws a line, using the current color, between the points 
     * <code>(x1,&nbsp;y1)</code> and <code>(x2,&nbsp;y2)</code> 
     * in this graphics context's coordinate system. 
     * @param   x1  the first point's <i>x</i> coordinate.
     * @param   y1  the first point's <i>y</i> coordinate.
     * @param   x2  the second point's <i>x</i> coordinate.
     * @param   y2  the second point's <i>y</i> coordinate.
     */
    public void drawLine(int x1, int y1, int x2, int y2) {
			
	Paint paint = getPaint();

    	try {
	    AffineTransform deviceTransform = getTransform();
	    if (getClip() != null) {
		deviceClip(getClip().getPathIterator(deviceTransform));
	    }

	    deviceDrawLine(x1, y1, x2, y2, (Color) paint);
    
	} catch (ClassCastException e) {
	    throw new IllegalArgumentException("Expected a Color instance");
    	}
    }


    /** 
     * Draws the outline of the specified rectangle. 
     * The left and right edges of the rectangle are at 
     * <code>x</code> and <code>x&nbsp;+&nbsp;width</code>. 
     * The top and bottom edges are at 
     * <code>y</code> and <code>y&nbsp;+&nbsp;height</code>. 
     * The rectangle is drawn using the graphics context's current color.
     * @param         x   the <i>x</i> coordinate 
     *                         of the rectangle to be drawn.
     * @param         y   the <i>y</i> coordinate 
     *                         of the rectangle to be drawn.
     * @param         width   the width of the rectangle to be drawn.
     * @param         height   the height of the rectangle to be drawn.
     * @see          java.awt.Graphics#fillRect
     * @see          java.awt.Graphics#clearRect
     */
    public void drawRect(int x, int y, int width, int height) {

	Paint paint = getPaint();

    	try {
	    AffineTransform deviceTransform = getTransform();
	    if (getClip() != null) {
		deviceClip(getClip().getPathIterator(deviceTransform));
	    }

	    deviceFrameRect(x, y, width, height, (Color) paint);

	} catch (ClassCastException e) {
	    throw new IllegalArgumentException("Expected a Color instance");
	}

    }

    /** 
     * Fills the specified rectangle. 
     * The left and right edges of the rectangle are at 
     * <code>x</code> and <code>x&nbsp;+&nbsp;width&nbsp;-&nbsp;1</code>. 
     * The top and bottom edges are at 
     * <code>y</code> and <code>y&nbsp;+&nbsp;height&nbsp;-&nbsp;1</code>. 
     * The resulting rectangle covers an area 
     * <code>width</code> pixels wide by 
     * <code>height</code> pixels tall.
     * The rectangle is filled using the graphics context's current color. 
     * @param         x   the <i>x</i> coordinate 
     *                         of the rectangle to be filled.
     * @param         y   the <i>y</i> coordinate 
     *                         of the rectangle to be filled.
     * @param         width   the width of the rectangle to be filled.
     * @param         height   the height of the rectangle to be filled.
     * @see           java.awt.Graphics#clearRect
     * @see           java.awt.Graphics#drawRect
     */
    public void fillRect(int x, int y, int width, int height){

	Paint paint = getPaint();

	try {
	    AffineTransform deviceTransform = getTransform();
	    if (getClip() != null) {
		deviceClip(getClip().getPathIterator(deviceTransform));
	    }

	    deviceFillRect(x, y, width, height, (Color) paint);

	} catch (ClassCastException e) {
	    throw new IllegalArgumentException("Expected a Color instance");
	}
    }

       /** 
     * Clears the specified rectangle by filling it with the background
     * color of the current drawing surface. This operation does not 
     * use the current paint mode. 
     * <p>
     * Beginning with Java&nbsp;1.1, the background color 
     * of offscreen images may be system dependent. Applications should 
     * use <code>setColor</code> followed by <code>fillRect</code> to 
     * ensure that an offscreen image is cleared to a specific color. 
     * @param       x the <i>x</i> coordinate of the rectangle to clear.
     * @param       y the <i>y</i> coordinate of the rectangle to clear.
     * @param       width the width of the rectangle to clear.
     * @param       height the height of the rectangle to clear.
     * @see         java.awt.Graphics#fillRect(int, int, int, int)
     * @see         java.awt.Graphics#drawRect
     * @see         java.awt.Graphics#setColor(java.awt.Color)
     * @see         java.awt.Graphics#setPaintMode
     * @see         java.awt.Graphics#setXORMode(java.awt.Color)
     */
    public void clearRect(int x, int y, int width, int height) {

	fill(new Rectangle2D.Float(x, y, width, height), getBackground());
    }

        /** 
     * Draws an outlined round-cornered rectangle using this graphics 
     * context's current color. The left and right edges of the rectangle 
     * are at <code>x</code> and <code>x&nbsp;+&nbsp;width</code>, 
     * respectively. The top and bottom edges of the rectangle are at 
     * <code>y</code> and <code>y&nbsp;+&nbsp;height</code>. 
     * @param      x the <i>x</i> coordinate of the rectangle to be drawn.
     * @param      y the <i>y</i> coordinate of the rectangle to be drawn.
     * @param      width the width of the rectangle to be drawn.
     * @param      height the height of the rectangle to be drawn.
     * @param      arcWidth the horizontal diameter of the arc 
     *                    at the four corners.
     * @param      arcHeight the vertical diameter of the arc 
     *                    at the four corners.
     * @see        java.awt.Graphics#fillRoundRect
     */
    public void drawRoundRect(int x, int y, int width, int height,
			      int arcWidth, int arcHeight) {
	
	draw(new RoundRectangle2D.Float(x, y,
				        width, height,
					arcWidth, arcHeight));
    }


    /** 
     * Fills the specified rounded corner rectangle with the current color.
     * The left and right edges of the rectangle 
     * are at <code>x</code> and <code>x&nbsp;+&nbsp;width&nbsp;-&nbsp;1</code>, 
     * respectively. The top and bottom edges of the rectangle are at 
     * <code>y</code> and <code>y&nbsp;+&nbsp;height&nbsp;-&nbsp;1</code>. 
     * @param       x the <i>x</i> coordinate of the rectangle to be filled.
     * @param       y the <i>y</i> coordinate of the rectangle to be filled.
     * @param       width the width of the rectangle to be filled.
     * @param       height the height of the rectangle to be filled.
     * @param       arcWidth the horizontal diameter 
     *                     of the arc at the four corners.
     * @param       arcHeight the vertical diameter 
     *                     of the arc at the four corners.
     * @see         java.awt.Graphics#drawRoundRect
     */
    public void fillRoundRect(int x, int y, int width, int height,
			      int arcWidth, int arcHeight) {

	fill(new RoundRectangle2D.Float(x, y,
				        width, height,
					arcWidth, arcHeight));
    }

    /** 
     * Draws the outline of an oval.
     * The result is a circle or ellipse that fits within the 
     * rectangle specified by the <code>x</code>, <code>y</code>, 
     * <code>width</code>, and <code>height</code> arguments. 
     * <p> 
     * The oval covers an area that is 
     * <code>width&nbsp;+&nbsp;1</code> pixels wide 
     * and <code>height&nbsp;+&nbsp;1<code> pixels tall. 
     * @param       x the <i>x</i> coordinate of the upper left 
     *                     corner of the oval to be drawn.
     * @param       y the <i>y</i> coordinate of the upper left 
     *                     corner of the oval to be drawn.
     * @param       width the width of the oval to be drawn.
     * @param       height the height of the oval to be drawn.
     * @see         java.awt.Graphics#fillOval
     * @since       JDK1.0
     */
    public void drawOval(int x, int y, int width, int height) {
	draw(new Ellipse2D.Float(x, y, width, height));
    }

        /** 
     * Fills an oval bounded by the specified rectangle with the
     * current color.
     * @param       x the <i>x</i> coordinate of the upper left corner 
     *                     of the oval to be filled.
     * @param       y the <i>y</i> coordinate of the upper left corner 
     *                     of the oval to be filled.
     * @param       width the width of the oval to be filled.
     * @param       height the height of the oval to be filled.
     * @see         java.awt.Graphics#drawOval
     */
    public void fillOval(int x, int y, int width, int height){

	fill(new Ellipse2D.Float(x, y, width, height));
    }

    /**
     * Draws the outline of a circular or elliptical arc 
     * covering the specified rectangle.
     * <p>
     * The resulting arc begins at <code>startAngle</code> and extends  
     * for <code>arcAngle</code> degrees, using the current color.
     * Angles are interpreted such that 0&nbsp;degrees 
     * is at the 3&nbsp;o'clock position. 
     * A positive value indicates a counter-clockwise rotation
     * while a negative value indicates a clockwise rotation.
     * <p>
     * The center of the arc is the center of the rectangle whose origin 
     * is (<i>x</i>,&nbsp;<i>y</i>) and whose size is specified by the 
     * <code>width</code> and <code>height</code> arguments. 
     * <p>
     * The resulting arc covers an area 
     * <code>width&nbsp;+&nbsp;1</code> pixels wide
     * by <code>height&nbsp;+&nbsp;1</code> pixels tall.
     * <p>
     * The angles are specified relative to the non-square extents of
     * the bounding rectangle such that 45 degrees always falls on the
     * line from the center of the ellipse to the upper right corner of
     * the bounding rectangle. As a result, if the bounding rectangle is
     * noticeably longer in one axis than the other, the angles to the
     * start and end of the arc segment will be skewed farther along the
     * longer axis of the bounds.
     * @param        x the <i>x</i> coordinate of the 
     *                    upper-left corner of the arc to be drawn.
     * @param        y the <i>y</i>  coordinate of the 
     *                    upper-left corner of the arc to be drawn.
     * @param        width the width of the arc to be drawn.
     * @param        height the height of the arc to be drawn.
     * @param        startAngle the beginning angle.
     * @param        arcAngle the angular extent of the arc, 
     *                    relative to the start angle.
     * @see         java.awt.Graphics#fillArc
     */
    public void drawArc(int x, int y, int width, int height,
			         int startAngle, int arcAngle) {
	draw(new Arc2D.Float(x, y, width, height,
			     startAngle, arcAngle,
			     Arc2D.OPEN));
    }


    /** 
     * Fills a circular or elliptical arc covering the specified rectangle.
     * <p>
     * The resulting arc begins at <code>startAngle</code> and extends  
     * for <code>arcAngle</code> degrees.
     * Angles are interpreted such that 0&nbsp;degrees 
     * is at the 3&nbsp;o'clock position. 
     * A positive value indicates a counter-clockwise rotation
     * while a negative value indicates a clockwise rotation.
     * <p>
     * The center of the arc is the center of the rectangle whose origin 
     * is (<i>x</i>,&nbsp;<i>y</i>) and whose size is specified by the 
     * <code>width</code> and <code>height</code> arguments. 
     * <p>
     * The resulting arc covers an area 
     * <code>width&nbsp;+&nbsp;1</code> pixels wide
     * by <code>height&nbsp;+&nbsp;1</code> pixels tall.
     * <p>
     * The angles are specified relative to the non-square extents of
     * the bounding rectangle such that 45 degrees always falls on the
     * line from the center of the ellipse to the upper right corner of
     * the bounding rectangle. As a result, if the bounding rectangle is
     * noticeably longer in one axis than the other, the angles to the
     * start and end of the arc segment will be skewed farther along the
     * longer axis of the bounds.
     * @param        x the <i>x</i> coordinate of the 
     *                    upper-left corner of the arc to be filled.
     * @param        y the <i>y</i>  coordinate of the 
     *                    upper-left corner of the arc to be filled.
     * @param        width the width of the arc to be filled.
     * @param        height the height of the arc to be filled.
     * @param        startAngle the beginning angle.
     * @param        arcAngle the angular extent of the arc, 
     *                    relative to the start angle.
     * @see         java.awt.Graphics#drawArc
     */
    public void fillArc(int x, int y, int width, int height,
				 int startAngle, int arcAngle) {

	fill(new Arc2D.Float(x, y, width, height,
			     startAngle, arcAngle,
			     Arc2D.PIE));
    }

    /** 
     * Draws a sequence of connected lines defined by 
     * arrays of <i>x</i> and <i>y</i> coordinates. 
     * Each pair of (<i>x</i>,&nbsp;<i>y</i>) coordinates defines a point.
     * The figure is not closed if the first point 
     * differs from the last point.
     * @param       xPoints an array of <i>x</i> points
     * @param       yPoints an array of <i>y</i> points
     * @param       nPoints the total number of points
     * @see         java.awt.Graphics#drawPolygon(int[], int[], int)
     * @since       JDK1.1
     */
    public void drawPolyline(int xPoints[], int yPoints[],
			     int nPoints) {
	float fromX;
	float fromY;
	float toX;
	float toY;

	if (nPoints > 0) {
	    fromX = xPoints[0];
	    fromY = yPoints[0];
	    for(int i = 1; i < nPoints; i++) {
		toX = xPoints[i];
		toY = yPoints[i];
		draw(new Line2D.Float(fromX, fromY, toX, toY));
		fromX = toX;
		fromY = toY;
	    }
	}

    }


    /** 
     * Draws a closed polygon defined by 
     * arrays of <i>x</i> and <i>y</i> coordinates. 
     * Each pair of (<i>x</i>,&nbsp;<i>y</i>) coordinates defines a point.
     * <p>
     * This method draws the polygon defined by <code>nPoint</code> line 
     * segments, where the first <code>nPoint&nbsp;-&nbsp;1</code> 
     * line segments are line segments from 
     * <code>(xPoints[i&nbsp;-&nbsp;1],&nbsp;yPoints[i&nbsp;-&nbsp;1])</code> 
     * to <code>(xPoints[i],&nbsp;yPoints[i])</code>, for 
     * 1&nbsp;&le;&nbsp;<i>i</i>&nbsp;&le;&nbsp;<code>nPoints</code>.  
     * The figure is automatically closed by drawing a line connecting
     * the final point to the first point, if those points are different.
     * @param        xPoints   a an array of <code>x</code> coordinates.
     * @param        yPoints   a an array of <code>y</code> coordinates.
     * @param        nPoints   a the total number of points.
     * @see          java.awt.Graphics#fillPolygon
     * @see          java.awt.Graphics#drawPolyline
     */
    public void drawPolygon(int xPoints[], int yPoints[],
				     int nPoints) {

	draw(new Polygon(xPoints, yPoints, nPoints));
    }

    /** 
     * Draws the outline of a polygon defined by the specified 
     * <code>Polygon</code> object. 
     * @param        p the polygon to draw.
     * @see          java.awt.Graphics#fillPolygon
     * @see          java.awt.Graphics#drawPolyline
     */
    public void drawPolygon(Polygon p) {
	draw(p);
    }

     /** 
     * Fills a closed polygon defined by 
     * arrays of <i>x</i> and <i>y</i> coordinates. 
     * <p>
     * This method draws the polygon defined by <code>nPoint</code> line 
     * segments, where the first <code>nPoint&nbsp;-&nbsp;1</code> 
     * line segments are line segments from 
     * <code>(xPoints[i&nbsp;-&nbsp;1],&nbsp;yPoints[i&nbsp;-&nbsp;1])</code> 
     * to <code>(xPoints[i],&nbsp;yPoints[i])</code>, for 
     * 1&nbsp;&le;&nbsp;<i>i</i>&nbsp;&le;&nbsp;<code>nPoints</code>.  
     * The figure is automatically closed by drawing a line connecting
     * the final point to the first point, if those points are different.
     * <p>
     * The area inside the polygon is defined using an 
     * even-odd fill rule, also known as the alternating rule.
     * @param        xPoints   a an array of <code>x</code> coordinates.
     * @param        yPoints   a an array of <code>y</code> coordinates.
     * @param        nPoints   a the total number of points.
     * @see          java.awt.Graphics#drawPolygon(int[], int[], int)
     */
    public void fillPolygon(int xPoints[], int yPoints[],
			    int nPoints) {

	fill(new Polygon(xPoints, yPoints, nPoints));
    }


    /** 
     * Fills the polygon defined by the specified Polygon object with
     * the graphics context's current color. 
     * <p>
     * The area inside the polygon is defined using an 
     * even-odd fill rule, also known as the alternating rule.
     * @param        p the polygon to fill.
     * @see          java.awt.Graphics#drawPolygon(int[], int[], int)
     */
    public void fillPolygon(Polygon p) {

	fill(p);
    }

    /** 
     * Draws the text given by the specified string, using this 
     * graphics context's current font and color. The baseline of the 
     * first character is at position (<i>x</i>,&nbsp;<i>y</i>) in this 
     * graphics context's coordinate system. 
     * @param       str      the string to be drawn.
     * @param       x        the <i>x</i> coordinate.
     * @param       y        the <i>y</i> coordinate.
     * @see         java.awt.Graphics#drawBytes
     * @see         java.awt.Graphics#drawChars
     * @since       JDK1.0
     */
    public void drawString(String str, int x, int y) {
	drawString(str, (float) x, (float) y);
    }
    public void drawString(String str, float x, float y) {
        if (str.length() == 0) {
            return;
        }
	TextLayout layout =
	    new TextLayout(str, getFont(), getFontRenderContext());
	Shape textShape =
	    layout.getOutline(AffineTransform.getTranslateInstance(x, y));
	fill(textShape);
    }

    /**
     * Draws the text given by the specified iterator, using this 
     * graphics context's current color. The iterator has to specify a font
     * for each character. The baseline of the 
     * first character is at position (<i>x</i>,&nbsp;<i>y</i>) in this 
     * graphics context's coordinate system. 
     * @param       iterator the iterator whose text is to be drawn
     * @param       x        the <i>x</i> coordinate.
     * @param       y        the <i>y</i> coordinate.
     * @see         java.awt.Graphics#drawBytes
     * @see         java.awt.Graphics#drawChars
     */
    public void drawString(AttributedCharacterIterator iterator,
			   int x, int y) {
	drawString(iterator, (float) x, (float) y);
    }
    public void drawString(AttributedCharacterIterator iterator,
			   float x, float y) {
        if (iterator == null) {
            throw 
                new NullPointerException("attributedcharacteriterator is null");
        }
	TextLayout layout =
	    new TextLayout(iterator, getFontRenderContext());
	Shape textShape =
	    layout.getOutline(AffineTransform.getTranslateInstance(x, y));
	fill(textShape);
    }

    /**
     * Draws a GlyphVector.
     * The rendering attributes applied include the clip, transform,
     * paint or color, and composite attributes.  The GlyphVector specifies
     * individual glyphs from a Font.
     * @param g The GlyphVector to be drawn.
     * @param x,y The coordinates where the glyphs should be drawn.
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see #transform
     * @see #setTransform
     * @see #setComposite
     * @see #clip
     * @see #setClip
     */
    public void drawGlyphVector(GlyphVector g,
				float x,
				float y) {
	fill(g.getOutline(x, y));
    }

    /**
     * Strokes the outline of a Shape using the settings of the current
     * graphics state.  The rendering attributes applied include the
     * clip, transform, paint or color, composite and stroke attributes.
     * @param s The shape to be drawn.
     * @see #setStroke
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see #transform
     * @see #setTransform
     * @see #clip
     * @see #setClip
     * @see #setComposite
     */
    public void draw(Shape s) {

	fill(getStroke().createStrokedShape(s));
    }

    /**
     * Fills the interior of a Shape using the settings of the current
     * graphics state. The rendering attributes applied include the
     * clip, transform, paint or color, and composite.
     * @see #setPaint
     * @see java.awt.Graphics#setColor
     * @see #transform
     * @see #setTransform
     * @see #setComposite
     * @see #clip
     * @see #setClip
     */
    public void fill(Shape s) {
	Paint paint = getPaint();

	try {
	    fill(s, (Color) paint);

	/* The PathGraphics class only supports filling with
	 * solid colors and so we do not expect the cast of Paint
	 * to Color to fail. If it does fail then something went
	 * wrong, like the app draw a page with a solid color but
	 * then redrew it with a Gradient.
	 */
	} catch (ClassCastException e) {
	    throw new IllegalArgumentException("Expected a Color instance");
	}
    }

    public void fill(Shape s, Color color) {
	AffineTransform deviceTransform = getTransform();

	if (getClip() != null) {
	    deviceClip(getClip().getPathIterator(deviceTransform));
	}
	deviceFill(s.getPathIterator(deviceTransform), color);
    }

    /**
     * Fill the path defined by <code>pathIter</code>
     * with the specified color.
     * The path is provided in device coordinates.
     */
    protected abstract void deviceFill(PathIterator pathIter, Color color);

    /*
     * Set the clipping path to that defined by
     * the passed in <code>PathIterator</code>.
     */
    protected abstract void deviceClip(PathIterator pathIter);

    /*
     * Draw the outline of the rectangle without using path
     * if supported by platform.
     */
    protected abstract void deviceFrameRect(int x, int y,
					    int width, int height, 
					    Color color);
	
    /*
     * Draw a line without using path if supported by platform.
     */
    protected abstract void deviceDrawLine(int xBegin, int yBegin,
					   int xEnd, int yEnd, Color color);

    /*
     * Fill a rectangle using specified color.
     */
    protected abstract void deviceFillRect(int x, int y,
					   int width, int height, Color color);
}
