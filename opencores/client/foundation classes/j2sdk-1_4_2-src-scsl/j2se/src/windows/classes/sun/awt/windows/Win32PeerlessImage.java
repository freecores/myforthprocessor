/*
 * @(#)Win32PeerlessImage.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.image.DirectColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.WritableRaster;

import sun.awt.image.AcceleratedOffScreenImage;
import sun.java2d.SurfaceData;
import sun.java2d.SunGraphics2D;
import sun.java2d.loops.CompositeType;


/**
 * Win32PeerlessImage
 *
 * This class represents a specialized "behind-the-scenes" accelerated
 * image that is created as a result of loading an on-disk image (e.g. GIF,
 * PNG, JPG) through one of the Toolkit.createImage() variants or through
 * the GraphicsConfiguration.createCompatibleImage() methods.  Two things
 * separate this variety of images from their parents:
 *   1) the image is created without an associated peer component
 *   2) the image may be "bitmask" transparent
 * To account for (2) there are methods in this class that assist in finding
 * a color that DirectDraw can use to represent any transparent pixels in the
 * source image.
 * REMIND: Note that the only transparent images we currently support
 * are those with either 1) an IndexColorModel with bitmask transparency or
 * 2) a 25 bit (1888 RGB) DirectColorModel with bitmask transparency.  Also
 * note that we must punt if the destination has 8-bit depth since the
 * dithering will upset our "unused color" algorithms.
 */
public class Win32PeerlessImage extends Win32OffScreenImage {

    /**
     * Represents the maximum size (width * height) of an image that we should
     * scan for an unused color.  Any image larger than this would probably
     * require too much computation time.
     */
    private static final int MAX_SIZE = 65536;

    /**
     * The following constants determine the size of the histograms used when
     * searching for an unused color
     */
    private static final int ICM_HISTOGRAM_SIZE = 256;
    private static final int ICM_HISTOGRAM_MASK = ICM_HISTOGRAM_SIZE - 1;
    private static final int DCM_HISTOGRAM_SIZE = 1024;
    private static final int DCM_HISTOGRAM_MASK = DCM_HISTOGRAM_SIZE - 1;

    private ColorModel cmHw;
    private int transparency;
    private Color transColor;

    /**
     * Regulates whether we enable acceleration for translucent
     * images.
     */
    static protected boolean isTranslucentImagesAccEnabled = false;
    static {
	String tr = (String) java.security.AccessController.doPrivileged(
		new sun.security.action.GetPropertyAction("sun.java2d.translaccel"));
	if (tr != null && tr.equals("true")) {
	    isTranslucentImagesAccEnabled = true;
	    System.out.println("Acceleration for translucent images is enabled.");
	}
    }

    public Win32PeerlessImage(ColorModel cmSw, ColorModel cmHw,
                              WritableRaster raster,
                              boolean isRasterPremultiplied,
                              int transparency)
    {
        super(null, cmSw, raster, isRasterPremultiplied);

        this.cmHw = cmHw;
        this.transparency = transparency;
	// create the accelerated surface on image creation.
	// This allows pre-loading of the images.
	// REMIND: may be need a separate flag for this.
	if (AcceleratedOffScreenImage.getAccelerationThreshold() == 0) {
	    GraphicsConfiguration gc = 
		GraphicsEnvironment.getLocalGraphicsEnvironment().
                    getDefaultScreenDevice().getDefaultConfiguration();
	    initAcceleratedBackground(gc, getWidth(), getHeight());
	}
    }

    /**
     * The number of times we unsuccessfully tried
     * to accelerate the surface.
     * Used to avoid infinite retries to accelerate
     * the surface
     */
    int timesTried = 0;

