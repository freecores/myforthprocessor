/*
 * @(#)iiopnameURLContextFactory.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.iiopname;

import com.sun.jndi.url.iiop.iiopURLContextFactory;

/**
 * An iiopname URL context factory.
 * It just uses the iiop URL context factory but is needed here
 * so that NamingManager.getURLContext() will find it.
 * 
 * @author Rosanna Lee
 * @version 1.5 03/01/23
 */
final public class iiopnameURLContextFactory extends iiopURLContextFactory {
}
