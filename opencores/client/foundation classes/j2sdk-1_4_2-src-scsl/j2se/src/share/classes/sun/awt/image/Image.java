/*
 * @(#)Image.java	1.61 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.util.Hashtable;
import java.util.Enumeration;

import java.awt.Component;
import java.awt.Color;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.ImageProducer;
import java.awt.image.ImageConsumer;
import java.awt.image.ImageObserver;
import sun.awt.image.ImageRepresentation;
import sun.awt.image.FileImageSource;

public abstract class Image extends java.awt.Image {
    /**
     * The object which is used to reconstruct the original image data
     * as needed.
     */
    ImageProducer source;

    InputStreamImageSource src;

    ImageRepresentation imagerep;

    static {
        /* ensure that the necessary native libraries are loaded */
	NativeLibLoader.loadLibraries();
    }

    protected Image() {
    }

    /**
     * Construct an image from an ImageProducer object.
     */
    protected Image(ImageProducer is) {
	source = is;
	if (is instanceof InputStreamImageSource) {
	    src = (InputStreamImageSource) is;
	}
    }

    public ImageProducer getSource() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	return source;
    }

    private int width = -1;
    private int height = -1;
    private Hashtable properties;

    private int availinfo;

    /**
     * Return the width of the original image source.
     * If the width isn't known, then the image is reconstructed.
     */
    public int getWidth() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.WIDTH) == 0) {
	    reconstruct(ImageObserver.WIDTH);
	}
	return width;
    }

    /**
     * Return the width of the original image source.
     * If the width isn't known, then the ImageObserver object will be
     * notified when the data is available.
     */
    public synchronized int getWidth(ImageObserver iw) {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.WIDTH) == 0) {
	    addWatcher(iw, true);
	    if ((availinfo & ImageObserver.WIDTH) == 0) {
		return -1;
	    }
	}
	return width;
    }

    /**
     * Return the height of the original image source.
     * If the height isn't known, then the image is reconstructed.
     */
    public int getHeight() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.HEIGHT) == 0) {
	    reconstruct(ImageObserver.HEIGHT);
	}
	return height;
    }

    /**
     * Return the height of the original image source.
     * If the height isn't known, then the ImageObserver object will be
     * notified when the data is available.
     */
    public synchronized int getHeight(ImageObserver iw) {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.HEIGHT) == 0) {
	    addWatcher(iw, true);
	    if ((availinfo & ImageObserver.HEIGHT) == 0) {
		return -1;
	    }
	}
	return height;
    }

    /**
     * Return a property of the image by name.  Individual property names
     * are defined by the various image formats.  If a property is not
     * defined for a particular image, then this method will return the
     * UndefinedProperty object.  If the properties for this image are
     * not yet known, then this method will return null and the ImageObserver
     * object will be notified later.  The property name "comment" should
     * be used to store an optional comment which can be presented to
     * the user as a description of the image, its source, or its author.
     */
    public Object getProperty(String name, ImageObserver observer) {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if (properties == null) {
	    addWatcher(observer, true);
	    if (properties == null) {
		return null;
	    }
	}
	Object o = properties.get(name);
	if (o == null) {
	    o = Image.UndefinedProperty;
	}
	return o;
    }

    public boolean hasError() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	return (availinfo & ImageObserver.ERROR) != 0;
    }

    public int check(ImageObserver iw) {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.ERROR) == 0 &&
	    ((~availinfo) & (ImageObserver.WIDTH |
			     ImageObserver.HEIGHT |
			     ImageObserver.PROPERTIES)) != 0) {
	    addWatcher(iw, false);
	}
	return availinfo;
    }

    public void preload(ImageObserver iw) {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if ((availinfo & ImageObserver.ALLBITS) == 0) {
	    addWatcher(iw, true);
	}
    }

    private synchronized void addWatcher(ImageObserver iw, boolean load) {
	if ((availinfo & ImageObserver.ERROR) != 0) {
	    if (iw != null) {
		iw.imageUpdate(this, ImageObserver.ERROR|ImageObserver.ABORT,
			       -1, -1, -1, -1);
	    }
	    return;
	}
	ImageRepresentation ir = getImageRep();
	ir.addWatcher(iw);
	if (load) {
	    ir.startProduction();
	}
    }

    private synchronized void reconstruct(int flags) {
	if ((flags & ~availinfo) != 0) {
	    if ((availinfo & ImageObserver.ERROR) != 0) {
		return;
	    }
	    ImageRepresentation ir = getImageRep();
	    ir.startProduction();
	    while ((flags & ~availinfo) != 0) {
		try {
		    wait();
		} catch (InterruptedException e) {
		    Thread.currentThread().interrupt();
		    return;
		}
		if ((availinfo & ImageObserver.ERROR) != 0) {
		    return;
		}
	    }
	}
    }

    synchronized void addInfo(int newinfo) {
	availinfo |= newinfo;
	notifyAll();
    }

    void setDimensions(int w, int h) {
	width = w;
	height = h;
	addInfo(ImageObserver.WIDTH | ImageObserver.HEIGHT);
    }

    void setProperties(Hashtable props) {
	if (props == null) {
	    props = new Hashtable();
	}
	properties = props;
	addInfo(ImageObserver.PROPERTIES);
    }

    synchronized void infoDone(int status) {
	if (status == ImageConsumer.IMAGEERROR ||
	    ((~availinfo) & (ImageObserver.WIDTH |
			     ImageObserver.HEIGHT)) != 0) {
	    addInfo(ImageObserver.ERROR);
	} else if ((availinfo & ImageObserver.PROPERTIES) == 0) {
	    setProperties(null);
	}
    }

    public void flush() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}

        ImageRepresentation ir;
        synchronized (this) {
            availinfo &= ~ImageObserver.ERROR;
            ir = imagerep;
            imagerep = null;
        }
        if (ir != null) {
            ir.abort();
        }
        if (src != null) {
            src.flush();
	}
    }

    protected abstract ImageRepresentation makeImageRep();

    public synchronized ImageRepresentation getImageRep() {
	if (src != null) {
	    src.checkSecurity(null, false);
	}
	if (imagerep == null) {
	    imagerep = makeImageRep();
	}
	return imagerep;
    }

    /* this method is needed by printing code */
    public ColorModel getColorModel() {
        ImageRepresentation imageRep = getImageRep();
        return imageRep.getColorModel();
    }

    /* this method is needed by printing code */
    public BufferedImage getBufferedImage() {
        ImageRepresentation imageRep = getImageRep();
        return imageRep.getBufferedImage();
    }
}