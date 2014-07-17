/*
 * @(#)TextLineComponent.java	1.25 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 1998, All Rights Reserved
 *
 */

package sun.awt.font;

import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.geom.Rectangle2D;
import java.awt.font.GlyphJustificationInfo;
import java.awt.font.LineMetrics;

public interface TextLineComponent {
    
    public LineMetrics getLineMetrics();
    public void draw(Graphics2D g2d, float x, float y);
    public Rectangle2D getCharVisualBounds(int index);
    public Rectangle2D getVisualBounds();
    public float getItalicAngle();
    public Shape getOutline(float x, float y);
    
    public int getNumCharacters();

    public float getCharX(int index);
    public float getCharY(int index);
    public float getCharAdvance(int index);
    public boolean caretAtOffsetIsValid(int index);

    // measures characters in context, in logical order
    public int getLineBreakIndex(int start, float width);

    // measures characters in context, in logical order
    public float getAdvanceBetween(int start, int limit);

    public Rectangle2D getLogicalBounds();
    /**
     * Force subset characters to run left-to-right.
     */
    public static final int LEFT_TO_RIGHT = 0;
    /**
     * Force subset characters to run right-to-left.
     */
    public static final int RIGHT_TO_LEFT = 1;
    
    /**
     * Leave subset character direction and ordering unchanged.
     */
    public static final int UNCHANGED = 2;

    /**
     * Return a TextLineComponent for the characters in the range
     * start, limit.  The range is relative to this TextLineComponent
     * (ie, the first character is at 0).
     * @param dir one of the constants LEFT_TO_RIGHT, RIGHT_TO_LEFT, or UNCHANGED
     */
    public TextLineComponent getSubset(int start, int limit, int dir);

    /** 
     * Return the number of justification records this uses.
     */
    public int getNumJustificationInfos();

    /**
     * Return GlyphJustificationInfo objects for the characters between
     * charStart and charLimit, starting at offset infoStart.  Infos
     * will be in visual order.  All positions between infoStart and
     * getNumJustificationInfos will be set.  If a position corresponds
     * to a character outside the provided range, it is set to null.
     */
    public void getJustificationInfos(GlyphJustificationInfo[] infos, int infoStart, int charStart, int charLimit);

    /**
     * Apply deltas to the data in this component, starting at offset 
     * deltaStart, and return the new component.  There are two floats
     * for each justification info, for a total of 2 * getNumJustificationInfos.
     * The first delta is the left adjustment, the second is the right 
     * adjustment.
     * <p>
     * If flags[0] is true on entry, rejustification is allowed.  If
     * the new component requires rejustification (ligatures were 
     * formed or split), flags[0] will be set on exit.
     */
    public TextLineComponent applyJustificationDeltas(float[] deltas, int deltaStart, boolean[] flags);
}
