/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KDCRep.java	1.9 03/06/24
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

package sun.security.krb5.internal;

import sun.security.krb5.*;
import sun.security.util.*;
import java.util.Vector;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 KDC-REP type.
 *
 * <xmp>
 *
 *   KDC-REP ::=  SEQUENCE {
 *                pvno[0]       INTEGER,
 *                msg-type[1]   INTEGER,
 *                padata[2]     SEQUENCE OF PA-DATA OPTIONAL,
 *                crealm[3]     Realm,
 *                cname[4]      PrincipalName,
 *                ticket[5]     Ticket,
 *                enc-part[6]   EncryptedData
 *  }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class KDCRep {
	public Realm crealm;
	public PrincipalName cname;
	public Ticket ticket;
	public EncryptedData encPart;
	public EncKDCRepPart encKDCRepPart; //not part of ASN.1 encoding

	private int pvno;
	private int msgType;
	private PAData[] pAData = null; //optional
	private boolean DEBUG = Krb5.DEBUG;

	public KDCRep(
		PAData[] new_pAData,
		Realm new_crealm,
		PrincipalName new_cname,
		Ticket new_ticket,
		EncryptedData new_encPart,
		int req_type
	) throws IOException {
		pvno = Krb5.PVNO;
		msgType = req_type;
		if (new_pAData != null) {
		    pAData = new PAData[new_pAData.length];
		    for (int i = 0; i < new_pAData.length; i++) {
			if (new_pAData[i] == null) {
			    throw new IOException("Cannot create a KDCRep");
			} else {
			    pAData[i] = (PAData)new_pAData[i].clone();
			}
		    }
		}
		crealm = new_crealm;
		cname = new_cname;
		ticket = new_ticket;
		encPart = new_encPart;
	}

	public KDCRep() {
	}

	public KDCRep(byte[] data, int req_type) throws Asn1Exception, KrbApErrException, RealmException, IOException {
		init(new DerValue(data), req_type);
	}

	public KDCRep(DerValue encoding, int req_type) throws Asn1Exception,
		RealmException, KrbApErrException, IOException {
		init(encoding, req_type);
	}
    
	public void decrypt(EncryptionKey key) throws Asn1Exception,
		IOException, KrbException, RealmException {
		encKDCRepPart = new EncKDCRepPart(encPart.decrypt(key),
			msgType);
	}

	/**
	 * Initializes an KDCRep object.
	 *
	 * @param encoding a single DER-encoded value.
	 * @param req_type reply message type.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 * @exception RealmException if an error occurs while constructing a Realm object from DER-encoded data.
	 * @exception KrbApErrException if the value read from the DER-encoded data stream does not match the pre-defined value.
	 *
	 */
	protected void init(DerValue encoding, int req_type) 
	    throws Asn1Exception, RealmException, IOException,
		   KrbApErrException {
	    DerValue der, subDer;
	    if ((encoding.getTag() & 0x1F) != req_type) {
		if (DEBUG) {
		    System.out.println(">>> KDCRep: init() " +
				       "encoding tag is " +
				       encoding.getTag() + 
				       " req type is " + req_type);
		}
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);			
	    }  	 
	    der = encoding.getData().getDerValue();
	    if (der.getTag() != DerValue.tag_Sequence) { 
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    }
	    subDer = der.getData().getDerValue();
	    if ((subDer.getTag() & 0x1F) == 0x00) {
		pvno = subDer.getData().getBigInteger().intValue();
		if (pvno != Krb5.PVNO)
		    throw new KrbApErrException(Krb5.KRB_AP_ERR_BADVERSION);				
	    } else {
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    }
	    subDer = der.getData().getDerValue();
	    if ((subDer.getTag() & 0x1F) == 0x01) {
		msgType = subDer.getData().getBigInteger().intValue();
		if (msgType != req_type) {
		    throw new KrbApErrException(Krb5.KRB_AP_ERR_MSG_TYPE);
		}
	    } else {
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    }
	    if ((der.getData().peekByte() & 0x1F) == 0x02) {
		subDer = der.getData().getDerValue();
		Vector tempPAData = new Vector();
		DerValue[] padata = subDer.getData().getSequence(1);
		pAData = new PAData[padata.length];
		for (int i = 0; i < padata.length; i++) {
		    pAData[i] = new PAData(padata[i]);
		}
	    } else {
		pAData = null;
	    }
	    crealm = Realm.parse(der.getData(), (byte)0x03, false);
	    cname = PrincipalName.parse(der.getData(), (byte)0x04, false);
	    ticket = Ticket.parse(der.getData(), (byte)0x05, false);
	    encPart = EncryptedData.parse(der.getData(), (byte)0x06, false);
	    if (der.getData().available() > 0) {
		throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	    }
	}


	/**
	 * Encodes this object to a byte array.
	 * @return byte array of encoded APReq object.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 *
	 */
	public byte[] asn1Encode() throws Asn1Exception, IOException {

	    DerOutputStream bytes = new DerOutputStream();
	    DerOutputStream temp = new DerOutputStream();
	    temp.putInteger(BigInteger.valueOf(pvno));
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	    temp = new DerOutputStream();
	    temp.putInteger(BigInteger.valueOf(msgType));
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	    if (pAData != null && pAData.length > 0) {
		DerOutputStream padata_stream = new DerOutputStream();
		for (int i = 0; i < pAData.length; i++) {
		    padata_stream.write(pAData[i].asn1Encode());
		}
		temp = new DerOutputStream();
		temp.write(DerValue.tag_SequenceOf, padata_stream);
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), temp);
	    }
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), crealm.asn1Encode());
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x04), cname.asn1Encode());
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x05), ticket.asn1Encode());
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x06), encPart.asn1Encode());
	    temp = new DerOutputStream();
	    temp.write(DerValue.tag_Sequence, bytes);
	    return temp.toByteArray();
	}
}
