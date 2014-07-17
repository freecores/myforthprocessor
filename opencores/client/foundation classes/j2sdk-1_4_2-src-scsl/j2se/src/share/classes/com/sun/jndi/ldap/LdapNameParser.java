/*
 * @(#)LdapNameParser.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;


import javax.naming.*;


class LdapNameParser implements NameParser {

    public LdapNameParser() {
    }

    public Name parse(String name) throws NamingException {
	return new LdapName(name);
    }
}
