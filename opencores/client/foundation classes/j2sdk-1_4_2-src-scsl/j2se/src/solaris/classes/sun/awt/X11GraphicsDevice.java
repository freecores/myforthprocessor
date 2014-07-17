/*
 * @(#)X11GraphicsDevice.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.DisplayMode;
import java.awt.GraphicsEnvironment;
import java.awt.GraphicsDevice;
import java.awt.GraphicsConfiguration;
import java.awt.Rectangle;
import java.util.HashSet;

/**
 * This is an implementation of a GraphicsDevice object for a single
 * X11 screen.
 *
 * @see GraphicsEnvironment
 * @see GraphicsConfiguration
 * @version 10 Feb 1997
 */
public class X11GraphicsDevice extends GraphicsDevice {
    int screen;
    private static DisplayMode displayMode;

    public X11GraphicsDevice(int screennum) {
	this.screen = screennum;
    }

    /*
     * Initialize JNI field and method IDs for fields that may be
     * accessed from C.
     */
    private static native void initIDs();

    static {
        if (!GraphicsEnvironment.isHeadless()) {
            initIDs();
        }
    }

    /**
     * Returns the X11 screen of the device.
     */
    public int getScreen() {
        return screen;
    }

    /**
     * Returns the X11 Display of this device.
     * This method is also in MDrawingSurfaceInfo but need it here
     * to be able to allow a GraphicsConfigTemplate to get the Display.
     */
    public native long getDisplay();

    /**
     * Returns the type of the graphics device.
     * @see #TYPE_RASTER_SCREEN
     * @see #TYPE_PRINTER
     * @see #TYPE_IMAGE_BUFFER
     */
    public int getType() {
	return TYPE_RASTER_SCREEN;
    }

    /**
     * Returns the identification string associated with this graphics
     * device.
     */
    public String getIDstring() {
	return ":0."+screen;
    }


    GraphicsConfiguration[] configs;
    GraphicsConfiguration defaultConfig;
    HashSet doubleBufferVisuals;

    /**
     * Returns all of the graphics
     * configurations associated with this graphics device.
     */
    public GraphicsConfiguration[] getConfigurations() {
	GraphicsConfiguration[] ret = configs;
	if (ret == null) {
	    int i = 0;
	    int num = getNumConfigs(screen);
	    ret = new GraphicsConfiguration[num];
	    if (defaultConfig != null) {
	        ret [0] = defaultConfig;
		i++;
	    }
	    if (isDBESupported() && doubleBufferVisuals == null) {
	        doubleBufferVisuals = new HashSet();
	        getDoubleBufferVisuals(screen);
	    }
	    for ( ; i < num; i++) {
	        int visNum = getConfigVisualId (i, screen);
		boolean doubleBuffer = (isDBESupported()
		    && doubleBufferVisuals.contains(new Integer(visNum)));
		ret[i] = X11GraphicsConfig.getConfig(this, visNum,
		    doubleBuffer);
	    }
	    configs = ret;
	}
	return ret;
    }

    /*
     * Returns the number of X11 visuals representable as an
     * X11GraphicsConfig object.
     */
    public native int getNumConfigs(int screen);

    /*
     * Returns the visualid for the given index of graphics configurations.
     */
    public native int getConfigVisualId (int index, int screen);

    // Whether or not double-buffering extension is supported
    public static native boolean isDBESupported();
    // Callback for adding a new double buffer visual into our set
    private void addDoubleBufferVisual(int visNum) {
        doubleBufferVisuals.add(new Integer(visNum));
    }
    // Enumerates all visuals that support double buffering
    private native void getDoubleBufferVisuals(int screen);
    
    /**
     * Returns the default graphics configuration
     * associated with this graphics device.
     */
    public GraphicsConfiguration getDefaultConfiguration() {
	if (defaultConfig == null) {
	   int visNum = getConfigVisualId (0, screen);
	   //defaultConfig = new GraphicsConfiguration [1];
           boolean doubleBuffer = false;
           if (isDBESupported() && doubleBufferVisuals == null) {
               doubleBufferVisuals = new HashSet();
               getDoubleBufferVisuals(screen);
               doubleBuffer = doubleBufferVisuals.contains(
                   new Integer(visNum));
           }
           defaultConfig = X11GraphicsConfig.getConfig (this, visNum,
               doubleBuffer);
	}

	return defaultConfig;
    }

    public DisplayMode getDisplayMode() {
        if (displayMode == null) {
            GraphicsConfiguration gc = getDefaultConfiguration();
            Rectangle r = gc.getBounds();
            displayMode = new DisplayMode(r.width, r.height,
                DisplayMode.BIT_DEPTH_MULTI, DisplayMode.REFRESH_RATE_UNKNOWN);
        }
        return displayMode;
    }

    public String toString() {
	return ("X11GraphicsDevice[screen="+screen+"]");
    }

}