    public void initAcceleratedBackground(GraphicsConfiguration gc, 
					  int width, int height) {
	synchronized (this) {
	    super.initAcceleratedBackground(gc, width, height);
	    if (surfaceDataHw == null) {
		if (timesTried++ > 3) {
		    localAccelerationEnabled = false;
		}
	    } else {
		timesTried = 0;
	    }
	    if (surfaceDataHw != null && transparency == Transparency.BITMASK &&
		getBackgroundColor() == null) {

		// we can't cache on this hardware surface since
		// we were unable to find the color which could
		// be used as color key.
		flush();
		surfaceDataHw = null;

		// REMIND: getBackgroundColor method will
		// set localAccelerationEnabled to false
		// if it can't find the unused color. This
		// prevents us from further attempts to cache this image 
		// on other hw surfaces even though they may
		// have unused pixels.
	    }
	}
    }

    /**
     * Returns whether an opaque Win32PeerlessImage can be created given
     * the source ColorModel, destination ColorModel, and total number of 
     * pixels in the image (width * height).  This helps the caller determine
     * whether it would be best to create a Win32PeerlessImage or to just 
     * stick with a BufferedImage (giving up on advanced acceleration
     * features).
     */
    public static boolean isValidOpaqueConfig(ColorModel cmSw,
                                              ColorModel cmHw,
                                              int numPixels)
    {
        return (cmSw.getTransparency() == Transparency.OPAQUE);
    }

    /**
     * Returns whether a bitmask transparent Win32PeerlessImage can be 
     * created given the source ColorModel, destination ColorModel, and 
     * total number of pixels in the image (width * height).  This helps the 
     * caller determine whether it would be best to create a 
     * Win32PeerlessImage or to just stick with a BufferedImage (giving up 
     * on advanced acceleration features).
     */
    public static boolean isValidBitmaskConfig(ColorModel cmSw, 
                                               ColorModel cmHw,
                                               int numPixels)
    {
        if ((cmSw.getTransparency() == Transparency.BITMASK) &&
            (cmHw.getPixelSize() != 8)) 
        {
            if (cmSw instanceof IndexColorModel) {
                return true;
            } else if ((cmSw instanceof DirectColorModel) &&
                       (cmSw.getPixelSize() == 25) &&
                       (cmSw.getTransferType() == DataBuffer.TYPE_INT) &&
                       (numPixels <= MAX_SIZE)) 
            {
                return true;
            }
        }

        return false;
    }

    public static boolean isValidTranslucentConfig(ColorModel cmSw,
						   ColorModel cmHw,
						   int numPixels)
    {
        return isTranslucentImagesAccEnabled &&
	    (cmSw.getTransparency() == Transparency.TRANSLUCENT);
    }

    protected boolean operationSupported(CompositeType comp, 
					 Color bgColor,
					 boolean scale) {
	if (transparency == Transparency.OPAQUE) {
	    // we save a read from video memory for compositing
	    // operations by copying from the buffered image sd
	    if (CompositeType.SrcOverNoEa.equals(comp) ||
		CompositeType.SrcNoEa.equals(comp)) {
		// allow using accelerated surface for scale blits
		// if DD scaling is enabled
		return (!scale || Win32OffScreenSurfaceData.isDDScaleEnabled());
	    }
	} else if (transparency == Transparency.TRANSLUCENT) {
	    if ((CompositeType.SrcOverNoEa.equals(comp) || 
		 CompositeType.SrcOver.equals(comp)) && 
		bgColor == null) 
	    {
		return !scale;
	    }
	} else {
	    // We have accelerated loops only for blits with SrcOverNoEa
	    // (no blit bg loops or blit loops with SrcNoEa)
	    if (CompositeType.SrcOverNoEa.equals(comp) && bgColor == null) {
		return !scale;
	    }
	}
	return false;
    }

    public Graphics getGraphics() {
	return createGraphics();
    }

    /**
     * This image has been created without a peer, so we must use the backup
     * means of creating a Graphics2D object.
     */
    public Graphics2D createGraphics() {
        GraphicsEnvironment env =
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        return env.createGraphics(this);
    }

    protected ColorModel getDeviceColorModel() {
        return cmHw;
    }

    protected int getTransparency() {
        return transparency;
    }

