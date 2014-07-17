/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Confounder.java	1.7 03/06/24
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

import java.util.Random;
import java.security.MessageDigest;
import java.security.Provider;
import java.security.Security;
import sun.security.krb5.KrbCryptoException;

public final class Confounder {
    private static long count = 0;
    private static long index = 0;
    private static long lastKeyTime = 0;
    private static Random rand = new Random();	
    private static byte[] buffer = new byte[16];

    private static synchronized void reinitialize() throws KrbCryptoException{
	long reinitializeTime = System.currentTimeMillis();
	rand.setSeed(lastKeyTime + reinitializeTime + count + index);
	count++;
	for (int i = 0; i < buffer.length; i++)
	    buffer[i] = (byte)rand.nextInt();
	MessageDigest md5 = null;
	byte[] result = new byte[16];
	try {
	    md5 = MessageDigest.getInstance("MD5");
	} catch (Exception e) {
	    throw new KrbCryptoException("JCE provider may not be installed. " + e.getMessage());
	}
	try {
	    md5.update(buffer);
	    result = md5.digest();
	    buffer = result;						 
	} catch (Exception e) {
	    throw new KrbCryptoException(e.getMessage());
	}
		
    }

    public static synchronized byte[] bytes(int size) throws KrbCryptoException {
	lastKeyTime = System.currentTimeMillis();
	byte[] data = new byte[size];
	for (int i = 0; i < data.length; i++) {
	    if ((index % buffer.length) == 0)
		reinitialize();
	    data[i] = buffer[(int)(index % buffer.length)];
	    index++;
	}
	return data;
    }

    public static synchronized int intValue() throws KrbCryptoException {
	byte[] data = Confounder.bytes(4);
	int result = 0;
	for (int i = 0; i < 4; i++)
	    result += data[i] * (16 ^ i);
	return result;
    }

    public static synchronized long longValue() throws KrbCryptoException {
	byte[] data = Confounder.bytes(4);
	long result = 0;
	for (int i = 0; i < 8; i++)
	    result += ((long)data[i]) * (16L ^ i);
	return result;
    }

}

