/*
 * @(#)StandardGlyphVector.java	1.50 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @author Charlton Innovations, Inc.
 */

package sun.awt.font;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.Shape;
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphMetrics;
import java.awt.font.GlyphJustificationInfo;
import java.awt.font.GlyphVector;
import java.awt.geom.AffineTransform;
import java.awt.geom.GeneralPath;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;
import java.awt.geom.Rectangle2D;
import java.lang.ref.SoftReference;
import java.text.CharacterIterator;

/**
*   GlyphVector is a collection of Glyphs containing geometric information
*   for the placement the GlyphVector and of each Glyph in a transformed
*   coordinate space which should correspond to the device on which the
*   GlyphVector is ultimately to be displayed.  The GlyphVector does not
*   attempt any interpretation of the sequence of Glyphs it contains.
*   Relationships between adjacent Glyphs in sequence are solely used
*   in determination of placement of the Glyphs in the visual coordinate
*   space.
*   <p>
*   Instances of GlyphVector are created by a Font.
*   <p>
*   In a text processing application which can cache intermediate
*   representations of text, creation and subsequent caching of a GlyphVector
*   for use during rendering is the fastest method to present the visual
*   representation of characters to a user.
*   <p>
*   A GlyphVector is associated with exactly one Font, and can provide data
*   useful only in relation to the Font specified for the GlyphVector.  In
*   addition, metrics obtained from a GlyphVector are not generally
*   geometrically scaleable since the pixelization and spacing are
*   dependent on grid-fitting algorithms within a Font.  To facilitate
*   accurate measurement of a GlyphVector and its component Glyphs, you must
*   specify a scaling transform, anti-alias mode, and fractional metrics
*   mode at the time the GlyphVector is created.  (These characteristics can
*   be derived from the destination device.)
*   <p>
*   For each Glyph in the GlyphVector, you may obtain:
*       1) the position of the Glyph
*       2) the transform associated with the Glyph
*       3) the metrics of the Glyph in the context of the GlyphVector
*          (the metrics of the Glyph may be different under different
*           transforms, application specified rendering hints, and
*           the specific instance of the Glyph within the GlyphVector)
*   <p>
*   Methods are provided to create new GlyphVectors which are the result of
*   editing editing operations on the GlyphVector such as Glyph insertion and
*   deletion.  These methods are most appropriate to applications which
*   are forming combinations such as ligatures from existing Glyphs or are
*   breaking such combinations into their component parts for visual
*   presentation.
*   <p>
*   Methods are provided to create new GlyphVectors which are the result of
*   specifying new positions for the Glyphs within the GlyphVector.  These
*   methods are most appropriate to applications which are performing
*   justification operations for the presentation of the Glyphs.
*   <p>
*   Methods are provided to return both the visual and logical bounds
*   of the entire GlyphVector or of individual Glyphs within the GlyphVector.
*   <p>
*   Methods are provided to return a Shape for the GlyphVector, and for
*   individual Glyphs within the GlyphVector.
*
*   @see Font
*   @see GlyphMetrics
*   @see TextLayout
*   @version 30 Mar 1998
*   @author Charlton Innovations, Inc.
*/

public class StandardGlyphVector extends GlyphVector {

    private Font font;
    public int [] glyphs;          // name used by native code
    public int [] charIndices;     // name used by native code
    public float [] positions;     // name used by native code
    public double[] transforms;    // name used by native code
    public int [] txIndices;       // name used by native code
    private int flags = UNINITIALIZED_FLAGS;
    private FontRenderContext frc;

    private static final int UNINITIALIZED_FLAGS = -1;

    // transform matrix for: fontSize * fontTX
    public double [] fontTX;       // name used by sun.java2D.TextRendering

    // transform matrix for: frcTX
    public double [] devTX;        // name used by sun.java2D.TextRendering

    private boolean haveLM; // logical metrics, should ref font for these
    private int baseline;
    private float ascent;
    private float descent;
    private float leading;
    private boolean horizontal = true;

