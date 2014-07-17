/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KeyTabOutputStream.java	1.5 03/06/24
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

import sun.security.krb5.internal.*;
import sun.security.krb5.internal.util.KrbDataOutputStream;
import java.io.IOException;
import java.io.FileOutputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;

/**
 * This class implements a buffered input stream. It is used for parsing key table 
 * data to memory.
 *
 * @author Yanni Zhang
 * @version 1.00 13 Mar 2000
 *
 */
public class KeyTabOutputStream extends KrbDataOutputStream implements KeyTabConstants {
    private KeyTabEntry entry;
    private int keyType;
    private byte[] keyValue;
    public int version;

    public KeyTabOutputStream(OutputStream os) {
	super(os);
    }

    public void writeVersion(int num) throws IOException {
	version = num;
	write16(num);		//we use the standard version.
    }

    public void writeEntry(KeyTabEntry entry) throws IOException {
	write32(entry.entryLength());
	String[] serviceNames =  entry.service.getNameStrings();
	int comp_num = serviceNames.length;
	if (version == KRB5_KT_VNO_1) {
	    write16(comp_num + 1);
	}
	else write16(comp_num);

	byte[] realm = null;
	try {
	    realm = entry.service.getRealmString().getBytes("8859_1");
	} catch (UnsupportedEncodingException exc) {
	}

	write16(realm.length);
	write(realm);
	for (int i = 0; i < comp_num; i++) {
	    try {
		write16(serviceNames[i].getBytes("8859_1").length);
		write(serviceNames[i].getBytes("8859_1"));
	    } catch (UnsupportedEncodingException exc) {
	    }
	}
	write32(entry.service.getNameType());
	//time is long, but we only use 4 bytes to store the data.
	write32((int)(entry.timestamp.getTime()/1000));
	write8(entry.keyVersion);
	write16(entry.keyType);
	write16(entry.keyblock.length);
	write(entry.keyblock);			  
    }	
}
	
