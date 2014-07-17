/*
 * @(#)OffScreenImage.java	1.31 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Component;
import java.awt.Color;
import java.awt.SystemColor;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.ImageProducer;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.WritableRaster;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;

public class OffScreenImage extends BufferedImage {
    protected Component c;
    OffScreenImageSource osis;
    protected SurfaceData surfaceData;

    /**
     * Constructs an OffScreenImage with the default image type,
     * TYPE_4BYTE_ABGR, for offscreen rendering to be used with a
     * given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     * REMIND:  At some point, we might want to look at the component
     * and figure out the best image type to construct.
     */
    public OffScreenImage(Component c, int width, int height) {
        // REMIND:  Should get image type from the component.getGC
        super(width, height, TYPE_4BYTE_ABGR);
        this.c = c;
	initBackground(width, height);
    }

    /**
     * Constructs an OffScreenImage with the given image type,
     * for offscreen rendering to be used with a given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     */
    public OffScreenImage(Component c, int width, int height,
                             int imageType)
    {
        super (width, height, imageType);
        this.c = c;
	initBackground(width, height);
    }

    /**
     * Constructs an OffScreenImage given a color model and tile,
     * for offscreen rendering to be used with a given component.
     * The component is used to obtain the foreground color, background
     * color and font.
     */
    public OffScreenImage(Component c, ColorModel cm, WritableRaster raster,
                            boolean isRasterPremultiplied)
    {
        super(cm, raster, isRasterPremultiplied, null);
        this.c = c;
	initBackground(raster.getWidth(), raster.getHeight());
    }

    public Graphics getGraphics() {
        return createGraphics();
    }

    public Graphics2D createGraphics() {
        Color bg = c.getBackground();
        if (bg == null) {
            bg = SystemColor.window;
        }
        Color fg = c.getForeground();
        if (fg == null) {
            fg = SystemColor.windowText;
        }
        Font font = c.getFont();
        if (font == null) {
            font = new Font("Dialog", Font.PLAIN, 12);
        }
        return new SunGraphics2D(surfaceData, fg, bg, font);
    }

    public SurfaceData getSourceSurfaceData(SurfaceData destSD,
					    CompositeType comp,
					    Color bgColor, boolean scale) {
	return surfaceData;
    }

    public SurfaceData restoreContents() {
	// OffScreenImages should never lost their surfaceData objects,
	// so simply return the original sd.  Subclasses (e.g., 
	// Win32OffScreenImage) may override this method to do something
	// different (such as restoring a lost hardware-accelerated 
	// surfaceData).
	return surfaceData;
    }
	        
    public void initBackground(int width, int height) {
	surfaceData = BufImgSurfaceData.createData(this);
	// init background only if the image was created
	// from a component.
	if (c == null) {
	    return;
	}

	Graphics2D g2 = createGraphics();
	try {
	    g2.clearRect(0, 0, width, height);
	} finally {
	    g2.dispose();
	}
    }

    public ImageProducer getSource() {
	if (osis == null) {
	    osis = new OffScreenImageSource(this);
	}
        return osis;
    }

}
