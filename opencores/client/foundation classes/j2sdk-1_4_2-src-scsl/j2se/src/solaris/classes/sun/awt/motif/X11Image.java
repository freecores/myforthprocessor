/*
 * @(#)X11Image.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ImageProducer;

import sun.awt.image.ImageRepresentation;

public class X11Image extends sun.awt.image.Image {
    /**
     * Construct an image from an ImageProducer object.
     */
    public X11Image(ImageProducer producer) {
	super(producer);
    }

    public Graphics getGraphics() {
	throw new IllegalAccessError("getGraphics() only valid for images " +
                                     "created with createImage(w, h)");
    }

    protected ImageRepresentation makeImageRep() {
	return new X11ImageRepresentation(this, ColorModel.getRGBdefault(),
                                          false);
    }
}
