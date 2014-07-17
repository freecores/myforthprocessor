/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbAsReq.java	1.16 03/06/24
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
import sun.security.krb5.internal.crypto.Nonce;
import sun.security.util.*;
import java.io.IOException;
import java.io.ByteArrayInputStream;
import java.net.UnknownHostException;
import java.util.StringTokenizer;

/**
 * This class encapsulates the KRB-AS-REQ message that the client
 * sends to the KDC.
 */
public class KrbAsReq extends KrbKdcReq {
    private PrincipalName princName;
    private ASReq asReqMessg;
    
    private boolean DEBUG = Krb5.DEBUG;
    private static KDCOptions defaultKDCOptions = new KDCOptions();
    
    /**
     * Creates a KRB-AS-REQ to send to the default KDC
     * @throws //TBD
     */
    public KrbAsReq(PrincipalName principal) 
	throws KrbException, IOException {
	this((EncryptionKey)null, // for pre-authentication
	     defaultKDCOptions,
	     principal,
	     null, // PrincipalName sname
	     null, // KerberosTime from
	     null, // KerberosTime till
	     null, // KerberosTime rtime
	     null, // int[] eTypes
	     null, // HostAddresses addresses
	     null); // Ticket[] additionalTickets
    }

    /**
     * Creates a KRB-AS-REQ to send to the default KDC
     * @throws //TBD
     */
    public KrbAsReq(PrincipalName principal, EncryptionKey key) 
	throws KrbException, IOException {
	this(key, // for pre-authentication
	     defaultKDCOptions,
	     principal,
	     null, // PrincipalName sname
	     null, // KerberosTime from
	     null, // KerberosTime till
	     null, // KerberosTime rtime
	     null, // int[] eTypes
	     null, // HostAddresses addresses
	     null); // Ticket[] additionalTickets
    }
    
    /**
     * Creates a KRB-AS-REQ to send to the default KDC
     * @throws //TBD
     */
    public KrbAsReq(PrincipalName principal, StringBuffer password) 
	throws KrbException, IOException {
	this(password, // for pre-authentication
	     defaultKDCOptions,
	     principal,
	     null, // PrincipalName sname
	     null, // KerberosTime from
	     null, // KerberosTime till
	     null, // KerberosTime rtime
	     null, // int[] eTypes
	     null, // HostAddresses addresses
	     null); // Ticket[] additionalTickets
    }

    public KrbAsReq(
		    StringBuffer password,
		    KDCOptions options,
		    PrincipalName cname,
		    PrincipalName sname,
		    KerberosTime from,
		    KerberosTime till,
		    KerberosTime rtime,
		    int[] eTypes,
		    HostAddresses addresses,
		    Ticket[] additionalTickets)
	throws KrbException, IOException {

	EncryptionKey key = null;
	if (password != null)
	    key = new EncryptionKey(password, cname.getSalt());
	if (DEBUG) {
	    System.out.println(">>>KrbAsReq salt is " + cname.getSalt());
	}
	try {
	    init(
		 key,
		 options,
		 cname,
		 sname,
		 from,
		 till,
		 rtime,
		 eTypes,
		 addresses,
		 additionalTickets);
	}
	finally {
	    /*
	     * Its ok to destroy the key here because we created it and are
	     * now done with it.
	     */
	    if (key != null)
		key.destroy();
	}
    }
    
    public KrbAsReq(
		    EncryptionKey key,
		    KDCOptions options,
		    PrincipalName cname,
		    PrincipalName sname,
		    KerberosTime from,
		    KerberosTime till,
		    KerberosTime rtime,
		    int[] eTypes,
		    HostAddresses addresses,
		    Ticket[] additionalTickets) throws KrbException, IOException {
	init(
	     key,
	     options,
	     cname,
	     sname,
	     from,
	     till,
	     rtime,
	     eTypes,
	     addresses,
	     additionalTickets);
    }
    
    KrbAsReq(KDCOptions options,
	     PrincipalName cname,
	     PrincipalName sname,
	     KerberosTime from,
	     KerberosTime till,
	     KerberosTime rtime,
	     int[] eTypes,
	     HostAddresses addresses,
	     Ticket[] additionalTickets)
	throws KrbException, IOException {
	init(null,
	     options,
	     cname,
	     sname,
	     from,
	     till,
	     rtime,
	     eTypes,
	     addresses,
	     additionalTickets);
    }
    
