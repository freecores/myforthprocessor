/*
 * @(#)Win32BackBuffer.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Component;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;

public class Win32BackBuffer extends WVolatileImage {
    
    /**
     * Create an image for an attached surface
     */
    public Win32BackBuffer(Component c, Win32SurfaceData parentData) {
        super(c, c.getWidth(), c.getHeight(), parentData);
    }
    
    protected void fallBackToSoftware() {
        // Do nothing when falling back to software
    }
    
    protected Win32OffScreenSurfaceData createHWData() {
        return Win32BackBufferSurfaceData.createData(getWidth(),
            getHeight(), getDeviceColorModel(), graphicsConfig, this,
            (Win32SurfaceData)context);
    }
    
    public void displayChanged() {
        // Recreate java object with new display parameters
	initAcceleratedBackground();
    }
    
    public Win32BackBufferSurfaceData getHWSurfaceData() {
        if (surfaceData instanceof Win32BackBufferSurfaceData) {
            return (Win32BackBufferSurfaceData)surfaceData;
        } else {
            return null;
        }
    }
}

