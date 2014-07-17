/*
 * @(#)SunVolatileImage.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.ImageCapabilities;
import java.awt.Toolkit;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.VolatileImage;

import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;

/**
 * This class is the base implementation of the VolatileImage
 * abstract class.  The class should be overridden by platform-
 * specific classes which provide additional functionality beyond
 * these default methods.
 */
public class SunVolatileImage extends VolatileImage
{
    protected SurfaceData	    surfaceData;
    protected Component		    comp;
    protected GraphicsConfiguration graphicsConfig;
    protected Object context; // Context for extra initialization parameters
    protected ImageCapabilities imageCaps;
    private Font		    defaultFont;
    private int			    width, height;
    // Tracks loss of surface contents; queriable by user to see whether
    // contents need to be restored
    protected boolean		    surfaceLoss;	// = false;
    // Tracks loss of hardware-based surfaceData.  Separate from surfaceLoss
    // variable because we can have a fallback software-based surfaceData
    // object that is used (with no "surfaceLoss") while we attempt to 
    // restore the hardware-based surfaceData.
    protected boolean		    surfaceLossHw;	// = false;

    // Accelerated surfaceData object
    protected SurfaceData	    surfaceDataHw;

    // Software-based surfaceData object.  Only create when first asked
    // to (otherwise it is a waste of memory as it will only be used in
    // situations of surface loss).
    protected SurfaceData	    surfaceDataSw;

    // Record-keeping object.  This keeps track of which surfaceData was
    // in use during the last call to validate().  This lets us see whether
    // the surfaceData object has changed since then and let's us return
    // the correct returnCode to the user in the validate() call.
    protected SurfaceData	    previousSurfaceData; // = null

    // Tracks whether acceleration is enabled.  If not, we simply use the software
    // surfaceData object and return quickly from most methods in this class.
    static protected boolean	    accelerationEnabled = 
	java.awt.GraphicsEnvironment.isHeadless() ? false : true;


    private SunVolatileImage(int width, int height, Object context) {
	this.width = width;
	this.height = height;
        this.context = context;
    }

    public SunVolatileImage(Component comp, int width, int height) {
        this(comp, width, height, null);
    }

    public SunVolatileImage(Component comp, int width, int height,
        Object context) {
	this(width, height, context);
	this.comp = comp;
	this.graphicsConfig = comp.getGraphicsConfiguration();
	if (accelerationEnabled) {
	    initAcceleratedBackground();
	}
	if (surfaceDataHw == null) {
	    surfaceData = getSurfaceDataSw();
	}
    }

    public SunVolatileImage(GraphicsConfiguration graphicsConfig, 
			    int width, int height) {
	this(width, height, null);
	this.graphicsConfig = graphicsConfig;
	if (accelerationEnabled) {
	    initAcceleratedBackground();
	}
	if (surfaceDataHw == null) {
	    surfaceData = getSurfaceDataSw();
	}
    }

    public int getWidth() {
	return width;
    }

    public int getHeight() {
	return height;
    }

    /**
     * Creates a software-based surface (of type BufImgSurfaceData).
     * The software representation is only created when needed, which
     * is only during some situation in which the hardware surface
     * cannot be allocated.  This allows apps to at least run,
     * albeit more slowly than they would otherwise.
     */
    public SurfaceData getSurfaceDataSw() {
	if (surfaceDataSw == null) {
	    BufferedImage bImg = getBufferedImage();
	    surfaceDataSw = BufImgSurfaceData.createData(bImg);
	}
	return surfaceDataSw;
    }

    /**
     * Called from a SurfaceData object to notify us that our
     * accelerated surface has been lost.
     */
    public SurfaceData restoreContents() {
	return getSurfaceDataSw();
    }

    public SurfaceData getSurfaceData() {
	return surfaceData;
    }

    protected ColorModel getColorModel() {
	if (comp != null) {
	    return comp.getColorModel();
	} else {
	    return graphicsConfig.getColorModel();
	}
    }

    private Color getForeground() {
	if (comp != null) {
	    return comp.getForeground();
	} else {
	    return Color.black;
	}
    }

    private Color getBackground() {
	if (comp != null) {
	    return comp.getBackground();
	} else {
	    return Color.white;
	}
    }

    private Font getFont() {
	if (comp != null) {
	    return comp.getFont();
	} else {
	    if (defaultFont == null) {
		defaultFont = new Font("Dialog", Font.PLAIN, 12);
	    }
	    return defaultFont;
	}
    }

    public Graphics2D createGraphics() {
	return new SunGraphics2D(surfaceData,
				 getForeground(),
				 getBackground(),
				 getFont());
    }

    public void initAcceleratedBackground() {
    }

    /** 
     * Set contents of surfaceData to default state
     */
    public void initContents() {
	Graphics g = createGraphics();
	g.clearRect(0, 0, getWidth(), getHeight());
	g.dispose();
    }

    // Image method implementations
    public Object getProperty(String name, ImageObserver observer) {
	/* REMIND: Is this good enough?  There is no way currently
	 * to set the properties of a VolatileImage, so this doesn't
	 * really seem relevant to the kind of object a VolatileImage
	 * represents.  But we may, eventually, add methods to load an
	 * image directly into a VolatileImage, e.g., 
	 * Toolkit.loadVolatileImage(), and those type of images may want
	 * to set properties.  But for now, we just return null.
	 */
	return null;
    }

