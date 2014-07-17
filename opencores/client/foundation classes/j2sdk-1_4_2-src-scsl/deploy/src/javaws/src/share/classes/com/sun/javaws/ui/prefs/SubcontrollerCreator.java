/*
 * @(#)SubcontrollerCreator.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import com.sun.javaws.ui.general.*;

/**
 * @version 1.8 11/29/01
 */
public abstract class SubcontrollerCreator {
    /** The shared instance of SubcontrollerCreator. */
    private static SubcontrollerCreator _instance;

    /**
     * Returns the SubcontrollerCreator appropriate for the current platform.
     * This should never return null.
     */
    public static synchronized SubcontrollerCreator getInstance() {
        if (_instance == null) {
            _instance = SubcontrollerCreatorFactory.newInstance();
        }
        return _instance;
    }

    public abstract int getSubcontrollerCount();
    public abstract String getSubcontrollerTitle(int index);
    public abstract Subcontroller getSubcontroller(boolean create, int index);
}
