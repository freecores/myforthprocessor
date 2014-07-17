/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbApReq.java	1.9 03/06/24
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
import sun.security.krb5.internal.crypto.*;
import sun.security.krb5.internal.rcache.*;
import java.net.InetAddress;
import sun.security.util.*;
import java.io.IOException;

/**
 * This class encapsulates a KRB-AP-REQ that a client sends to a
 * server for authentication.
 */
public class KrbApReq {

    private byte[] obuf;
    private KerberosTime ctime;
    private int cusec;
    private Authenticator authenticator;
    private Credentials creds;
    private APReq apReqMessg;

    private static CacheTable table = new CacheTable();
    
    private static boolean DEBUG = Krb5.DEBUG;

    /**
     * Contructs a AP-REQ message to send to the peer.
     * @param tgsCred the <code>Credentials</code> to be used to construct the 
     *		AP Request  protocol message. 
     * @param mutualRequired Whether mutual authentication is required
     * @param useSubkey Whether the subkey is to be used to protect this 
     *	      specific application session. If this is not set then the 
     *	      session key from the ticket will be used.
     * @throws KrbException for any Kerberos protocol specific error
     * @throws IOException for any IO related errors 
     *		(e.g. socket operations)
     */
    public KrbApReq(Credentials tgsCred,
		    boolean mutualRequired,
		    boolean useSubKey,
		    boolean useSeqNumber) throws Asn1Exception,
		    KrbCryptoException, KrbException, IOException {
	
	this(tgsCred, mutualRequired, useSubKey, useSeqNumber, null);
    }

    /**
     * Contructs a AP-REQ message to send to the peer.
     * @param tgsCred the <code>Credentials</code> to be used to construct the 
     *		AP Request  protocol message. 
     * @param mutualRequired Whether mutual authentication is required
     * @param useSubkey Whether the subkey is to be used to protect this 
     *	      specific application session. If this is not set then the 
     *	      session key from the ticket will be used.
     * @param checksum checksum of the the application data that accompanies 
     *	      the KRB_AP_REQ.
     * @throws KrbException for any Kerberos protocol specific error
     * @throws IOException for any IO related errors 
     *		(e.g. socket operations)
     */

    public KrbApReq(Credentials tgsCred,
		    boolean mutualRequired,
		    boolean useSubKey,
		    boolean useSeqNumber,
		    Checksum cksum) throws Asn1Exception,
		    KrbCryptoException, KrbException, IOException  {

	APOptions apOptions = (mutualRequired?
			       new APOptions(Krb5.AP_OPTS_MUTUAL_REQUIRED): 
			       new APOptions());
	if (DEBUG)
	    System.out.println(">>> KrbApReq: APOptions are " + apOptions);
	
	EncryptionKey subKey = (useSubKey?
				new EncryptionKey(tgsCred.getSessionKey()):
				null);
	
	SeqNumber seqNum = new LocalSeqNumber();
	
	init(apOptions,
	     tgsCred,
	     cksum,
	     subKey,
	     seqNum,
	     null);  // AuthorizationData authzData
    }
    
    /**
     * Contructs a AP-REQ message from the bytes received from the
     * peer.
     * @param message The message received from the peer
     * @param key <code>EncrtyptionKey</code> to decrypt the message
     * @throws KrbException for any Kerberos protocol specific error
     * @throws IOException for any IO related errors 
     *		(e.g. socket operations)
     */
    public KrbApReq(byte[] message,
		    EncryptionKey key) 
	throws KrbException, IOException {
	obuf = message;
	if (apReqMessg == null)
	    decode();
	authenticate(key, null);
    }

    /**
     * Contructs a AP-REQ message from the bytes received from the
     * peer.
     * @param value The <code>DerValue</code> that contains the 
     *		    DER enoded AP-REQ protocol message
     * @param key <code>EncrtyptionKey</code> to decrypt the message
     * @throws KrbException for any Kerberos protocol specific error
     * @throws IOException for any IO related errors 
     *		(e.g. socket operations)
     */

    public KrbApReq(DerValue value, EncryptionKey key) 
	throws KrbException, IOException {
	obuf = value.toByteArray();
	if (apReqMessg == null)
	    decode(value);
	authenticate(key, null);
    }

    KrbApReq(APOptions options,
	     Credentials tgs_creds,
	     Checksum cksum,
	     EncryptionKey subKey,
	     SeqNumber seqNumber,
	     AuthorizationData authorizationData)
	throws KrbException, IOException {
	init(options, tgs_creds, cksum, subKey, seqNumber, authorizationData);
    }
    