    public int getWidth(ImageObserver observer) {
	return getWidth();
    }

    public int getHeight(ImageObserver observer) {
	return getHeight();
    }

    protected BufferedImage getBufferedImage() {
	return 
	    (BufferedImage)graphicsConfig.createCompatibleImage(getWidth(), 
								getHeight());
    }

    public BufferedImage getSnapshot() {
	BufferedImage bImg = getBufferedImage();
	Graphics2D g = bImg.createGraphics();
	g.drawImage(this, 0, 0, null);
	g.dispose();
	return bImg;
    }

    // Volatile management methods

    /**
     * Get the image ready for rendering.  This method is called to make
     * sure that the hardware version of the surfaceData exists and is
     * ready to be used.  Users call this method prior to any set of 
     * rendering to or from the image, to make sure the image is ready
     * and compatible with the given GraphicsConfiguration.
     *
     * The image may not be "ready" if either we had problems creating
     * it in the first place (e.g., there was no space in vram) or if
     * the surface became lost (e.g., some other app or the OS caused
     * vram surfaces to be removed).
     *
     * Note that we want to return RESTORED in any situation where the 
     * surfaceData is different than it was last time.  So whether it's
     * software or hardware, if we have a different surfaceData object, 
     * then the contents have been altered and we must reflect that 
     * change to the user.
     */
    public int validate(GraphicsConfiguration gc) {
	int returnCode = IMAGE_OK;
	boolean surfaceLossTmp = surfaceLoss;
	surfaceLoss = false;

	if (accelerationEnabled) {
	    if (!isGCValid(gc)) {
		// If we're asked to render to a different device than the
		// one we were created under, return INCOMPATIBLE error code.
		// Note that a null gc simply ignores the incompatibility 
		// issue
		returnCode = IMAGE_INCOMPATIBLE;
	    } else if (surfaceDataHw == null) {
		// We either had problems creating the surface or the display
		// mode changed and we nullified the old one.  Try it again.
		initAcceleratedBackground();
		if (surfaceDataHw != null) {
		    // Creation ok: surfaceLossHw no longer true.
		    surfaceLossHw = false;
		    returnCode = IMAGE_RESTORED;
		}
	    } else if (surfaceLossHw) {
		try {
		    restoreSurfaceDataHw();
		    // set the surfaceData object to hw version and init the surface
		    surfaceData = surfaceDataHw;
		    initContents();
		    surfaceLossHw = false;
		    returnCode = IMAGE_RESTORED;
		}
		catch (sun.java2d.InvalidPipeException e) {
		    // Set surfaceData to be the software
		    // version so that drawing can continue.  Note that we still have
		    // the surfaceLossHw flag set so that we will continue to attempt
		    // to restore the accelerated surface.
		    surfaceData = getSurfaceDataSw();
		}
	    } else if (surfaceLossTmp) {
		// Something else triggered this loss/restoration.  Could
		// be a palette change that didn't require a surfaceData
		// recreation but merely a re-rendering of the pixels.
		returnCode = IMAGE_RESTORED;
	    }
	    if ((returnCode != IMAGE_INCOMPATIBLE) &&
		(surfaceData != previousSurfaceData)) 
	    {
		// Contents have changed - return RESTORED to user
		previousSurfaceData = surfaceData;
		returnCode = IMAGE_RESTORED;
	    }
	}
	return returnCode;
    }

    /**
     * Restore surfaceDataHw in case it was lost.  Do nothing in this
     * default case; platform-specific implementations may do more in
     * this situation as appropriate.
     */
    protected void restoreSurfaceDataHw() {
    }

    /**
     * Called by validate() to see whether the GC passed in is ok for
     * rendering to.  This generic implementation checks to see
     * whether the GC is either null or is from the same
     * device as the one that this image was created on.  Platform-
     * specific implementations may perform other checks as
     * appropriate.
     */
    protected boolean isGCValid(GraphicsConfiguration gc) {
	return ((gc == null) || 
		(gc.getDevice() == graphicsConfig.getDevice()));
    }
        
    public boolean contentsLost() {
	return surfaceLoss;
    }

    /**
     * Returns an ImageCapabilities object which can be
     * inquired as to the specific capabilities of this
     * VolatileImage.  This would allow programmers to find
     * out more runtime information on the specific VolatileImage
     * object that they have created.  For example, the user
     * might create a VolatileImage but the system may have
     * no video memory left for creating an image of that
     * size, so although the object is a VolatileImage, it is
     * not as accelerated as other VolatileImage objects on
     * this platform might be.  The user might want that
     * information to find other solutions to their problem.
     * @since 1.4
     */
    public ImageCapabilities getCapabilities() {
        if (imageCaps == null) {
            imageCaps = new DefaultImageCapabilities();
        }
        return imageCaps;
    }

    protected class DefaultImageCapabilities extends ImageCapabilities {
        public DefaultImageCapabilities() {
            super(false);
	}
        public boolean isAccelerated() {
            return surfaceData == surfaceDataHw;
	}
    }
}

