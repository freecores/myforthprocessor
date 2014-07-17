/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)EncKrbPrivPart.java	1.8 03/06/24
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
 * Implements the ASN.1 EncKrbPrivPart type.
 *
 * <xmp>
 *  EncKrbPrivPart ::= [APPLICATION 28] SEQUENCE {
 *               user-data[0]   OCTET STRING,
 *               timestamp[1]   KerberosTime OPTIONAL,
 *               usec[2]        INTEGER OPTIONAL,
 *               seq-number[3]  INTEGER OPTIONAL,
 *               s-address[4]   HostAddress, -- sender's addr
 *               r-address[5]   HostAddress OPTIONAL  --recip's addr
 *  }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */

public class EncKrbPrivPart {
	public byte[] userData = null;
	public KerberosTime timestamp; //optional
	public Integer usec; //optional
	public Integer seqNumber; //optional
	public HostAddress sAddress; //optional
	public HostAddress rAddress; //optional

	public EncKrbPrivPart(
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

	public EncKrbPrivPart(byte[] data) throws Asn1Exception, IOException {
		init(new DerValue(data));
	}

	public EncKrbPrivPart(DerValue encoding) throws Asn1Exception, IOException {
		init(encoding);
	}

	/**
	 * Initializes an EncKrbPrivPart object.
	 * @param encoding a single DER-encoded value.
	 * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
	 * @exception IOException if an I/O error occurs while reading encoded data.
	 */
	private void init(DerValue encoding) throws Asn1Exception, IOException {
        DerValue der, subDer;
		if (((encoding.getTag() & (byte)0x1F) != (byte)0x1C)
			|| (encoding.isApplication() != true)
			|| (encoding.isConstructed() != true))
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		der = encoding.getData().getDerValue();
        if (der.getTag() != DerValue.tag_Sequence)
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		subDer = der.getData().getDerValue();
		if ((subDer.getTag() & (byte)0x1F) == (byte)0x00) {
			userData = subDer.getData().getOctetString();
		}
        else
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
		timestamp = KerberosTime.parse(der.getData(), (byte)0x01, true);
		if ((der.getData().peekByte() & 0x1F) == 0x02) {
			subDer = der.getData().getDerValue();
			usec = new Integer(subDer.getData().getBigInteger().intValue());
		}  
		else usec = null;
		if ((der.getData().peekByte() & 0x1F) == 0x03 ) {
			subDer = der.getData().getDerValue();
			seqNumber = new Integer(subDer.getData().getBigInteger().intValue());
		}
		else seqNumber = null;
		sAddress = HostAddress.parse(der.getData(), (byte)0x04, false);
		if (der.getData().available() > 0) {
			rAddress = HostAddress.parse(der.getData(), (byte)0x05, true);
		}
		if (der.getData().available() > 0) 
			throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}

       /**
        * Encodes an EncKrbPrivPart object.
        * @return byte array of encoded EncKrbPrivPart object.
        * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
        * @exception IOException if an I/O error occurs while reading encoded data.
        */
	public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream temp = new DerOutputStream();
        DerOutputStream bytes = new DerOutputStream();
		
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
		if (rAddress != null) {
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x05), rAddress.asn1Encode());
		}
		temp = new DerOutputStream();
		temp.write(DerValue.tag_Sequence, bytes);
		bytes = new DerOutputStream();
		bytes.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x1C), temp);
		return bytes.toByteArray();
	}
}
