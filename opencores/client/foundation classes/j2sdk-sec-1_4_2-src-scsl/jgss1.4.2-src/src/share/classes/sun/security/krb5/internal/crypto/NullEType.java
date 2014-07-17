/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)NullEType.java	1.5 03/06/24
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
import sun.security.krb5.internal.*;

public class NullEType extends EType {

    public NullEType() {
    }

    public int eType() {
	return EncryptedData.ETYPE_NULL;
    }

    public int minimumPadSize() {
	return 0;
    }

    public int confounderSize() {
	return 0;
    }

    public int checksumType() {
	return Checksum.CKSUMTYPE_NULL;
    }

    public int checksumSize() {
	return 0;
    }

    public int blockSize() {
	return 1;
    }

    public int keyType() {
	return Krb5.KEYTYPE_NULL;
    }

    public int keySize() {
	return 0;
    }

    public byte[] encrypt(byte[] data, byte[] key) {
	byte[] cipher = new byte[data.length];
	System.arraycopy(data, 0, cipher, 0, data.length);
	return cipher;
    }

    public byte[] encrypt(byte[] data, byte[] key, byte[] ivec) {
	byte[] cipher = new byte[data.length];
	System.arraycopy(data, 0, cipher, 0, data.length);
	return cipher;
    }

    public void decrypt(byte[] cipher, byte[] key)
	throws KrbApErrException {
    }

    public void decrypt(byte[] cipher, byte[] key, byte[] ivec)
	throws KrbApErrException {
    }

    public byte[] calculateChecksum(byte[] data, int size) {
	return null;
    }

}

