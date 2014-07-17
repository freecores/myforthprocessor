/*
 * @(#)corbanameURLContextFactory.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.corbaname;

import com.sun.jndi.url.iiop.iiopURLContextFactory;

/**
 * A corbaname URL context factory.
 * It just uses the iiop URL context factory but is needed here
 * so that NamingManager.getURLContext() will find it.
 * 
 * @author Rosanna Lee
 * @version 1.4, 01/23/03
 */
final public class corbanameURLContextFactory extends iiopURLContextFactory {
}
