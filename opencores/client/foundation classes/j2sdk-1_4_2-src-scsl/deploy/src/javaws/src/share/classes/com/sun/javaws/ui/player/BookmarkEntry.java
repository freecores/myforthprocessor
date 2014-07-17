/*
 * @(#)BookmarkEntry.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.ui.player;

public class BookmarkEntry {

    private String entryName;
    private String entryUrl;

    public BookmarkEntry(String name, String url) {
	entryName = name;
	entryUrl = url;
    }

    public String getName() {
	return entryName;
    }

    public String getUrl() {
	return entryUrl;
    }

}
