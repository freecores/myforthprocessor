/*
 * @(#)AcceleratedOffScreenImage.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;

import sun.awt.image.BufferedImageGraphicsConfig;
import sun.awt.image.OffScreenImage;
import sun.java2d.SurfaceData;
import sun.java2d.SunGraphics2D;
import sun.java2d.loops.Blit;
import sun.java2d.loops.BlitBg;
import sun.java2d.loops.CompositeType;

/**
 * AcceleratedOffScreenImage
 *
 * This image manages a hidden acceleration mechanism.
 * The surfaceData object is set to be the SurfaceData
 * that is created during the superclass construction.  Hence, all 
 * rendering to the image will use the system-memory version of the
 * surface.  When copying from the image, however, the image may elect to
 * use a different version of the surface that resides in some 
 * accelerated memory (e.g., vram on some platforms).
 * These accelerated copies can be much faster than the software-based
 * copies using SurfaceData.  
 *
 * Any situation that causes hardware
 * surfaces to be lost will cause no disruption; the image will continue
 * using the software surface until the hardware surface can be automatically
 * restored.
 */
public abstract class AcceleratedOffScreenImage 
    extends OffScreenImage
    implements RasterListener
{
    static protected boolean accelerationEnabled = true;
    protected SurfaceData surfaceDataSw;	// default software surface
    protected SurfaceData surfaceDataHw;	// cached, accelerated surface
    protected boolean surfaceLossHw       /* = false */;

    /**
     * This next flag is used to track whether anyone has requested
     * direct access to the pixels, via one of the many get*() methods
     * of BufferedImage.  If so, then we punt on trying to use the
     * cached surfaceData object because we cannot ensure that its
     * data matches that of the surfaceDataSw version.
     */
    protected boolean localAccelerationEnabled = true;

    /**
     * Regulates whether we disable local acceleration on rasterStolen
     * events.  Normally, when the user gets a handle to the Raster of
     * a hidden-acceleration image, we punt on trying to accelerate it
     * because we cannot guarantee that our cached version will match
     * the software version.  Use of this flag can force us to
     * proceed with acceleration anyway.  
     * This workaround was introduced for a particular customer 
     * situation where we could not provide a way for the 
     * user to set the bits in an image without disabling acceleration
     * for that image, so they had to have 2 copies of that data.  This
     * flag was used to allow that application to run without these
     * duplicate images to preserve memory usage.
     * Use of this flag means applications have to let us know when
     * the image data has changed.  Since there is no API for that
     * currently, they must use the workaround of forcing some write
     * to the image (such as a transparent image operation); this 
     * will trigger a dirty flag for the image and we will then 
     * copy the new data down to the cached version.
     * A real fix in the future would be to implement new API that
     * allows applications to lock and unlock an image for direct
     * pixel manipulation.
     */
    static protected boolean allowRasterSteal = false;

    /**
     * Value that determines how many copies we will do from the system
     * memory version of the image before we attempt to cache and
     * accelerate future copies.
     */
    private static int accelerationThreshold = 1;

    static {
	String num = (String) java.security.AccessController.doPrivileged(
		new sun.security.action.GetPropertyAction("sun.java2d.accthreshold"));
	if (num != null) {
	    try {
		int parsed = Integer.parseInt(num);
		if (parsed >= 0 ) {
		    accelerationThreshold = parsed;
		    System.out.println("New Acceleration Threshold: " + accelerationThreshold);
		}
	    } catch (NumberFormatException e) {
		System.err.println("Error setting new threshold:" + e);
	    }
	}
	String ras = (String) java.security.AccessController.doPrivileged(
		new sun.security.action.GetPropertyAction("sun.java2d.allowrastersteal"));
	if (ras != null && ras.equals("true")) {
	    allowRasterSteal = true;
	    System.out.println("Raster steal allowed");
	}
    }

    public AcceleratedOffScreenImage(Component c, ColorModel cm, 
				     WritableRaster raster, 
				     boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
	surfaceDataSw = surfaceData;
        
        if (raster instanceof SunWritableRaster) {
            ((SunWritableRaster)raster).setRasterListener(this);
        } else {
            localAccelerationEnabled = false;
        }
	// REMIND: we used to call initAcceleratedBackground here,
	// but the new design is that we create a HW surface only
	// when requested (and we create it for a specific 
	// destination gc)
    }

    /**
     * Create an accelerated surfaceData object to provide hidden acceleration
     * mechanism.  
     */    
    public abstract void 
	initAcceleratedBackground(GraphicsConfiguration gc,
				  int width, int height);

    /**
     * Return the Color object used to replace any transparent pixels in
     * the backup BufferedImage when copying to the hardware surface.
     */
    protected Color getBackgroundColor() {
        return null;
    }

    /**
     * This method copies the contents
     * of the BufferedImage backup onto the hardware surface data.  This
     * call could be made either when the user updated the contents since
     * our last copy from the surface, or when there has been a surface
     * loss occurrence.
     */
    protected void copyBackupToAccelerated() {
	try {
	    if (surfaceDataHw != null && !surfaceLossHw) {
		Color bgcolor = getBackgroundColor();
		if (bgcolor == null) {
		    Blit blit = Blit.getFromCache(surfaceDataSw.getSurfaceType(), 
						  CompositeType.SrcNoEa,
						  surfaceDataHw.getSurfaceType());
		    blit.Blit(surfaceDataSw, surfaceDataHw,
			      AlphaComposite.Src, null,
			      0, 0, 0, 0, 
			      getWidth(), getHeight());
		} else {
		    // we know that in this case the image
		    // is transparent, so we use SrcNoEa
		    BlitBg blit = BlitBg.getFromCache(surfaceDataSw.getSurfaceType(), 
						      CompositeType.SrcNoEa,
						      surfaceDataHw.getSurfaceType());
		    blit.BlitBg(surfaceDataSw, surfaceDataHw, 
				AlphaComposite.SrcOver, null, bgcolor,
				0, 0, 0, 0, 
				getWidth(), getHeight());
		}
		surfaceDataSw.setNeedsBackup(false);
	    }
	} catch (Exception e) {
	    // Catch the exception so as to not propagate it.  We will
	    // just continue to use the sw surfaceData
	    surfaceLossHw = true;
	}
    }

    /**
     * This method is called by the raster that we registered ourselves with
     * to notify us that the underlying raster for this image has been
     * modified.  Upon receiving this notification, we mark the sw SurfaceData
     * as needing backup.
     */
    public void rasterChanged() {
        surfaceDataSw.setNeedsBackup(true);
    }

    /**
     * This method is called by the raster that we registered ourselves with
     * to notify us that the underlying raster for this image has been
     * taken by a third party (e.g. through a call to Raster.getDataBuffer()).
     * Upon receiving this notification, we disable acceleration for this
     * image since we no longer have control over the raster.
     */
    public void rasterStolen() {
	if (!allowRasterSteal) {
	    localAccelerationEnabled = false;
	}
    }
    
    /**
     * Abstract method returning a boolean value indicating whether the
     * destination surface passed in is accelerated.  This is implemented
     * in platform-specific classes to return values based on whether 
     * copies to the specific type of destination surface can be
     * accelerated.
     */
    protected abstract boolean destSurfaceAccelerated(SurfaceData destSD);

    /**
     * This method checks if the operation for which the surface
     * data was requested is 'compatible' with the hardware
     * surface data. Sometimes it doesn't make sense to 
     * use hardware surface data when there is no special loop
     * which could benefit from the surface data being accelerated
     * (i.e. in case of alpha blending operations).
     * Particular platform implementation could override this 
     * method if needed.
     */
    protected abstract boolean operationSupported(CompositeType comp, 
						  Color bgColor, boolean scale);

    /**
     * This method is called from SunGraphics2D before it schedules
     * a Blit call.  The intention is to get the optimal surface
     * data object from which to do the copy; either the original
     * system-memory version of the Image or the version cached
     * in accelerated memory.
     */
    public SurfaceData getSourceSurfaceData(SurfaceData destSD, 
					    CompositeType comp, 
					    Color bgColor, boolean scale) {
	// Only return the accelerated version of the surfaceData if all of
	// the following are true:
	//	- acceleration is globally enabled
        //      - acceleration for this specific image is enabled
	//	- we're not in the middle of a copyBackupToAccelerated
	//	operation (destSD != surfaceDataHw)
	//	- the destination SD is accelerated, so copies from
	//	surfaceDataHw will be faster than copies from system memory
	//      - current operation (composite type + bg color + scaling op)
	//        is suitable for the accelerated surface which represents the
	//        image. See operationSupported() for the definition of 
	//        'suitable'
	//	- We have already done this copy before since the last 
	//	time the image was modified.
	if (accelerationEnabled			    &&
	    localAccelerationEnabled		    &&
	    (destSD != surfaceDataHw)		    &&
	    destSurfaceAccelerated(destSD)	    &&
	    operationSupported(comp, bgColor, scale)&&
	    (surfaceDataSw.increaseNumCopies() > accelerationThreshold))
	{
	    // First, we validate the hw sd if necessary and then return
	    // the appropriate surfaceData object.
	    validate(destSD.getDeviceConfiguration());
	    if (surfaceDataHw != null && !surfaceLossHw) {
		return surfaceDataHw;
	    }
	}
	// Fallback case: return the system-memory version
	return surfaceDataSw;
    }

    /**
     * Called from platform-specific SurfaceData objects to attempt to 
     * auto-restore the contents of an accelerated surface that has been lost.
     */
    public SurfaceData restoreContents() {
	// Always return software sd
	return surfaceDataSw;
    }

    /**
     * Restore surfaceDataHw in case it was lost.  Do nothing in this
     * default case; platform-specific implementations may do more in
     * this situation as appropriate.
     */
    protected void restoreSurfaceDataHw() {
    }

    /**
     * Returns true if current surfaceDataHw is suitable
     * for using with given GraphicsConfiguration.
     */
    protected abstract boolean isValidHWSD(GraphicsConfiguration gc);

    /**
     * Get the hardware surface ready for rendering.  This method is
     * called internally whenever we want to make sure that the surface
     * exists in a usable state.
     *
     * The surface may not be "ready" if either we had problems creating
     * it in the first place (e.g., there was no space in vram) or if
     * the surface was lost (e.g., due to a display change or other
     * surface-loss situation).
     */
    public void validate(GraphicsConfiguration gc) {
	if (accelerationEnabled && localAccelerationEnabled) {
	    if (surfaceDataHw == null || !isValidHWSD(gc)) {
		// Must have lost the surface of had problems creating it.
		initAcceleratedBackground(gc, getWidth(), getHeight());
		if (surfaceDataHw != null) {
		    surfaceDataSw.setNeedsBackup(true);
		    surfaceLossHw = false;
		} else {
		    return;
		}
	    } 
	    else if (surfaceLossHw) {
		try {
		    restoreSurfaceDataHw();
		    surfaceLossHw = false;
		    surfaceDataSw.setNeedsBackup(true);
		}
		catch (sun.java2d.InvalidPipeException e) {
		    // Note that we still have
		    // the surfaceLossHw flag set so that we will continue to 
		    // attempt to restore the accelerated surface.
		    return;
		}
	    }
	    if (surfaceDataSw.needsBackup()) {
		copyBackupToAccelerated();
	    }
	}
    }

    /**
     * Returns a BufferedImage representation of this image.
     * In this default method, it just returns this object.
     * Platform-dependent implementations may choose to return
     * something different.
     */
    public BufferedImage getSnapshot() {
	return this;
    }

    public static int getAccelerationThreshold() {
	return accelerationThreshold;
    }
}
