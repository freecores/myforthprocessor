/*
 * @(#)SaslInputStream.java	1.6 03/01/23
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
import java.io.EOFException;
import java.io.InputStream;

/**
 * This class is used by clients of Java SASL that need to create streams
 * using SaslClient's wrap/unwrap.
 *
 * @author Rosanna Lee
 */
class SaslInputStream extends InputStream {
    private static final boolean debug = false;

    private byte[] saslBuffer;	// buffer for storing raw bytes
    private byte[] lenBuf = new byte[4];  // buffer for storing length

    private byte[] buf = new byte[0];   // buffer for storing processed bytes
					// Initialized to empty buffer
    private int bufPos = 0;		// read position in buf
    private InputStream in;		// underlying input stream
    private SaslClient sc;
    private int recvMaxBufSize = 65536;

    SaslInputStream(SaslClient sc, InputStream in) throws IOException {
	super();
	this.in = in;
	this.sc = sc;

	String str = sc.getNegotiatedProperty(Sasl.MAX_BUFFER);
	if (str != null) {
	    try {
		recvMaxBufSize = Integer.parseInt(str);
	    } catch (NumberFormatException e) {
		throw new SaslException(Sasl.MAX_BUFFER + 
		    " property must be numeric string: " + str);
	    }
	}
	saslBuffer = new byte[recvMaxBufSize];
    }

    public int read() throws IOException {
	byte[] inBuf = new byte[1];
	int count = read(inBuf, 0, 1);
	if (count > 0) {
	    return inBuf[0];
	} else {
	    return -1;
	}
    }

    public int read(byte[] inBuf, int start, int count) throws IOException {

	if (bufPos >= buf.length) {
	    int actual = fill();   // read and unwrap next SASL buffer
	    while (actual == 0) {  // ignore zero length content
		actual = fill();
	    }
	    if (actual == -1) {
		return -1;    // EOF
	    }
	}

	int avail = buf.length - bufPos;
	if (count > avail) {
	    // Requesting more that we have stored
	    // Return all that we have; next invocation of read() will
	    // trigger fill()
	    System.arraycopy(buf, bufPos, inBuf, start, avail);
	    bufPos = buf.length;
	    return avail;
	} else {
	    // Requesting less than we have stored
	    // Return all that was requested
	    System.arraycopy(buf, bufPos, inBuf, start, count);
	    bufPos += count;
	    return count;
	}
    }

    /**
     * Fills the buf with more data by reading a SASL buffer, unwrapping it,
     * and leaving the bytes in buf for read() to return.
     * @return The number of unwrapped bytes available
     */
    private int fill() throws IOException {
	// Read in length of buffer
	int actual = readFully(lenBuf, 4);
	if (actual != 4) {
	    return -1;
	}
	int len = SaslImpl.networkByteOrderToInt(lenBuf, 0, 4);

	if (len > recvMaxBufSize) {
	    throw new SaslException(
		len + "exceeds the negotiated receive buffer size limit:" + 
		recvMaxBufSize);
	}

	if (debug) {
	    System.err.println("reading " + len + " bytes from network");
	}

	// Read SASL buffer
	actual = readFully(saslBuffer, len);
	if (actual != len) {
	    throw new EOFException("Expecting to read " + len +
		" bytes but got " + actual + " bytes before EOF");
	}

	// Unwrap
	buf = sc.unwrap(saslBuffer, 0, len);

	bufPos = 0;

	return buf.length;
    }

    /**
     * Read requested number of bytes before returning.
     * @return The number of bytes actually read; -1 if none read
     */
    private int readFully(byte[] inBuf, int total) throws IOException {
	int count, pos = 0;

	if (debug) {
	    System.err.println("readFully " + total + " from " + in);
	}

	while (total > 0) {
	    count = in.read(inBuf, pos, total);

	    if (debug) {
		System.err.println("readFully read " + count);
	    }

	    if (count == -1 ) {
		return (pos == 0? -1 : pos);
	    }
	    pos += count;
	    total -= count;
	}
	return pos;
    }

    public int available() throws IOException {
	return buf.length - bufPos;
    }

    public void close() throws IOException {
	sc.dispose();
	in.close();
    }
}
