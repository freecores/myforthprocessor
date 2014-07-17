/*
 * @(#)InstallerDesc.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.xml.*;

/**
 *   Describes an Extension Installer Descriptor
 */
public class InstallerDesc implements XMLable {
    private String _mainClass;
    
    public InstallerDesc(String mainClass) {
        _mainClass = mainClass;
    }
             
    /** Get mainclass for the installer. If null, the mainclass
     * should be looked up in a manifest.
     */
    public String getMainClass() { return _mainClass; }
    
    /** Outputs as XML */
    public XMLNode asXML() {
        XMLAttributeBuilder ab = new XMLAttributeBuilder();
        ab.add("main-class", _mainClass);
        XMLNodeBuilder nb = new XMLNodeBuilder("installer-desc", ab.getAttributeList());
        return nb.getNode();
    }
}




