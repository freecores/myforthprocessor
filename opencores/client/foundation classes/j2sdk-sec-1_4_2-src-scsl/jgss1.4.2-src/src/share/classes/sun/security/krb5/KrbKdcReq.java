/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbKdcReq.java	1.12 03/06/24
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

import sun.security.krb5.internal.Krb5;
import sun.security.krb5.internal.UDPClient;
import sun.security.krb5.internal.TCPClient;
import java.io.IOException;
import java.io.InterruptedIOException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.StringTokenizer;
import java.security.AccessController;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;

abstract class KrbKdcReq {

    //protected KrbKdcReq() {}

    /**
     * Default port for a KDC.
     */
    private static final int DEFAULT_KDC_PORT = Krb5.KDC_INET_DEFAULT_PORT;

    // Currently there is no option to specify retries  
    // in the kerberos configuration file

    private static final int DEFAULT_KDC_RETRY_LIMIT = Krb5.KDC_RETRY_LIMIT;

    /**
     * Default timeout period when requesting a ticket from a KDC.
     * If not specified in the configuration file, 
     * a value of 30 seconds is used.
     */
    public static final int DEFAULT_KDC_TIMEOUT; // milliseconds

    private static boolean DEBUG = Krb5.DEBUG;

    static {

        /*
         * Get default timeout.
         */
       
        int timeout = -1;
        try {
            Config cfg = Config.getInstance();
            String temp = cfg.getDefault("kdc_timeout", "libdefaults");
            timeout = parsePositiveIntString(temp);
        } catch (Exception exc) {
	   // ignore any exceptions; use the default time out values
	   if (DEBUG) {
		System.out.println ("Exception in getting kdc_timeout value, " +
				    "using default value " +
				    exc.getMessage());
	   }
        }

        if (timeout > 0)
            DEFAULT_KDC_TIMEOUT = timeout;
        else
            DEFAULT_KDC_TIMEOUT = 30*1000; // 30 seconds
    }

    protected byte[] obuf;
    protected byte[] ibuf;


    /**
     * Sends the provided data to the KDC of the specified realm.
     * Returns the response from the KDC.
     * Default realm/KDC is used if realm is null.
     * @param realm the realm of the KDC where data is to be sent.
     * @returns the kdc to which the AS request was sent to
     * @exception InterruptedIOException if timeout expires
     * @exception KrbException
     */

    public String send(String realm)
	throws IOException, KrbException {
	return (send(realm, false));	// default is to use UDP 
    }
 
    public String send(String realm, boolean useTCP) 
        throws IOException, KrbException {

        if (obuf == null)
            return null;
	Exception savedException = null;
        Config cfg = Config.getInstance();
        
	if (realm == null) {
            realm = cfg.getDefaultRealm();
            if (realm == null) {
                throw new KrbException(Krb5.KRB_ERR_GENERIC,
                                       "Cannot find default realm");
	    }
        }

        /*
         * Get timeout.
         */

	int timeout = getKdcTimeout(realm);

	String kdcList = cfg.getKDCList(realm);
	if (kdcList == null) {
	    throw new KrbException("Cannot get kdc for realm " + realm);
	}
	String tempKdc = null; // may include the port number also
	StringTokenizer st = new StringTokenizer(kdcList);
	while (st.hasMoreTokens()) {
	    tempKdc = st.nextToken();
	    try {
	        send(realm,tempKdc,useTCP);
		break;
	    } catch (Exception e) {
		savedException = e; 
	    }
	}
	if (ibuf == null && savedException != null) {
	    if (savedException instanceof IOException) {
		throw (IOException) savedException;
	    } else {
		throw (KrbException) savedException;
	    }
	}
	return tempKdc;
    }

    // send the AS Request to the specified KDC

