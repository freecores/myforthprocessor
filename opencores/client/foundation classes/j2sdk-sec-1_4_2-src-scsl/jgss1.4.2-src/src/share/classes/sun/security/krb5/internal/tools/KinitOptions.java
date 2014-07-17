/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KinitOptions.java	1.12 03/06/24
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

package sun.security.krb5.internal.tools;

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.ccache.*;
import java.io.File;
import java.io.IOException;
import java.util.StringTokenizer;
import java.util.Vector;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.FileInputStream;

/**
 * Maintains user-specific options or default settings when the user requests
 * a KDC ticket using Kinit.
 *
 * @author Yanni Zhang
 * @author Ram Marti 
 * @version 1.00 12 Apr 2000
 */
class KinitOptions {
    public boolean validate = false;

    // forwardable and proxiable flags have two states: 
    // -1 - flag set to be not forwardable or proxiable;
    // 1 - flag set to be forwardable or proxiable.
    public short forwardable = -1; 
    public short proxiable = -1;      
    public boolean renew = false;
    public KerberosTime lifetime;
    public KerberosTime renewable_lifetime;
    public String target_service;
    public String keytab_file;
    public String cachename;
    private PrincipalName principal;
    public String realm;
    StringBuffer password = null;
    public boolean keytab;
    private boolean DEBUG = Krb5.DEBUG;
    private boolean includeAddresses = true; // default.
    private boolean useKeytab = false; // default = false.
    private String ktabName; // keytab file name

    public KinitOptions() throws RuntimeException, RealmException {
	// no args were specified in the command line; 
	// use default values
        cachename = FileCredentialsCache.getDefaultCacheName();
        if (cachename == null) {
            throw new RuntimeException("default cache name error");
        }
        principal = getDefaultPrincipal();
    }

    public void setKDCRealm(String r) throws RealmException {
        realm = r;
    }

    public String getKDCRealm() {
        if (realm == null) {
            if (principal != null) {
                return principal.getRealmString();
            }
        }
        return null;
    }

    public KinitOptions(String[] args) 
	throws KrbException, RuntimeException, IOException {
        // currently we provide support for -f -p -c principal options
        String p = null; // workarea for principal
	
        for (int i = 0; i < args.length; i++) {
	    if (args[i].equals("-f")) {
		forwardable = 1;
	    } else if (args[i].equals("-p")) {
		proxiable = 1;
	    } else if (args[i].equals("-c")) {
		
		if (args[i + 1].startsWith("-")) {
		    throw new IllegalArgumentException("input format " +
						       " not correct: " +
						       " -c  option " +
						       "must be followed " + 
						       "by the cache name");
		}
		cachename = args[++i];
		if ((cachename.length() >= 5) &&
		    cachename.substring(0, 5).equalsIgnoreCase("FILE:")) {
		    cachename = cachename.substring(5);
		};
	    } else if (args[i].equals("-A")) {
		includeAddresses = false;
	    } else if (args[i].equals("-k")) {
		useKeytab = true;
	    } else if (args[i].equals("-t")) {
		if (ktabName != null) {
		    throw new IllegalArgumentException
			("-t option/keytab file name repeated");
		} else if (i + 1 < args.length) {
		    ktabName = args[++i];
		} else {
		    throw new IllegalArgumentException
			("-t option requires keytab file name");
		}
		
		useKeytab = true;
	    } else if (args[i].equalsIgnoreCase("-help")) {
		printHelp();
		System.exit(0);
	    } else if (p == null) { // Haven't yet processed a "principal"
		p = args[i];
		try {
		    principal = new PrincipalName(p);
		} catch (Exception e) {
		    throw new IllegalArgumentException("invalid " +
						       "Principal name: " + p +
						       e.getMessage());
		}
		if (principal.getRealm() == null) {
		    String realm =
			Config.getInstance().getDefault("default_realm",
							"libdefaults");
		    if (realm != null) {
			principal.setRealm(realm);
		    } else throw new IllegalArgumentException("invalid " +
							      "Realm name");
		}
	    } else if (this.password == null) {
		// Have already processed a Principal, this must be a password
		password = new StringBuffer().append(args[i]);
	    } else { 
		throw new IllegalArgumentException("too many parameters");
	    }
        }
	// we should get cache name before getting the default principal name
        if (cachename == null) {
            cachename = FileCredentialsCache.getDefaultCacheName();
            if (cachename == null) {
                throw new RuntimeException("default cache name error");
            }
        }
        if (principal == null) {
	    principal = getDefaultPrincipal();
        }
    }

    PrincipalName getDefaultPrincipal() {
        String cname;
        String realm = null;
        try {
            realm = Config.getInstance().getDefaultRealm();
        } catch (KrbException e) {
	    System.out.println ("Can not get default realm " +
				e.getMessage());
	    e.printStackTrace();
	    return null;
	}

        // get default principal name from the cachename if it is 
	// available.
	
	try {
	    CCacheInputStream cis =
		new CCacheInputStream(new FileInputStream(cachename));
	    int version;
	    if ((version = cis.readVersion()) == 0x504) {
		cis.readTag();
	    }
	    PrincipalName p = cis.readPrincipal(version);
	    cis.close();
	    String temp = p.getRealmString();
	    if (temp == null) {
		p.setRealm(realm);
	    }
	    if (DEBUG) {
		System.out.println(">>>KinitOptions principal name from "+
				   "the cache is :" + p);
	    }
	    return p;
	} catch (IOException e) {
	    // ignore any exceptions; we will use the user name as the
	    // principal name
	    if (DEBUG) {
		e.printStackTrace();
	    }
	} catch (RealmException e) {
	    if (DEBUG) {
		e.printStackTrace();
	    }
	}

        String username = System.getProperty("user.name");
        if (DEBUG) {
            System.out.println(">>>KinitOptions default username is :" 
			       + username);
        }
        if (realm != null) {
            try {
                PrincipalName p = new PrincipalName(username);
                if (p.getRealm() == null) 
                    p.setRealm(realm);
                return p;
            } catch (RealmException e) {
		// ignore exception , return null
		if (DEBUG) {
		    System.out.println ("Exception in getting principal " +
				        "name " + e.getMessage());
		    e.printStackTrace();
		}
	    }
	}
	return null;
    }
    

    void printHelp() {
        System.out.println("Usage: kinit " +
                           "[-A] [-f] [-p] [-c cachename] " +
			   "[[-k [-t keytab_file_name]] [principal] " +
			   "[password]");
        System.out.println("\tavailable options to " +
			   "Kerberos 5 ticket request:");
        System.out.println("\t    -A   do not include addresses");
        System.out.println("\t    -f   forwardable");
        System.out.println("\t    -p   proxiable");
        System.out.println("\t    -c   cache name " +
			   "(i.e., FILE:\\d:\\myProfiles\\mykrb5cache)");
        System.out.println("\t    -k   use keytab");
        System.out.println("\t    -t   keytab file name");
        System.out.println("\t    principal   the principal name "+
			   "(i.e., qweadf@ATHENA.MIT.EDU qweadf)");
        System.out.println("\t    password   " +
			   "the principal's Kerberos password");
    }

    public boolean getAddressOption() {
        return includeAddresses;
    }

    public boolean useKeytabFile() {
        return useKeytab;
    }

    public String keytabFileName() {
        return ktabName;
    }

    public PrincipalName getPrincipal() {
        return principal;
    }
}
