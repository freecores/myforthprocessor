/*
 * @(#)rmiURLContext.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.url.rmi;


import java.util.Hashtable;
import java.rmi.registry.LocateRegistry;

import javax.naming.*;
import javax.naming.spi.ResolveResult;
import com.sun.jndi.toolkit.url.GenericURLContext;
import com.sun.jndi.rmi.registry.RegistryContext;


/**
 * An RMI URL context resolves names that are URLs of the form
 * <pre>
 *   rmi://[host][:port][/[object]]
 * or
 *   rmi:[/][object]
 * </pre>
 * If an object is specified, the URL resolves to the named object.
 * Otherwise, the URL resolves to the specified RMI registry.
 *
 * @author Scott Seligman
 * @version 1.5 03/01/23
 */


public class rmiURLContext extends GenericURLContext {

    public rmiURLContext(Hashtable env) {
	super(env);
    }

    /**
     * Resolves the registry portion of "url" to the corresponding
     * RMI registry, and returns the atomic object name as the
     * remaining name.
     */
    protected ResolveResult getRootURLContext(String url, Hashtable env)
	    throws NamingException
    {
	if (!url.startsWith("rmi:")) {
	    throw (new IllegalArgumentException(
		    "rmiURLContext: name is not an RMI URL: " + url));
	}

	// Parse the URL.

	String host = null;
	int port = -1;
	String objName = null;

	int i = 4;		// index into url, following the "rmi:"

	if (url.startsWith("//", i)) {		// parse "//host:port"
	    i += 2;				// skip past "//"
	    int slash = url.indexOf('/', i);
	    if (slash < 0) {
		slash = url.length();
	    }
	    int colon = url.indexOf(':', i);
	    if (colon < 0 || colon > slash) {	// no ":port"
		host = url.substring(i, slash);
	    } else {
		host = url.substring(i, colon);
		port = Integer.parseInt(url.substring(colon + 1, slash));
	    }
	    i = slash;
	}
	if ("".equals(host)) {
	    host = null;
	}
	if (url.startsWith("/", i)) {		// skip "/" before object name
	    i++;
	}
	if (i < url.length()) {
	    objName = url.substring(i);
	}

	// Represent object name as empty or single-component composite name.
	CompositeName remaining = new CompositeName();
	if (objName != null) {
	    remaining.add(objName);
	}

	// Debug
	//System.out.println("host=" + host + " port=" + port +
	//		   " objName=" + remaining.toString() + "\n");

	// Create a registry context.
	Context regCtx = new RegistryContext(host, port, env);

	return (new ResolveResult(regCtx, remaining));
    }
}