    private void init(EncryptionKey key,
		      KDCOptions options,
		      PrincipalName cname,
		      PrincipalName sname,
		      KerberosTime from,
		      KerberosTime till,
		      KerberosTime rtime,
		      int[] eTypes,
		      HostAddresses addresses,
		      Ticket[] additionalTickets )
	throws KrbException, IOException {
	
	// check if they are valid arguments. The optional fields should be
	// consistent with settings in KDCOptions. Mar 17 2000
	if (options.get(KDCOptions.FORWARDED) ||
	    options.get(KDCOptions.PROXY) ||
	    options.get(KDCOptions.ENC_TKT_IN_SKEY) ||
	    options.get(KDCOptions.RENEW) ||
	    options.get(KDCOptions.VALIDATE)) {
	    // this option is only specified in a request to the
	    // ticket-granting server
	    throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
	}	   
        if (options.get(KDCOptions.POSTDATED)) {
	    //  if (from == null) 
	    //  	throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
	} else {
	    if (from != null)  from = null;
	}
        if (options.get(KDCOptions.RENEWABLE)) { 
	    //  if (rtime == null) 
	    //  	throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
	} else {
	    if (rtime != null)  rtime = null;
	}
	
	princName = cname;
	
	PAData[] paData = null;
	if (Krb5.PA_ENC_TIMESTAMP_REQUIRED) {
	    PAEncTSEnc ts = new PAEncTSEnc();
	    byte[] temp = ts.asn1Encode();
	    if (key != null) {
		EncryptedData encTs = new EncryptedData(key, temp);
		paData = new PAData[1]; 
		paData[0] = new PAData( Krb5.PA_ENC_TIMESTAMP,
					encTs.asn1Encode());
	    }
	}

        if (DEBUG) {
	    System.out.println(">>> KrbAsReq calling createMessage");
	}

	asReqMessg = createMessage(
				   paData,
				   options,
				   cname,
				   cname.getRealm(),
				   sname,
				   from,
				   till,
				   rtime,
				   eTypes,
				   addresses,
				   additionalTickets);
	obuf = asReqMessg.asn1Encode();
    }

    /**
     * Returns an AS-REP message corresponding to the AS-REQ that
     * was sent.
     * @param password The password that will be used to derive the
     * secret key that will decrypt the AS-REP from  the KDC.
     * @exception KrbException if an error occurs while reading the data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public KrbAsRep getReply(StringBuffer password) 
	throws KrbException, IOException {
	if (password == null)
	    throw new KrbException(Krb5.API_INVALID_ARG);
	KrbAsRep temp = null;
	EncryptionKey key = null;
	try {
	    key = new EncryptionKey(password, princName.getSalt());
	    temp = getReply(key);
	} finally {
            /* 
             * Its ok to destroy the key here because we created it and are 
             * now done with it. 
             */ 
	    if (key != null)
		key.destroy();
	}
	return temp;
    }

    /**
     * Sends an AS request to the realm of the client.
     * returns the KDC hostname that the request was sent to
     */

    public String send()
        throws IOException, KrbException
    {
        String realmStr = null;
        if (princName != null)
            realmStr = princName.getRealmString();

        return (send(realmStr));
    }
    
    /**
     * Returns an AS-REP message corresponding to the AS-REQ that
     * was sent.
     * @param key The secret key that will decrypt the AS-REP from
     * the KDC.
     * @exception KrbException if an error occurs while reading the data.
     * @exception IOException if an I/O error occurs while reading encoded 
     * data.
     *
     */
    public KrbAsRep getReply(EncryptionKey key)
	throws KrbException,IOException {
	return new KrbAsRep(ibuf, key, this);
    }

    ASReq createMessage(
			PAData[] paData,
			KDCOptions kdc_options,
			PrincipalName cname,
			Realm crealm,
			PrincipalName sname,
			KerberosTime from,
			KerberosTime till,
			KerberosTime rtime,
			int[] eTypes,
			HostAddresses addresses,
			Ticket[] additionalTickets
			) throws Asn1Exception,	KrbApErrException,
			RealmException,	UnknownHostException, IOException {

	if (DEBUG) {
	    System.out.println(">>> KrbAsReq in createMessage");
	}

	PrincipalName req_sname = null;
	if (sname == null) {
	    if (crealm == null) {
		throw new RealmException(Krb5.REALM_NULL,
					 "default realm not specified ");
	    }	
	    req_sname = new PrincipalName(
					  "krbtgt" + 
					  PrincipalName.NAME_COMPONENT_SEPARATOR +
					  crealm.toString(),
					  PrincipalName.KRB_NT_UNKNOWN);
	} else
	    req_sname = sname;
		
	KerberosTime req_till = null;
	if (till == null) {
	    req_till = new KerberosTime();
        } else {
	    req_till = till;
	}

        int[] reqETypes;
	if (eTypes == null) {
            try {
	        reqETypes = 
		    Config.getInstance().defaultEtype("default_tkt_enctypes");
                if (DEBUG) {
		    System.out.print(">>> KrbAsReq etypes are: ");
		    for (int i = 0; i < reqETypes.length; i++)
			System.out.print(reqETypes[i] + " ");
		    System.out.print('\n');
                }
	    } catch (KrbException e) {
		if (DEBUG) {
		    System.out.println(">>> KrbAsReq exception " +
				       "obtaining etypes from configuration;" + 
				       "using default values " +
				       e.getMessage());
		}
		
		reqETypes = new int[2];
		reqETypes[0] = EncryptedData.ETYPE_DES_CBC_MD5;
		reqETypes[1] = EncryptedData.ETYPE_DES_CBC_CRC;
	    }
	} else {
	    reqETypes = eTypes;
	}
	
	KDCReqBody kdc_req_body = new KDCReqBody(kdc_options,
						 cname,
						 crealm,
						 req_sname,
						 from,
						 req_till,
						 rtime,
						 Nonce.value(),
						 reqETypes,
						 addresses,
						 null,
						 additionalTickets);
	
	return new ASReq(
			 paData, 
			 kdc_req_body);
    }
    
    ASReq getMessage() {
	return asReqMessg;
    }
    // Workaround for a bug in DashoPro
    public String send(String realm) throws IOException, KrbException {
	return (super.send(realm));	
    }
}
