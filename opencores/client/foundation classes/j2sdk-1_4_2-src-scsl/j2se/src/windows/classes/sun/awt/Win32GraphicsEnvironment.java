/*
 * @(#)Win32GraphicsEnvironment.java	1.24 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import sun.java2d.SunGraphicsEnvironment;
import java.awt.GraphicsDevice;
import java.awt.Toolkit;
import java.awt.print.PrinterJob;
import sun.awt.font.NativeFontWrapper;
import sun.awt.windows.WPrinterJob;
import sun.awt.windows.WToolkit;
import sun.awt.DisplayChangedListener;
import sun.awt.SunDisplayChanger;
import sun.awt.windows.WFontProperties;

import java.io.File;

/**
 * This is an implementation of a GraphicsEnvironment object for the
 * default local GraphicsEnvironment used by the JavaSoft JDK in Win32
 * environments.
 *
 * @see GraphicsDevice
 * @see GraphicsConfiguration
 * @version 10 Feb 1997
 */

public class Win32GraphicsEnvironment extends SunGraphicsEnvironment
 implements DisplayChangedListener {

    SunDisplayChanger displayChanger = new SunDisplayChanger();

    static {
	java.security.AccessController.doPrivileged(
		    new sun.security.action.LoadLibraryAction("awt"));
	initDisplayWrapper();
    }

    private static native void initDisplay();

    private static boolean displayInitialized;	    // = false;
    public static void initDisplayWrapper() {
	if (!displayInitialized) {
	    displayInitialized = true;
	    initDisplay();
	}
    }

    public Win32GraphicsEnvironment() {
    }

    protected native int getNumScreens();
    protected native int getDefaultScreen();

    public GraphicsDevice getDefaultScreenDevice() {
	return getScreenDevices()[getDefaultScreen()];
    }
  
/*
 * ----DISPLAY CHANGE SUPPORT----
 */

    /*
     * From DisplayChangeListener interface.
     * Called from WToolkit and executed on the event thread when the
     * display settings are changed.
     */
    public void displayChanged() {

        // 1. Reset screen array, saving old array
        GraphicsDevice[] oldgds = resetDisplays();

        // 2. Reset the static GC for the (possibly new) default screen
        WToolkit.resetGC();

        // 3. Tell screens in old screen array to do display update stuff
        //    This will update all top-levels windows so they can be added
        //    to the new graphics devices.

        //NOTE: for multiscreen, this could only reset the devices that
        //changed.  Be careful of situations where screens are added
        //as well as removed, yielding no net change in the number of
        //displays.  For now, we're being robust and doing them all.

        for(int s = 0; s < oldgds.length; s++) {
            if (oldgds[s] instanceof Win32GraphicsDevice) {
                ((Win32GraphicsDevice)oldgds[s]).displayChanged();
            }
        }

        // 4. Do displayChanged for SunDisplayChanger list (i.e. WVolatileImages
        //    and Win32OffscreenImages)
        displayChanger.notifyListeners();

    }

    /**
     * Part of the DisplayChangedListener interface: 
     * propagate this event to listeners
     */
    public void paletteChanged() {
        displayChanger.notifyPaletteChanged();
    }

    /*
     * Updates the array of screen devices to the current configuration.
     * Returns the previous array of screen devices.
     */
    public synchronized GraphicsDevice[] resetDisplays() {
        // REMIND : We should go through the array and update any displays
        // which may have been added or removed.  Also, we should update
        // any display state information which may now be different.
        // Note that this function used to re-create all Java
        // Win32GraphicsDevice objects.  We cannot do this for many reasons,
        // since those objects keep state that is independent of display
        // changes, and because programs may keep references to those
        // java objects.
        GraphicsDevice[] ret = screens;
        return ret;
    }

    /*
     * Add a DisplayChangeListener to be notified when the display settings
     * are changed.  
     */
    public void addDisplayChangedListener(DisplayChangedListener client) {
        displayChanger.add(client);
    }

    /*
     * Remove a DisplayChangeListener from Win32GraphicsEnvironment
     */
    public void removeDisplayChangedListener(DisplayChangedListener client) {
        displayChanger.remove(client);
    }

/*
 * ----END DISPLAY CHANGE SUPPORT----
 */

  /* register only TrueType/OpenType fonts */
    protected void registerFontsWithPlatform(String pathName) {
        File f1 = new File(pathName);
	String[] ls = f1.list(new TTFilter());
	if (ls == null) {
	  return;
	}
	for (int i=0; i < ls.length; i++ ) {
	  File fontFile = new File(f1, ls[i]);	  
	  registerFontWithPlatform(fontFile.getAbsolutePath());
	}
    }

    protected native void registerFontWithPlatform(String fontName);

    protected native void deRegisterFontWithPlatform(String fontName);

    protected String getPlatformFontPath(boolean noType1Font) {
        return NativeFontWrapper.getFontPath(noType1Font);
    }
  
    protected GraphicsDevice makeScreenDevice(int screennum) {
        return new Win32GraphicsDevice(screennum);
    }

    /**
     * Gets a <code>PrinterJob</code> object suitable for the
     * the current platform.
     * @return    a <code>PrinterJob</code> object.
     * @see       java.awt.PrinterJob
     * @since     JDK1.2
     */
    public PrinterJob getPrinterJob() {
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
	    security.checkPrintJobAccess();
	}

	return new WPrinterJob();
    }

    // Implements SunGraphicsEnvironment.createFontProperties.
    protected FontProperties createFontProperties() {
	return new WFontProperties();
    }
}

