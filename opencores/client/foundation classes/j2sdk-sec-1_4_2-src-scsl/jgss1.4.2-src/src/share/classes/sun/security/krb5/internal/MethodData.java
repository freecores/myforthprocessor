/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)MethodData.java	1.8 03/06/24
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
import java.io.IOException;
import java.math.BigInteger;

/**
 * Implements the ASN.1 EncKrbPrivPart type.
 *
 * <xmp>
 *     METHOD-DATA ::=    SEQUENCE {
 *                        method-type[0]   INTEGER,
 *                        method-data[1]   OCTET STRING OPTIONAL
 *  }
 * </xmp>
 */
public class MethodData {
    private int methodType;
    private byte[] methodData = null; //optional

    public MethodData(int type, byte[] data) {
	methodType = type;
	if (data != null) {
	    methodData = (byte[])data.clone();
	}
    }
    
    /**
     * Constructs a MethodData object.
     * @param encoding a Der-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public MethodData(DerValue encoding) throws Asn1Exception, IOException {
        DerValue der;
        if (encoding.getTag() != DerValue.tag_Sequence) {
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
        der = encoding.getData().getDerValue();
        if ((der.getTag() & 0x1F) == 0x00) {
	    BigInteger bint = der.getData().getBigInteger();
	    methodType = bint.intValue();
	}		
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	if (encoding.getData().available() > 0) {
	    der = encoding.getData().getDerValue();
	    if ((der.getTag() & 0x1F) == 0x01) {
		methodData = der.getData().getOctetString();
	    }
	    else throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	if (encoding.getData().available() > 0) 
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }
    
    /**
     * Encodes an MethodData object.
     * @return the byte array of encoded MethodData object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */

    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	temp.putInteger(BigInteger.valueOf(methodType));
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	if (methodData != null) {
            temp = new DerOutputStream();
	    temp.putOctetString(methodData);
            bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	}

        temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }

}