    public void send(String realm, String tempKdc, boolean useTCP) 
	throws IOException, KrbException {
       
	if (obuf == null)
	    return;
	PrivilegedActionException savedException = null;
	int port = Krb5.KDC_INET_DEFAULT_PORT;
	
	/*
	 * Get timeout.
	 */
	int timeout = getKdcTimeout(realm);
	/*
	 * Get port number for this KDC.
	 */
	StringTokenizer strTok = new StringTokenizer(tempKdc, ":");
	String kdc = strTok.nextToken();
	if (strTok.hasMoreTokens()) {
	    String portStr = strTok.nextToken();
	    int tempPort = parsePositiveIntString(portStr);
	    if (tempPort > 0)
		port = tempPort;
	}
	
	if (DEBUG) {
	    System.out.println(">>> KrbKdcReq send: kdc=" + kdc 
			       + (useTCP ? " TCP:":" UDP:")
			       +  port +  ", timeout="
			       + timeout
			       + ", number of retries =" 
			       + DEFAULT_KDC_RETRY_LIMIT
			       + ", #bytes=" + obuf.length);
	}
	
	PrivilegedExceptionAction kdcCommunication =
	    new KdcCommunication(kdc, port, useTCP, timeout, obuf);
	try {
	    ibuf = (byte[]) AccessController.doPrivileged(kdcCommunication);
	    if (DEBUG) {
                System.out.println(">>> KrbKdcReq send: #bytes read=" 
			+ (ibuf != null ? ibuf.length : 0));
            }
	} catch (PrivilegedActionException e) {
	    Exception wrappedException = e.getException();
	    if (wrappedException instanceof IOException) {
		throw (IOException) wrappedException;
	    } else {
		throw (KrbException) wrappedException;
	    }
	}
	if (DEBUG) {
	    System.out.println(">>> KrbKdcReq send: #bytes read=" 
			       + (ibuf != null ? ibuf.length : 0));
	}
    }
    
    private static class KdcCommunication 
	implements PrivilegedExceptionAction {

	private String kdc;
	private int port;
	private boolean useTCP;
	private int timeout;
	private byte[] obuf;

	public KdcCommunication(String kdc, int port, boolean useTCP, 
				int timeout, byte[] obuf) {
	    this.kdc = kdc;
	    this.port = port;
	    this.useTCP = useTCP;
	    this.timeout = timeout;
	    this.obuf = obuf;
	}

	// The caller only casts IOException and KrbException so don't
	// add any new ones!

	public Object run() throws IOException, KrbException {
	    
	    byte[] ibuf = null;
	    
	    if (useTCP) {
		TCPClient kdcClient = null;
		kdcClient = new TCPClient(kdc, port); 
		/*
		 * Send the data to the kdc.
		 */
		kdcClient.send(obuf);
		/*
		 * And get a response.
		 */
		ibuf = kdcClient.receive();
	    } else {
		// For each KDC we try DEFAULT_KDC_RETRY_LIMIT (3) times to
		// get the response
		for (int i=1; i <= DEFAULT_KDC_RETRY_LIMIT; i++) {
		    UDPClient kdcClient = new UDPClient(kdc, port, timeout);
		    
		    if (DEBUG) {
	    		System.out.println(">>> KDCCommunication: kdc=" + kdc 
			       + (useTCP ? " TCP:":" UDP:")
			       +  port +  ", timeout="
			       + timeout
			       + ",Attempt =" + i 
			       + ", #bytes=" + obuf.length);
		    }
		    /*
		     * Send the data to the kdc.
		     */
		    
		    kdcClient.send(obuf);
		    
		    /*
		     * And get a response.
		     */
		    try {
			ibuf = kdcClient.receive();
			break;
		    } catch (SocketTimeoutException se) {
			if (DEBUG) {
			    System.out.println ("SocketTimeOutException with " +
						"attempt: " + i);
			}
			if (i == DEFAULT_KDC_RETRY_LIMIT) {
			    ibuf = null;
			    throw se;  
			}
		    }
		}
	    }
	    return ibuf;
	}
    }

    /**
     * Returns a timeout value for the KDC of the given realm.
     * A KDC-specific timeout, if specified in the config file,
     * overrides the default timeout (which may also be specified
     * in the config file). Default timeout is returned if null
     * is specified for realm.
     * @param realm the realm which kdc's timeout is requested
     * @return KDC timeout
     */
    private int getKdcTimeout(String realm)
    {
        int timeout = DEFAULT_KDC_TIMEOUT;

        if (realm == null)
            return timeout;

        int tempTimeout = -1;
        try {
            String temp = 
               Config.getInstance().getDefault("kdc_timeout", realm);
            tempTimeout = parsePositiveIntString(temp);
        } catch (Exception exc) {
        }

        if (tempTimeout > 0)
            timeout = tempTimeout;

        return timeout;
    }

    private static int parsePositiveIntString(String intString)
    {
        if (intString == null)
            return -1;

        int ret = -1;

        try {
            ret = Integer.parseInt(intString);
        } catch (Exception exc) {
            return -1;
        }

        if (ret >= 0)
            return ret;

        return -1;
    }
}    
