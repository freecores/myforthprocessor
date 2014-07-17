/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Credentials.java	1.16 03/06/24
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

package sun.security.krb5;

import sun.security.krb5.internal.*;
import sun.security.krb5.internal.ccache.CredentialsCache;
import java.util.StringTokenizer;
import sun.security.krb5.internal.ktab.*;
import java.io.File;
import java.io.IOException;
import java.util.Date;
import java.util.Vector;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.InetAddress;

/**
 * This class encapsulates the concept of a Kerberos service
 * credential. That includes a Kerberos ticket and an associated
 * session key.
 */
public class Credentials {

    Ticket ticket;
    PrincipalName client;
    PrincipalName server;
    EncryptionKey key;
    TicketFlags flags;
    KerberosTime authTime;
    KerberosTime startTime;
    KerberosTime endTime;
    KerberosTime renewTill;
    HostAddresses cAddr;
    EncryptionKey serviceKey;
    private static boolean DEBUG = Krb5.DEBUG;
    private static CredentialsCache cache;
    private static boolean alreadyLoaded = false;
    private static boolean alreadyTried = false;
    private static native Credentials acquireDefaultNativeCreds();

    public Credentials(Ticket new_ticket,
		       PrincipalName new_client,
		       PrincipalName new_server,
		       EncryptionKey new_key,
		       TicketFlags new_flags,
		       KerberosTime authTime,
		       KerberosTime new_startTime,
		       KerberosTime new_endTime,
		       KerberosTime renewTill,
		       HostAddresses cAddr) {
	ticket = new_ticket;
	client = new_client;
	server = new_server;
	key = new_key;
	flags = new_flags;
	this.authTime = authTime;
        startTime = new_startTime;
        endTime = new_endTime;
	this.renewTill = renewTill;
	this.cAddr = cAddr;
    }
	
    public Credentials(byte[] encoding,
		       String client,
		       String server,
		       byte[] keyBytes,
		       int keyType,
		       boolean[] flags,
		       Date authTime,
		       Date startTime,
		       Date endTime,
		       Date renewTill,
		       InetAddress[] cAddrs) throws KrbException, IOException {
	this(new Ticket(encoding),
	     new PrincipalName(client),
	     new PrincipalName(server),
	     new EncryptionKey(keyType, keyBytes),
	     (flags == null? null: new TicketFlags(flags)),
	     (authTime == null? null: new KerberosTime(authTime)),
	     (startTime == null? null: new KerberosTime(startTime)),
	     (endTime == null? null: new KerberosTime(endTime)),
	     (renewTill == null? null: new KerberosTime(renewTill)),
	     null); // caddrs are in the encoding at this point
    }


    /**
     * Acquires a service ticket for the specified service
     * principal. If the service ticket is not already available, it
     * obtains a new one from the KDC.
     */
    /*
      public Credentials(Credentials tgt, PrincipalName service)
      throws KrbException {
      }
    */

    public final PrincipalName getClient() {
	return client;
    }

    public final PrincipalName getServer() {
	return server;
    }

    public final EncryptionKey getSessionKey() {
	return key;
    }

    public final Date getAuthTime() {
	if (authTime != null) {
	    return authTime.toDate();
	} else {
	    return null;
	}
    }

    public final Date getStartTime() {
        if (startTime != null)
	    {
		return startTime.toDate();
	    }
        return null;
    }

    public final Date getEndTime() {
        if (endTime != null)
	    {
		return endTime.toDate();
	    }
        return null;
    }

    public final Date getRenewTill() {
        if (renewTill != null)
	    {
		return renewTill.toDate();
	    }
        return null;
    }

    public final boolean[] getFlags() {
	if (flags == null) // Can be in a KRB-CRED
	return null;
	return flags.toBooleanArray();
    }

    public final InetAddress[] getClientAddresses() {

	if (cAddr == null)
	return null;

	return cAddr.getInetAddresses();
    }

    public final byte[] getEncoded() {
	byte[] retVal= null;
	try {
	    retVal = ticket.asn1Encode();
	} catch (Asn1Exception e) {
	    if (DEBUG)
	    System.out.println(e);
	} catch (IOException ioe) {
	    if (DEBUG)
	    System.out.println(ioe);
	}
	return retVal;
    }

