/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbCred.java	1.9 03/06/24
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
import java.io.IOException;
import sun.security.util.DerValue;

/**
 * This class encapsulates the KRB-CRED message that a client uses to
 * send its delegated credentials to a server.
 *
 * Supports delegation of one ticket only.
 * @author Mayank Upadhyay
 * @version 1.9, 06/24/03
 */
public class KrbCred {

    private static boolean DEBUG = Krb5.DEBUG;

    private byte[] obuf = null;
    private KRBCred credMessg = null;
    private Ticket ticket = null;
    private EncKrbCredPart encPart = null;
    private Credentials creds = null;
    private KerberosTime timeStamp = null;

    /**
     * @param TBD
     */
    public KrbCred(Credentials tgt,
		   Credentials serviceTicket,
		   EncryptionKey key)
	throws KrbException, IOException {

	PrincipalName client = tgt.getClient();
	PrincipalName tgService = tgt.getServer();
	PrincipalName server = serviceTicket.getServer();
	if (!serviceTicket.getClient().equals(client))
	    throw new KrbException("TBD");
	
	// Check Windows flag OK-TO-FORWARD-TO
	// TBD

	// Invoke TGS-REQ to get a forwarded TGT for the peer

	KDCOptions options = new KDCOptions();
	options.set(KDCOptions.FORWARDED, true);
	options.set(KDCOptions.FORWARDABLE, true);

	HostAddresses sAddrs = null;
	// TBD: What about a NT_GSS_KRB5_PRINCIPAL that is actually a host
	// based principal? Also bug: GSSName.NT_HOSTBASED_SERVICE was not
	// showing up here as KRB_NT_SRV_HST
	if (server.getNameType() == PrincipalName.KRB_NT_SRV_HST)
	    sAddrs=  new HostAddresses(server);

	KrbTgsReq tgsReq = new KrbTgsReq(options, tgt, tgService,
					 null, null, null, null, sAddrs, null, null, null);
	KrbTgsRep tgsRep = null;
	String kdc = null;
	try {
	    kdc = tgsReq.send();
	    tgsRep = tgsReq.getReply(tgt);
	} catch (KrbException ke) {
		if (ke.returnCode() == Krb5.KRB_ERR_RESPONSE_TOO_BIG) {
		    tgsReq.send(tgService.getRealmString(), kdc, true); // useTCP is set
	            tgsRep = tgsReq.getReply(tgt);
		} else {
		    throw ke;
		}
	}

	credMessg = createMessage(tgsRep.getCreds(), key);

	obuf = credMessg.asn1Encode();
    }

    KRBCred createMessage(Credentials delegatedCreds, EncryptionKey key) 
	throws KrbException, IOException {

	EncryptionKey sessionKey 
	    = (EncryptionKey) delegatedCreds.getSessionKey();
	PrincipalName princ = delegatedCreds.getClient();
	Realm realm = princ.getRealm();
	PrincipalName tgService = delegatedCreds.getServer();
	Realm tgsRealm = tgService.getRealm();

	KrbCredInfo credInfo = new KrbCredInfo(sessionKey, realm,
					       princ, delegatedCreds.flags, delegatedCreds.authTime,
					       delegatedCreds.startTime, delegatedCreds.endTime,
					       delegatedCreds.renewTill, tgsRealm, tgService,
					       delegatedCreds.cAddr); 
       
	timeStamp = new KerberosTime(KerberosTime.NOW);
	KrbCredInfo[] credInfos = {credInfo};
	EncKrbCredPart encPart = 
	    new EncKrbCredPart(credInfos,
			       timeStamp, null, null, null, null);

	EncryptedData encEncPart = new EncryptedData(key,
						     encPart.asn1Encode());

	Ticket[] tickets = {delegatedCreds.ticket};

	credMessg = new KRBCred(tickets, encEncPart);

	return credMessg;
    }

    /**
     * @param TBD
     */
    public KrbCred(byte[] asn1Message, EncryptionKey key) 
	throws KrbException, IOException {
	
	credMessg = new KRBCred(asn1Message);

	ticket = credMessg.tickets[0];

	byte[] temp = credMessg.encPart.decrypt(key);
	byte[] plainText = credMessg.encPart.reset(temp, true); 
	DerValue encoding = new DerValue(plainText);
	EncKrbCredPart encPart = new EncKrbCredPart(encoding);

	timeStamp = encPart.timeStamp;

	KrbCredInfo credInfo = encPart.ticketInfo[0];
	EncryptionKey credInfoKey = credInfo.key;
	Realm prealm = credInfo.prealm; 
	// TBD: Deos principal name represent a realm + principalname
	// of rjust a principal name? Assume it can store both realm
	// and principalname and thus create such an instance
	// below. Revist krb5 code to see where it used PrincipalName 
	// in a diffferent sense.ie.e, where it stores realm in a
	// different field then principal name.
	PrincipalName pname = credInfo.pname; 
	pname.setRealm(prealm);
	TicketFlags flags = credInfo.flags; 
	KerberosTime authtime = credInfo.authtime; 
	KerberosTime starttime = credInfo.starttime; 
	KerberosTime endtime = credInfo.endtime; 
	KerberosTime renewTill = credInfo.renewTill; 
	Realm srealm = credInfo.srealm; 
	PrincipalName sname = credInfo.sname; 
	sname.setRealm(srealm);
	HostAddresses caddr = credInfo.caddr; 

	if (DEBUG) {
	    System.out.println(">>>Delegated Creds have pname=" + pname
			       + " sname=" + sname
			       + " authtime=" + authtime
			       + " starttime=" + starttime
			       + " endtime=" + endtime
			       + "renewTill=" + renewTill);
	}
	creds = new Credentials(ticket, pname, sname, credInfoKey, 
				flags, authtime, starttime, endtime, renewTill, caddr);
    }

    /**
     * Returns the delegated credentials from the peer.
     */
    public Credentials[] getDelegatedCreds() {

	Credentials[] allCreds = {creds};
	return allCreds;
    }

    /**
     * Returns the ASN.1 encoding that should be sent to the peer.
     */
    public byte[] getMessage() {
	return obuf;
    }
}
