/*
 * @(#)RMIDirectSocketFactory.java	1.9 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.rmi.transport.proxy;

import java.io.IOException;
import java.net.Socket;
import java.net.ServerSocket;
import java.rmi.server.RMISocketFactory;

/**
 * RMIDirectSocketFactory creates a direct socket connection to the
 * specified port on the specified host.
 */
public class RMIDirectSocketFactory extends RMISocketFactory {

    public Socket createSocket(String host, int port) throws IOException
    {
	return new Socket(host, port);
    }

    public ServerSocket createServerSocket(int port) throws IOException
    {
	return new ServerSocket(port);
    }
}
