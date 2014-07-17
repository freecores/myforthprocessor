/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbDataInputStream.java	1.5 03/06/24
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

package sun.security.krb5.internal.util;

import java.io.BufferedInputStream;
import java.io.InputStream;
import java.io.IOException;

/**
 * This class implements a buffered input stream. It provides methods to read a chunck
 * of data from underlying data stream.
 *
 * @author Yanni Zhang
 * @version 1.00
 *
 */
public class KrbDataInputStream extends BufferedInputStream{
    public KrbDataInputStream(InputStream is){
	super(is);
    }
    /**
     * Reads up to the specific number of bytes from this input stream.
     * @param num the number of bytes to be read.
     * @return the int value of this byte array.
     * @exception IOException.
     */
    public int read(int num) throws IOException{
	byte[] bytes = new byte[num];
	read(bytes, 0, num);
	int result = 0;
	for (int i = 0; i < num; i++) {
	    result |= (bytes[i] & 0xff) << (num - i - 1) * 8;
	}
	return result;
    }
    public int readVersion() throws IOException {
	return read(2);
    }
}
	
