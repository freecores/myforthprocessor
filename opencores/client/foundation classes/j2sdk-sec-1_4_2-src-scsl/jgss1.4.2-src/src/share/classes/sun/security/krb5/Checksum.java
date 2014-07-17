/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Checksum.java	1.8 03/06/24
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

import sun.security.util.*;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.crypto.*;
import java.io.IOException;
import java.math.BigInteger;

/**
 * This class encapsulates the concept of a Kerberos checksum.
 */
public class Checksum {

    private int cksumType;
    private byte[] checksum;

    // ----------------------------------------------+-------------+-----------
    //                      Checksum type            |sumtype      |checksum
    //                                               |value        | size
    // ----------------------------------------------+-------------+-----------
    public static final int CKSUMTYPE_NULL          = 0;               // 0
    public static final int CKSUMTYPE_CRC32         = 1;               // 4
    public static final int CKSUMTYPE_RSA_MD4       = 2;               // 16
    public static final int CKSUMTYPE_RSA_MD4_DES   = 3;               // 24
    public static final int CKSUMTYPE_DES_MAC       = 4;               // 16
    public static final int CKSUMTYPE_DES_MAC_K     = 5;               // 8
    public static final int CKSUMTYPE_RSA_MD4_DES_K = 6;               // 16
    public static final int CKSUMTYPE_RSA_MD5       = 7;               // 16
    public static final int CKSUMTYPE_RSA_MD5_DES   = 8;               // 24
    
    public static int CKSUMTYPE_DEFAULT;
    public static int SAFECKSUMTYPE_DEFAULT;
    
    private static boolean DEBUG = Krb5.DEBUG; 
    static {
	String temp = null;
	Config cfg = null;
	try {
	    cfg = Config.getInstance();
	    temp = cfg.getDefault("default_checksum", "libdefaults");
	    if (temp != null)
                {
                    CKSUMTYPE_DEFAULT = cfg.getType(temp);
                } else {
		    /*
		     * If the default checksum is not
		     * specified in the configuration we 
		     * set it to RSA_MD5. We follow the MIT and
		     * SEAM implementation even though RFC1510
		     * specifies RSA_MD5_DES as "must" support
		     * and not RSA_MD5. W2K does not seem
		     * to support RSA_MD5_DES. 
		     */
                    CKSUMTYPE_DEFAULT = CKSUMTYPE_RSA_MD5;
                }
	} catch (Exception exc) {
	    if (DEBUG) {
		System.out.println("Exception in getting default checksum "+
				   "value from the configuration " + 
				   "Setting default checksum to be RSA-MD5");
		exc.printStackTrace();
	    }
	    CKSUMTYPE_DEFAULT = CKSUMTYPE_RSA_MD5;
	}
	
	
	try {
	    temp = cfg.getDefault("safe_checksum_type", "libdefaults");
	    if (temp != null)
                {
                    SAFECKSUMTYPE_DEFAULT = cfg.getType(temp);
                } else {
                    SAFECKSUMTYPE_DEFAULT = CKSUMTYPE_RSA_MD5_DES;
                }
	} catch (Exception exc) {
	    if (DEBUG) {
		System.out.println("Exception in getting safe default " +
				   "checksum value " +
				   "from the configuration Setting  " + 
				   "safe default checksum to be RSA-MD5");
		exc.printStackTrace();
	    }
	    SAFECKSUMTYPE_DEFAULT = CKSUMTYPE_RSA_MD5_DES;
	}
    }
    
    /**
     * Constructs a new Checksum using the raw data and type.
     * @data the byte array of checksum.
     * @new_cksumType the type of checksum.
     * 
     */
    public Checksum(byte[] data, int new_cksumType) {
	cksumType = new_cksumType;
	checksum = data;
    }
    
    /**
     * Constructs a new Checksum by calculating the checksum over the data
     * using specified checksum type.
     * @new_cksumType the type of checksum.
     * @data the data that needs to be performed a checksum calculation on.
     */
    public Checksum(int new_cksumType, byte[] data)	   
	throws KdcErrException, KrbCryptoException {
	
	cksumType = new_cksumType;
	CksumType cksumEngine = CksumType.getInstance(cksumType);
	if (!cksumEngine.isSafe()) {
	    checksum = cksumEngine.calculateChecksum(data, data.length);
	} else {
	    throw new KdcErrException(Krb5.KRB_AP_ERR_INAPP_CKSUM);
	}
    }
    
