/*
 * @(#)ExtensionInstallHandlerFactory.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * Returns the Windows specific install handler.
 *
 * @version 1.5 02/14/01
 */
public class ExtensionInstallHandlerFactory {
    public static ExtensionInstallHandler newInstance() {
        return new WinExtensionInstallHandler();
    }
}
