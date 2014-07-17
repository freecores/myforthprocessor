/*
 * @(#)RemoteReference.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.rmi.registry;


import java.rmi.*;

import javax.naming.*;


/**
 * The RemoteReference interface wraps a Reference in a Remote wrapper.
 *
 * @author Scott Seligman
 * @version 1.5 03/01/23
 */


public interface RemoteReference extends Remote {

        Reference getReference() throws NamingException, RemoteException;
}
