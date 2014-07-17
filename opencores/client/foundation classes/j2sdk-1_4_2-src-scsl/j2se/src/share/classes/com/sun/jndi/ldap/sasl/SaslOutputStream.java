/*
 * @(#)SaslOutputStream.java	1.5 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap.sasl;

import com.sun.security.sasl.preview.Sasl;
import com.sun.security.sasl.preview.SaslClient;
import com.sun.security.sasl.preview.SaslException;
import com.sun.security.sasl.util.SaslImpl;
import java.io.IOException;
import java.io.FilterOutputStream;
import java.io.OutputStream;

class SaslOutputStream extends FilterOutputStream {
    private static final boolean debug = false;

    private byte[] lenBuf = new byte[4];  // buffer for storing length
    private int rawSendSize = 65536;
    private SaslClient sc;

    SaslOutputStream(SaslClient sc, OutputStream out) throws IOException {
	super(out);
	this.sc = sc;

	if (debug) {
	    System.err.println("SaslOutputStream: " + out);
	}

	String str = sc.getNegotiatedProperty(Sasl.RAW_SEND_SIZE);
	if (str != null) {
	    try {
		rawSendSize = Integer.parseInt(str);
	    } catch (NumberFormatException e) {
		throw new SaslException(Sasl.RAW_SEND_SIZE + 
		    " property must be numeric string: " + str);
	    }
	} 
    }

    public void write(int b) throws IOException {
	byte[] buffer = new byte[1];
	buffer[0] = (byte)b;
	write(buffer, 0, 1);
    }

    public void write(byte[] buffer, int offset, int total) throws IOException {
	int count;
	byte[] wrappedToken, saslBuffer;
	    
	// "Packetize" buffer to be within rawSendSize
	if (debug) {
	    System.err.println("Total size: " + total);
	}

	for (int i = 0; i < total; i += rawSendSize) {

	    // Calculate length of current "packet"
	    count = (total - i) < rawSendSize ? (total - i) : rawSendSize;

	    // Generate wrapped token 
	    wrappedToken = sc.wrap(buffer, offset+i, count);

	    // Write out length
	    SaslImpl.intToNetworkByteOrder(wrappedToken.length, lenBuf, 0, 4);

	    if (debug) {
		System.err.println("sending size: " + wrappedToken.length);
	    }
	    out.write(lenBuf, 0, 4);

	    // Write out wrapped token
	    out.write(wrappedToken, 0, wrappedToken.length);
	}
    }

    public void close() throws IOException {
	sc.dispose();
	super.close();
    }
}
