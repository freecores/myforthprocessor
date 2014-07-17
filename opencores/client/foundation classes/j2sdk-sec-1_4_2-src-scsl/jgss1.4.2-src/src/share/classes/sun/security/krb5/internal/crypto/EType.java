/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)EType.java	1.8 03/06/24
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

import sun.security.krb5.internal.*;
import sun.security.krb5.Config;
import sun.security.krb5.EncryptedData;
import sun.security.krb5.KrbException;
import sun.security.krb5.Asn1Exception;
import sun.security.krb5.KrbCryptoException;

//only needed if dataSize() implementation changes back to spec;
//see dataSize() below

public abstract class EType {
    
    static ClassLoader sysClassLoader = null;
    private static boolean DEBUG = Krb5.DEBUG;

    public static EType getInstance  (int eTypeConst)
	throws KdcErrException {
	EType eType = null;
	String eTypeName = null;
	switch (eTypeConst) {
	case EncryptedData.ETYPE_NULL:
	    eType = new NullEType();
	    eTypeName = "sun.security.krb5.internal.crypto.NullEType";
	    break;
	case EncryptedData.ETYPE_DES_CBC_CRC:
	    eType = new DesCbcCrcEType();
	    eTypeName = "sun.security.krb5.internal.crypto.DesCbcCrcEType";
	    break;
	case EncryptedData.ETYPE_DES_CBC_MD5:
	    eType = new DesCbcMd5EType();
	    eTypeName = "sun.security.krb5.internal.crypto.DesCbcMd5EType";
	    break;
	default:
	    throw new KdcErrException(Krb5.KDC_ERR_ETYPE_NOSUPP);
	}
	if (DEBUG) {
	    System.out.println(">>> EType: " + eTypeName);
	}
	return eType;
    }

    public static EType getInstance() throws KdcErrException {
	return getInstance(getDefaultEType());
   	
    }
    public abstract int eType();

    public abstract int minimumPadSize();

    public abstract int confounderSize();

    public abstract int checksumType();

    public abstract int checksumSize();

    public abstract int blockSize();

    public abstract int keyType();

    public abstract int keySize();

    public abstract byte[] encrypt(byte[] data, byte[] key) throws KrbCryptoException;

    public abstract byte[] encrypt(byte[] data, byte[] key, byte[] ivec) throws KrbCryptoException;

    public abstract void decrypt(byte[] cipher, byte[] key)
	throws KrbApErrException, KrbCryptoException;

    public abstract void decrypt(byte[] cipher, byte[] key, byte[] ivec)
	throws KrbApErrException, KrbCryptoException;

    public abstract byte[] calculateChecksum(byte[] data, int size) throws KrbCryptoException;
    //for debugging purpose  public abstract byte[] old_calculateChecksum(byte[] data, int size);
    public int dataSize(byte[] data)
    //throws Asn1Exception
    {
	//EncodeRef ref = new EncodeRef(data, startOfData());
	//return ref.end - startOfData();
	//should be the above according to spec, but in fact
	//implementations include the pad bytes in the data size
	return data.length - startOfData();
    }

    public int padSize(byte[] data) {
	return data.length - confounderSize() - checksumSize() -
	    dataSize(data);
    }

    public int startOfChecksum() {
	return confounderSize();
    }

    public int startOfData() {
	return confounderSize() + checksumSize();
    }

    public int startOfPad(byte[] data) {
	return confounderSize() + checksumSize() + dataSize(data);
    }

    public byte[] decryptedData(byte[] data) {
	int tempSize = dataSize(data);
	byte[] result = new byte[tempSize];
	System.arraycopy(data, startOfData(), result, 0, tempSize);
	return result;
    }

    public void copyChecksumField(byte[] data, byte[] cksum) {
	for (int i = 0; i < checksumSize();  i++)
	    data[startOfChecksum() + i] = cksum[i];
    }

    public byte[] checksumField(byte[] data) {
	byte[] result = new byte[checksumSize()];
	for (int i = 0; i < checksumSize(); i++)
	result[i] = data[startOfChecksum() + i];
	return result;
    }

    public void resetChecksumField(byte[] data) {
	for (int i = startOfChecksum(); i < startOfChecksum() +
		 checksumSize();  i++)
	    data[i] = 0;
    }

    public void setChecksum(byte[] data, int size) throws KrbCryptoException{
	resetChecksumField(data);
	byte[] cksum = calculateChecksum(data, size);
	copyChecksumField(data, cksum);
    }

    public byte[] generateChecksum(byte[] data) throws KrbCryptoException{
	byte[] cksum1 = checksumField(data);
	resetChecksumField(data);
	byte[] cksum2 = calculateChecksum(data, data.length);
	copyChecksumField(data, cksum1);
	return cksum2;
    }

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

    public boolean isChecksumValid(byte[] data) throws KrbCryptoException {
	byte[] cksum1 = checksumField(data);
	byte[] cksum2 = generateChecksum(data);
	return isChecksumEqual(cksum1, cksum2);
    }

    private static int getDefaultEType() {
	int type;
	try {
	    Config c = Config.getInstance();
	    int eTypes[] = c.defaultEtype("default_tkt_enctypes");
	    type = eTypes[0];
	}
	catch (KrbException e) {
	    type = EncryptedData.ETYPE_DES_CBC_CRC;
	}
	return type;
    }
}
