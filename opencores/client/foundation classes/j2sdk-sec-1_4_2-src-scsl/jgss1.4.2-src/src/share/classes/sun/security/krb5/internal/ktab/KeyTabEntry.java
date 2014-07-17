/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KeyTabEntry.java	1.8 03/06/24
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

package sun.security.krb5.internal.ktab;

import sun.security.krb5.*;
import sun.security.krb5.internal.*;
import java.io.UnsupportedEncodingException;

/**
 * This class represents a Key Table entry. Each entry contains the service principal of
 * the key, time stamp, key version and secret key itself.
 *
 * @author Yanni Zhang
 * @version 1.00  09 Mar 2000
 */
public class KeyTabEntry implements KeyTabConstants {
    PrincipalName service;
    Realm realm;
    KerberosTime timestamp;
    int keyVersion;
    int keyType;
    byte[] keyblock = null;
    boolean DEBUG = Krb5.DEBUG;

    public KeyTabEntry (PrincipalName new_service, Realm new_realm, KerberosTime new_time, 
			int new_keyVersion, int new_keyType, byte[] new_keyblock) {
	service = new_service;
	realm = new_realm;
	timestamp = new_time;
	keyVersion = new_keyVersion;
	keyType = new_keyType;
	if (new_keyblock != null) {
	    keyblock = (byte[])new_keyblock.clone();
	}
    }

    public PrincipalName getService() {
	return service;
    }

    public EncryptionKey getKey() {
	EncryptionKey key = new EncryptionKey(keyblock, 
					      keyType,
					      new Integer(keyVersion));
	return key;
    }

    public String getKeyString() {
        StringBuffer sb = new StringBuffer("0x");
        for (int i = 0; i < keyblock.length; i++) {
            sb.append(Integer.toHexString(keyblock[i]&0xff));
        }
        return sb.toString();
    }
    public int entryLength() {
	int totalPrincipalLength = 0;
	String[] names = service.getNameStrings();
	for (int i = 0; i < names.length; i++) {
	    try {
		totalPrincipalLength += principalSize + names[i].getBytes("8859_1").length;
	    } catch (UnsupportedEncodingException exc) {
	    }
	}

	int realmLen = 0;
	try {
	    realmLen = realm.toString().getBytes("8859_1").length;
	} catch (UnsupportedEncodingException exc) {
	}

	int size = principalComponentSize +  realmSize + realmLen
	    + totalPrincipalLength + principalTypeSize 
	    + timestampSize + keyVersionSize
	    + keyTypeSize + keySize + keyblock.length;

	if (DEBUG) {
	    System.out.println(">>> KeyTabEntry: key tab entry size is " + size);
	}
	return size;
    }

    public KerberosTime getTimeStamp() {
        return timestamp;
    }
}
