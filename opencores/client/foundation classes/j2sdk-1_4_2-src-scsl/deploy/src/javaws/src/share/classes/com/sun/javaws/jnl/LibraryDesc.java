/*
 * @(#)LibraryDesc.java	1.9 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.jnl;
import com.sun.javaws.xml.*;

/**
 *   Tag for a extension library descriptor
 */
public class LibraryDesc implements XMLable {
        
    public LibraryDesc() { /* nothing to do */ }
         
    /** Outputs as XML */
    public XMLNode asXML() {
        return new XMLNode("library-desc", null);
    }
}




