/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbApRep.java	1.7 03/06/24
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
import sun.security.util.*;
import java.io.IOException;

/**
 * This class encapsulates a KRB-AP-REP sent from the service to the
 * client.
 */
public class KrbApRep {
    private byte[] obuf;
    private byte[] ibuf;
    private EncAPRepPart encPart; // although in plain text
    private APRep apRepMessg;

    /**
     * Constructs a KRB-AP-REP to send to a client.
     * @param //TBD
     * @throws //TBD
     */
    public KrbApRep(KrbApReq incomingReq,
		    boolean useSeqNumber,
		    boolean useSubKey) throws KrbException, IOException {
	
	EncryptionKey subKey = 
	    (useSubKey?
	     new EncryptionKey((EncryptionKey)incomingReq.getCreds().getSessionKey()):null);
	SeqNumber seqNum = new LocalSeqNumber();
	
	init(incomingReq, subKey, seqNum);
    }

    /**
     * Constructs a KRB-AP-REQ from the bytes received from a service.
     * @param //TBD
     * @throws //TBD
     */
    public KrbApRep(byte[] message, Credentials tgtCreds, 
		    KrbApReq outgoingReq) throws KrbException, IOException {
	this(message, tgtCreds);
	authenticate(outgoingReq);
    }

    /**
     * Constructs a KRB-AP-REQ from the bytes received from a service.
     * @param //TBD
     * @throws //TBD
     */
    public KrbApRep(DerValue value, Credentials tgtCreds, 
		    KrbApReq outgoingReq) throws KrbException, IOException {
	this(value, tgtCreds);
	authenticate(outgoingReq);
    }

    
    void init(KrbApReq apReq,
	      EncryptionKey subKey,
	      SeqNumber seqNumber)
	throws KrbException, IOException {
	createMessage(
		      apReq.getCreds().key,
		      apReq.getCtime(),
		      apReq.cusec(),
		      subKey,
		      seqNumber);
	obuf = apRepMessg.asn1Encode();
    }
    
    
    /**
     * Constructs a KrbApRep object.
     * @param msg a byte array of reply message.
     * @param tgs_creds client's credential.
     * @exception KrbException 
     * @exception IOException
     */
    KrbApRep(byte[] msg, Credentials tgs_creds)
	throws KrbException, IOException {
	this(new DerValue(msg), tgs_creds);
    }

    /**
     * Constructs a KrbApRep object.
     * @param msg a byte array of reply message.
     * @param tgs_creds client's credential.
     * @exception KrbException 
     * @exception IOException
     */
    KrbApRep(DerValue encoding, Credentials tgs_creds)
	throws KrbException, IOException {
	APRep rep = null;
	try {
	    rep = new APRep(encoding);			
    	} catch (Asn1Exception e) {
	    rep = null;
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
	
	byte[] temp = rep.encPart.decrypt(tgs_creds.key);
	byte[] enc_ap_rep_part = rep.encPart.reset(temp, true); 
	
        encoding = new DerValue(enc_ap_rep_part);
	encPart = new EncAPRepPart(encoding);			   
    }
    
    void authenticate(KrbApReq apReq) throws KrbException, IOException {
	if (encPart.ctime.getSeconds() != apReq.getCtime().getSeconds() ||
	    encPart.cusec != apReq.getCtime().getMicroSeconds())
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_MUT_FAIL);
    }
    

    /**
     * Returns the optional subkey stored in
     * this message. Returns null if none is stored. 
     */
    public EncryptionKey getSubKey() {
	// Can encPart be null?
	return encPart.getSubKey();
	
    }
    
    /**
     * Returns the optional sequence number stored in the
     * this message. Returns null if none is stored.
     */
    public Integer getSeqNumber() {
	// Can encPart be null?
	return encPart.getSeqNumber();
    }

    /**
     * Returns the ASN.1 encoding that should be sent to the peer.
     */
    public byte[] getMessage() {
	return obuf;
    }

    private void createMessage(
			       EncryptionKey key,
			       KerberosTime ctime,
			       int cusec,
			       EncryptionKey subKey,
			       SeqNumber seqNumber) 
	throws Asn1Exception, IOException, 
	       KdcErrException, KrbCryptoException {

	Integer seqno = null;
	
	if (seqNumber != null)
	    seqno = new Integer(seqNumber.current());

	encPart = new EncAPRepPart(ctime,
				   cusec,
				   subKey,
				   seqno);
	
	byte[] encPartEncoding = encPart.asn1Encode();
	
	EncryptedData encEncPart = new EncryptedData(key, encPartEncoding);
	
	apRepMessg = new APRep(encEncPart);
    }
    
}
