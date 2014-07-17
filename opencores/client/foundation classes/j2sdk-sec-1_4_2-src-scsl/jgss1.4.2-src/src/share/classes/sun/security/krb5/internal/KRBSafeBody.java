/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KRBSafeBody.java	1.8 03/06/24
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

import sun.security.util.*;
import sun.security.krb5.Asn1Exception;
import java.util.Vector;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 KRBSafeBody type.
 *
 * <xmp>
 *   KRB-SAFE-BODY ::=   SEQUENCE {
 *              user-data[0]          OCTET STRING,
 *              timestamp[1]          KerberosTime OPTIONAL,
 *              usec[2]               INTEGER OPTIONAL,
 *              seq-number[3]         INTEGER OPTIONAL,
 *              s-address[4]          HostAddress,
 *              r-address[5]          HostAddress OPTIONAL
 *  }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */

public class KRBSafeBody {
    public byte[] userData = null;
    public KerberosTime timestamp; //optional
    public Integer usec; //optional
    public Integer seqNumber; //optional
    public HostAddress sAddress;
    public HostAddress rAddress; //optional

    public KRBSafeBody(
		       byte[] new_userData,
		       KerberosTime new_timestamp,
		       Integer new_usec,
		       Integer new_seqNumber,
		       HostAddress new_sAddress,
		       HostAddress new_rAddress
			   ) {
	if (new_userData != null) {
	    userData = (byte[])new_userData.clone();
	}
	timestamp = new_timestamp;
	usec = new_usec;
	seqNumber = new_seqNumber;
	sAddress = new_sAddress;
	rAddress = new_rAddress;
    }


    /**
     * Constructs a KRBSafeBody object.
     * @param encoding a Der-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public KRBSafeBody(DerValue encoding) throws Asn1Exception, IOException {
        DerValue der;
        if (encoding.getTag() != DerValue.tag_Sequence) {
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
        der = encoding.getData().getDerValue();
        if ((der.getTag() & 0x1F) == 0x00) {
	    userData = der.getData().getOctetString();
	}		
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	timestamp = KerberosTime.parse(encoding.getData(), (byte)0x01, true);
	if ((encoding.getData().peekByte() & 0x1F) == 0x02) {
	    der = encoding.getData().getDerValue();
	    usec = new Integer(der.getData().getBigInteger().intValue());
	}
	if ((encoding.getData().peekByte() & 0x1F) == 0x03) {
	    der = encoding.getData().getDerValue();
	    seqNumber = new Integer(der.getData().getBigInteger().intValue());
	}
	sAddress = HostAddress.parse(encoding.getData(), (byte)0x04, false);
	if (encoding.getData().available() > 0) 
	    rAddress = HostAddress.parse(encoding.getData(), (byte)0x05, true);
        if (encoding.getData().available() > 0) 
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }

    /**
     * Encodes an KRBSafeBody object.
     * @return the byte array of encoded KRBSafeBody object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	temp.putOctetString(userData);
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	if (timestamp != null)
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), timestamp.asn1Encode());
	if (usec != null) {
	    temp = new DerOutputStream();
	    temp.putInteger(BigInteger.valueOf(usec.intValue()));
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x02), temp);			
	}
	if (seqNumber != null) {
            temp = new DerOutputStream();
	    temp.putInteger(seqNumber);
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), temp);			
	}
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x04), sAddress.asn1Encode());
	if (rAddress != null)
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }

    /**
     * Parse (unmarshal) a KRBSafeBody from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception on error.
     * @param data the Der input stream value, which contains one or more marshaled value.
     * @param explicitTag tag number.
     * @param optional indicates if this data field is optional
     * @return an instance of KRBSafeBody.
     *
     */
    public static KRBSafeBody parse(DerInputStream data, byte explicitTag, boolean optional) throws Asn1Exception, IOException {
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F) != explicitTag))
	    return null;		
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	else {
	    DerValue subDer = der.getData().getDerValue();
	    return new KRBSafeBody(subDer);
	}
    }



}
