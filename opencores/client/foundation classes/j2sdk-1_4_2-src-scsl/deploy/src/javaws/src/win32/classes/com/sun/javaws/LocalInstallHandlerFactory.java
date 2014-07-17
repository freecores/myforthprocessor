/*
 * @(#)LocalInstallHandlerFactory.java	1.7 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * Returns the Windows specific install handler.
 *
 * @version 1.7 01/23/03
 */
public class LocalInstallHandlerFactory {
    public static LocalInstallHandler newInstance() {
        return new WinInstallHandler();
    }
}
