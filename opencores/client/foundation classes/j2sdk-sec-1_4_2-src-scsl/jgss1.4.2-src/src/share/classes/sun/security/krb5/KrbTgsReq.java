/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbTgsReq.java	1.9 03/06/24
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

import sun.security.util.*;
import sun.security.krb5.EncryptionKey;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.crypto.*;
import java.io.IOException;
import java.net.UnknownHostException;
import java.util.StringTokenizer;
import java.io.InterruptedIOException;

/**
 * This class encapsulates a Kerberos TGS-REQ that is sent from the
 * client to the KDC. 
 */
public class KrbTgsReq extends KrbKdcReq {

    private PrincipalName princName;
    private PrincipalName servName;
    private TGSReq tgsReqMessg;
    private KerberosTime ctime;
    private Ticket secondTicket = null; 
 
    private static boolean DEBUG = Krb5.DEBUG;

    private int defaultTimeout = 30*1000; // 30 seconds

   /**
    * Creates a TGS-REQ to send to the KDC. It uses the TGT
    * credentials passed in to determine the name of the client
    * principal.
    * @param asCreds the credentials received from the AS-REQ. In
    * other words, the credentials consisting of the TGT and its
    * session key.
    * @param sname the service principal for which a ticket is
    * desired
    * @param desiredLifetime the lifetime in seconds desired for the
    * service ticket. This may not be the actual lifetime that the
    * KDC assigns.
    * @ throws KrbException TBD
    * @throws IOException TBD
    */
    public KrbTgsReq(Credentials asCreds,
		     PrincipalName sname,
		     int desiredLifetime) 
	throws KrbException, IOException {
	this(new KDCOptions(),
	     asCreds,
	     sname,
	     null, //KerberosTime from
	     null, //KerberosTime till
	     null, //KerberosTime rtime
	     null, //eTypes, //null, //int[] eTypes
	     null, //HostAddresses addresses
	     null, //AuthorizationData authorizationData
	     null, //Ticket[] additionalTickets
	     null //EncryptionKey subSessionKey
	     );

    }

    public KrbTgsReq(Credentials asCreds,
		     PrincipalName sname)
	throws KrbException, IOException {
	this(new KDCOptions(),
	     asCreds,
	     sname,
	     null, //KerberosTime from
	     null, //KerberosTime till
	     null, //KerberosTime rtime
	     null, //eTypes, //null, //int[] eTypes
	     null, //HostAddresses addresses
	     null, //AuthorizationData authorizationData
	     null, //Ticket[] additionalTickets
	     null //EncryptionKey subSessionKey
	     );

    }

