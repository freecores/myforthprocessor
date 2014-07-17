/*
 * @(#)Win32BackBufferSurfaceData.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.GraphicsConfiguration;
import java.awt.Image;
import java.awt.Transparency;
import java.awt.image.ColorModel;
import sun.awt.Win32GraphicsDevice;
import sun.java2d.loops.SurfaceType;

public class Win32BackBufferSurfaceData extends Win32OffScreenSurfaceData {
    
    Win32SurfaceData parentData;
    
    protected Win32BackBufferSurfaceData(int width, int height,
        SurfaceType sType, ColorModel cm, GraphicsConfiguration graphicsConfig,
        Image image, Win32SurfaceData parentData) {
        super(width, height, sType, cm, graphicsConfig, image, Transparency.OPAQUE);
        this.parentData = parentData;
    }
    
    public void initSurface(int depth, int width, int height, int screen, 
			    boolean transparent) {
        // Do nothing; delay initializing the surface to the method below
    }
    
    private native void initSurface(int depth, int width, int height,
        int screen, Win32SurfaceData parentData);
    
    public void restoreSurface() {
        parentData.restoreSurface();
    }
    
    public static Win32BackBufferSurfaceData createData(int width, int height,
        ColorModel cm, GraphicsConfiguration graphicsConfig, Image image,
        Win32SurfaceData parentData) {
        Win32BackBufferSurfaceData ret =
            new Win32BackBufferSurfaceData(width, height,
		getSurfaceType(cm, Transparency.OPAQUE), 
		cm, graphicsConfig, image, parentData);
        Win32GraphicsDevice gd = 
            (Win32GraphicsDevice)graphicsConfig.getDevice();
        ret.initSurface(cm.getPixelSize(), width, height, gd.getScreen(),
            parentData);
        // d3dClippingEnabled is set during the call to initSurface
        if (ret.d3dClippingEnabled) {
	    ret.d3dPipe = d3dClipPipe;
	    ret.d3dTxPipe = d3dTxClipPipe;
	} else {
	    ret.d3dPipe = d3dNoClipPipe;
	    ret.d3dTxPipe = d3dTxNoClipPipe;
	}
        return ret;
    }
}

