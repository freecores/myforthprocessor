/*
 * @(#)X11ImageRepresentation.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.Transparency;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.IndexColorModel;
import java.awt.image.WritableRaster;
import sun.awt.image.Image;
import sun.awt.image.ImageRepresentation;
import sun.awt.X11SurfaceData;
import java.util.Hashtable;

public class X11ImageRepresentation extends ImageRepresentation {

    public X11ImageRepresentation(sun.awt.image.Image im, ColorModel cmodel, 
                                  boolean forceCMhint) 
    {
        super(im, cmodel, forceCMhint);
    }

    protected BufferedImage createImage(ColorModel cm, 
                                        WritableRaster raster, 
                                        boolean isRasterPremultiplied,
                                        Hashtable properties) 
    {
	if (X11SurfaceData.isAccelerationEnabled()) {
	    if (cm.getTransparency() == Transparency.OPAQUE) {
		return new X11OffScreenImage(null, cm, raster,
					     isRasterPremultiplied, false);
	    } else if (cm.getTransparency() == Transparency.BITMASK) {
                // 4673490: we can't easily handle ByteBinary data in
                // X11OSI.updateBitmask(), so we should avoid creating an
                // X11OSI in this situation
                if (!(cm instanceof IndexColorModel &&
                      cm.getPixelSize() < 8))
                {
                    return new X11OffScreenImage(null, cm, raster,
                                                 isRasterPremultiplied, true);
                }
	    } 
	}

	return super.createImage(cm, raster, isRasterPremultiplied, 
				 properties);
    }
}
