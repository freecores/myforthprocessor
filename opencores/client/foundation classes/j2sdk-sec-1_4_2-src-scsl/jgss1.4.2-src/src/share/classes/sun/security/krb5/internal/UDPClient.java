/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)UDPClient.java	1.6 03/06/24
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

public class UDPClient {
    InetAddress iaddr;
    int iport;
    int bufSize = 65507;
    DatagramSocket dgSocket;
    DatagramPacket dgPacketIn;

    public UDPClient(InetAddress newIAddr, int port)
	throws SocketException {
	iaddr = newIAddr;
	iport = port;
	dgSocket = new DatagramSocket();
    }

    public UDPClient(String hostname, int port)
	throws UnknownHostException, SocketException {
	iaddr = InetAddress.getByName(hostname);
	iport = port;
	dgSocket = new DatagramSocket();
    }

    public UDPClient(String hostname, int port, int timeout)
	throws UnknownHostException, SocketException {
	iaddr = InetAddress.getByName(hostname);
	iport = port;
	dgSocket = new DatagramSocket();
	dgSocket.setSoTimeout(timeout);
    }

    public void setBufSize(int newBufSize) {
	bufSize = newBufSize;
    }

    public InetAddress getInetAddress() {
	if (dgPacketIn != null)
	    return dgPacketIn.getAddress();
	return null;
    }

    public void send(byte[] data) throws IOException {
	DatagramPacket dgPacketOut = new DatagramPacket(data, data.length,
							iaddr, iport);
	dgSocket.send(dgPacketOut);
    }

    public byte[] receive() throws IOException {
	byte ibuf[] = new byte[bufSize];
	dgPacketIn = new DatagramPacket(ibuf, ibuf.length);
        try {
            dgSocket.receive(dgPacketIn);
        }
        catch (SocketException e) {
            dgSocket.receive(dgPacketIn);
        }
	byte[] data = new byte[dgPacketIn.getLength()];
	System.arraycopy(dgPacketIn.getData(), 0, data, 0,
			 dgPacketIn.getLength());
	return data;
    }

}
