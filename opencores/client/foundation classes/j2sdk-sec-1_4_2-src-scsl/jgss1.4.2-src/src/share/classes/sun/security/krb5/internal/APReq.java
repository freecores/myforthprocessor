/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)APReq.java	1.7 03/06/24
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
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 AP-REQ type.
 *
 * <xmp>
 * AP-REQ ::= [APPLICATION 14] SEQUENCE {
 *				pvno[0] INTEGER,
 * 				msg-type[1] INTEGER,
 *				ap-options[2] APOptions,
 *				ticket[3] Ticket,
 *				authenticator[4] EncryptedData
 * }
 * </xmp>
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
 
public class APReq {
	public int pvno;
	public int msgType;
	public APOptions apOptions;
	public Ticket ticket;
	public EncryptedData authenticator;

	public APReq(
		APOptions new_apOptions,
		Ticket new_ticket,
		EncryptedData new_authenticator
	) {
		pvno = Krb5.PVNO;
		msgType = Krb5.KRB_AP_REQ;
		apOptions = new_apOptions;
		ticket = new_ticket;
		authenticator = new_authenticator;
	}

	public APReq(byte[] data) throws Asn1Exception,IOException, KrbApErrException, RealmException {
        init(new DerValue(data));
	}

    public APReq(DerValue encoding) throws Asn1Exception, IOException, KrbApErrException, RealmException {
		init(encoding);
	}

	/**
	 * Initializes an APReq object.
	 * @param encoding a single DER-encoded value.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 * @exception KrbApErrException if the value read from the DER-encoded data stream does not match the pre-defined value.
	 * @exception RealmException if an error occurs while parsing a Realm object.
	 */
	private void init(DerValue encoding) throws Asn1Exception,
	   IOException, KrbApErrException, RealmException { 
 		DerValue der, subDer;
        if (((encoding.getTag() & (byte)0x1F) != Krb5.KRB_AP_REQ)
			|| (encoding.isApplication() != true)
			|| (encoding.isConstructed() != true))
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		der = encoding.getData().getDerValue();
		if (der.getTag() != DerValue.tag_Sequence)
		   throw new Asn1Exception(Krb5.ASN1_BAD_ID);		
		subDer = der.getData().getDerValue();
        if ((subDer.getTag() & (byte)0x1F) != (byte)0x00) 
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);			
        pvno = subDer.getData().getBigInteger().intValue();
        if (pvno != Krb5.PVNO)
				throw new KrbApErrException(Krb5.KRB_AP_ERR_BADVERSION);
		subDer = der.getData().getDerValue();
		if ((subDer.getTag() & (byte)0x1F) != (byte)0x01) 
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);			  	
		msgType	= subDer.getData().getBigInteger().intValue();
		if (msgType != Krb5.KRB_AP_REQ)
			 throw new KrbApErrException(Krb5.KRB_AP_ERR_MSG_TYPE);
		apOptions = APOptions.parse(der.getData(), (byte)0x02, false);
		ticket = Ticket.parse(der.getData(), (byte)0x03, false);
		authenticator = EncryptedData.parse(der.getData(), (byte)0x04, false);
		if (der.getData().available() > 0) 
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}

	/**
	 * Encodes an APReq object.
	 * @return byte array of encoded APReq object.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 */
	public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	    DerOutputStream temp = new DerOutputStream();
		temp.putInteger(BigInteger.valueOf(pvno));
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
		temp = new DerOutputStream();
		temp.putInteger(BigInteger.valueOf(msgType));
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), apOptions.asn1Encode());
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), ticket.asn1Encode());
		bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x04), authenticator.asn1Encode());
	    temp = new DerOutputStream();
		temp.write(DerValue.tag_Sequence, bytes);
		DerOutputStream apreq = new DerOutputStream();
		apreq.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x0E), temp);
		return apreq.toByteArray();
		
	}

}
