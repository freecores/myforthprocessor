/*
 * @(#)X11VolatileImage.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.awt.*;
import java.awt.image.BufferedImage;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.awt.image.SunVolatileImage;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11SurfaceData;
import sun.awt.X11SurfaceData.X11PixmapSurfaceData;

/**
 * X11 platform implementation of the VolatileImage class.
 * The class attempts to create
 * and use a pixmap-based surfaceData object (X11PixmapSurfaceData).
 * If this object cannot be created or re-created as necessary, the
 * class falls back to a software-based SurfaceData object 
 * (BufImgSurfaceData) that will be used until the hardware-based
 * surfaceData can be restored.
 */
public class X11VolatileImage extends SunVolatileImage 
{
    private long drawable;

    /**
     * Enable/disable use of acceleration based on system properties
     */
    static {
	accelerationEnabled = X11SurfaceData.isAccelerationEnabled();
    }
    
    /**
     * Constructor for X11-based VolatileImage using Component
     */
    public X11VolatileImage(Component c, int width, int height)
    {
	super(c, width, height);
    }

    /**
     * Constructor for X11-based VolatileImage using Component and back buffer
     */
    public X11VolatileImage(Component c, int width, int height, long drawable)
    {
	super(c, width, height, new Long(drawable));
	if (drawable != 0 && !accelerationEnabled) {
	    initAcceleratedBackground();
	}
    }

    /**
     * Constructor for X11-based VolatileImage using GraphicsConfiguration
     */
    public X11VolatileImage(GraphicsConfiguration graphicsConfig, int width, int height)
    {
	super(graphicsConfig, width, height);
    }

    /**
     * Create a pixmap-based surfaceData object  
     */    
    public void initAcceleratedBackground() {
	X11GraphicsConfig gc;
	if (comp != null) {
	    gc = X11SurfaceData.getGC((MComponentPeer)comp.getPeer());
	} else {
	    gc = (X11GraphicsConfig)this.graphicsConfig;
	}
	try {
            long drawable = 0;
            if (context instanceof Long) {
                drawable = ((Long)context).longValue();
	    }
	    surfaceDataHw = 
		X11PixmapSurfaceData.createData(gc, 
						getWidth(), getHeight(),
						getColorModel(), this, drawable, 0);
	    surfaceData = surfaceDataHw;
	} catch (NullPointerException ex) {
	    surfaceDataHw = null;
	    surfaceData = getSurfaceDataSw();
	} catch (OutOfMemoryError er) {
	    surfaceDataHw = null;
	    surfaceData = getSurfaceDataSw();
	}
    }

    /**
     * X11 needs to ensure that the given GC is compatible with
     * the one that this image was created with.
     */
    protected boolean isGCValid(GraphicsConfiguration gc) {
	// REMIND: we might be too paranoid here, requiring that
	// the GC be exactly the same as the original one.  The
	// real answer is one that guarantees that pixmap copies
	// will be correct (which requires like bit depths and
	// formats).
	return (super.isGCValid(gc) &&
		((gc == null) || (gc == graphicsConfig)));
    }

    /**
     * Return true if copies to destSD can be accelerated
     */
    protected boolean destSurfaceAccelerated(SurfaceData destSD) {
	return (destSD instanceof X11SurfaceData);
    }

}
