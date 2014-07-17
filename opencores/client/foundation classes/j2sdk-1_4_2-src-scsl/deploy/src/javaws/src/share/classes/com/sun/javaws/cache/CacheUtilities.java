/*
 * @(#)CacheUtilities.java	1.8 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.cache;

import com.sun.javaws.*;
import com.sun.javaws.debug.*;
import java.awt.*;
import java.io.*;
import java.net.*;

/**
 *  CacheUtilities
 *
 * Handful of utilities for interacting with the cache.
 *
 * @version 1.8, 01/23/03
 */
public class CacheUtilities {
    private static CacheUtilities _instance = null;
    /**
     * Used for loading images.
     */
    private Component _component;
    
    /** Returns singleton instance of the CacheUtilities */
    static public CacheUtilities getSharedInstance() {
	if (_instance == null) {
	    synchronized(CacheUtilities.class) {
		if (_instance == null) {
		    _instance = new CacheUtilities();
		}
	    }
	}
	return _instance;
    }


    private CacheUtilities() {}

    /**
     * Loads the image at the path <code>path</code>.
     */
    public Image loadImage(String path) throws IOException {
        Image image = Toolkit.getDefaultToolkit().createImage(path);
        if (image != null) {
            Component c = getComponent();
            MediaTracker mt = new MediaTracker(c);
            mt.addImage(image, 0);
            try {
                // Give it 5 seconds to download.
                mt.waitForID(0, 5000);
            } catch (InterruptedException e) {
                throw new IOException("Failed to load");
            }
            return image;
        }
        return null;
    }

    /**
     * Returns a component that can be used for image loading.
     */
    private Component getComponent() {
        if (_component == null) {
            synchronized(this) {
                if (_component == null) {
                    _component = new Component() {};
                }
            }
        }
        return _component;
    }
}
