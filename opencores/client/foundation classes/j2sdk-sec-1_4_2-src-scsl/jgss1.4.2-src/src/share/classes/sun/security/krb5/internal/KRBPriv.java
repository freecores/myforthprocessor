/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KRBPriv.java	1.7 03/06/24
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
import sun.security.util.*;
import java.io.IOException;
import java.math.BigInteger;
/**
 * Implements the ASN.1 KRB-PRIV type.
 *
 * <xmp>
 * KRB-PRIV ::=         [APPLICATION 21] SEQUENCE {
 *               pvno[0]                   INTEGER,
 *               msg-type[1]               INTEGER,
 *               enc-part[3]               EncryptedData
 *  }
 * </xmp>
 *
 * <p>
 * This definition reflects the Network Working Group RFC 1510
 * specifications dated September 1993 and available at
 * <a href="http://www.ietf.org/rfc/rfc1510.txt">http://www.ietf.org/rfc/rfc1510.txt</a>.
 */

public class KRBPriv {
    public int pvno;
    public int msgType;
    public EncryptedData encPart;

    public KRBPriv(EncryptedData new_encPart) {
	pvno = Krb5.PVNO;
	msgType = Krb5.KRB_PRIV;
	encPart = new_encPart;
    }

    public KRBPriv(byte[] data) throws Asn1Exception,
    KrbApErrException, IOException {
	init(new DerValue(data));
    }

    public KRBPriv(DerValue encoding) throws Asn1Exception,
    KrbApErrException, IOException {
	init(encoding);
    }


    /**
     * Initializes an KRBPriv object.
     * @param encoding a single DER-encoded value.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     * @exception KrbApErrException if the value read from the DER-encoded data 
     *  stream does not match the pre-defined value.
     */
    private void init(DerValue encoding) throws Asn1Exception,
    KrbApErrException, IOException {
        DerValue der, subDer;
        if (((encoding.getTag() & (byte)0x1F) != (byte)0x15)
	    || (encoding.isApplication() != true)
	    || (encoding.isConstructed() != true))
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
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
            if (msgType != Krb5.KRB_PRIV)
                throw new KrbApErrException(Krb5.KRB_AP_ERR_MSG_TYPE);
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	encPart = EncryptedData.parse(der.getData(), (byte)0x03, false);
	if (der.getData().available() >0) 
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
    }

    /**
     * Encodes an KRBPriv object.
     * @return byte array of encoded EncAPRepPart object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream temp, bytes;
        temp = new DerOutputStream();
        temp.putInteger(BigInteger.valueOf(pvno));
        bytes = new DerOutputStream();
        bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x00), temp);
	temp = new DerOutputStream();		
	temp.putInteger(BigInteger.valueOf(msgType));
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x01), temp);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, (byte)0x03), encPart.asn1Encode());
	temp = new DerOutputStream();
        temp.write(DerValue.tag_Sequence, bytes);
        bytes = new DerOutputStream();
        bytes.write(DerValue.createTag(DerValue.TAG_APPLICATION, true, (byte)0x15), temp);
	return bytes.toByteArray();
    }

}
