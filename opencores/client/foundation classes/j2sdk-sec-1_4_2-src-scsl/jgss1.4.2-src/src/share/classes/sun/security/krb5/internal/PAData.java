/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)PAData.java	1.8 03/06/24
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

/**
 * Implements the ASN.1 PA-DATA type.
 *
 * <xmp>
 * PA-DATA ::= SEQUENCE {
 *	padata-type[1] INTEGER,
 *	padata-value[2] OCTET STRING,
 * }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class PAData {
    private int pADataType;
    private byte[] pADataValue = null;
    private static final byte TAG_PATYPE = 1;
    private	static final byte TAG_PAVALUE = 2;

    private PAData() {
    }

    public PAData(int new_pADataType, byte[] new_pADataValue) {
	pADataType = new_pADataType;
	if (new_pADataValue != null) {
	    pADataValue = (byte[])new_pADataValue.clone();
	}
    }

    public Object clone() {
	PAData new_pAData = new PAData();
	new_pAData.pADataType = pADataType;
	if (pADataValue != null) {
	    new_pAData.pADataValue = new byte[pADataValue.length];
	    System.arraycopy(pADataValue, 0, new_pAData.pADataValue,
			     0, pADataValue.length);
	}
	return new_pAData;
    }

    /**
     * Constructs a PAData object.
     * @param encoding a Der-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public PAData(DerValue encoding) throws Asn1Exception, IOException { 
	DerValue der = null; 
	if (encoding.getTag() != DerValue.tag_Sequence) {
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	der = encoding.getData().getDerValue();
	if ((der.getTag() & 0x1F) == 0x01) {
	    this.pADataType = der.getData().getBigInteger().intValue();
	}
	else
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	der = encoding.getData().getDerValue();
	if ((der.getTag() & 0x1F) == 0x02) {
	    this.pADataValue = der.getData().getOctetString();
	}
	if (encoding.getData().available() > 0) 
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }

    /**
     * Encodes this object to an OutputStream.
     *
     * @return byte array of the encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @exception Asn1Exception on encoding errors.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {

	DerOutputStream bytes = new DerOutputStream();		 
	DerOutputStream temp = new DerOutputStream();
		
	temp.putInteger(pADataType);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, TAG_PATYPE), temp);
	temp = new DerOutputStream();
	temp.putOctetString(pADataValue);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, TAG_PAVALUE), temp);
	
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }

}
