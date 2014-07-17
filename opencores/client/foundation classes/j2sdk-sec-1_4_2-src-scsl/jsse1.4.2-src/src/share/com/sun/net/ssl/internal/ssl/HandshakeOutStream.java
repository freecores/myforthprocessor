/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.OutputStream;
import java.io.IOException;
import java.security.MessageDigest;

/**
 * Output stream for handshake data.  This is used only internally
 * to the SSL classes.
 *
 * MT note:  one thread at a time is presumed be writing handshake
 * messages, but (after initial connection setup) it's possible to
 * have other threads reading/writing application data.  It's the
 * SSLSocketImpl class that synchronizes record writes.
 *
 * @version 1.23 06/24/03
 * @author  David Brownell
 */
class HandshakeOutStream extends OutputStream {

    private SSLSocketImpl c;
    OutputRecord r;

    HandshakeOutStream(ProtocolVersion protocolVersion,
	    ProtocolVersion helloVersion, HandshakeHash handshakeHash,
	    SSLSocketImpl c) {
	r = new OutputRecord(Record.ct_handshake);
	r.setVersion(protocolVersion);
	r.setHelloVersion(helloVersion);
	r.setHandshakeHash(handshakeHash);
	this.c = c;
    }

    /*
     * Update the handshake data hashes ... mostly for use after a
     * client cert has been sent, so the cert verify message can be
     * constructed correctly yet without forcing extra I/O.  In all
     * other cases, automatic hash calculation suffices.
     */
    void doHashes() {
	r.doHashes();
    }

    /*
     * Write some data out onto the stream ... buffers as much as possible.
     * Hashes are updated automatically if something gets flushed to the
     * network (e.g. a big cert message etc).
     */
    public void write(byte buf[], int off, int len) throws IOException {
	while (len > 0) {
	    int howmuch = Math.min(len, r.availableDataBytes());

	    if (howmuch == 0) {
		flush();
	    } else {
		r.write(buf, off, howmuch);
		off += howmuch;
		len -= howmuch;
	    }
	}
    }

    /*
     * write-a-byte
     */
    public void write(int i) throws IOException {
	if (r.availableDataBytes() < 1) {
	    flush();
	}
	r.write(i);
    }

    public void flush() throws IOException {
	try {
	    c.writeRecord(r);
	} catch (IOException e) {
	    // Had problems writing; check if there was an
	    // alert from peer. If alert received, waitForClose
	    // will throw an exception for the alert
	    c.waitForClose(true);

	    // No alert was received, just rethrow exception
	    throw e;
	}
    }

    /*
     * Put integers encoded in standard 8, 16, 24, and 32 bit
     * big endian formats. Note that OutputStream.write(int) only
     * writes the least significant 8 bits and ignores the rest.
     */

    void putInt8(int i) throws IOException {
	r.write(i);
    }

    void putInt16(int i) throws IOException {
	if (r.availableDataBytes() < 2) {
	    flush();
	}
	r.write(i >> 8);
	r.write(i);
    }

    void putInt24(int i) throws IOException {
	if (r.availableDataBytes() < 3) {
	    flush();
	}
	r.write(i >> 16);
	r.write(i >> 8);
	r.write(i);
    }

    void putInt32(int i) throws IOException {
	if (r.availableDataBytes() < 4) {
	    flush();
	}
	r.write(i >> 24);
	r.write(i >> 16);
	r.write(i >> 8);
	r.write(i);
    }

    /*
     * Put byte arrays with length encoded as 8, 16, 24 bit
     * integers in big-endian format.
     */
    void putBytes8(byte b[]) throws IOException {
	if (b == null) {
	    putInt8(0);
	    return;
	}
	putInt8(b.length);
	write(b, 0, b.length);
    }

    void putBytes16(byte b[]) throws IOException {
	if (b == null) {
	    putInt16(0);
	    return;
	}
	putInt16(b.length);
	write(b, 0, b.length);
    }

    void putBytes24(byte b[]) throws IOException {
	if (b == null) {
	    putInt24(0);
	    return;
	}
	putInt24(b.length);
	write(b, 0, b.length);
    }
}
