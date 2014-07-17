/*
 * @(#)WImageRepresentation.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Transparency;

import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.WritableRaster;
import sun.awt.image.Image;
import sun.awt.image.ImageRepresentation;
import java.util.Hashtable;

class WImageRepresentation extends ImageRepresentation {

    ColorModel deviceCM;

    public WImageRepresentation(sun.awt.image.Image im, ColorModel cmodel, 
                                boolean forceCMhint) 
    {
        super(im, cmodel, forceCMhint);

        if (forceCMhint) {
            deviceCM = cmodel;
        } else {
            deviceCM = this.cmodel;
        }
    }

    protected BufferedImage createImage(ColorModel cm, 
                                        WritableRaster raster, 
                                        boolean isRasterPremultiplied,
                                        Hashtable properties) 
    {
        if (Win32PeerlessImage.isValidOpaqueConfig(cm, deviceCM, 0)) {
            return new Win32PeerlessImage(cm, deviceCM, raster,
                                          isRasterPremultiplied, Transparency.OPAQUE);
        } else if (Win32PeerlessImage.isValidBitmaskConfig(cm, deviceCM, 0)) {
            return new Win32PeerlessImage(cm, deviceCM, raster,
                                          isRasterPremultiplied, Transparency.BITMASK);
        } else if (Win32PeerlessImage.isValidTranslucentConfig(cm, deviceCM, 0)) {
	    ColorModel cmSw = sun.awt.Win32GraphicsConfig.getTranslucentColorModel();
	    // REMIND: currently we accelerate only images 
	    // with compatible raster, because we assume that software
	    // and hardware surfaces are of the same type. This may be
	    // changed later if we decide to support the model we have for
	    // opaque and translucent accelerated images
	    if (cmSw.isCompatibleRaster(raster)) {
		return new Win32PeerlessImage(cmSw, cmSw, raster,
					      isRasterPremultiplied, Transparency.TRANSLUCENT);
	    }
        }
	return super.createImage(cm, raster, isRasterPremultiplied, properties);
    }
}
