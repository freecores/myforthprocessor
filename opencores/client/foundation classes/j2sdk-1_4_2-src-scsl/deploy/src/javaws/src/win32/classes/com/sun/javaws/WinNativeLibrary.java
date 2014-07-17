/*
 * @(#)WinNativeLibrary.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.io.File;
import com.sun.javaws.JnlpxArgs;

/**
 * Loads javawspl.dll
 */
public class WinNativeLibrary extends NativeLibrary
{
    private static boolean isLoaded = false;

    public synchronized void load() {
	if (!isLoaded) {
	    String home = JnlpxArgs.getHome();
	    System.load(home + File.separator + "javawspl.dll");
	    isLoaded = true;
	}
    }
}