    public boolean isForwardable() {
	return flags.get(Krb5.TKT_OPTS_FORWARDABLE);
    }

    public boolean isRenewable() {
	return flags.get(Krb5.TKT_OPTS_RENEWABLE);
    }
	
    public Ticket getTicket() {
        return ticket;
    }

    public TicketFlags getTicketFlags() {
        return flags;
    }

    /**
     * Checks if the service ticket returned by the KDC has the OK-AS-DELEGATE
     * flag set
     * @return true if OK-AS_DELEGATE flag is set, otherwise, return false.
     */
    public boolean checkDelegate() {
        return (flags.get(Krb5.TKT_OPTS_DELEGATE));
    }
    
    public Credentials renew() throws KrbException, IOException {
	KDCOptions options = new KDCOptions();
	options.set(KDCOptions.RENEW, true);
	/*
	 * Added here to pass KrbKdcRep.check:73
	 */
	options.set(KDCOptions.RENEWABLE, true);

	KrbTgsReq tgsReq = new KrbTgsReq(options,
					 this,
					 server,
					 null, // from
					 null, // till
					 null, // rtime
					 null, // eTypes
					 cAddr,
					 null,
					 null,
					 null);
	String kdc = null;
	KrbTgsRep tgsRep = null;
	try {
	    kdc = tgsReq.send();
	    tgsRep = tgsReq.getReply(this);
	} catch (KrbException ke) {
		if (ke.returnCode() == Krb5.KRB_ERR_RESPONSE_TOO_BIG) {
		    tgsReq.send(server.getRealmString(), kdc, true); // useTCP is set
	            tgsRep = tgsReq.getReply(this);
		} else {
		    throw ke;
		}
	}
	return tgsRep.getCreds();
    }
    
    /**
     * Returns a TGT for the given client principal from a ticket cache.
     *
     * @param princ the client principal. A value of null means that the
     * default principal name in the credentials cache will be used.
     * @param ticketCache the path to the tickets file. A value 
     * of null will be accepted to indicate that the default 
     * path should be searched
     * @returns the TGT credentials or null if none were found. If the tgt
     * expired, it is the responsibility of the caller to determine this.
     */
    public static Credentials acquireTGTFromCache(PrincipalName princ,
						  String ticketCache) 
	throws KrbException, IOException {
	
        if (ticketCache == null) {
            // The default ticket cache on Windows 2000 is not a file.
            String os = (String)java.security.AccessController.doPrivileged(
									    new sun.security.action.GetPropertyAction("os.name"));
            if (os.equalsIgnoreCase("windows 2000")) {
                Credentials creds = acquireDefaultCreds();
                if (creds == null) {
		    if (DEBUG) {
			System.out.println(">>> Found no TGT's in LSA");
		    }
                    return null;
                }
                if (princ != null) {
                    if (creds.getServer().equals(princ)) {
                        if (DEBUG) {
                            System.out.println(">>> Obtained TGT from LSA: "
                                               + creds);
                        }
                        return creds;
                    } else {
			if (DEBUG) {
			    System.out.println(">>> LSA contains TGT for "
					       + creds.getServer() 
					       + " not "
					       + princ);
			}
                        return null;
                    }
                } else {
                    if (DEBUG) {
                        System.out.println(">>> Obtained TGT from LSA: "
                                           + creds);
                    }
                    return creds;
                }
            }
        }

	/*
	 * Returns the appropriate cache. If ticketCache is null, it is the 
	 * default cache otherwise it is the cache filename contained in it.
	 */
	CredentialsCache ccache =
	    CredentialsCache.getInstance(princ, ticketCache); 

	if (ccache == null)
	    return null;
	
	sun.security.krb5.internal.ccache.Credentials tgtCred  =
	    ccache.getDefaultCreds();

	return tgtCred.setKrbCreds();
    }

