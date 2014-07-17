/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package javax.net.ssl;

import java.io.IOException;
import java.net.InetAddress;
import java.net.ServerSocket;
import java.net.SocketException;
import javax.net.ServerSocketFactory;
import java.security.*;

import com.sun.net.ssl.internal.ssl.ExportControl;


/**
 * <code>SSLServerSocketFactory</code>s create 
 * <code>SSLServerSocket</code>s.
 *
 * @since 1.4
 * @see SSLSocket
 * @see SSLServerSocket
 * @version 1.21
 * @author David Brownell
 */
public abstract class SSLServerSocketFactory extends ServerSocketFactory
{
    private static SSLServerSocketFactory theFactory;


    /**
     * Constructor is used only by subclasses.
     */
    protected SSLServerSocketFactory() { /* NOTHING */ }


    /**
     * Returns the default SSL server socket factory.
     * The default implementation can be changed by setting the value of the
     * "ssl.ServerSocketFactory.provider" security property (in the Java
     * security properties file) to the desired class.
     *
     * <p>If SSL has not been
     * configured properly for this virtual machine, the factory will be
     * inoperative (use of which will report instantiation exceptions).
     *
     * @return the default <code>ServerSocketFactory</code>
     */
    public static synchronized ServerSocketFactory getDefault()
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
		    theFactory = (SSLServerSocketFactory)cls.newInstance();
		} else {
		    /*
		     * If export version then insist on our implementation.
		     */
		    Class ours = com.sun.net.ssl.internal.ssl.SSLServerSocketFactoryImpl.class;
		    if (cls != ours)
			theFactory = new DefaultSSLServerSocketFactory(
			    "Export restriction: " +
			    "this JSSE implementation is non-pluggable.");
		    else
			theFactory = (SSLServerSocketFactory)cls.newInstance();
		}
	    } catch (Exception e) {
		theFactory = new DefaultSSLServerSocketFactory(e.getMessage());
	    }
	}

	return theFactory;
    }

    private static String getFactoryProvider() {
	String s = (String)AccessController.doPrivileged
	    (new PrivilegedAction() {
	    public Object run() {
		return java.security.Security.getProperty
					("ssl.ServerSocketFactory.provider");
	    }
	});
	if (s == null) {
	    s = "com.sun.net.ssl.internal.ssl.SSLServerSocketFactoryImpl";
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
     * on an SSL connection created by this factory.
     * Normally, only a subset of these will actually
     * be enabled by default, since this list may include cipher suites which
     * do not meet quality of service requirements for those defaults.  Such
     * cipher suites are useful in specialized applications.
     *
     * @return an array of cipher suite names
     * @see #getDefaultCipherSuites()
     */
    public abstract String [] getSupportedCipherSuites();
}


//
// The default factory does NOTHING.
//
class DefaultSSLServerSocketFactory extends SSLServerSocketFactory {

    String reason;

    DefaultSSLServerSocketFactory(String reasonStr)
    {
	reason = reasonStr;
    }

    public ServerSocket createServerSocket(int port)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public ServerSocket createServerSocket(int port, int backlog)
    throws IOException
    {
	throw new SocketException(reason);
    }

    public ServerSocket
    createServerSocket(int port, int backlog, InetAddress ifAddress)
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
