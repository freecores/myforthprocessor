/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)Crc32CksumType.java	1.5 03/06/24
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

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import java.util.zip.CRC32;

public class Crc32CksumType extends CksumType {

    public Crc32CksumType() {
    }

    public int confounderSize() {
	return 0;
    }

    public int cksumType() {
	return Checksum.CKSUMTYPE_CRC32;
    }

    public boolean isSafe() {
	return false;
    }

    public int cksumSize() {
	return 4;
    }

    public int keyType() {
	return Krb5.KEYTYPE_NULL;
    }

    public int keySize() {
	return 0;
    }

    public byte[] calculateChecksum(byte[] data, int size) {
	return crc32.byte2crc32sum_bytes(data, size);
    }

    public byte[] calculateKeyedChecksum(byte[] data, int size,
					 byte[] key) {
					     return null;
					 }

    public boolean verifyKeyedChecksum(byte[] data, int size,
				       byte[] key, byte[] checksum) {
	return false;
    }

    public static byte[] int2quad(long input) {
	byte[] output = new byte[4];
	for (int i = 0; i < 4; i++) {
	    output[i] =	(byte)((input >>> (i * 8)) & 0xff);
	}
	return output;
    }

    //  /*  public static void main(String[] args) {
    //        if (args.length == 0) {
    //            System.out.println("Input a message string.");
    //            System.exit(0);
    //        }
    //        String message = args[0];
    //        byte[] msgData = message.getBytes();
    //
    //        System.out.println("Original message is " + message);
    //        System.out.println("\n====Result from JDK's crc32 alg====");
    //        CRC32 newcrc = new CRC32();
    //        newcrc.update(msgData);
    //        long value = newcrc.getValue();
    //        //System.out.println("new crc32 checksum is " + newcrc.getValue());
    //        System.out.println("new crc32 checksum is " + Long.toHexString(value));
    //        System.out.println("new crc32 checksum is " + Long.toBinaryString(value));
    //        int value2 = (int)(newcrc.getValue());
    //
    //        System.out.println("\n===Result calculated from crc32===");
    //         Crc32CksumType crc = new Crc32CksumType();
    //        byte[] resultData = crc.calculateChecksum(msgData, msgData.length);
    //        for (int i = 0; i < resultData.length; i++) {
    //            System.out.print(resultData[i]);
    //        }
    //        System.out.println(" Result length is: " + resultData.length);
    //
    //    }    */

    public static long bytes2long(byte[] input) {
        long result = 0;

        result |= (((long)input[0]) & 0xffL) << 24;
        result |= (((long)input[1]) & 0xffL) << 16;
        result |= (((long)input[2]) & 0xffL) << 8;
        result |= (((long)input[3]) & 0xffL);
        return result;
    }
}