    public StandardGlyphVector(Font theFont, String unicodeStr,
                               FontRenderContext frc)
    {
        this.font = theFont;
        this.glyphs = null;
        this.positions = null;
        this.frc = frc;
	initTX();
        NativeFontWrapper.populateGlyphVector(
            font, unicodeStr.toCharArray(), 0, unicodeStr.length(),
            fontTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

    public StandardGlyphVector(Font theFont, char[] unicodes,
                               FontRenderContext frc)
    {
        this.font = theFont;
        this.glyphs = null;
        this.positions = null;
        this.frc = frc;
	initTX();
        NativeFontWrapper.populateGlyphVector(
            font, unicodes, 0, unicodes.length,
            fontTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

  /**
   * Create a StandardGlyphVector from a range of unicodes.
   * @throws IndexOutOfBoundsException if start < 0 or count < 0 or start + count > unicodes.length.
   */
    public StandardGlyphVector(Font theFont, char[] unicodes, int start, int count,
                               FontRenderContext frc)
    {
        if (start < 0 || count < 0 || start + count > unicodes.length) {
            throw new ArrayIndexOutOfBoundsException("start or count out of bounds");
        }
        this.font = theFont;
        this.glyphs = null;
        this.positions = null;
        this.frc = frc;
	initTX();
        NativeFontWrapper.populateGlyphVector(
            font, unicodes, start, count,
            fontTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

    public StandardGlyphVector(Font theFont, char[] unicodes, int start, int count,
                               int flags, FontRenderContext frc)
    {
//         if (start < 0 || count < 0 || start + count > unicodes.length) {
//             throw new ArrayIndexOutOfBoundsException("start or count out of bounds");
//         }
        this.font = theFont;
        this.glyphs = null;
        this.positions = null;
        this.frc = frc;
        initTX();

        NativeFontWrapper.populateAndLayoutGlyphVector(
            font, unicodes, start, count, flags,
            fontTX, devTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

    public StandardGlyphVector(Font theFont, CharacterIterator ciUnicode,
                               FontRenderContext frc)
    {
        char [] unicodes = new char [ciUnicode.getEndIndex()
                                            - ciUnicode.getBeginIndex()];
        for(char c = ciUnicode.first();
                c != CharacterIterator.DONE;
                c = ciUnicode.next()) {
            unicodes[ciUnicode.getIndex() - ciUnicode.getBeginIndex()] = c;
        }
        this.font = theFont;
        this.glyphs = null;
        this.positions = null;
        this.frc = frc;
	initTX();
        NativeFontWrapper.populateGlyphVector(
            font, unicodes, 0, unicodes.length,
            fontTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

    public StandardGlyphVector(Font theFont, int [] glyphCodes,
                               FontRenderContext frc)
    {
        this.font = theFont;
        this.glyphs = glyphCodes;
        this.frc = frc;
	initTX();
    }

    /**
     * If the gv is not a StandardGlyphVector, create a new one and return it.
     * Note that this does _not_ clone the passed-in gv if it is an instance of StandardGlyphVector.
     */
    public static StandardGlyphVector getStandardGV(GlyphVector gv) {
	if (gv instanceof StandardGlyphVector) {
	    return (StandardGlyphVector)gv;
	}
	return new StandardGlyphVector(gv);
    }

    /**
     * Create a new StandardGlyphVector from another (presumably not a StandardGlyphVector).
     */
    public StandardGlyphVector(GlyphVector gv) {
	if (gv instanceof StandardGlyphVector) {
	    StandardGlyphVector rhs = (StandardGlyphVector)gv;

	    this.font = rhs.font;
	    this.frc = rhs.frc;
	    this.glyphs = rhs.glyphs;
	    this.charIndices = rhs.charIndices;
	    
	    // positions, txIndices, transforms are mutable so we have to clone them.
	    if (rhs.positions != null) {
		this.positions = (float[])rhs.positions.clone();
	    }
	    
	    if (rhs.txIndices != null) {
		this.txIndices = (int[])rhs.txIndices.clone();
		this.transforms = (double[])rhs.transforms.clone();
	    }

	    this.initTX();
	    // let remainder default
	} else {
	    this.font = gv.getFont();
	    this.frc = gv.getFontRenderContext();
	
	    int nGlyphs = gv.getNumGlyphs();
	    this.glyphs = gv.getGlyphCodes(0, nGlyphs, null);
	    this.flags = gv.getLayoutFlags() & FLAG_MASK;

	    // should really test flags for this
	    // if ((flags & FLAG_HAS_POSITIONS) != 0) {
	    this.positions = gv.getGlyphPositions(0, nGlyphs + 1, null);
	    // }

	    if ((flags & FLAG_COMPLEX_GLYPHS) != 0) {
		this.charIndices =gv.getGlyphCharIndices(0, nGlyphs, null);
	    }

	    if ((flags & FLAG_HAS_TRANSFORMS) != 0) {
		AffineTransform[] txs = new AffineTransform[nGlyphs]; // worst case
		for (int i = 0; i < nGlyphs; ++i) {
		    txs[i] = gv.getGlyphTransform(i); // gv doesn't have getGlyphsTransforms
		}

		setGlyphTransforms(txs);
	    }

	    this.initTX();
	}
    }

    private void initTX()
    {
        float ptSize = font.getSize2D();
	AffineTransform ftx = font.getTransform();
	ftx.scale(ptSize, ptSize);

	fontTX = new double[6];
	ftx.getMatrix(fontTX);

        AffineTransform dtx = (AffineTransform)frc.getTransform();
	if (!dtx.isIdentity()) {
	    devTX = new double[6];
	    dtx.getMatrix(devTX);

	    // don't use translation components of devtx
	    devTX[4] = devTX[5] = 0;
	}
    }

    private void initLM() {
      // need to record ascent, descent, leading in
      // case they differ for different ranges of text in the font.
      // for now assume fonts only have one value for these that we can use
      // use fontTX transform, assume these metrics should not be influenced by FRC

      if (!haveLM) {
        float [] metrics = new float[4];
        NativeFontWrapper.getFontMetrics(font, fontTX,
                                        frc.isAntiAliased(),
                                        frc.usesFractionalMetrics(),
                                        metrics);
	haveLM = true;

	// baseline = 0;
	// horizontal = true;
	ascent = metrics[0];
	descent = metrics[1];
	leading = metrics[2];
      }
    }

    /*
     * The default layout is primarily useful for simple text.  It positions
     * glyphs in order according to the advance vector of each glyph, including
     * glyph transforms.
     */
    public void performDefaultLayout()
    {
	clearCaches();
        NativeFontWrapper.layoutGlyphVector(
            font, 
	    fontTX,
	    devTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics(),
            this);
    }

    // methods associated with creation-time state
    /**
    *   Return the Font associated with this GlyphVector.
    *   @return Font used to create this GlyphVector
    *   @see Font
    */
    public Font getFont()
    {
        return this.font;
    }

    public FontRenderContext getFontRenderContext()
    {
        return this.frc;
    }

    // methods associated with the GlyphVector as a whole
    /**
    *   Return the number of Glyphs in this GlyphVector, you should
    *   Use this information to create arrays which are to be
    *   filled with results of other information retrieval
    *   operations.
    *   @return number of Glyphs in this GlyphVector
    */
    public int getNumGlyphs()
    {
        return glyphs.length;
    }

    /**
    *   Return the glyphcode of the specified Glyph.
    *   This return value is meaningless to anything other
    *   than a Font and can be used to ask the Font
    *   about the existing of ligatures and other context
    *   sensitive information.
    *   @see Font#getLigature
    */
    public int getGlyphCode(int glyphIndex)
    {
        return glyphs[glyphIndex];
    }

    /**
    *   Return an array of glyphcodes for the the specified Glyphs.
    *   The contents of this return value are meaningless to anything other
    *   than a Font and can be used to ask the Font about the existence of
    *   ligatures and other context sensitive information.  Use this method
    *   for convenience and performance in processing of glyphcodes.
    *   If no array is passed in, one will be created for you.
    *   @see Font#getLigature
    */
    public int[] getGlyphCodes(int beginGlyphIndex, int numEntries,
                               int[] codeReturn)
    {
        if (numEntries < 0) {
            throw new IllegalArgumentException("numEntries = " + numEntries);
        }
        if (beginGlyphIndex < 0) {
            throw new IndexOutOfBoundsException("beginGlyphIndex = " + beginGlyphIndex);
        }
        if (beginGlyphIndex > glyphs.length - numEntries) { // watch out for overflow if index + count overlarge
            throw new IndexOutOfBoundsException("beginGlyphIndex + numEntries = " + (beginGlyphIndex + numEntries));
        }

        int [] retval = codeReturn;
        if (retval == null) {
            retval = new int[numEntries];
        }
        for (int i=0; i < numEntries; i++) {
            retval[i] = glyphs[i + beginGlyphIndex];
        }

        return retval;
    }

    /**
    *   Return the character index of the specified Glyph.
    *   The character index shows where the glyph was in the
    *   character string before any rearrangement or reordering
    *   took place.
    */
    public int getGlyphCharIndex(int glyphIndex)
    {
        return charIndices == null ? glyphIndex : charIndices[glyphIndex];
    }

    /**
    *   Return the character index of the specified Glyph.
    *   The character index shows where the glyph was in the
    *   character string before any rearrangement or reordering
    *   took place.  Use this method for convenience and performance
    *   in processing of glyphcodes. If no array is passed in,
    *   one will be created for you.
    */
    public int[] getGlyphCharIndices(int beginGlyphIndex, int numEntries,
                                int[] codeReturn)
    {
        int [] retval = codeReturn;
        if (retval == null) {
            retval = new int[numEntries];
        }
        for (int i=0; i < numEntries; i++) {
	    int n = i + beginGlyphIndex;
            retval[i] = charIndices == null ? n : charIndices[n];
        }
        return retval;
    }

    // methods associated with the GlyphVector as a whole,
    // after layout.
    /**
     *   Return the logical bounds of this GlyphVector.
     *   Use when positioning this GlyphVector in relation
     *   to visually adjacent GlyphVectors.
     */
    public Rectangle2D getLogicalBounds()
    {
        initLM();

	float minX, minY, maxX, maxY;

	if (horizontal) {
	  minX = 0;
	  minY = -ascent;
	  maxX = 0;
	  maxY = descent + leading;

	  if (glyphs.length > 0) {
	    initPositions();
	    maxX = positions[positions.length - 2];
	  }
	} else {
	  minY = 0;
	  maxX = ascent;
	  maxY = 0;
	  minX = - descent - leading;

	  if (glyphs.length > 0) {
	    initPositions();
	    maxY = positions[positions.length - 1];
	  }
	}
        
        return new Rectangle2D.Float(minX, minY, maxX - minX, maxY - minY);     
    }

    /**
     * Return the bounding box of the outline of the glyphvector.
     * This is not guaranteed to bound all the pixels rendered by the glyphvector.
     */
    public Rectangle2D getVisualBounds()
    {
      initPositions();
      return NativeFontWrapper.getVisualBounds(
            font, 
	    glyphs,
	    positions,
	    transforms,
	    txIndices,
	    fontTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics());
    }

    /*
     * This returns the pixel bounds of the glyphvector when rendered in the given
     * frc at the given location.  The renderFRC need not be the same as the
     * glyphvector frc, and may be null.  If it is null the glyphvector frc is used.
     */
    public Rectangle getPixelBounds(FontRenderContext renderFRC, float x, float y) {

      double[] renTX;

      if (renderFRC == null) {
	renderFRC = frc;
	renTX = devTX;
      } else {
	  renTX = new double[6];
	  AffineTransform renTrans = renderFRC.getTransform();
	  renTrans.getMatrix(renTX);
      }

      initPositions();
      return NativeFontWrapper.getPixelBounds(
            font, 
	    x, y,
	    glyphs,
	    positions,
	    transforms,
	    txIndices,
	    fontTX,
	    renTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics());
    }

    /*
     * This returns the pixel bounds of the glyph at index when rendered in the given
     * frc at the given location.  The renderFRC need not be the same as the
     * glyphvector frc, and may be null.  If it is null the glyphvector frc is used.
     */
    public Rectangle getGlyphPixelBounds(int index, FontRenderContext renderFRC, float x, float y) {
      
      double[] renTX;
      if (renderFRC == null) {
	renderFRC = frc;
	renTX = devTX;
      } else {
	  renTX = new double[6];
	  AffineTransform renTrans = renderFRC.getTransform();
	  renTrans.getMatrix(renTX);
      }

      initPositions();
      return NativeFontWrapper.getGlyphPixelBounds(
            font,
	    index,
            x, y,
            glyphs,
	    positions,
	    transforms,
	    txIndices,
	    fontTX,
	    renTX,
            frc.isAntiAliased(),
            frc.usesFractionalMetrics());
    }

    public void pixellate(FontRenderContext renderFRC, Point2D loc, Point pxResult) {
      if (renderFRC == null) {
	renderFRC = frc;
      }

      // it is a total pain that you have to copy the transform.

      AffineTransform at = renderFRC.getTransform();
      at.transform(loc, loc);
      pxResult.x = (int)loc.getX(); // but must not behave oddly around zero
      pxResult.y = (int)loc.getY();
      loc.setLocation(pxResult.x, pxResult.y);
      try {
	at.inverseTransform(loc, loc);
      }
      catch (NoninvertibleTransformException e) {
	throw new IllegalArgumentException("must be able to invert frc transform");
      }
    }

    /**
    *   Return a Shape whose interior corresponds to the
    *   visual representation of the GlyphVector.
    */
    public Shape getOutline() {
        return getOutline(0, 0);
    }

    /**
    *   Return a Shape whose interior corresponds to the
    *   visual representation of the GlyphVector. Offset
    *   to x, y
    */
    public Shape getOutline(float x, float y)
    {
	initPositions();
        return NativeFontWrapper.getGlyphVectorOutline(this, font, 
						       fontTX, devTX,
						       frc.isAntiAliased(),
						       frc.usesFractionalMetrics(),
						       x, y);
    }

    /**
    *   Return a Shape whose interior corresponds to the
    *   visual representation of the specified glyph
    *   within the GlyphVector.
    */
  public Shape getGlyphOutline(int glyphIndex) {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	initPositions();
	return NativeFontWrapper.getGlyphOutline(this, glyphIndex, font, 
						 fontTX, devTX, 
						 frc.isAntiAliased(),
						 frc.usesFractionalMetrics(),
						 0, 0);
  }

    /**
     * Return a Shape whose interior corresponds to the
     * visual representation of the specified glyph
     * within the GlyphVector when the glyphvector is rendered
     * at the given position.
     */
    public Shape getGlyphOutline(int glyphIndex, float x, float y) {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	initPositions();
	return NativeFontWrapper.getGlyphOutline(this, glyphIndex, font, 
						 fontTX, devTX, 
						 frc.isAntiAliased(),
						 frc.usesFractionalMetrics(),
						 x, y);
    }

    /**
    *   Return the position of the specified glyph within the GlyphVector.
    *   This position corresponds to the leading edge of the baseline for
    *   the glyph.
    */
    public Point2D getGlyphPosition(int glyphIndex)
    {
      initPositions();
      return new Point2D.Float(positions[glyphIndex * 2],
                               positions[glyphIndex * 2 + 1]);
    }

  
    /**
    *   Set the position of the specified glyph within the GlyphVector.
    *   This position corresponds to the leading edge of the baseline for
    *   the glyph.
    */
    public void setGlyphPosition(int glyphIndex, Point2D position)
    {
        initPositions();
        positions[glyphIndex * 2] = (float)position.getX();
        positions[glyphIndex * 2 + 1] = (float)position.getY();

	clearCaches(glyphIndex);

	addFlags(FLAG_HAS_POSITION_ADJUSTMENTS);
    }

    /**
    *   Ensure that positions vector is initialized
    */
    private void initPositions()
    {
        if (positions == null) {
            performDefaultLayout();
        }
    }

    /**
    *   Return an array of glyph positions for the the specified Glyphs.
    *   The position of each glyph corresponds to the leading edge of the
    *   baseline for that glyph.
    *   Use this method for convenience and performance in processing of glyph
    *   positions.
    *   If no array is passed in, one will be created for you.
    *   Even numbered array entries beginning with position zero are the X
    *   coordinates of the glyph numbered beginGlyphIndex + position/2
    *   Odd numbered array entries beginning with position one are the Y
    *   coordinates of the glyph numbered beginGlyphIndex + (position-1)/2
    */
    public float[] getGlyphPositions(int beginGlyphIndex, int numEntries,
                                              float[] positionReturn)
    {
        if (numEntries < 0) {
            throw new IllegalArgumentException("numEntries = " + numEntries);
        }
        if (beginGlyphIndex < 0) {
            throw new IndexOutOfBoundsException("beginGlyphIndex = " + beginGlyphIndex);
        }
        if (beginGlyphIndex > glyphs.length + 1 - numEntries) { // watch for overflow
            throw new IndexOutOfBoundsException("beginGlyphIndex + numEntries = " + (beginGlyphIndex + numEntries));
        }

        return internalGetGlyphPositions(beginGlyphIndex, numEntries, positionReturn);
    }

    /**
     * Return the positions of all the glyphs, plus the position at the end
     * of the glyph vector.  The length of the returned array is
     * <code>getNumGlyphs() * 2 + 2</code>.
     */
    public float[] getGlyphPositions(float[] positionReturn) {
        return internalGetGlyphPositions(0, glyphs.length + 1, positionReturn);
    }

    float[] internalGetGlyphPositions(int beginGlyphIndex, int numEntries, float[] positionReturn) {
        float[] retval = positionReturn;
        if (retval == null) {
            retval = new float[numEntries * 2];
        }

        initPositions();
        for (int i=0; i < numEntries; i++) {
            retval[i*2] = positions[ (i + beginGlyphIndex) * 2];
            retval[i*2+1] = positions[ (i + beginGlyphIndex) * 2 + 1];
        }

        return retval;
    }

    /**
    *   Set some or all of the positions of each glyph.
    *   The position of each glyph corresponds to the leading edge of the
    *   baseline for that glyph.
    *   Use this method for convenience and performance in processing of glyph
    *   positions.
    *   Even numbered array entries beginning with position zero are the X
    *   coordinates of the glyph numbered beginGlyphIndex + position/2
    *   Odd numbered array entries beginning with position one are the Y
    *   coordinates of the glyph numbered beginGlyphIndex + (position-1)/2
    */
    public void setGlyphPositions(float[] srcPositions, int srcStart,
                                  int beginGlyphIndex, int numEntries)
    {
        if (numEntries < 0) {
	  throw new IllegalArgumentException("numEntries = " + numEntries);
        }

	clearCaches();

        initPositions();
        System.arraycopy(srcPositions, srcStart, positions, beginGlyphIndex * 2, numEntries * 2);
    }

    /**
     * Set the positions of the glyphvector to those in the provided array.
     * The array must contain exactly <code>getNumGlyphs() * 2 + 2</code> values.
     */
    public void setGlyphPositions(float[] srcPositions) {
        int requiredLength = glyphs.length * 2 + 2;
        if (srcPositions.length != requiredLength) {
            throw new IllegalArgumentException("srcPositions.length != " + requiredLength);
        }

	clearCaches();

        if (positions == null) {
            positions = (float[])srcPositions.clone();
        } else {
            System.arraycopy(srcPositions, 0, positions, 0, requiredLength);
        }
    }

    /**
     * The logical bounds is a rectangular shape.  It extends from the ascent to the 
     * descent + leading, and from the origin of the glyph to the origin + advance.
     * If the glyph has a transform, then this shape is passed through the
     * transform before being returned.
     *
     * @see #getGlyphVisualBounds
     */
    public Shape getGlyphLogicalBounds(int glyphIndex)
    {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
          throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	Shape[] lbcache;
	if (lbcacheRef == null || (lbcache = (Shape[])lbcacheRef.get()) == null) {
	    lbcache = new Shape[glyphs.length];
	    lbcacheRef = new SoftReference(lbcache);
	}

	Shape result = lbcache[glyphIndex];
	if (result == null) {
	    initPositions();
	    result = NativeFontWrapper.getGlyphLogicalBounds(
                 font, 
		 glyphIndex,
		 glyphs,
		 positions,
		 transforms,
		 txIndices,
		 fontTX,
		 frc.isAntiAliased(),
		 frc.usesFractionalMetrics());

      	    lbcache[glyphIndex] = result;
	}

	return result;
    }
    private SoftReference lbcacheRef;

    /**
     * The visual bounds is a rectangular shape.  It is the bounding box
     * of the outline of the glyph.  If the glyph has a transform, then this
     * shape is passed through the transform before being returned.
     * 
     * @see #getGlyphLogicalBounds
    */
    public Shape getGlyphVisualBounds(int glyphIndex)
    {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	Shape[] vbcache;
	if (vbcacheRef == null || (vbcache = (Shape[])vbcacheRef.get()) == null) {
	    vbcache = new Shape[glyphs.length];
	    vbcacheRef = new SoftReference(vbcache);
	}

	Shape result = vbcache[glyphIndex];
	if (result == null) {
	    initPositions();
	    result = NativeFontWrapper.getGlyphVisualBounds(
                font, 
		glyphIndex,
		glyphs,
		positions,
		transforms,
		txIndices,
		fontTX,
		frc.isAntiAliased(),
		frc.usesFractionalMetrics());

      	    vbcache[glyphIndex] = result;
	}

	return result;
    }
    private SoftReference vbcacheRef;

    private void clearCaches(int glyphIndex) {
	if (lbcacheRef != null) {
	   Shape[] lbcache = (Shape[])lbcacheRef.get();
	   if (lbcache != null) {
	       lbcache[glyphIndex] = null;
	   }
	}

	if (vbcacheRef != null) {
	    Shape[] vbcache = (Shape[])vbcacheRef.get();
	    if (vbcache != null) {
		vbcache[glyphIndex] = null;
	    }
	}
    }

    private void clearCaches() {
	lbcacheRef = null;
	vbcacheRef = null;
    }

    /*
     * The glyph metrics does not take into account the glyph transform.
     *
     * The bounds is the bounds of the glyph outline relative to the glyph
     * origin (e.g., it generally includes the point 0,0).  The advance 
     * is the projection of the advance vector of the glyph onto the
     * baseline.
     */
    public GlyphMetrics getGlyphMetrics(int glyphIndex)
    {

      // forget pixels, use the bounding box of the outline instead.

        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	float[] result = new float[6]; 
        NativeFontWrapper.getGlyphMetrics(
                            font,
                            glyphs[glyphIndex], 
			    fontTX, devTX,
                            frc.isAntiAliased(),
                            frc.usesFractionalMetrics(),
                            result);

	float advx = result[0];
	float advy = result[1];
	float l = result[2];
	float t = result[3];
	float w = result[4];
	float h = result[5];

	// !!! just use projection of advance onto baseline
	// force glyph type for now until we get real info

        GlyphMetrics gm = new GlyphMetrics(horizontal, advx, advy,
                                           new Rectangle2D.Float(l, t, w, h),
                                           GlyphMetrics.STANDARD);

        return gm;
    }


    /**
    *   Get the transform of the specified glyph within the GlyphVector.
    */
    public AffineTransform getGlyphTransform(int glyphIndex) {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	if (txIndices == null || txIndices[glyphIndex] == 0) {
	    return new AffineTransform();
	}

	int x = (txIndices[glyphIndex] - 1) * 6;
	return new AffineTransform(transforms[x + 0],
				   transforms[x + 1],
				   transforms[x + 2],
				   transforms[x + 3],
				   transforms[x + 4],
				   transforms[x + 5]);
    }

    /**
    *   Set the transform of the specified glyph within the GlyphVector.
    *   This can be used to rotate, mirror, translate and scale the
    *   glyph.  Adding a transform can result in signifant performance changes
    */
    public void setGlyphTransform(int glyphIndex, AffineTransform newTX) {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

	clearCaches(glyphIndex);

	// we store all the glyph transforms as a double array, and for each glyph there
	// is an entry in the txIndices array indicating which transform to use.  0 means
	// there's no transform, 1 means use the first transform (the 6 doubles at offset
	// 0), 2 means use the second transform (the 6 doubles at offset 6), etc.
	// 
	// Since this can be called multiple times, and since the number of transforms
	// affects the time it takes to construct the glyphs, we try to keep the arrays as
	// compact as possible, by removing transforms that are no longer used, and reusing
	// transforms where we already have them.

	double[] temp = new double[6];
        boolean isIdentity = true;
        if (newTX == null) {
            // Fill in temp
            temp[0] = temp[3] = 1.0;
        }
        else {
            isIdentity = newTX.isIdentity();
            newTX.getMatrix(temp);
        }

	if (txIndices == null) {
	    if (isIdentity) { // ignore
		return;
	    }
	
	    txIndices = new int[glyphs.length];
	    txIndices[glyphIndex] = 1;
	    transforms = temp;
	} else {
	    boolean addSlot = false;
	    int newIndex = -1;
	    if (isIdentity) {
		newIndex = 0;
	    } else {
		addSlot = true;
		int i;
	        loop: for (i = 0; i < transforms.length; i += 6) {
		    for (int j = 0; j < 6; ++j) {
			if (transforms[i + j] != temp[j]) {
			    continue loop;
			}
		    }
		    addSlot = false;
		    break;
		}
		newIndex = i / 6 + 1;
	    }

	    int oldIndex = txIndices[glyphIndex];

	    if (newIndex != oldIndex) { // otherwise nothing to do

		boolean removeSlot = false;
		if (oldIndex != 0) { // see if we are removing last use of this slot
		    removeSlot = true;
		    int slot = txIndices[glyphIndex];
		    for (int i = 0; i < txIndices.length; ++i) {
			if (i != glyphIndex && txIndices[i] == slot) {
			    removeSlot = false;
			    break;
			}
		    }
		}
			
		if (removeSlot && addSlot) { // reuse slot with new transform
		    System.arraycopy(temp, 0, transforms, (newIndex - 1) * 6, 6);
		} else if (removeSlot) {
		    if (transforms.length == 6) { // removing last one, so clear arrays
			transforms = null;
			txIndices = null;

			clearFlags(FLAG_HAS_TRANSFORMS);

			return;
		    }
		    double[] ttemp = new double[transforms.length - 6];
		    System.arraycopy(transforms, 0, ttemp, 0, (oldIndex - 1) * 6);
		    System.arraycopy(transforms, oldIndex * 6, ttemp, (oldIndex - 1) * 6, transforms.length - oldIndex * 6);
		    transforms = ttemp;

		    // clean up indices
		    for (int i = 0; i < txIndices.length; ++i) {
			if (txIndices[i] > oldIndex) { // ignore == oldIndex, it's going away
			    txIndices[i] -= 1;
			}
		    }
		} else if (addSlot) {
		    double[] ttemp = new double[transforms.length + 6];
		    System.arraycopy(transforms, 0, ttemp, 0, transforms.length);
		    System.arraycopy(temp, 0, ttemp, transforms.length, 6);
		    transforms = ttemp;
		}
		    
		txIndices[glyphIndex] = newIndex;
	    }
	}

	addFlags(FLAG_HAS_TRANSFORMS);
    }

    /**
     * Set transform information for the indicated glyphs.  Null means identity transform.
     */
    public void setGlyphTransforms(AffineTransform[] srcTransforms, int srcStart, int beginGlyphIndex, int count) {
	// dumb implementation for now, but ensures we share transforms, check for identity, etc.
	for (int i = beginGlyphIndex, e = beginGlyphIndex + count; i < e; ++i) {
	    setGlyphTransform(i, srcTransforms[srcStart + i]);
	}
    }

    /**
     * Set transform information for all glyphs.  The srcTransforms array must have numGlyphs transforms.
     */
    public void setGlyphTransforms(AffineTransform[] srcTransforms) {
	setGlyphTransforms(srcTransforms, 0, 0, glyphs.length);
    }

    /**
     * Get transform information for the requested range of glyphs.  If no glyphs have a transform, return null.
     */
    public AffineTransform[] getGlyphTransforms(int beginGlyphIndex, int count, AffineTransform[] transformReturn) {
	// follows existing inconvenient api that assumes start of returned array passed in
	if (txIndices != null) {
	    boolean nonull = true;

	    if (beginGlyphIndex != 0 || count != glyphs.length) {
		nonull = false;
		for (int i = beginGlyphIndex, e = beginGlyphIndex + count; i < e; ++i) {
		    if (txIndices[i] != 0) {
			nonull = true;
			break;
		    }
		}
	    }

	    if (nonull) {
		if (transformReturn == null) {
		    transformReturn = new AffineTransform[count];
		}

		for (int i = beginGlyphIndex, e = beginGlyphIndex + count; i < e; ++i) {
		    if (transformReturn[i] == null) {
			int txi = txIndices[i];
			if (txi != 0) {
			    txi = txi * 6 - 6;
			    AffineTransform at = new AffineTransform(transforms[txi],
								     transforms[txi+1],
								     transforms[txi+2],
								     transforms[txi+3],
								     transforms[txi+4],
								     transforms[txi+5]);
			    for (int j = i; j < e; ++j) {
				if (txIndices[j] == txi) {
				    transformReturn[j] = at;
				}
			    }
			}
		    }
		}

		return transformReturn;
	    }
	}

	return null;
    }

    /**
     * Get transform information for all glyphs.  If no glyphs have a transform, return null.  Otherwise return
     * an array of transforms, one per glyph.  Glyphs might share transforms, so different entries in the array
     * might point to the same AffineTransform object.  Null entries in the array indicate that the corresponding
     * glyph has no transform (that is, has the identity transform).
     */
    public AffineTransform[] getGlyphTransforms() {
	return getGlyphTransforms(0, glyphs.length, null);
    }

    // internal use only for possible future extension

    /**
     * A flag used with getLayoutFlags that indicates whether this <code>GlyphVector</code> uses
     * a vertical baseline.
     */
    public static final int FLAG_USES_VERTICAL_BASELINE = 128;

    /**
     * A flag used with getLayoutFlags that indicates whether this <code>GlyphVector</code> uses
     * vertical glyph metrics.  A <code>GlyphVector</code> can use vertical metrics on a
     * horizontal line, or vice versa.
     */
    public static final int FLAG_USES_VERTICAL_METRICS = 256;

    /**
     * A flag used with getLayoutFlags that indicates whether this <code>GlyphVector</code> uses
     * the 'alternate orientation.'  Glyphs have a default orientation given a 
     * particular baseline and metrics orientation, this is the orientation appropriate
     * for left-to-right text.  For example, the letter 'A' can have four orientations, 
     * with the point at 12, 3, 6, or 9 'o clock.  The following table shows where the
     * point displays for different values of vertical baseline (vb), vertical 
     * metrics (vm) and alternate orientation (fo):<br>
     * <blockquote>
     * vb vm ao
     * -- -- --  --
     *  f  f  f  12   ^  horizontal metrics on horizontal lines
     *  f  f  t   6   v 
     *  f  t  f   9   <  vertical metrics on horizontal lines
     *  f  t  t   3   >
     *  t  f  f   3   >  horizontal metrics on vertical lines
     *  t  f  t   9   <
     *  t  t  f  12   ^  vertical metrics on vertical lines
     *  t  t  t   6   v
     * </blockquote>
     */
    public static final int FLAG_USES_ALTERNATE_ORIENTATION = 512;


    public int getLayoutFlags() {
	if (flags == UNINITIALIZED_FLAGS) {
	    // default java initialization until we move this into native
	    flags = 0;

	    if (charIndices != null) {
		boolean ltr = true;
		boolean rtl = true;

		int rtlix = charIndices.length; // rtl index
		for (int i = 0; i < charIndices.length && (ltr || rtl); ++i) {
		    int cx = charIndices[i];

		    ltr = ltr && (cx == i);
		    rtl = rtl && (cx == --rtlix);
		}

		if (rtl) flags |= FLAG_RUN_RTL;
		if (!rtl && !ltr) flags |= FLAG_COMPLEX_GLYPHS;
	    }
	}
	    
	return flags;
    }
    
    private void addFlags(int newflags) {
	flags = getLayoutFlags() | newflags;
    }

    private void clearFlags(int clearedFlags) {
	flags = getLayoutFlags() & ~clearedFlags;
    }

    public Object clone() {
        // positions, txIndices, transforms are mutable so we have to clone them.
        try {
            StandardGlyphVector result = (StandardGlyphVector)super.clone();

	    result.clearCaches();

            if (positions != null) {
              result.positions = (float[])positions.clone();
            }
	    if (txIndices != null) {
		result.txIndices = (int[])txIndices.clone();
		result.transforms = (double[])transforms.clone();
	    }
	    return result;
        }
        catch (CloneNotSupportedException e) {
        }

        return this;
    }

  /**
   * For each glyph return posx, posy, advx, advy, visx, visy, visw, vish.
   */
    public float[] getGlyphInfo() {

	return NativeFontWrapper.getGlyphInfo(font, glyphs, positions, transforms, txIndices,
				       fontTX, devTX,
				       frc.isAntiAliased(), frc.usesFractionalMetrics());

    }

    public GlyphJustificationInfo getGlyphJustificationInfo(int glyphIndex) {
        if (glyphIndex < 0 || glyphIndex >= glyphs.length) {
            throw new IndexOutOfBoundsException("glyphIndex = " + glyphIndex);
        }

        // REMIND: finish me
        return null;
    }

    // general utility methods

    /**
     * Hashcode
     */
    public int hashCode() {
      return font.hashCode() ^ glyphs.length;
    }

    /**
     * Test for complete equality.
     */
    public boolean equals(GlyphVector set) {
      if (this == set) {
	return true;
      }
      if (set == null) {
	return false;
      }
      try {
	StandardGlyphVector other = (StandardGlyphVector)set;

	if (glyphs.length != other.glyphs.length) {
	  return false;
	}

	for (int i = 0; i < glyphs.length; ++i) {
	  if (glyphs[i] != other.glyphs[i]) {
	    return false;
	  }
	}

	if (!font.equals(other.font)) {
	  return false;
	}

	if (!frc.equals(other.frc)) {
	  return false;
	}

	if ((other.positions == null) != (positions == null)) {
	  if (positions == null) {
	    performDefaultLayout();
	  } else {
	    other.performDefaultLayout();
	  }
	}

	if (positions != null) {
	  for (int i = 0; i < positions.length; ++i) {
	    if (positions[i] != other.positions[i]) {
	      return false;
	    }
	  }
	}

	// the glyph transforms may have been set in a different order, but
	// still be the same.  So we need to compare each transform, not merely
	// the indices.  But since we take care to null the arrays if the
	// transforms are all identity, and only add transforms if they are
	// unique and not identity, we can still optimize some tests.

	if ((txIndices == null) != (other.txIndices == null)) {
	    return false;
	} else if (txIndices != null) {
	    if (transforms.length != other.transforms.length) { // if different # of unique transforms, one must differ
		return false;
	    }
	    for (int i = 0; i < glyphs.length; ++i) {
		int tx = txIndices[i];
		int ox = other.txIndices[i];
		if ((tx == 0) != (ox == 0)) { // value of zero means identity tx
		    return false;
		}
		if (tx != 0) { // must compare tx, not just index
		    tx = (tx - 1) * 6;
		    ox = (ox - 1) * 6;
		    for (int j = 0; j < 6; ++j) {
			if (transforms[tx++] != other.transforms[ox++]) {
			    return false;
			}
		    }
		}
	    }
	}
      }
      catch (ClassCastException e) {
	// assume they are different simply by virtue of the class difference

	return false;
      }

      return true;
    }
}
