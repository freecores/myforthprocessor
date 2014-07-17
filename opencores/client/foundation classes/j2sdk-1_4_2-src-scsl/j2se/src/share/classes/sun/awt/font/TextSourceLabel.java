/*
 * @(#)TextSourceLabel.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * @(#)TextSourceLabel.java	1.8 00/03/15
 *
 * (C) Copyright IBM Corp. 1998, 1999 - All Rights Reserved
 */

package sun.awt.font;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Shape;

import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;

import java.awt.geom.Rectangle2D;

/**
 * Implementation of TextLabel based on String.
 */

public class TextSourceLabel extends TextLabel {
  TextSource source;

  // caches
  Rectangle2D lb;
  Rectangle2D ab;
  Rectangle2D vb;
  GlyphVector gv;

  public TextSourceLabel(TextSource source) {
    this(source, null, null, null);
  }

  public TextSourceLabel(TextSource source, Rectangle2D lb, Rectangle2D ab, GlyphVector gv) {
    this.source = source;

    this.lb = lb;
    this.ab = ab;
    this.gv = gv;
  }

  public TextSource getSource() {
    return source;
  }

  public final Rectangle2D getLogicalBounds(float x, float y) {
    if (lb == null) {
      lb = createLogicalBounds();
    }
    return new Rectangle2D.Float((float)(lb.getX() + x), 
				 (float)(lb.getY() + y), 
				 (float)lb.getWidth(), 
				 (float)lb.getHeight());
  }

  public final Rectangle2D getVisualBounds(float x, float y) {
    if (vb == null) {
      vb = createVisualBounds();
    }
    return new Rectangle2D.Float((float)(vb.getX() + x), 
				 (float)(vb.getY() + y), 
				 (float)vb.getWidth(), 
				 (float)vb.getHeight());
  }

  public final Rectangle2D getAlignBounds(float x, float y) {
    if (ab == null) {
      ab = createAlignBounds();
    }
    return new Rectangle2D.Float((float)(ab.getX() + x), 
				 (float)(ab.getY() + y), 
				 (float)ab.getWidth(),
				 (float)ab.getHeight());
  }

  public Shape getOutline(float x, float y) {
    return getGV().getOutline(x, y);
  }

  public void draw(Graphics2D g, float x, float y) {
    g.drawGlyphVector(getGV(), x, y);
  }

  protected Rectangle2D createLogicalBounds() {
    return getGV().getLogicalBounds();
  }

  protected Rectangle2D createVisualBounds() {
    return getGV().getVisualBounds();
  }

  protected Rectangle2D createAlignBounds() {
    return createLogicalBounds();
  }

  private final GlyphVector getGV() {
    if (gv == null) {
      gv = createGV();
    }

    return gv;
  }

  protected GlyphVector createGV() {
    Font f = source.getFont();
    FontRenderContext frc = source.getFRC();
    int flags = source.getLayoutFlags();
    char[] context = source.getChars();
    int start = source.getStart();
    int length = source.getLength();
    StandardGlyphVector gv;

    gv = new StandardGlyphVector(f, context, start, length, flags, frc);

    return gv;
  }
}