    /**
     * Returns a TGT for the given client principal via an AS-Exchange.
     * This method causes pre-authentication data to be sent in the 
     * AS-REQ.
     *
     * @param princ the client principal. This value cannot be null.
     * @param secretKey the secret key of the client principal.This value
     * cannot be null.
     * @returns the TGT credentials
     */
    public static Credentials acquireTGT(PrincipalName princ,
					 EncryptionKey secretKey) 
	throws KrbException, IOException {

	if (princ == null)
	    throw new IllegalArgumentException(
					       "Cannot have null principal to do AS-Exchange");

	if (secretKey == null)
	    throw new IllegalArgumentException(
					       "Cannot have null secretKey to do AS-Exchange");

	KrbAsReq asReq = new KrbAsReq(princ, secretKey);
	String kdc = null;
	KrbAsRep asRep  = null;
	try {
	    kdc = asReq.send(); 
	    asRep =  asReq.getReply(secretKey); 
	} catch (KrbException ke) {
		if (ke.returnCode() == Krb5.KRB_ERR_RESPONSE_TOO_BIG) {
		    asReq.send(princ.getRealmString(),kdc,true);
		    asRep =  asReq.getReply(secretKey); 
		} else {
		    throw ke;
		}
	}

	return asRep.getCreds(); 
    }

    /**
     * Acquires default credentials.
     * <br>The possible locations for default credentials cache is searched in
     * the following order:
     * <ol>
     * <li> The directory and cache file name specified by "KRB5CCNAME" system.
     * property.
     * <li> The directory and cache file name specified by "KRB5CCNAME" 
     * environment variable.
     * <li> A cache file named krb5cc_{user.name} at {user.home} directory.
     * </ol>
     * @return a <code>KrbCreds</code> object if the credential is found,
     * otherwise return null.
     */
   
    // this method is intentionally changed to not check if the caller's 
    // principal name matches cache file's principal name. 
    // It assumes that the GSS call has
    // the privilege to access the default cache file.

    public static synchronized Credentials acquireDefaultCreds() {
	Credentials result = null;

	if (cache == null) {
            cache = CredentialsCache.getInstance();
	}
	if (cache != null) {
            if (DEBUG) {
		System.out.println(">>> KrbCreds found the default ticket " + 
				   "granting ticket in credential cache.");
            }
            sun.security.krb5.internal.ccache.Credentials temp =
		cache.getDefaultCreds();
            result = temp.setKrbCreds();
	} else { 
	    // Doesn't seem to be a default cache on this system
            if (!alreadyTried) { 
		// See if there's any native code to load
                try {
                    ensureLoaded();
                } catch (Exception e) {
		    if (DEBUG) {
			System.out.println("Can not load credentials cache");
			e.printStackTrace();
		    }
                    alreadyTried = true;
                }
            }
            if (alreadyLoaded) { 
		// There is some native code
                result = acquireDefaultNativeCreds();
            }
	}
	return result;
    }


    /**
     * Gets service credential from key table. The credential is used to
     * decrypt the received client message
     * and authenticate the client by verifying the client's credential.
     *
     * @param serviceName the name of service, using format component@realm
     * @param keyTabFile the file of key table.
     * @return a <code>KrbCreds</code> object.
     */
    public static Credentials getServiceCreds(String serviceName, 
					      File keyTabFile) {
        EncryptionKey k = null;
        PrincipalName service = null;
        Credentials result = null;
        try {
            service = new PrincipalName(serviceName);
            if (service.getRealm() == null) {
                String realm = Config.getInstance().getDefaultRealm();
                if (realm == null) {
                    return null;
                } else {
                    service.setRealm(realm);
                }
            }
	} catch (RealmException e) {
	    if (DEBUG) {
		e.printStackTrace();
	    }
	    return null;
	} catch (KrbException e) {
	    if (DEBUG) {
		e.printStackTrace();
	    }
            return null;
        }
        KeyTab kt;
        if (keyTabFile == null) {
            kt = KeyTab.getInstance();
        } else {
	    kt = KeyTab.getInstance(keyTabFile);
	}
	if ((kt != null) && (kt.findServiceEntry(service))) {
	    k = kt.readServiceKey(service);
	    result = new Credentials(null, service, null, null, null,
				     null, null, null, null, null);
	    result.serviceKey = k;
	}
	return result;
    }


    
    /**
     * Acquires credentials for a specified service using initial credential.
     * When the service has a different realm
     * from the initial credential, we do cross-realm authentication
     * - first, we use the current credential to get
     * a cross-realm credential from the local KDC, then use that
     * cross-realm credential to request service credential
     * from the foreigh KDC.
     *
     * @param service the name of service principal using format 
     * components@realm
     * @param ccreds client's initial credential.
     * @exception IOException if an error occurs in reading the credentials 
     * cache
     * @exception KrbException if an error occurs specific to Kerberos
     * @return a <code>Credentials</code> object.
     */

