/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)RsaMd5CksumType.java	1.6 03/06/24
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

import sun.security.krb5.Checksum;
import sun.security.krb5.KrbCryptoException;
import sun.security.krb5.internal.*;
import java.security.MessageDigest;
import java.security.Provider;
import java.security.Security;

public final class RsaMd5CksumType extends CksumType {

    public RsaMd5CksumType() {
    }

    public int confounderSize() {
	return 0;
    }

    public int cksumType() {
	return Checksum.CKSUMTYPE_RSA_MD5;
    }

    public boolean isSafe() {
	return false;
    }

    public int cksumSize() {
	return 16;
    }

    public int keyType() {
	return Krb5.KEYTYPE_NULL;
    }

    public int keySize() {
	return 0;
    }

    /**
     * Calculates checksum using MD5.
     * @param data the data used to generate the checksum.
     * @param size length of the data.
     * @return the checksum.
     *
     * @modified by Yanni Zhang, 12/08/99. 
     */

    public byte[] calculateChecksum(byte[] data, int size) throws KrbCryptoException{
	MessageDigest md5;
	byte[] result = null;
	try {
	    md5 = MessageDigest.getInstance("MD5");
	} catch (Exception e) {
	    throw new KrbCryptoException("JCE provider may not be installed. " + e.getMessage());
	}
	try {
	    md5.update(data);
	    result = md5.digest();
	} catch (Exception e) {
	    throw new KrbCryptoException(e.getMessage());
	}
	return result;
    }

    public byte[] calculateKeyedChecksum(byte[] data, int size,
					 byte[] key) throws KrbCryptoException {
					     return null;
					 }

    public boolean verifyKeyedChecksum(byte[] data, int size,
				       byte[] key, byte[] checksum) throws KrbCryptoException {
	return false;
    }

}
