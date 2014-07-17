/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)PAEncTSEnc.java	1.7 03/06/24
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
 * Implements the ASN.1 PAEncTSEnc type.
 *
 * <xmp>
 *  PA-ENC-TS-ENC   ::= SEQUENCE {
 *          	patimestamp[0]  KerberosTime, -- client's time
 *          	pausec[1]       INTEGER OPTIONAL
 *  }
 * </xmp>
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */
public class PAEncTSEnc {
    public KerberosTime pATimeStamp;
    public Integer pAUSec; //optional

    public PAEncTSEnc(
		      KerberosTime new_pATimeStamp,
		      Integer new_pAUSec
			  ) {
	pATimeStamp = new_pATimeStamp;
	pAUSec = new_pAUSec;
    }

    public PAEncTSEnc() {
	KerberosTime now = new KerberosTime(KerberosTime.NOW);
	pATimeStamp = now;
	pAUSec = new Integer(now.getMicroSeconds());
    }

    /**
     * Constructs a PAEncTSEnc object.
     * @param encoding a Der-encoded data.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public PAEncTSEnc(DerValue encoding) throws Asn1Exception, IOException {
        DerValue der;
        if (encoding.getTag() != DerValue.tag_Sequence) {
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	pATimeStamp = KerberosTime.parse(encoding.getData(), (byte)0x00, false);
	if (encoding.getData().available() > 0) {
	    der = encoding.getData().getDerValue();
	    if ((der.getTag() & 0x1F) == 0x01) {
		pAUSec = new Integer(der.getData().getBigInteger().intValue());
	    }
	    else throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
        if (encoding.getData().available() > 0)
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }


    /**
     * Encodes a PAEncTSEnc object.
     * @return the byte array of encoded PAEncTSEnc object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), pATimeStamp.asn1Encode());
	if (pAUSec != null) {
	    temp = new DerOutputStream();
	    temp.putInteger(BigInteger.valueOf(pAUSec.intValue()));
	    bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	}
        temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }
}
