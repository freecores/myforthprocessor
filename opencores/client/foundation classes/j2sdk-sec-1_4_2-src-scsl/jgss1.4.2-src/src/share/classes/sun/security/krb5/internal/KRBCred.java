/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KRBCred.java	1.8 03/06/24
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

import sun.security.krb5.EncryptedData;
import sun.security.krb5.Asn1Exception;
import sun.security.krb5.RealmException;
import sun.security.util.*;
import java.util.Vector;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 Authenticator type.
 *
 * <xmp>
 *  KRB-CRED ::= [APPLICATION 22]   SEQUENCE {
 *                   pvno[0]     INTEGER,
 *                   msg-type[1] INTEGER, -- KRB_CRED
 *                   tickets[2]  SEQUENCE OF Ticket,
 *                   enc-part[3] EncryptedData
 *  				 }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */

public class KRBCred {
	public Ticket[] tickets = null;
	public EncryptedData encPart;

	private int pvno;
	private int msgType;

	public KRBCred(Ticket[] new_tickets, EncryptedData new_encPart) throws IOException {
		pvno = Krb5.PVNO;
		msgType = Krb5.KRB_CRED;
		if (new_tickets != null) {
		    tickets = new Ticket[new_tickets.length];
		    for (int i = 0; i < new_tickets.length; i++) {
			if (new_tickets[i] == null) {
			    throw new IOException("Cannot create a KRBCred");
			} else {
			    tickets[i] = (Ticket)new_tickets[i].clone();
			}
		    }
		}
		encPart = new_encPart;
	}

	public KRBCred(byte[] data) throws Asn1Exception,
		RealmException, KrbApErrException, IOException {
		init(new DerValue(data));
	}

	public KRBCred(DerValue encoding) throws Asn1Exception,
		RealmException, KrbApErrException, IOException {
		init(encoding);
	}

	/**
	 * Initializes an KRBCred object.
	 * @param encoding a single DER-encoded value.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 * @exception KrbApErrException if the value read from the DER-encoded data 
         *  stream does not match the pre-defined value.
	 * @exception RealmException if an error occurs while parsing a Realm object.
	 */
	private void init(DerValue encoding) throws Asn1Exception,
		RealmException, KrbApErrException, IOException {
        if (((encoding.getTag() & (byte)0x1F) != (byte)0x16)
			|| (encoding.isApplication() != true)
			|| (encoding.isConstructed() != true))
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        DerValue der, subDer;
		der = encoding.getData().getDerValue();
		if (der.getTag() != DerValue.tag_Sequence)
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		subDer = der.getData().getDerValue();
	    if ((subDer.getTag() & 0x1F) == 0x00) {
			pvno = subDer.getData().getBigInteger().intValue();
			if (pvno != Krb5.PVNO) {
                throw new KrbApErrException(Krb5.KRB_AP_ERR_BADVERSION);				
			}
		} 
        else
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		subDer = der.getData().getDerValue();
		if ((subDer.getTag() & 0x1F) == 0x01) {
			msgType = subDer.getData().getBigInteger().intValue();
            if (msgType != Krb5.KRB_CRED)
                throw new KrbApErrException(Krb5.KRB_AP_ERR_MSG_TYPE);
		}
		else
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        subDer = der.getData().getDerValue();		
		if ((subDer.getTag() & 0x1F) == 0x02) {
			DerValue subsubDer = subDer.getData().getDerValue();
            if (subsubDer.getTag() != DerValue.tag_SequenceOf) {
				throw new Asn1Exception(Krb5.ASN1_BAD_ID);
			}
            Vector v = new Vector();
            while (subsubDer.getData().available() > 0) {
				v.addElement(new Ticket(subsubDer.getData().getDerValue()));
			}
            if (v.size() > 0) {
				tickets = new Ticket[v.size()];
				v.copyInto(tickets);
			}
		}
		else
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		encPart = EncryptedData.parse(der.getData(), (byte)0x03, false);

		if (der.getData().available() > 0) 
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}


	/**
	 * Encodes an KRBCred object.
	 * @return the data of encoded EncAPRepPart object.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 */
	public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream temp, bytes, out;
        temp = new DerOutputStream();
        temp.putInteger(BigInteger.valueOf(pvno));
        out = new DerOutputStream();
        out.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
		temp = new DerOutputStream();		
		temp.putInteger(BigInteger.valueOf(msgType));
		out.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
        temp = new DerOutputStream();
		for (int i = 0; i < tickets.length; i++) {
			temp.write(tickets[i].asn1Encode());
		} 
        bytes = new DerOutputStream();
        bytes.write(DerValue.tag_SequenceOf, temp);
        out.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), bytes);
		out.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), encPart.asn1Encode());
        bytes = new DerOutputStream();
        bytes.write(DerValue.tag_Sequence, out);
		out = new DerOutputStream();
		out.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x16), bytes);
		return out.toByteArray();
	}

}
