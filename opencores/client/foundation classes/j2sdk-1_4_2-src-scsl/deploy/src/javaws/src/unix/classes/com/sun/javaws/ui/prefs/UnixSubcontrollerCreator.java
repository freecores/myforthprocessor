/*
 * @(#)UnixSubcontrollerCreator.java	1.5 03/01/23
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
public class UnixSubcontrollerCreator extends SubcontrollerCreator {
    private Subcontroller[] _controllers;
    private String [] _names;
    private int _len;
    private boolean _lih;

    public UnixSubcontrollerCreator() {
	LocalInstallHandler handler = LocalInstallHandler.getInstance();

	_lih = (handler != null && handler.isLocalInstallSupported()); 
	_len = ((_lih) ? 5 : 4);
	_names = new String[_len];
        _controllers = new Subcontroller[_len];

	_names[0] = Resources.getString("controlpanel.general.tabTitle");
	_names[1] = Resources.getString("controlpanel.install.tabTitle");
        _names[_len-3] = Resources.getString("controlpanel.jre.tabTitle");
        _names[_len-2] = Resources.getString("controlpanel.advanced.tabTitle");
        _names[_len-1] = Resources.getString("controlpanel.certs.tabTitle");

        for (int i = 0; i < _len; i++) {
            _controllers[i] = null;
        }
    }

    public int getSubcontrollerCount() {
        return _len;
    }

    public String getSubcontrollerTitle(int index) {
        if (index < 0 || index >= _len) {
            Debug.fatal("Invalid Subcontroller index: " + index);
        }
	return _names[index];
    }

    public Subcontroller getSubcontroller(boolean create, int index) {
        if (index < 0 || index >= _len) {
            Debug.fatal("Invalid Subcontroller index: " + index);
        }
        if (_controllers[index] == null && create) {
            switch(index) {
            case 0:
                _controllers[0] = new UnixGeneralPanel();
                break;
            case 1:
                _controllers[1] =  ((_lih) ? 	
			((Subcontroller)new InstallPanel()) : 
			((Subcontroller)new JREPanel()));
                break;
            case 2:
                _controllers[2] =  ((_lih) ?     
			((Subcontroller)new JREPanel()) :
			((Subcontroller)new AdvancedPanel()));
                break;
            case 3:
                _controllers[3] =  ((_lih) ?     
			((Subcontroller)new AdvancedPanel()) :
			((Subcontroller)new CertificatePanel()));
                break;
            case 4:
               _controllers[4] =  new CertificatePanel();
                break;
            }
        }
        return _controllers[index];
    }
}

