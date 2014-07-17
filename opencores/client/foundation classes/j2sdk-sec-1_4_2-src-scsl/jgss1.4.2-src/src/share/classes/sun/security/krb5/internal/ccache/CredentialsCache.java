/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CredentialsCache.java	1.8 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */
package sun.security.krb5.internal.ccache;

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import java.util.StringTokenizer;
import java.util.Vector;
import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.BufferedReader;
import java.io.InputStreamReader;

/**
 * CredentialsCache stores credentials(tickets, session keys, etc) in a semi-permanent store
 * for later use by different program.
 *
 * @author Yanni Zhang
 * @version 1.00
 */
public abstract class CredentialsCache {
    static CredentialsCache singleton = null;
    static String cacheName;
    private static boolean DEBUG = Krb5.DEBUG;

    public static CredentialsCache getInstance(PrincipalName principal) {
        return FileCredentialsCache.acquireInstance(principal, null);
    }

    public static CredentialsCache getInstance(String cache) {
        if ((cache.length() >= 5) && cache.substring(0, 5).equalsIgnoreCase("FILE:")) {
            return FileCredentialsCache.acquireInstance(null, cache.substring(5));
        }
        //else, memory credential cache 
        //default is file credential cache.
        return FileCredentialsCache.acquireInstance(null, cache);
    }

    public static CredentialsCache getInstance(PrincipalName principal,
					       String cache) {  

	// TBD: Modify this to use URL framework of the JDK
	if (cache != null &&
	    (cache.length() >= 5) && 
	    cache.regionMatches(true, 0, "FILE:", 0, 5)) { 
	    return FileCredentialsCache.acquireInstance(principal, 
							cache.substring(5));
	}

	/* TBD:
	 * When cache is null, read the default cache. This might involve
	 * native code on platforms like W2K.
	 */

	//else ..we haven't provided support for memory credential cache
	// yet. (supported in native code)
	return FileCredentialsCache.acquireInstance(principal, cache); //default is file credential cache.

    }

    /**
     * Gets the default credentials cache.
     */
    public static CredentialsCache getInstance() {
	//Default credentials cache is file-based.
	return FileCredentialsCache.acquireInstance();
    }
    
    public static CredentialsCache create(PrincipalName principal, String name) {
	if (name == null) {
            throw new RuntimeException("cache name error");
	}
	if ((name.length() >= 5) 
	    && name.regionMatches(true, 0, "FILE:", 0, 5)) {
	    name = name.substring(5);
	    return (FileCredentialsCache.New(principal, name));
	}
	// else return file credentials cache 
	// default is file credentials cache.
	return (FileCredentialsCache.New(principal, name));
    }

    public static CredentialsCache create(PrincipalName principal) {
	//create a default credentials cache for a specified principal
	return (FileCredentialsCache.New(principal));
    }

    public static String cacheName() {
	return cacheName;
    }

    public abstract PrincipalName getPrimaryPrincipal();
    public abstract void update(Credentials c);
    public abstract void save() throws IOException, KrbException;
    public abstract Credentials[] getCredsList();
    public abstract Credentials getDefaultCreds();
    public abstract Credentials getCreds(PrincipalName sname, Realm srealm) ; 
    public abstract Credentials getCreds(LoginOptions options, PrincipalName sname, Realm srealm) ;
}
