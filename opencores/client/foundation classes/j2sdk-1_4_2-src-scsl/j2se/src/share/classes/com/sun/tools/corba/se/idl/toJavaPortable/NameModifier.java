/*
 * @(#)NameModifier.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.corba.se.idl.toJavaPortable ;

public interface NameModifier {
    /** Create a modified name from the base name.
    */
    String makeName( String base ) ;
} ;

