/*
 * @(#)CacheImageLoaderCallback.java	1.2 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.cache;

import com.sun.javaws.debug.*;
import com.sun.javaws.jnl.IconDesc;
import java.awt.Image;
import java.io.File;

/**
 * interface to give image loader so it can tell you
 * 1) when it has something you can display, and
 * 2) when it has the updated, cached image for you to display.
 */
public interface CacheImageLoaderCallback {
    public void imageAvailable(IconDesc id, Image image, File file);
    public void finalImageAvailable(IconDesc id, Image image, File file);
}
