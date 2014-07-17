/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)DesCbcCrcEType.java	1.6 03/06/24
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
import sun.security.krb5.EncryptedData;
import sun.security.krb5.KrbCryptoException;
import sun.security.krb5.internal.*;

public class DesCbcCrcEType extends DesCbcEType {

    public DesCbcCrcEType() {
    }

    public int eType() {
	return EncryptedData.ETYPE_DES_CBC_CRC;
    }

    public int minimumPadSize() {
	return 4;
    }

    public int confounderSize() {
	return 8;
    }

    public int checksumType() {
	return Checksum.CKSUMTYPE_CRC32;
    }

    public int checksumSize() {
	return 4;
    }

    /**
     * Encrypts data using DES in CBC mode with CRC32. 
     * @param data the data to be encrypted.
     * @param key  the secret key to encrypt the data. It is also used as initialization vector during cipher block chaining.
     * @return the buffer for cipher text.
     *
     * @written by Yanni Zhang, Dec 10, 1999
     */
    public byte[] encrypt(byte[] data, byte[] key) throws KrbCryptoException {	   
	return encrypt(data, key, key);
    }	 
 	
    /**
     * Decrypts data with provided key using DES in CBC mode with CRC32. 
     * @param cipher the cipher text to be decrypted.
     * @param key  the secret key to decrypt the data.
     *
     * @written by Yanni Zhang, Dec 10, 1999
     */
    public void decrypt(byte[] cipher, byte[] key) throws KrbApErrException, KrbCryptoException{  
	decrypt(cipher, key, key);
    } 

    public byte[] calculateChecksum(byte[] data, int size) {
	return crc32.byte2crc32sum_bytes(data, size);
    }

}
