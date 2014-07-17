/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.OutputStream;
import java.io.IOException;

/*
 * Output stream for application data. This is the kind of stream
 * that's handed out via SSLSocket.getOutputStream(). It's all the application
 * ever sees.
 *
 * Once the initial handshake has completed, application data may be
 * interleaved with handshake data. That is handled internally and remains
 * transparent to the application.
 *
 * @version 1.27 06/24/03
 * @author  David Brownell
 */
class AppOutputStream extends OutputStream {

    private SSLSocketImpl c;
    OutputRecord r;
    
    // One element array used to implement the write(byte) method
    private final byte[] oneByte = new byte[1];

    AppOutputStream(SSLSocketImpl conn) {
        r = new OutputRecord(Record.ct_application_data);
        c = conn;
    }

    /**
     * Write the data out, NOW.
     */
    synchronized public void write(byte b[], int off, int len) 
	    throws IOException {
	// check if the Socket is invalid (error or closed)
	c.checkWrite();
	//
	// Always flush at the end of each application level record.
	// This lets application synchronize read and write streams
	// however they like; if we buffered here, they couldn't.
	//
	// NOTE: *must* call c.writeRecord() even for len == 0
	try {
	    do {
		int howmuch = Math.min(len, r.availableDataBytes());
    
		if (howmuch > 0) {
		    r.write(b, off, howmuch);
		    off += howmuch;
		    len -= howmuch;
		}
		c.writeRecord(r);
		c.checkWrite();
	    } while (len > 0);
	} catch (Exception e) {
	    // shutdown and rethrow (wrapped) exception as appropriate
	    c.handleException(e);
	}
    }

    /**
     * Write one byte now.
     */
    synchronized public void write(int i) throws IOException {
        oneByte[0] = (byte)i;
	write(oneByte, 0, 1);
    }

    synchronized public void close() throws IOException {
        c.close();
    }
    
    // inherit no-op flush()
}