    /**
     * Constructs a new Checksum by calculating the keyed checksum
     * over the data using specified checksum type.
     * @new_cksumType the type of checksum.
     * @data the data that needs to be performed a checksum calculation on.
     */
    public Checksum(int new_cksumType, byte[] data, EncryptionKey key)
	throws KdcErrException, KrbApErrException, KrbCryptoException {
	cksumType = new_cksumType;
	CksumType cksumEngine = CksumType.getInstance(cksumType);
	if (!cksumEngine.isSafe())
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_INAPP_CKSUM);
	checksum =
	    cksumEngine.calculateKeyedChecksum(data,
					       data.length,
					       key.getBytes());
    }
    
    /**
     * Verifies the keyed checksum over the data passed in.
     */
    public boolean verifyKeyedChecksum(byte[] data, EncryptionKey key)
	throws KdcErrException, KrbApErrException, KrbCryptoException {
	CksumType cksumEngine = CksumType.getInstance(cksumType);
	if (!cksumEngine.isSafe())
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_INAPP_CKSUM);
	return cksumEngine.verifyKeyedChecksum(data,
					       data.length,
					       key.getBytes(),
					       checksum);
    }
	
    Checksum(byte[] data) throws KdcErrException, KrbCryptoException {
	this(Checksum.CKSUMTYPE_DEFAULT, data);
    }
    
    boolean isEqual(Checksum cksum) throws KdcErrException {
	if (cksumType != cksum.cksumType)
	    return false;
	CksumType cksumEngine = CksumType.getInstance(cksumType);
	return cksumEngine.isChecksumEqual(checksum, cksum.checksum);
    }
    
    /**
     * Constructs an instance of Checksum from an ASN.1 encoded representation.
     * @param encoding a single DER-encoded value.
     * @exception Asn1Exception if an error occurs while decoding an ASN1 
     * encoded data.
     * @exception IOException if an I/O error occurs while reading encoded data.
     *
     */
    public Checksum(DerValue encoding) throws Asn1Exception, IOException { 
	DerValue der;
	if (encoding.getTag() != DerValue.tag_Sequence) {
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	der = encoding.getData().getDerValue();
	if ((der.getTag() & (byte)0x1F) == (byte)0x00) {
	    cksumType = der.getData().getBigInteger().intValue();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	der = encoding.getData().getDerValue();
	if ((der.getTag() & (byte)0x1F) == (byte)0x01) {
	    checksum = der.getData().getOctetString();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	if (encoding.getData().available() > 0) {
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
    }

    /**
     * Encodes a Checksum object.
     * <xmp>
     * Checksum ::= SEQUENCE {
     *                 cksumtype[0]   INTEGER,
     *                 checksum[1]    OCTET STRING
     * }
     * </xmp>
     *
     * <p>
     * This definition reflects the Network Working Group RFC 1510
     * specifications dated September 1993 and available at
     * <a href="http://www.ietf.org/rfc/rfc1510.txt">
     * http://www.ietf.org/rfc/rfc1510.txt</a>.
     * @return byte array of enocded Checksum.
     * @exception Asn1Exception if an error occurs while decoding an
     * ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading
     * encoded data.
     *
     */
    public byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	temp.putInteger(BigInteger.valueOf(cksumType));
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT,
				       true, (byte)0x00), temp);
	temp = new DerOutputStream();
	temp.putOctetString(checksum);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT,
				       true, (byte)0x01), temp);
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }
    
    
    /**
     * Parse (unmarshal) a checksum object from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @exception Asn1Exception if an error occurs while decoding an
     * ASN1 encoded data.
     * @exception IOException if an I/O error occurs while reading
     * encoded data.
     * @param data the Der input stream value, which contains one or more 
     * marshaled value.
     * @param explicitTag tag number.
     * @param optional indicates if this data field is optional
     * @return an instance of Checksum.
     *
     */
    public static Checksum parse(DerInputStream data, 
				 byte explicitTag, boolean optional)
	throws Asn1Exception, IOException { 

	if ((optional) &&
	    (((byte)data.peekByte() & (byte)0x1F) != explicitTag)) { 
	    return null;
	}
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	} else {
	    DerValue subDer = der.getData().getDerValue();
	    return new Checksum(subDer);
	}
    }

    /**
     * Returns the raw bytes of the checksum, not in ASN.1 encoded form.
     */
    public final byte[] getBytes() {
	return checksum;
    }

    public final int getType() {
        return cksumType;
    }
}
