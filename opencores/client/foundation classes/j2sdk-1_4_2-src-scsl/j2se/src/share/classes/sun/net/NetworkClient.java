/*
 * @(#)NetworkClient.java	1.34 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net;

import java.io.*;
import java.net.Socket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.net.URL;
import sun.io.CharToByteConverter;
import java.util.Arrays;


/**
 * This is the base class for network clients.
 *
 * @version	1.34, 01/23/03
 * @author	Jonathan Payne
 */
public class NetworkClient {
    /** Socket for communicating with server. */
    protected Socket	serverSocket = null;

    /** Stream for printing to the server. */
    public PrintStream	serverOutput;

    /** Buffered stream for reading replies from server. */
    public InputStream	serverInput;

    protected static int defaultSoTimeout;
    protected static int defaultConnectTimeout;

    /* Name of encoding to use for output */
    protected static String encoding;

    static {
	Integer tm = (Integer) java.security.AccessController.doPrivileged(
				      new java.security.PrivilegedAction() {
	    public Object run() {
		return (Integer.getInteger("sun.net.client.defaultReadTimeout"));
	    }
	});
	if (tm == null)
	    defaultSoTimeout = -1;
	else 
	    defaultSoTimeout = tm.intValue();

	tm = (Integer) java.security.AccessController.doPrivileged(
                                      new java.security.PrivilegedAction() {
            public Object run() {
                return (Integer.getInteger("sun.net.client.defaultConnectTimeout"));
            }
        });
        if (tm == null)
            defaultConnectTimeout = -1;
        else
            defaultConnectTimeout = tm.intValue();


	encoding = (String) java.security.AccessController.doPrivileged(
                    new sun.security.action.GetPropertyAction("file.encoding", 
							      "ISO8859_1")
    	);
	try {
    	    if (!isASCIISuperset (encoding)) {
	    	encoding = "ISO8859_1";
    	    }
	} catch (Exception e) {
	    encoding = "ISO8859_1";
    	}
    }


    /**
     * Test the named character encoding to verify that it converts ASCII 
     * characters correctly. We have to use an ASCII based encoding, or else 
     * the NetworkClients will not work correctly in EBCDIC based systems.
     * However, we cannot just use ASCII or ISO8859_1 universally, because in 
     * Asian locales, non-ASCII characters may be embedded in otherwise 
     * ASCII based protocols (eg. HTTP). The specifications (RFC2616, 2398) 
     * are a little ambiguous in this matter. For instance, RFC2398 [part 2.1] 
     * says that the HTTP request URI should be escaped using a defined 
     * mechanism, but there is no way to specify in the escaped string what 
     * the original character set is. It is not correct to assume that 
     * UTF-8 is always used (as in URLs in HTML 4.0).  For this reason, 
     * until the specifications are updated to deal with this issue more
     * comprehensively, and more importantly, HTTP servers are known to 
     * support these mechanisms, we will maintain the current behavior 
     * where it is possible to send non-ASCII characters in their original 
     * unescaped form. 
     */
    private static boolean isASCIISuperset (String encoding) throws Exception {
	String chkS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"+
			"abcdefghijklmnopqrstuvwxyz-_.!~*'();/?:@&=+$,";

	// Expected byte sequence for string above
	byte[] chkB = { 48,49,50,51,52,53,54,55,56,57,65,66,67,68,69,70,71,72,
		73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,97,98,99,
		100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,
		115,116,117,118,119,120,121,122,45,95,46,33,126,42,39,40,41,59,
		47,63,58,64,38,61,43,36,44};

	CharToByteConverter ctob = CharToByteConverter.getConverter (encoding);
	byte[] b = ctob.convertAll (chkS.toCharArray());
	return Arrays.equals (b, chkB);
    }

    /** Open a connection to the server. */
    public void openServer(String server, int port)
	throws IOException, UnknownHostException {
	if (serverSocket != null)
	    closeServer();
	serverSocket = doConnect (server, port);
	try {
	    serverOutput = new PrintStream(new BufferedOutputStream(
					serverSocket.getOutputStream()),
				       	true, encoding);
	} catch (UnsupportedEncodingException e) {
	    throw new InternalError(encoding +"encoding not found");
	}
	serverInput = new BufferedInputStream(serverSocket.getInputStream());
    }

    /**
     * Return a socket connected to the server, with any
     * appropriate options pre-established
     */
    protected Socket doConnect (String server, int port)
    throws IOException, UnknownHostException {
	Socket s = new Socket();
	if (defaultConnectTimeout > 0) {
	    s.connect(new InetSocketAddress(server, port), defaultConnectTimeout);
	} else {
	    s.connect(new InetSocketAddress(server, port));
	}
	if (defaultSoTimeout > 0) {
            s.setSoTimeout(defaultSoTimeout);
        }
	return s;
    }

    protected InetAddress getLocalAddress() throws IOException {
	if (serverSocket == null)
	    throw new IOException("not connected");
	return serverSocket.getLocalAddress();
    }

    /** Close an open connection to the server. */
    public void closeServer() throws IOException {
	if (! serverIsOpen()) {
	    return;
	}
	serverSocket.close();
	serverSocket = null;
	serverInput = null;
	serverOutput = null;
    }

    /** Return server connection status */
    public boolean serverIsOpen() {
	return serverSocket != null;
    }

    /** Create connection with host <i>host</i> on port <i>port</i> */
    public NetworkClient(String host, int port) throws IOException {
	openServer(host, port);
    }

    public NetworkClient() {}
}
