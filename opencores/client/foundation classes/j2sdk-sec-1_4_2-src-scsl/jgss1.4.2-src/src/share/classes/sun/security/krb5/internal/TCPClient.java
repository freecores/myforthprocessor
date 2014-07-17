/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)TCPClient.java	1.5 03/06/24
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

package sun.security.krb5.internal;

import java.io.*;
import java.net.*;

public class TCPClient {
    int bufSize = 65507;
    Socket tcpSocket;
    BufferedOutputStream out;
    BufferedInputStream in;

    public void setBufSize(int newBufSize) {
	bufSize = newBufSize;
    }

    public TCPClient(InetAddress iaddr, int port) throws IOException {
	tcpSocket = new Socket(iaddr, port);
	out = new BufferedOutputStream(tcpSocket.getOutputStream());
	in = new BufferedInputStream(tcpSocket.getInputStream());
    }

    public TCPClient(String hostname, int port) throws IOException {
	tcpSocket = new Socket(hostname, port);
	out = new BufferedOutputStream(tcpSocket.getOutputStream());
	in = new BufferedInputStream(tcpSocket.getInputStream());
    }

    public TCPClient(Socket sock) throws IOException {
	tcpSocket = sock;
	out = new BufferedOutputStream(tcpSocket.getOutputStream());
	in = new BufferedInputStream(tcpSocket.getInputStream());
    }

    public InetAddress getInetAddress() {
	return tcpSocket.getInetAddress();
    }

    public void send(byte[] data) throws IOException {
	out.write(data);
	out.flush();
    }

    public byte[] receive() throws IOException {
	byte temp[] = new byte[bufSize];
	int size = in.read(temp);
	if (size >= 0) {
	    byte[] data = new byte[size];
	    System.arraycopy(temp, 0, data, 0, size);
	    return data;
	}
	return null;
    }

    public void close() throws IOException {
	tcpSocket.close();
    }

}
