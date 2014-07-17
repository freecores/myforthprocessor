/*
 * @(#)ImageWatched.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.image;

import java.util.Vector;
import java.util.Enumeration;
import java.util.NoSuchElementException;
import java.awt.Image;
import java.awt.image.ImageObserver;

public class ImageWatched {
    public ImageWatched() {
    }

    protected Vector watchers;

    public synchronized void addWatcher(ImageObserver iw) {
	if (iw != null && !isWatcher(iw)) {
	    if (watchers == null) {
		watchers = new Vector();
	    }
	    watchers.addElement(iw);
	}
    }

    public synchronized boolean isWatcher(ImageObserver iw) {
	return (watchers != null && iw != null && watchers.contains(iw));
    }

    public synchronized void removeWatcher(ImageObserver iw) {
	if (iw != null && watchers != null) {
	    watchers.removeElement(iw);
	    if (watchers.size() == 0) {
		watchers = null;
	    }
	}
    }

    public void newInfo(Image img, int info, int x, int y, int w, int h) {
	if (watchers != null) {
	    Enumeration enum = watchers.elements();
	    Vector uninterested = null;
	    while (enum.hasMoreElements()) {
		ImageObserver iw;
		try {
		    iw = (ImageObserver) enum.nextElement();
		} catch (NoSuchElementException e) {
		    break;
		}
		if (!iw.imageUpdate(img, info, x, y, w, h)) {
		    if (uninterested == null) {
			uninterested = new Vector();
		    }
		    uninterested.addElement(iw);
		}
	    }
	    if (uninterested != null) {
		enum = uninterested.elements();
		while (enum.hasMoreElements()) {
		    ImageObserver iw = (ImageObserver) enum.nextElement();
		    removeWatcher(iw);
		}
	    }
	}
    }
}