    KrbApReq(APOptions apOptions,
	     Ticket ticket,
	     EncryptionKey key,
	     Realm crealm,
	     PrincipalName cname,
	     Checksum cksum,
	     KerberosTime ctime,
	     EncryptionKey subKey,
	     SeqNumber seqNumber,
	     AuthorizationData authorizationData)
	throws Asn1Exception, IOException,
	       KdcErrException, KrbCryptoException {

	init(apOptions, ticket, key, crealm, cname,
	     cksum, ctime, subKey, seqNumber, authorizationData);
	
    }

    private void init(APOptions options,
		      Credentials tgs_creds,
		      Checksum cksum,
		      EncryptionKey subKey,
		      SeqNumber seqNumber,
		      AuthorizationData authorizationData) 
	throws KrbException, IOException {
	
	ctime = new KerberosTime(KerberosTime.NOW);
	init(options,
	     tgs_creds.ticket,
	     tgs_creds.key,
	     tgs_creds.client.getRealm(),
	     tgs_creds.client,
	     cksum,
	     ctime, 
	     subKey,
	     seqNumber,
	     authorizationData);
    }

    private void init(APOptions apOptions,
		      Ticket ticket,
		      EncryptionKey key,
		      Realm crealm,
		      PrincipalName cname,
		      Checksum cksum,
		      KerberosTime ctime,
		      EncryptionKey subKey,
		      SeqNumber seqNumber,
		      AuthorizationData authorizationData) 
	throws Asn1Exception, IOException,
	       KdcErrException, KrbCryptoException {

	createMessage(apOptions, ticket, key, crealm, cname,
		      cksum, ctime, subKey, seqNumber, authorizationData);
	obuf = apReqMessg.asn1Encode();
    }
	
    
    void decode() throws KrbException, IOException {
	DerValue encoding = new DerValue(obuf); 
	decode(encoding);
    }

    void decode(DerValue encoding) throws KrbException, IOException {
	apReqMessg = null;
	try {
	    apReqMessg = new APReq(encoding);
    	} catch (Asn1Exception e) {
	    apReqMessg = null;
	    KRBError err = new KRBError(encoding);
	    String eText;
	    if (err.eText.charAt(err.eText.length() - 1) == 0)
		eText = err.eText.substring(0, err.eText.length() - 1);
	    else
		eText = err.eText;
	    KrbException ke = new KrbException(err.errorCode, eText);
	    ke.initCause(e);
	    throw ke; 
	}
    }
    
    void authenticate(EncryptionKey key, HostAddress sender)
	throws KrbException, IOException {
        byte[] bytes = apReqMessg.ticket.encPart.decrypt(key);
	byte[] temp = apReqMessg.ticket.encPart.reset(bytes, true); 
	EncTicketPart enc_ticketPart = new EncTicketPart(temp);
	
	byte[] bytes2 = apReqMessg.authenticator.decrypt(enc_ticketPart.key);
	byte[] temp2 = apReqMessg.authenticator.reset(bytes2, true); 
	authenticator = new Authenticator(temp2);
	ctime = authenticator.ctime;
	cusec = authenticator.cusec;
	authenticator.ctime.setMicroSeconds(authenticator.cusec);
	authenticator.cname.setRealm(authenticator.crealm);
	apReqMessg.ticket.sname.setRealm(apReqMessg.ticket.realm);
	enc_ticketPart.cname.setRealm(enc_ticketPart.crealm);

        Config.getInstance().resetDefaultRealm(apReqMessg.ticket.realm.toString());

	if (!authenticator.cname.equals(enc_ticketPart.cname))
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_BADMATCH);

