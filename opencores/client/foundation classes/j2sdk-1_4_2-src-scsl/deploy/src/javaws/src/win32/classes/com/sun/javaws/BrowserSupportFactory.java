/*
 * @(#)BrowserSupportFactory.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;
import java.net.URL;
import com.sun.javaws.debug.Globals;

/**
 * Create an instance of WinBrowserSupport
 *
 * @version 1.3, 08/29/00
 */
public class BrowserSupportFactory {
    public static BrowserSupport newInstance() {
	return new WinBrowserSupport();
    }
}