    public static Credentials acquireServiceCreds(String service, 
						  Credentials ccreds)
	throws KrbException, IOException {
	return CredentialsUtil.acquireServiceCreds(service, ccreds);
    }

    
    /*
     * This method does the real job to request the service credential.
     */

    private static Credentials serviceCreds(ServiceName service,
					    Credentials ccreds)
	throws KrbException, IOException {
	KrbTgsReq tgs_req = new KrbTgsReq(
					  new KDCOptions(),
					  ccreds,
					  service,
					  null, // KerberosTime from
					  null, // KerberosTime till
					  null, // KerberosTime rtime
					  null, // int[] eTypes
					  null, // HostAddresses addresses
					  null, // AuthorizationData authorizationData
					  null, // Ticket[] additionalTickets
					  null);  // EncryptionKey subSessionKey
	String kdc = null;
	KrbTgsRep tgs_rep = null;
	try {
	    kdc = tgs_req.send();
	    tgs_rep = tgs_req.getReply(ccreds);
	} catch (KrbException ke) {
		if (ke.returnCode() == Krb5.KRB_ERR_RESPONSE_TOO_BIG) {
			// try with TCP to connect to the same KDC
		    tgs_req.send(service.getRealmString(),kdc, true); 
	            tgs_rep = tgs_req.getReply(ccreds);
		} else {
		    throw ke;
		}
	}
	return tgs_rep.getCreds();
    }

    public CredentialsCache getCache() {
	return cache;
    }

    public EncryptionKey getServiceKey() {
	return serviceKey;
    }
    
    /*
     * Prints out debug info.
     */
    public static void printDebug(Credentials c) {
        System.out.println(">>> DEBUG: ----Credentials----");
        System.out.println("\tclient: " + c.client.toString());
        System.out.println("\tserver: " + c.server.toString());
        System.out.println("\tticket: realm: " + c.ticket.realm.toString());
        System.out.println("\t        sname: " + c.ticket.sname.toString());
        System.out.println("\tstartTime: " + c.startTime.getTime());
        System.out.println("\tendTime: " + c.endTime.getTime());
        System.out.println("        ----Credentials end----");
    }


    private static void ensureLoaded() {
        java.security.AccessController.doPrivileged(
						    new java.security.PrivilegedAction() {
							    public Object run() {
								System.loadLibrary("w2k_lsa_auth");
								return null;
							    }
							});
	alreadyLoaded = true;
    }
    
    public String toString() {
	StringBuffer buffer = new StringBuffer("Credentials:");
	buffer.append("\nclient=").append(client);
	buffer.append("\nserver=").append(server);
	buffer.append("\nauthTime=").append(authTime);
	buffer.append("\nstartTime=").append(startTime);
	buffer.append("\nendTime=").append(endTime);
	buffer.append("\nrenewTill=").append(renewTill);
	buffer.append("\nflags: ").append(flags);
	buffer.append("\nEType (int): ").append(key.getEType());
	return buffer.toString();
    }

    /*
      public static void main(String[] args) throws Exception {
      String user = args[0];
      String realm = args[1];
      String server = args[2];
      Credentials creds = acquireCreds(user, realm);
      System.out.println(creds);
      creds = creds.renew();
      System.out.println(creds);
      creds = acquireServiceCreds(server, creds);
      System.out.println(creds);
      //	creds=creds.renew();
      //	System.out.println(creds);
      }
    */
}