        if (!authenticator.ctime.inClockSkew())
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_SKEW);
	
        // start to check if it is a replay attack.
        AuthTime time =
	    new AuthTime(authenticator.ctime.getTime(), authenticator.cusec);
        String client = authenticator.cname.toString();
        if (table.get(time, authenticator.cname.toString()) != null) {
            throw new KrbApErrException(Krb5.KRB_AP_ERR_REPEAT);
        } else {
            table.put(client, time);
        }
   
	if (sender != null || !Krb5.KDC_EMPTY_ADDRESSES_ALLOWED) {
	    if (enc_ticketPart.caddr != null) {
		if (sender == null)
		    throw new KrbApErrException(Krb5.KRB_AP_ERR_BADADDR);
		if (!enc_ticketPart.caddr.inList(sender))
		    throw new KrbApErrException(Krb5.KRB_AP_ERR_BADADDR);
	    }
	}
	
	// check for repeated authenticator
	// if found
	//    throw new KrbApErrException(Krb5.KRB_AP_ERR_REPEAT);
	// else
	//    save authenticator to check for later
	
	KerberosTime now = new KerberosTime(KerberosTime.NOW);
	
	if ((enc_ticketPart.starttime != null &&
	     enc_ticketPart.starttime.greaterThanWRTClockSkew(now)) ||
	    enc_ticketPart.flags.get(Krb5.TKT_OPTS_INVALID))
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_TKT_NYV);
	// if the current time is later than end time by more
	// than the allowable clock skew, throws ticket expired exception.
	if (enc_ticketPart.endtime != null &&
	    now.greaterThanWRTClockSkew(enc_ticketPart.endtime)) {
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_TKT_EXPIRED);
	}
	
	creds = new Credentials(
				apReqMessg.ticket,
				authenticator.cname,
				apReqMessg.ticket.sname,
				enc_ticketPart.key,
				null, 
				enc_ticketPart.authtime,
				enc_ticketPart.starttime,
				enc_ticketPart.endtime,
				enc_ticketPart.renewTill,
				enc_ticketPart.caddr);
	if (DEBUG) {
	    System.out.println(">>> KrbApReq: authenticate succeed.");
	}
    }
    
    /**
     * Returns the credentials that are contained in the ticket that
     * is part of this this AP-REP.
     */
    public Credentials getCreds() {
	return creds;
    }

    KerberosTime getCtime() {
	if (ctime != null)
	    return ctime;
	return authenticator.ctime;
    }

    int cusec() {
	return cusec;
    }

    APOptions getAPOptions() throws KrbException, IOException {
	if (apReqMessg == null)
	    decode();
	if (apReqMessg != null)
	    return apReqMessg.apOptions;
	return null;
    }
    
    /**
     * Returns true if mutual authentication is required and hence an 
     * AP-REP will need to be generated.
     * @throws //TBD
     */
    public boolean getMutualAuthRequired() throws KrbException, IOException {
	if (apReqMessg == null)
	    decode();
	if (apReqMessg != null)
	    return apReqMessg.apOptions.get(Krb5.AP_OPTS_MUTUAL_REQUIRED);
	return false;
    }
    
    boolean useSessionKey() throws KrbException, IOException {
	if (apReqMessg == null)
	    decode();
	if (apReqMessg != null)
	    return apReqMessg.apOptions.get(Krb5.AP_OPTS_USE_SESSION_KEY);
	return false;
    }
    
    /**
     * Returns the optional subkey stored in the Authenticator for
     * this message. Returns null if none is stored. 
     */
    public EncryptionKey getSubKey() {
	// Can authenticator be null?
	return authenticator.getSubKey();
    }
    
    /**
     * Returns the optional sequence number stored in the
     * Authenticator for this message. Returns null if none is
     * stored.
     */
    public Integer getSeqNumber() {
	// Can authenticator be null?
	return authenticator.getSeqNumber();
    }

    /**
     * Returns the optional Checksum stored in the
     * Authenticator for this message. Returns null if none is
     * stored.
     */
    public Checksum getChecksum() {
	return authenticator.getChecksum();
    }

    /**
     * Returns the ASN.1 encoding that should be sent to the peer.
     */
    public byte[] getMessage() {
	return obuf;
    }

    /**
     * Returns the principal name of the client that generated this
     * message.
     */
    public PrincipalName getClient() {
	return creds.getClient();
    }

    private void createMessage(APOptions apOptions,
			       Ticket ticket,
			       EncryptionKey key,
			       Realm crealm,
			       PrincipalName cname,
			       Checksum cksum,
			       KerberosTime ctime,
			       EncryptionKey subKey,
			       SeqNumber seqNumber,
			       AuthorizationData authorizationData) 
	throws Asn1Exception, IOException,
	       KdcErrException, KrbCryptoException {
	
	Integer seqno = null;
	
	if (seqNumber != null)
	    seqno = new Integer(seqNumber.current());
	
	authenticator = 
	    new Authenticator(crealm,
			      cname,
			      cksum,
			      ctime.getMicroSeconds(),
			      ctime,
			      subKey,
			      seqno,
			      authorizationData);
	
	byte[] temp = authenticator.asn1Encode();
	
	EncryptedData encAuthenticator = 
	    new EncryptedData(key, temp);
	
	apReqMessg = 
	    new APReq(apOptions, ticket, encAuthenticator);
    }
}
