/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)DesCbcEType.java	1.8 03/06/24
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

import sun.security.krb5.KrbCryptoException;
import sun.security.krb5.internal.*;

abstract class DesCbcEType extends EType {

    public int blockSize() {
	return 8;
    }

    public int keyType() {
	return Krb5.KEYTYPE_DES;
    }

    public int keySize() {
	return 8;
    }

    /**
     * Encrypts the data using DES in CBC mode.
     * @param data the buffer for plain text.
     * @param key the key to encrypt the data.
     * @return the buffer for encrypted data. 
     *
     * @written by Yanni Zhang, Dec 6 99.
     */

    public byte[] encrypt(byte[] data, byte[] key) throws KrbCryptoException {
	byte[] ivec = new byte[keySize()];
	return encrypt(data, key, ivec);
    }

    /**
     * Encrypts the data using DES in CBC mode.
     * @param data the buffer for plain text.
     * @param key the key to encrypt the data.
     * @param ivec initialization vector.
     * @return buffer for encrypted data.
     *
     * @modified by Yanni Zhang, Feb 24 00.	
     */
    public byte[] encrypt(byte[] data, byte[] key, byte[] ivec) throws KrbCryptoException {

	/*
	 * To meet export control requirements, double check that the
	 * key being used is no longer than 64 bits. 
	 *
	 * Note that from a protocol point of view, an
	 * algorithm that is not DES will be rejected before this
	 * point. Also, a  DES key that is not 64 bits will be
	 * rejected by a good implementations of JCE.
	 */
	if (key.length > 8)
	throw new KrbCryptoException("Invalid DES Key!");

	int new_size = data.length + confounderSize() +	checksumSize();
        byte[] new_data;
	byte pad;
	/*Data padding: using Kerberos 5 GSS-API mechanism (1.2.2.3), Jun 1996.
	 *Before encryption, plaintext data is padded to the next higest multiple of blocksize.
	 *by appending between 1 and 8 bytes, the value of each such byte being the total number
	 *of pad bytes. For example, if new_size = 10, blockSize is 8, we should pad 2 bytes,
	 *and the value of each byte is 2.
	 *If plaintext data is a multiple of blocksize, we pad a 8 bytes of 8.
	 */
	if (new_size % blockSize() == 0) {
	    new_data = new byte[new_size + blockSize()];
	    pad = (byte)8;
	}
	else {
	    new_data = new byte[new_size + blockSize() - new_size % blockSize()];
	    pad = (byte)(blockSize() - new_size % blockSize());
	}
	for (int i = new_size; i < new_data.length; i++) {
	    new_data[i] = pad;
	}	 
	byte[] conf = Confounder.bytes(confounderSize());
	System.arraycopy(conf, 0, new_data, 0, confounderSize());
	System.arraycopy(data, 0, new_data, startOfData(), data.length);
	byte[] cksum = calculateChecksum(new_data, new_data.length);
	System.arraycopy(cksum, 0, new_data, startOfChecksum(),
			 checksumSize());
	byte[] cipher = new byte[new_data.length];
	Des.cbc_encrypt(new_data, cipher, key, ivec, true);
	return cipher;
    }

    /**
     * Decrypts the data using DES in CBC mode.
     * @param cipher the input buffer.
     * @param key the key to decrypt the data.
     * 
     * @written by Yanni Zhang, Dec 6 99.
     */
    public void decrypt(byte[] cipher, byte[] key)
	throws KrbApErrException, KrbCryptoException{
	byte[] ivec = new byte[keySize()];
	decrypt(cipher, key, ivec);
    }	

    /**
     * Decrypts the data using DES in CBC mode.
     * @param cipher the input buffer.
     * @param key the key to decrypt the data.
     * @param ivec initialization vector.
     * 
     * @modified by Yanni Zhang, Dec 6 99.
     */
    public void decrypt(byte[] cipher, byte[] key, byte[] ivec)
	throws KrbApErrException, KrbCryptoException {

	/*
	 * To meet export control requirements, double check that the
	 * key being used is no longer than 64 bits. 
	 *
	 * Note that from a protocol point of view, an
	 * algorithm that is not DES will be rejected before this
	 * point. Also, a DES key that is not 64 bits will be
	 * rejected by a good JCE provider.
	 */
	if (key.length > 8)
	    throw new KrbCryptoException("Invalid DES Key!");

	byte[] data = new byte[cipher.length];
	Des.cbc_encrypt(cipher, data, key, ivec, false);
	System.arraycopy(data, 0, cipher, 0, cipher.length);
	if (!isChecksumValid(data))
	    throw new KrbApErrException(Krb5.KRB_AP_ERR_BAD_INTEGRITY);
    }	
	
}
