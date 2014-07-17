/*
 * @(#)Win32OffScreenImage.java	1.29 03/01/30
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;
import java.util.Hashtable;
import java.util.Iterator;

import sun.awt.image.BufferedImageGraphicsConfig;
import sun.awt.image.AcceleratedOffScreenImage;
import sun.awt.image.OffScreenImage;
import sun.java2d.SurfaceData;
import sun.java2d.SunGraphics2D;
import sun.java2d.loops.CompositeType;
import sun.awt.Win32GraphicsEnvironment;
import sun.awt.Win32GraphicsConfig;
import sun.awt.DisplayChangedListener;

/**
 * Win32OffScreenImage
 *
 * This class implements the win32-specific details of the
 * AcceleratedOffScreenImage class.  The main platform-specific
 * items are in the creation of the accelerated SurfaceData object
 * and the restoration of that object (which could be lost due to the
 * display changes or other OS-related occurrences).
 */
public class Win32OffScreenImage extends AcceleratedOffScreenImage 
    implements DisplayChangedListener
{ 
    private static native void initIDs();
    private Hashtable cachedHWSD = new Hashtable();

    static {
	initIDs();
        String noddraw = (String) java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.java2d.noddraw"));
        String ddoffscreenProp = 
	    (String) java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.java2d.ddoffscreen"));
        boolean ddoffscreenDisable = (ddoffscreenProp != null && 
				      (ddoffscreenProp.equals("false") || 
				       ddoffscreenProp.equals("f")));
        if (noddraw == null && !ddoffscreenDisable) {
	    accelerationEnabled = true;
	} else {
	    accelerationEnabled = false;
	}
    }
    

    public Win32OffScreenImage(Component c, ColorModel cm, 
			       WritableRaster raster, 
			       boolean isRasterPremultiplied)
    {
	super(c, cm, raster, isRasterPremultiplied);
    	((Win32GraphicsEnvironment)GraphicsEnvironment.
	    getLocalGraphicsEnvironment()).addDisplayChangedListener(this);
    }

    protected boolean isValidHWSD(GraphicsConfiguration gc) {
	return (surfaceDataHw != null && 
		surfaceDataHw == cachedHWSD.get(gc.getDevice()));
    }


    /**
     * Returns the ColorModel of the hardware device.  This method can be
     * overridden by subclasses that retrieve this information through
     * a different mechanism.
     */
    protected ColorModel getDeviceColorModel() {
        return c.getColorModel();
    }
    
    /**
     * Returns whether this image has a supported transparency model.  While
     * Win32OffScreenImages must be opaque, subclasses may support certain
     * kinds of transparency and can therefore override this method.
     */
    protected int getTransparency() {
        return Transparency.OPAQUE;
    }
    
    /**
     * Create a vram-based surfaceData object, either to be our
     * main surfaceData (if we're volatile) or to be our hidden acceleration
     * mechanism for non-volatile images.  
     */    
    public void initAcceleratedBackground(GraphicsConfiguration gc, 
					  int width, int height) {
	synchronized (this) {
	    try {
		surfaceDataHw = (SurfaceData)cachedHWSD.get(gc.getDevice());
		if (surfaceDataHw == null) {
		    surfaceDataHw = Win32OffScreenSurfaceData.
			createData(getWidth(), 
				   getHeight(), 
				   ((Win32GraphicsConfig)gc).
				       getDeviceColorModel(), 
				   gc, this,
				   getTransparency());
		    if (surfaceDataHw != null) {
			cachedHWSD.put(gc.getDevice(), surfaceDataHw);
		    }
		}
	    } catch (sun.java2d.InvalidPipeException e) {
		// Problems during creation.  Don't propagate the exception, just
		// set the hardware surface data to null; the software surface
		// data will be used in the meantime
		surfaceDataHw = null;
	    }
	}
    }

    /**
     * Do not allow to copy from accelerated surface
     * if there is any compositing: it's better to copy
     * from system memory instead since reading from 
     * video memory is expensive.
     * Note that this class represents opaque images only.
     */
    protected boolean operationSupported(CompositeType comp, 
					 Color bgColor,
					 boolean scale) {
	return (CompositeType.SrcNoEa.equals(comp) || 
		CompositeType.SrcOverNoEa.equals(comp));
    }
    
    /**
     * The destination is considered "accelerated" if it is either a window
     * or an offscreen surface that has not been
     * punted into system memory.  This method is called to determine
     * which version (accelerated or not) of a source image to use when
     * copying to this destination; if the destination does not reside
     * in hardware, then the source image used should be the unaccelerated
     * version to avoid performance penalties of reading from VRAM.
     */
    protected boolean destSurfaceAccelerated(SurfaceData destSD) {
	return (((destSD instanceof Win32OffScreenSurfaceData) &&
		 !((Win32OffScreenSurfaceData)destSD).surfacePunted()) ||
	        (destSD instanceof Win32SurfaceData));
    }


    /**
     * Called from Win32OffScreenSurfaceData to attempt to auto-restore
     * the contents of an accelerated surface that has been lost.
     */
    public SurfaceData restoreContents() {
	if (accelerationEnabled) {
	    surfaceLossHw = true;
	    synchronized (this) {
		if (surfaceDataHw != null) {
		    validate(surfaceDataHw.getDeviceConfiguration());
		}
	    }
	}
	return super.restoreContents();
    }
	        
    protected void restoreSurfaceDataHw() {
	synchronized (this) {
	    if (surfaceDataHw != null) {
		((Win32OffScreenSurfaceData)surfaceDataHw).restoreSurface();
	    }
	}
    }

		    
    /**
     * Called from WToolkit when there has been a display mode change.
     * Note that we simply invalidate hardware surfaces here; we do not
     * attempt to recreate or re-render them.  This is to avoid doing
     * rendering operations on the AWT-Windows thread, which tends to
     * get into deadlock states with the rendering thread.  Instead,
     * we just nullify the old surface data object and wait for a future
     * method in the rendering process to recreate the surface and
     * copy the backup if appropriate.
     */
    public void displayChanged() {
	if (!accelerationEnabled) {
	    return;
	}
	// REMIND: may be playing too safe here: invalidate all SD
	// even if they're on a different device
	synchronized (this) {
	    if (surfaceDataHw != null) {
		Hashtable oldHWSD = cachedHWSD;
		surfaceDataHw = null;
		cachedHWSD = new Hashtable();
		Object array[] = oldHWSD.values().toArray();
		for (int i = 0; i < array.length; i++) {
		    ((SurfaceData)array[i]).invalidate();
		}
	    }
	}
    }

    /**
     * When device palette changes, need to force a new copy
     * of the image into our hardware cache to update the 
     * color indices of the pixels (indexed mode only).
     */
    public void paletteChanged() {
	surfaceDataSw.setNeedsBackup(true);
    }

    /**
     * Releases any associated hardware memory for this image by
     * calling flush on hw surfaceData objects.
     */
    public void flush() {
	surfaceLossHw = true;
	surfaceDataHw = null;
	Hashtable oldHWSD = cachedHWSD;
	cachedHWSD = new Hashtable();
	Object array[] = oldHWSD.values().toArray();
	for (int i = 0; i < array.length; i++) {
	    ((Win32OffScreenSurfaceData)array[i]).flush();
	}
    }

}