    protected Color getBackgroundColor() {
        if (!accelerationEnabled ||
            !localAccelerationEnabled ||
            transparency != Transparency.BITMASK)
        {
            return null;
        } else {
            // REMIND: might be able to cache transColor
            transColor = setupTransparentPixel();
            
            if (transColor == null) {
                localAccelerationEnabled = false;
            }

            return transColor;
        }
    }

    /**
     * Attempts to find an unused pixel value in the image and if successful,
     * sets up the DirectDraw surface so that it uses this value as its
     * color key
     */
    protected Color setupTransparentPixel() {
        Color transColor = null;
        Integer transPixel = null;
        
        ColorModel cm = getColorModel();
        if (cm instanceof IndexColorModel) {
            transPixel = findUnusedPixelICM();
        } else if (cm instanceof DirectColorModel) {
            transPixel = findUnusedPixelDCM();
        }

        if (transPixel != null) {
            int ipixel = transPixel.intValue();
            ((Win32OffScreenSurfaceData)surfaceDataHw).setTransparentPixel(ipixel);
            int rgb = surfaceDataHw.rgbFor(ipixel);
            transColor = new Color(rgb);
        }

        return transColor;
    }

    /**
     * Attempts to find an unused pixel value in the color map of an
     * IndexColorModel.  If successful, it returns that value (in the 
     * ColorModel of the destination surface) or null otherwise.
     */
    private Integer findUnusedPixelICM() {
        IndexColorModel icm = (IndexColorModel)getColorModel();
        int mapsize = icm.getMapSize();
        int[] histogram = new int[ICM_HISTOGRAM_SIZE];
        int[] cmap = new int[mapsize];
        icm.getRGBs(cmap);

        // load up the histogram
        for (int i = 0; i < mapsize; i++) {
            int pixel = surfaceDataHw.pixelFor(cmap[i]);

            histogram[pixel & ICM_HISTOGRAM_MASK]++;
        }

        // find an empty histo-bucket
        for (int j = 0; j < histogram.length; j++) {
            int value = histogram[j];

            if (value == 0) {
                return new Integer(j);
            }
        }

        return null;
    }

    /**
     * Attempts to find an unused pixel value in an image with a 
     * 25-bit DirectColorModel and a DataBuffer of TYPE_INT.  If successful, 
     * it returns that value (in the ColorModel of the destination surface) 
     * or null otherwise.
     */
    private Integer findUnusedPixelDCM() {
        DataBufferInt db = (DataBufferInt)getRaster().getDataBuffer();
        // REMIND: we need to offset the effects of the call to 
        // getDataBuffer() by re-enabling local acceleration (we could not
        // have entered this method if local acceleration was not already
        // enabled, so we can turn acceleration back on without side effects)
        localAccelerationEnabled = true;
        int[] pixels = db.getData();
        int[] histogram = new int[DCM_HISTOGRAM_SIZE];

        // load up the histogram
        // REMIND: we could possibly make this faster by keeping track of
        // the unique colors found, and only doing a pixelFor() when we come
        // across a new unique color
        for (int i = 0; i < pixels.length; i++) {
            int pixel = surfaceDataHw.pixelFor(pixels[i]);

            histogram[pixel & DCM_HISTOGRAM_MASK]++;
        }

        // find an empty histo-bucket
        for (int j = 0; j < histogram.length; j++) {
            int value = histogram[j];

            if (value == 0) {
                return new Integer(j);
            }
        }

        return null;
    }    

    public void displayChanged() {
	// REMIND: probably need to re-verify that we can support
	// the image in this new display mode
	super.displayChanged();
    }

    public String toString() {
        return new String("Win32PeerlessImage@" +
			  Integer.toHexString(hashCode()) + 
                          " transparency: " + 
			  (transparency == Transparency.OPAQUE ? 
			     "OPAQUE" : 
			     transparency == Transparency.BITMASK ? 
			        "BITMASK" : 
			        "TRANSLUCENT"));
    }
}
