/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;

import javax.net.ssl.SSLServerSocketFactory;

/**
 * This class creates SSL server sockets.
 * 
 * @version 1.15, 06/24/03
 * @author David Brownell
 */
final
public class SSLServerSocketFactoryImpl extends SSLServerSocketFactory
{
    private static final int DEFAULT_BACKLOG = 50;
    private SSLContextImpl context;


    /**
     * Constructor is used to instantiate factory.  It sets itself up
     * with the process-default authentication context.
     */
    public SSLServerSocketFactoryImpl () {
	this.context = SSLContextImpl.getDefaultContext();
    }

    /**
     * Called from SSLContextImpl's getSSLServerSocketFactory().
     */
    SSLServerSocketFactoryImpl (SSLContextImpl context)
    {
	this.context = context;
    }

    /**
     * Returns an unbound server socket.
     *
     * @return the unbound socket
     * @throws IOException if the socket cannot be created
     * @see java.net.Socket#bind(java.net.SocketAddress)
     */
    public ServerSocket createServerSocket() throws IOException {
	return new SSLServerSocketImpl(context);
    }

    public ServerSocket createServerSocket (int port)
    throws IOException
    {
	return new SSLServerSocketImpl (port, DEFAULT_BACKLOG, context);
    }


    public ServerSocket createServerSocket (int port, int backlog)
    throws IOException
    {
	return new SSLServerSocketImpl (port, backlog, context);
    }

    public ServerSocket
    createServerSocket (int port, int backlog, InetAddress ifAddress)
    throws IOException
    {
	return new SSLServerSocketImpl (port, backlog, ifAddress, context);
    }

    /**
     * Returns the subset of the supported cipher suites which are
     * enabled by default.  These cipher suites all provide a minimum
     * quality of service whereby the server authenticates itself
     * (preventing person-in-the-middle attacks) and where traffic
     * is encrypted to provide confidentiality.
     */  
    public String[] getDefaultCipherSuites() {
	CipherSuiteList.clearAvailableCache();
	return CipherSuiteList.getDefault().toStringArray();
    }

    /**
     * Returns the names of the cipher suites which could be enabled for use
     * on an SSL connection.  Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not support the mutual authentication of servers and clients, or
     * which do not protect data confidentiality.  Servers may also need
     * certain kinds of certificates to use certain cipher suites.
     *   
     * @return an array of cipher suite names
     */  
    public String[] getSupportedCipherSuites() {
	CipherSuiteList.clearAvailableCache();
	return CipherSuiteList.getSupported().toStringArray();
    }
    
}

