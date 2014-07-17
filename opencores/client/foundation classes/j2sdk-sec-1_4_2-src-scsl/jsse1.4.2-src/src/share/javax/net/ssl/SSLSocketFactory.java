/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

import java.net.*;
import javax.net.SocketFactory;
import java.io.IOException;
import java.security.*;

import com.sun.net.ssl.internal.ssl.ExportControl;

/**
 * <code>SSLSocketFactory</code>s create <code>SSLSocket</code>s.
 *
 * @since 1.4
 * @see SSLSocket
 * @version 1.18
 * @author David Brownell
 */
public abstract class SSLSocketFactory extends SocketFactory
{
    private static SSLSocketFactory		theFactory;

    /**
     * Returns the default SSL socket factory.
     * The default implementation can be changed by setting the value of the
     * "ssl.SocketFactory.provider" security property (in the Java
     * security properties file) to the desired class.
     *
     * <p>If SSL has not been
     * configured properly for this virtual machine, the factory will be
     * inoperative (reporting instantiation exceptions).
     *
     * @return the default <code>SocketFactory</code>
     */
    public static synchronized SocketFactory getDefault()
    {
	if (theFactory == null) {
	    String clsName = getFactoryProvider();
	    try {
		Class cls = null;
		try {
		    cls = Class.forName(clsName);
		} catch (ClassNotFoundException e) {
		    ClassLoader cl = ClassLoader.getSystemClassLoader();
		    if (cl != null) {
			cls = cl.loadClass(clsName);
		    }
		}
		if (ExportControl.isReplaceable) {
		    theFactory = (SSLSocketFactory)cls.newInstance();
		} else {
		    /*
		     * If export version then insist on our implementation.
		     */
		    Class ours =
			com.sun.net.ssl.internal.ssl.SSLSocketFactoryImpl.class;
		    if (cls != ours)
			theFactory = new DefaultSSLSocketFactory(
			    "Export restriction: " +
			    "this JSSE implementation is non-pluggable.");
		    else
			theFactory = (SSLSocketFactory)cls.newInstance();
		}
	    } catch (Exception e) {
		theFactory = new DefaultSSLSocketFactory(e.getMessage());
	    }
	}

	return theFactory;
    }

    private static String getFactoryProvider() {
	String s = (String)AccessController.doPrivileged
	    (new PrivilegedAction() {
	    public Object run() {
		return java.security.Security.getProperty
					("ssl.SocketFactory.provider");
	    }
	});
	if (s == null) {
	    s = "com.sun.net.ssl.internal.ssl.SSLSocketFactoryImpl";
	}
	return s;
    }

    /**
     * Returns the list of cipher suites which are enabled by default.
     * Unless a different list is enabled, handshaking on an SSL connection
     * will use one of these cipher suites.  The minimum quality of service
     * for these defaults requires confidentiality protection and server
     * authentication (that is, no anonymous cipher suites).
     *
     * @see #getSupportedCipherSuites()
     * @return array of the cipher suites enabled by default
     */
    public abstract String [] getDefaultCipherSuites();

    /**
     * Returns the names of the cipher suites which could be enabled for use
     * on an SSL connection.  Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not meet quality of service requirements for those defaults.  Such
     * cipher suites are useful in specialized applications.
     *
     * @see #getDefaultCipherSuites()
     * @return an array of cipher suite names
     */
    public abstract String [] getSupportedCipherSuites();

    /**
     * Returns a socket layered over an existing socket connected to the named
     * host, at the given port.  This constructor can be used when tunneling SSL
     * through a proxy or when negotiating the use of SSL over an existing
     * socket. The host and port refer to the logical peer destination.
     * This socket is configured using the socket options established for
     * this factory.
     *
     * @param s the existing socket
     * @param host the server host
     * @param port the server port
     * @param autoClose close the underlying socket when this socket is closed
     * @return a socket connected to the specified host and port
     * @throws IOException if an I/O error occurs when creating the socket
     * @throws UnknownHostException if the host is not known
     */
    public abstract Socket createSocket(Socket s, String host,
					int port, boolean autoClose)
    throws IOException;
}


// file private
class DefaultSSLSocketFactory extends SSLSocketFactory
{
    String reason;

    DefaultSSLSocketFactory(String reasonStr) {
	reason = reasonStr;
    }

    public Socket createSocket(String host, int port)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public Socket createSocket(Socket s, String host,
				int port, boolean autoClose)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public Socket createSocket(InetAddress address, int port)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public Socket createSocket(String host, int port,
	InetAddress clientAddress, int clientPort)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public Socket createSocket(InetAddress address, int port,
	InetAddress clientAddress, int clientPort)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public String [] getDefaultCipherSuites() {
	return new String[0];
    }

    public String [] getSupportedCipherSuites() {
	return new String[0];
    }
}