	 KrbTgsReq(
		KDCOptions options,
		Credentials asCreds,
		PrincipalName sname,
		KerberosTime from,
		KerberosTime till,
		KerberosTime rtime,
		int[] eTypes,
		HostAddresses addresses,
		AuthorizationData authorizationData,
		Ticket[] additionalTickets,
		EncryptionKey subKey
	) throws KrbException, IOException {

		princName = asCreds.client;
		servName = sname;
		ctime = new KerberosTime(KerberosTime.NOW);


		// check if they are valid arguments. The optional fields
		// should be  consistent with settings in KDCOptions.
		if (options.get(KDCOptions.FORWARDABLE) && (!(asCreds.flags.get(Krb5.TKT_OPTS_FORWARDABLE)))) {
		    throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		if (options.get(KDCOptions.FORWARDED)) {
		    if (!(asCreds.flags.get(KDCOptions.FORWARDABLE)))
			throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		if (options.get(KDCOptions.PROXIABLE) && (!(asCreds.flags.get(Krb5.TKT_OPTS_PROXIABLE)))) {
		    throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		if (options.get(KDCOptions.PROXY)) {
		    if (!(asCreds.flags.get(KDCOptions.PROXIABLE)))
			throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		if (options.get(KDCOptions.ALLOW_POSTDATE) && (!(asCreds.flags.get(Krb5.TKT_OPTS_MAY_POSTDATE)))) {
		    throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		if (options.get(KDCOptions.RENEWABLE) && (!(asCreds.flags.get(Krb5.TKT_OPTS_RENEWABLE)))) {
		    throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		}
		
		if (options.get(KDCOptions.POSTDATED)) {
		    if (!(asCreds.flags.get(KDCOptions.POSTDATED)))
			throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		} else {
		    if (from != null)  from = null;
		}
		if (options.get(KDCOptions.RENEWABLE)) { 
		    if (!(asCreds.flags.get(KDCOptions.RENEWABLE)))
			throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		} else {
		    if (rtime != null)  rtime = null;
		}
		if (options.get(KDCOptions.ENC_TKT_IN_SKEY)) {
		    if (additionalTickets == null) 
			throw new KrbException(Krb5.KRB_AP_ERR_REQ_OPTIONS);
		    // in TGS_REQ there could be more than one additional tickets,
		    // but in file-based credential cache, there is only one additional ticket field.
			secondTicket = additionalTickets[0];
		} else {
			if (additionalTickets != null) 	additionalTickets = null;
		} 

		tgsReqMessg = createRequest(
			options,
			asCreds.ticket,
			asCreds.key,
			ctime,
			princName,
			princName.getRealm(),
			servName,
			from,
			till,
			rtime,
			eTypes,
			addresses,
			authorizationData,
			additionalTickets,
			subKey
		);
		obuf = tgsReqMessg.asn1Encode();

		// TBD: We need to revisit this to see if can't move it
                //       up such that FORWARDED flag set in the options
                //       is included in the marshaled request.
		/*
		 * If this is based on a forwarded ticket, record that in the
		 * options, because the returned TgsRep will contain the
		 * FORWARDED flag set.
		 */
		if (asCreds.flags.get(KDCOptions.FORWARDED))
		    options.set(KDCOptions.FORWARDED, true);


	}

    /**
     * Sends a TGS request to the realm of the target.
     * @throws //TBD
     */
    public String send() throws IOException, KrbException {
        String realmStr = null;
        if (servName != null)
            realmStr = servName.getRealmString();
	
        return(send(realmStr));
    }

    /**
     * Returns the TGS-REP that the KDC sends back.
     * @throws //TBD
     */
    public KrbTgsRep getReply(Credentials asCreds) 
	throws KrbException, IOException {
	return new KrbTgsRep(ibuf, asCreds, this);
    }
    
    KerberosTime getCtime() {
	return ctime;
    }
    
    TGSReq createRequest(
			 KDCOptions kdc_options,
			 Ticket ticket,
			 EncryptionKey key,
			 KerberosTime ctime,
			 PrincipalName cname,
			 Realm crealm,
			 PrincipalName sname,
			 KerberosTime from,
			 KerberosTime till,
			 KerberosTime rtime,
			 int[] eTypes,
			 HostAddresses addresses,
			 AuthorizationData authorizationData,
			 Ticket[] additionalTickets,
			 EncryptionKey subKey) 
	throws Asn1Exception, IOException, KdcErrException, KrbApErrException,
	       UnknownHostException, KrbCryptoException {
	KerberosTime req_till = null;
	if (till == null) {
	    req_till = new KerberosTime();
	} else {
	    req_till = till;
	}
	
	int[] req_eTypes = null;
	if (eTypes == null) {
	    try {
		req_eTypes = 
		    Config.getInstance().defaultEtype("default_tgs_enctypes");
	    } catch (KrbException exc) {
		if (DEBUG) {
		    System.out.println("Exception while getting " +
				       "default_tgs_enctypes " + 
				       exc.getMessage());
		    System.out.println("Setting default_tgs_enctypes " +
				       " to DES_CBC_MD5 and DES_CBC_CRC");
		    
		    
		}
		req_eTypes = new int[2];
		//we set the default to be des-cbc-md5 and des-cbc-crc.
		req_eTypes[0] = EncryptedData.ETYPE_DES_CBC_MD5;
		req_eTypes[1] = EncryptedData.ETYPE_DES_CBC_CRC;
	    }
	} else {
	    req_eTypes = eTypes;
	}
	
	EncryptionKey reqKey = null;
	EncryptedData encAuthorizationData = null;
	if (authorizationData != null) {
	    byte[] ad = authorizationData.asn1Encode();
	    if (subKey != null) {
		reqKey = subKey;
		encAuthorizationData = new EncryptedData(reqKey, ad);
	    } else
		encAuthorizationData = new EncryptedData(key, ad);
	}
	
	KDCReqBody reqBody = new KDCReqBody(
					    kdc_options,
					    cname,
					    //crealm,
					    sname.getRealm(), //TO
					    sname,
					    from,
					    req_till,
					    rtime,
					    Nonce.value(),
					    req_eTypes,
					    addresses,
					    encAuthorizationData,
					    additionalTickets
					    );
	
	byte[] temp = reqBody.asn1Encode(Krb5.KRB_TGS_REQ);
	// if the checksum type is one of the keyed checksum types, 
	// use session key. 
	Checksum cksum;
	switch (Checksum.CKSUMTYPE_DEFAULT) {
	case Checksum.CKSUMTYPE_RSA_MD4_DES:
	case Checksum.CKSUMTYPE_DES_MAC:
	case Checksum.CKSUMTYPE_DES_MAC_K:
	case Checksum.CKSUMTYPE_RSA_MD4_DES_K:
	case Checksum.CKSUMTYPE_RSA_MD5_DES:
	    cksum = new Checksum(Checksum.CKSUMTYPE_DEFAULT, temp,	key);
	    break;
	case Checksum.CKSUMTYPE_CRC32:
	case Checksum.CKSUMTYPE_RSA_MD4:
	case Checksum.CKSUMTYPE_RSA_MD5:
	default:
	    cksum = new Checksum(Checksum.CKSUMTYPE_DEFAULT, temp);
	}
	
	byte[] tgs_ap_req = new KrbApReq(
					 new APOptions(),
					 ticket,
					 key,
					 crealm,
					 cname,
					 cksum,
					 ctime,
					 reqKey,
					 null,
					 null
					 ).getMessage();
	
	PAData[] tgsPAData = new PAData[1];
	tgsPAData[0] = new PAData(Krb5.PA_TGS_REQ, tgs_ap_req);
	
	return new TGSReq(tgsPAData, reqBody);
    }
    
    TGSReq getMessage() {
	return tgsReqMessg;
    }
    
    Ticket getSecondTicket() {
	return secondTicket;
    }
    
    private static void debug(String message) {
	//	System.err.println(">>> KrbTgsReq: " + message);
    }

    // Workaround for a bug in DashoPro
    public String send(String realm) throws IOException, KrbException {
	return (super.send(realm));	
    }
    
}
