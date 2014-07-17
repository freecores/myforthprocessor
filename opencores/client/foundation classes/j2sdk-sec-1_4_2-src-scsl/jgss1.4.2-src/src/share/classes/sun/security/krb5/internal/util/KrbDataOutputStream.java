/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)KrbDataOutputStream.java	1.5 03/06/24
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

import java.io.BufferedOutputStream;
import java.io.IOException;
import java.io.OutputStream;

/**
 * This class implements a buffered output stream. It provides methods to write a chunck of
 * bytes to underlying data stream.
 *
 * @author Yanni Zhang
 * @version 1.00
 *
 */
public class KrbDataOutputStream extends BufferedOutputStream {
    public KrbDataOutputStream(OutputStream os) {
	super(os);
    }
    public void write32(int num) throws IOException {
	byte[] bytes = new byte[4];
	bytes[0] = (byte)((num & 0xff000000) >> 24 & 0xff);
	bytes[1] = (byte)((num & 0x00ff0000) >> 16 & 0xff);
	bytes[2] = (byte)((num & 0x0000ff00) >> 8 & 0xff);
	bytes[3] = (byte)(num & 0xff);
	write(bytes, 0, 4);
    }

    public void write16(int num) throws IOException {
	byte[] bytes = new byte[2];
	bytes[0] = (byte)((num & 0xff00) >> 8 & 0xff);
	bytes[1] = (byte)(num & 0xff);
	write(bytes, 0, 2);
    }

    public void write8(int num) throws IOException {
	write(num & 0xff);
    }
}
	
