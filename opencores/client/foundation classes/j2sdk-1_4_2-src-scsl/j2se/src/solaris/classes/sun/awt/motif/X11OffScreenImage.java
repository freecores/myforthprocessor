/*
 * @(#)X11OffScreenImage.java	1.20 03/03/19
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.GraphicsConfiguration;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;
import java.util.Enumeration;
import java.util.Hashtable;
import sun.java2d.DisposerRecord;
import sun.java2d.DisposerTarget;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.java2d.loops.CompositeType;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11GraphicsDevice;
import sun.awt.X11SurfaceData;
import sun.awt.image.AcceleratedOffScreenImage;
import sun.awt.image.BufImgSurfaceData;
import sun.awt.X11SurfaceData.X11PixmapSurfaceData;
import java.awt.GraphicsEnvironment;


public class X11OffScreenImage extends AcceleratedOffScreenImage 
    implements DisposerTarget {

    private long drawable;
    private Hashtable cachedHWSD = new Hashtable();

    /**
     * Hashtable of bitmasks per graphics device.
     * Bitmask is 1 bit Pixmap, created on native level and 
     * based on software SD.
     * The bitmasks get updated when the image is changed.
     */
    private Hashtable bitmasks;
    private boolean transparent;
    private boolean bitmasksUpdateNeeded;
    private Object disposerReferent = new Object();

    /**
     * Enable/disable use of acceleration based on system properties
     */
    static {
	accelerationEnabled = X11SurfaceData.isAccelerationEnabled();
    }

    public X11OffScreenImage(Component c, ColorModel cm, WritableRaster raster,
			     boolean isRasterPremultiplied, boolean transparent) {
	this(c, cm, raster, isRasterPremultiplied, 0, transparent);
    }

    public X11OffScreenImage(Component c, ColorModel cm, WritableRaster raster,
			     boolean isRasterPremultiplied, long drawable, boolean transparent)
    {
	super(c, cm, raster, isRasterPremultiplied);
	surfaceDataSw = surfaceData;
        this.drawable = drawable;
	this.transparent = transparent;
	if (transparent) {
	    bitmasks = new Hashtable();
	}
	bitmasksUpdateNeeded = transparent;
    }

    protected boolean isTransparent() {
        return transparent;
    }

    protected boolean isValidHWSD(GraphicsConfiguration gc) {
	return (surfaceDataHw != null && surfaceDataHw == cachedHWSD.get(gc));
    }

    protected boolean operationSupported(CompositeType comp, 
					 Color bgColor, boolean scale) {
	// We don't have X11 scale loops, so always use
	// software surface in case of scaling
	if (!scale) {
	    if (!transparent) {
		// we save a read over the wire for compositing
		// operations by copying from the buffered image sd
		if (CompositeType.SrcOverNoEa.equals(comp) ||
		    CompositeType.SrcNoEa.equals(comp)) {
		    return true;
		}
	    } else {
		// for transparent images SrcNoEa+bgColor has the
		// same effect as SrcOverNoEa+bgColor, so we allow
		// copying from pixmap sd using accelerated blitbg loops:
		// SrcOver will be changed to SrcNoEa in DrawImage.blitSD
		if (CompositeType.SrcOverNoEa.equals(comp) || 
		    (CompositeType.SrcNoEa.equals(comp) && bgColor != null)) {
		    return true;
		}
	    }
	}
	return false;
    }

    public void initAcceleratedBackground(GraphicsConfiguration gc, int width, int height) {
	try {
	    // Don't need to update the bitmasks unless sw sd needs backup.
	    // Need to set the flag here because needsBackup will be
	    // set to true in AOSI.validate so we could update
	    // the bitmask unnecessarily.
	    bitmasksUpdateNeeded = surfaceDataSw.needsBackup();

	    surfaceDataHw = (SurfaceData)cachedHWSD.get(gc);
	    if (surfaceDataHw == null) {
		int bm = 0;
		if (transparent) {
		    Integer bmInt = (Integer)bitmasks.get(gc.getDevice());
		    if (bmInt == null) {
			// create new bitmask for this device
			bm = updateBitmask(surfaceDataSw, 0 /* means create a new one */, 
					   ((X11GraphicsDevice)gc.getDevice()).getScreen(),
					   getWidth(), getHeight());
			if (bm != 0) {
			    synchronized (bitmasks) {
				bitmasks.put(gc.getDevice(), new Integer(bm));
			    }
			}
		    } else {
			// already have a bitmask for this device
			bm = bmInt.intValue();
		    }
		}
		surfaceDataHw = 
		    X11PixmapSurfaceData.createData((X11GraphicsConfig)gc,
						    getWidth(), getHeight(),
						    gc.getColorModel(), this,
						    drawable, bm);
		if (surfaceDataHw != null) {
		    cachedHWSD.put(gc, surfaceDataHw);
		}
	    }
	} catch (NullPointerException ex) {
	    surfaceDataHw = null;
	} catch (OutOfMemoryError er) {
	    surfaceDataHw = null;
	}
    }

    private native int updateBitmask(SurfaceData sd, int oldBitmask, 
				      int screen, int width, int height);

    public void updateBitmasks() {
	synchronized (bitmasks) {
	    for (Enumeration keys = bitmasks.keys(); keys.hasMoreElements() ;) {
		X11GraphicsDevice gd = (X11GraphicsDevice)keys.nextElement();
		int bm = ((Integer)bitmasks.get(gd)).intValue();
		updateBitmask(surfaceDataSw, bm, gd.getScreen(), 
			      getWidth(), getHeight());
	    }
	}
    }

    protected void copyBackupToAccelerated() {
	super.copyBackupToAccelerated();
	if (transparent && surfaceDataHw != null && bitmasksUpdateNeeded) {
	    updateBitmasks();
	}
	bitmasksUpdateNeeded = true;
    }

    public Graphics getGraphics() {
	return createGraphics();
    }

    /**
     * If this image has been created without a peer, we must use the backup
     * means of creating a Graphics2D object.
     */
    public Graphics2D createGraphics() {
	if (c != null) {
	    return super.createGraphics();
	}
        GraphicsEnvironment env =
            GraphicsEnvironment.getLocalGraphicsEnvironment();
        return env.createGraphics(this);
    }

    protected boolean destSurfaceAccelerated(SurfaceData destSD) {
	return (destSD instanceof X11SurfaceData);
    }

    public Object getDisposerReferent() {
	return disposerReferent;
    }
    
    public void setDisposerRecord(DisposerRecord rec) {
    }
}
