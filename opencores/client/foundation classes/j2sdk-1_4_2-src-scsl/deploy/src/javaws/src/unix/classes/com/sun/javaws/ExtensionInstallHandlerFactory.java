/*
 * @(#)ExtensionInstallHandlerFactory.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;

/**
 * ExtensionInstallHandlerFactory for Solaris. Returns null as Solaris does not
 * yet support an install.
 *
 * @version 1.0 04/07/02
 */
public class ExtensionInstallHandlerFactory {
    /**
     * Returns null, Solaris does not support an installer yet.
     */
    public static ExtensionInstallHandler newInstance() {
        return null;
    }
}
