/*
 * @(#)XMLable.java	1.6 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.xml;

/** Interface that defines if a class can be converted to
 *  an XML document
 */
public interface XMLable {
    public XMLNode asXML();
}


