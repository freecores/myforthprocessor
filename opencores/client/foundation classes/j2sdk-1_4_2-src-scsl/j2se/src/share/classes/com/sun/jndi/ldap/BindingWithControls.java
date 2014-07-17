/*
 * @(#)BindingWithControls.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import javax.naming.*;
import javax.naming.ldap.*;

class BindingWithControls extends Binding implements HasControls {
    private Control[] controls;

    public BindingWithControls(String name, Object obj, Control[] controls) {
	super(name, obj);
	this.controls = controls;
    }

    public Control[] getControls() throws NamingException {
	return controls;
    }

    private static final long serialVersionUID = 9117274533692320040L;
}
