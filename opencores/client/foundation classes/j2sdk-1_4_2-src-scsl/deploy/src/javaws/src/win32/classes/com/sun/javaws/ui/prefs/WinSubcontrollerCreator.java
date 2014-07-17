/*
 * @(#)WinSubcontrollerCreator.java	1.5 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.ui.prefs;

import com.sun.javaws.*;
import com.sun.javaws.ui.general.*;
import com.sun.javaws.debug.*;
import com.sun.javaws.security.CertificatePanel;

/**
 * @version 1.8 02/14/01
 */
public class WinSubcontrollerCreator extends SubcontrollerCreator {
    private Subcontroller[] _controllers;

    public WinSubcontrollerCreator() {
        _controllers = new Subcontroller[5];
        for (int counter = _controllers.length - 1; counter >= 0; counter--) {
            _controllers[counter] = null;
        }
    }

    public int getSubcontrollerCount() {
        return _controllers.length;
    }

    public String getSubcontrollerTitle(int index) {
        if (index < 0 || index >= _controllers.length) {
            Debug.fatal("Invalid Subcontroller index: " + index);
        }
        switch(index) {
        case 0:
            return Resources.getString("controlpanel.general.tabTitle");
        case 1:
            return Resources.getString("controlpanel.install.tabTitle");
        case 2:
            return Resources.getString("controlpanel.jre.tabTitle");
        case 3:
            return Resources.getString("controlpanel.advanced.tabTitle");
        case 4:
            return Resources.getString("controlpanel.certs.tabTitle");
        }
        return null;
    }

    public Subcontroller getSubcontroller(boolean create, int index) {
        if (index < 0 || index >= _controllers.length) {
            Debug.fatal("Invalid Subcontroller index: " + index);
        }
        if (_controllers[index] == null && create) {
            switch(index) {
            case 0:
                _controllers[0] = new GeneralPanel();
                break;
            case 1:
                _controllers[1] = new InstallPanel();
                break;
            case 2:
                _controllers[2] = new JREPanel();
                break;
            case 3:
                _controllers[3] = new AdvancedPanel();
                break;
            case 4:
                _controllers[4] = new CertificatePanel();
                break;
            default:
                throw new RuntimeException("Invalid index!");
            }
        }
        return _controllers[index];
    }
}
