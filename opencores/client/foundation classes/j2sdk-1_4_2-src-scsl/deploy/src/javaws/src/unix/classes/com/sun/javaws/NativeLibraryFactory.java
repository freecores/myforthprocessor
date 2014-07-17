/*
 * @(#)NativeLibraryFactory.java	1.4 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.javaws;

/**
 * Creates an instance of UnixNativeLibarary
 */
public class NativeLibraryFactory 
{
    public static NativeLibrary newInstance() {
	return new UnixNativeLibrary();
    }
}
