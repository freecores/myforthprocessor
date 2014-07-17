/*
 * @(#)SearchResultWithControls.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import javax.naming.*;
import javax.naming.directory.*;
import javax.naming.ldap.*;

class SearchResultWithControls extends SearchResult implements HasControls {
    private Control[] controls;

    public SearchResultWithControls(String name, Object obj, Attributes attrs,
	boolean isRelative, Control[] controls) {

	super(name, obj, attrs, isRelative);
	this.controls = controls;
    }

    public Control[] getControls() throws NamingException {
	return controls;
    }

    private static final long serialVersionUID = 8476983938747908202L;
}
