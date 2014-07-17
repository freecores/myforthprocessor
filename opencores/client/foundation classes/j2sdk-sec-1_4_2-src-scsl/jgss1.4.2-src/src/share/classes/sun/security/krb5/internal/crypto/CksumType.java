/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)CksumType.java	1.8 03/06/24
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

package sun.security.krb5.internal.crypto;

import sun.security.krb5.Config;
import sun.security.krb5.Checksum;
import sun.security.krb5.EncryptedData;
import sun.security.krb5.KrbException;
import sun.security.krb5.KrbCryptoException;
import sun.security.krb5.internal.*;

public abstract class CksumType {
    
    static ClassLoader sysClassLoader = null;
    private static boolean DEBUG = Krb5.DEBUG;

    public static CksumType getInstance(int cksumTypeConst)
	throws KdcErrException {
	CksumType cksumType = null;
	String cksumTypeName = null;
	switch (cksumTypeConst) {
	case Checksum.CKSUMTYPE_CRC32:
	    cksumType = new Crc32CksumType();
	    cksumTypeName = "sun.security.krb5.internal.crypto.Crc32CksumType";
	    break;
	case Checksum.CKSUMTYPE_DES_MAC:
	    cksumType = new DesMacCksumType();
	    cksumTypeName = "sun.security.krb5.internal.crypto.DesMacCksumType";
	    break;
	case Checksum.CKSUMTYPE_DES_MAC_K:
	    cksumType = new DesMacKCksumType();
	    cksumTypeName = "sun.security.krb5.internal.crypto.DesMacKCksumType";
	    break;
	case Checksum.CKSUMTYPE_RSA_MD5:
	    cksumType = new RsaMd5CksumType();
	    cksumTypeName = "sun.security.krb5.internal.crypto.RsaMd5CksumType";
	    break;
	case Checksum.CKSUMTYPE_RSA_MD5_DES:
	    cksumType = new RsaMd5DesCksumType();
	    cksumTypeName = "sun.security.krb5.internal.crypto.RsaMd5DesCksumType";
	    break;
	    // currently we don't support MD4.
	case Checksum.CKSUMTYPE_RSA_MD4_DES_K:
	    // cksumType = new RsaMd4DesKCksumType();
	    // cksumTypeName = "sun.security.krb5.internal.crypto.RsaMd4DesKCksumType";
	case Checksum.CKSUMTYPE_RSA_MD4:
	    // cksumType = new RsaMd4CksumType(); 
	    // linux box support rsamd4, how to solve conflict?
	    // cksumTypeName = "sun.security.krb5.internal.crypto.RsaMd4CksumType"; 
	case Checksum.CKSUMTYPE_RSA_MD4_DES:
	    // cksumType = new RsaMd4DesCksumType();
	    //cksumTypeName = "sun.security.krb5.internal.crypto.RsaMd4DesCksumType";

	default:
	    throw new KdcErrException(Krb5.KDC_ERR_SUMTYPE_NOSUPP);
	}
	if (DEBUG) {
	    System.out.println(">>> CksumType: " + cksumTypeName);
	}
	return cksumType;
    }


    /**
     * Returns default checksum type.
     */
    public static CksumType getInstance() throws KdcErrException {
	//this method provided for Kerberos applications.
	int cksumType = Checksum.CKSUMTYPE_RSA_MD5; //default
	try {
	    Config c = Config.getInstance();
	    if ((cksumType = (c.getType(c.getDefault("ap_req_checksum_type", "libdefaults")))) == - 1) {
		if ((cksumType = c.getType(c.getDefault("checksum_type", "libdefaults"))) == -1) {
		    cksumType = Checksum.CKSUMTYPE_RSA_MD5; //default
		}
	    }
	} catch (KrbException e) {
	}
	return getInstance(cksumType);
    }

    public abstract int confounderSize();

    public abstract int cksumType();

    public abstract boolean isSafe();

    public abstract int cksumSize();

    public abstract int keyType();

    public abstract int keySize();

    public abstract byte[] calculateChecksum(byte[] data, int size) throws KrbCryptoException;

    public abstract byte[] calculateKeyedChecksum(byte[] data, int size,
						  byte[] key) throws KrbCryptoException;

    public abstract boolean verifyKeyedChecksum(byte[] data, int size,
						byte[] key, byte[] checksum) throws KrbCryptoException;

    public boolean isChecksumEqual(byte[] cksum1, byte[] cksum2) {
	if (cksum1 == cksum2)
	    return true;
	if ((cksum1 == null && cksum2 != null) ||
	    (cksum1 != null && cksum2 == null))
	    return false;
	if (cksum1.length != cksum2.length)
	    return false;
	for (int i = 0; i < cksum1.length; i++)
	    if (cksum1[i] != cksum2[i])
		return false;
	return true;
    }

}
