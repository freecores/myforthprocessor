/*
 * @(#)DefaultResponseControlFactory.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.io.IOException;
import javax.naming.*;
import javax.naming.directory.*;
import javax.naming.ldap.*;

/**
 * This class represents a factory for creating the response controls needed
 * by the LDAP provider. These include:
 *<p>
   the entry change response control as defined in
 * <a href="http://www.ietf.org/internet-drafts/draft-ietf-ldapext-psearch-02.txt">draft-ietf-ldapext-psearch-02.txt</a>.
 *
 * @see PersistentSearchControl
 * @see EntryChangeResponseControl
 * @author Vincent Ryan
 */
public class DefaultResponseControlFactory extends ControlFactory {

    /**
     * Constructs a new instance of the response control factory.
     */
    public DefaultResponseControlFactory() {
    }

    /**
     * Creates an instance of a Entry Change response control class from a more
     * generic control class (BasicControl).
     *
     * @param ctl A non-null control.
     * @return    The LDAP control created or null if it cannot be created.
     *            Null indicates that another factory should be attempted.
     * @exception NamingException if this control factory encountered an
     *            error condition while attempting to create the LDAP control,
     *            and no other control factories are to be tried.
     */
    public Control getControlInstance(Control ctl) 
	throws NamingException {

	String id = ctl.getID();
//System.out.println(id);

	try {
	    if (id.equals(EntryChangeResponseControl.OID)) {
		return new EntryChangeResponseControl(id, ctl.isCritical(),
		    ctl.getEncodedValue());

	    } 
	} catch (IOException e) {
	    NamingException ne = new NamingException();
	    ne.setRootCause(e);
	    throw ne;
	}
	return null;
    }
}
