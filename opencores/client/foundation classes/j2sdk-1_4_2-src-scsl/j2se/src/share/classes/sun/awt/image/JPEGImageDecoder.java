/*
 * @(#)JPEGImageDecoder.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*-
 *	Reads JPEG images from an InputStream and reports the
 *	image data to an InputStreamImageSource object.
 *
 * The native implementation of the JPEG image decoder was adapted from
 * release 6 of the free JPEG software from the Independent JPEG Group.
 */
package sun.awt.image;

import java.util.Vector;
import java.util.Hashtable;
import java.io.InputStream;
import java.io.IOException;
import java.awt.image.*;

/**
 * JPEG Image converter
 *
 * @version 1.20 01/23/03
 * @author Jim Graham
 */
public class JPEGImageDecoder extends ImageDecoder {
    private static ColorModel RGBcolormodel;
    private static ColorModel ARGBcolormodel;
    private static ColorModel Graycolormodel;

    private static final Class InputStreamClass = InputStream.class;
    private static native void initIDs(Class InputStreamClass);

    private ColorModel colormodel;

    static {
        java.security.AccessController.doPrivileged(
		  new sun.security.action.LoadLibraryAction("jpeg"));
	initIDs(InputStreamClass);
	RGBcolormodel = new DirectColorModel(24, 0xff0000, 0xff00, 0xff);
        ARGBcolormodel = ColorModel.getRGBdefault();
	byte g[] = new byte[256];
	for (int i = 0; i < 256; i++) {
	    g[i] = (byte) i;
	}
	Graycolormodel = new IndexColorModel(8, 256, g, g, g);
    }

    private native void readImage(InputStream is, byte buf[])
	throws ImageFormatException, IOException;

    PixelStore store;
    Hashtable props = new Hashtable();

    public JPEGImageDecoder(InputStreamImageSource src, InputStream is) {
	super(src, is);
    }

    public synchronized boolean catchupConsumer(InputStreamImageSource src,
						ImageConsumer ic)
    {
	return ((store == null) || store.replay(src, ic));
    }

    public synchronized void makeStore(int width, int height, boolean gray) {
	if (gray) {
	    store = new PixelStore8(width, height);
	} else {
	    store = new PixelStore32(width, height);
	}
    }

    /**
     * An error has occurred. Throw an exception.
     */
    private static void error(String s1) throws ImageFormatException {
	throw new ImageFormatException(s1);
    }

    public boolean sendHeaderInfo(int width, int height,
				  boolean gray, boolean hasalpha,
                                  boolean multipass) 
    {
	setDimensions(width, height);
	makeStore(width, height, gray);

	setProperties(props);
	store.setProperties(props);

        if (gray) {
            colormodel = Graycolormodel;
        } else {
            if (hasalpha) {
                colormodel = ARGBcolormodel;
            } else {
                colormodel = RGBcolormodel;
            }
        }

	setColorModel(colormodel);
	store.setColorModel(colormodel);

	int flags = hintflags;
	if (!multipass) {
	    flags |= ImageConsumer.SINGLEPASS;
	}
	setHints(hintflags);
	store.setHints(hintflags);

	headerComplete();

	return true;
    }

    public boolean sendPixels(int pixels[], int y) {
	int count = setPixels(0, y, pixels.length, 1, colormodel,
			      pixels, 0, pixels.length);
	if (count > 0) {
	    store.setPixels(0, y, pixels.length, 1, pixels, 0, pixels.length);
	} else {
	    aborted = true;
	}
	return !aborted;
    }

    public boolean sendPixels(byte pixels[], int y) {
	int count = setPixels(0, y, pixels.length, 1, colormodel,
			      pixels, 0, pixels.length);
	if (count > 0) {
	    store.setPixels(0, y, pixels.length, 1, pixels, 0, pixels.length);
	} else {
	    aborted = true;
	}
	return !aborted;
    }

    /**
     * produce an image from the stream.
     */
    public void produceImage() throws IOException, ImageFormatException {
	try {
	    readImage(input, new byte[1024]);
	    if (!aborted) {
		store.imageComplete();
		if (store.getBitState() != PixelStore.BITS_LOST) {
		    source.setPixelStore(this, store);
		}
		imageComplete(ImageConsumer.STATICIMAGEDONE, true);
	    }
	} catch (IOException e) {
	    if (!aborted) {
		throw e;
	    }
	} finally {
	    close();
	}
    }

    /**
     * The ImageConsumer hints flag for a JPEG image.
     */
    private static final int hintflags =
	ImageConsumer.TOPDOWNLEFTRIGHT | ImageConsumer.COMPLETESCANLINES |
	ImageConsumer.SINGLEFRAME;
}
