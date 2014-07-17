/*
 * @(#)RasterListener.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;


/**
 * This interface is implemented by any object that wants notification
 * when an associated Raster has been modified.
 */
public interface RasterListener {

    /**
     * Invoked when the raster's contents have changed (via one of the 
     * modifier methods in WritableRaster such as setPixel())
     */
    public void rasterChanged();

    /**
     * Invoked when the raster's contents have been taken (via the 
     * Raster.getDataBuffer() method)
     */
    public void rasterStolen();

}
