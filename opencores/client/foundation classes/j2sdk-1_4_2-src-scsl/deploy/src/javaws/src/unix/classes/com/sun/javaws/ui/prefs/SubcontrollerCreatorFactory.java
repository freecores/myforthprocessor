/*
 * @(#)SubcontrollerCreatorFactory.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

/**
 * Returns the Unix specific install handler.
 *
 * @version 1.4 02/14/01
 */
public class SubcontrollerCreatorFactory {
    public static SubcontrollerCreator newInstance() {
        return new UnixSubcontrollerCreator();
    }
}
