/*
 * @(#)FtpURLConnection.java	1.61 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * FTP stream opener.
 */

package sun.net.www.protocol.ftp;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.BufferedInputStream;
import java.io.FilterInputStream;
import java.io.FilterOutputStream;
import java.io.FileNotFoundException;
import java.net.URL;
import java.net.URLStreamHandler;
import java.net.SocketPermission;
import java.net.UnknownHostException;
import java.net.MalformedURLException;
import java.util.StringTokenizer;
import java.security.Permission;
import sun.net.www.MessageHeader;
import sun.net.www.MeteredStream;
import sun.net.www.URLConnection;
import sun.net.ftp.FtpClient;
import sun.net.ftp.FtpProtocolException;
import sun.net.ProgressData;
import sun.net.ProgressEntry;
import sun.net.www.ParseUtil;
import sun.security.action.GetPropertyAction;


/** 
 * This class Opens an FTP input (or output) stream given a URL.
 * It works as a one shot FTP transfer :
 * <UL>
 * <LI>Login</LI>
 * <LI>Get (or Put) the file</LI>
 * <LI>Disconnect</LI>
 * </UL>
 * You should not have to use it directly in most cases because all will be handled
 * in a abstract layer. Here is an example of how to use the class :
 * <P>
 * <code>URL url = new URL("ftp://ftp.sun.com/pub/test.txt");<p>
 * UrlConnection con = url.openConnection();<p>
 * InputStream is = con.getInputStream();<p>
 * ...<p>
 * is.close();</code>
 * 
 * @see sun.net.ftp.FtpClient
 */
public class FtpURLConnection extends URLConnection {

    InputStream is = null;
    OutputStream os = null;

    FtpClient ftp = null;
    Permission permission;

    String password;
    String user;

    String host;
    String pathname;
    String filename;
    String fullpath;
    int port;
    static final int NONE = 0;
    static final int ASCII = 1;
    static final int BIN = 2;
    static final int DIR = 3;
    int type = NONE;

    /**
     * For FTP URLs we need to have a special InputStream because we
     * need to close 2 sockets after we're done with it :
     *	- The Data socket (for the file).
     *   - The command socket (FtpClient).
     * Since that's the only class that needs to see that, it is an inner class.
     */
    protected class FtpInputStream extends FilterInputStream {
	FtpClient ftp;
	FtpInputStream(FtpClient cl, InputStream fd) {
	    super(new BufferedInputStream(fd));
	    ftp = cl;
	}

	public void close() throws IOException {
	    super.close();
	    try {
		if (ftp != null) 
		    ftp.closeServer();
	    } catch (IOException ex) {
	    }
	}
    }

    /**
     * For FTP URLs we need to have a special OutputStream because we
     * need to close 2 sockets after we're done with it :
     *	- The Data socket (for the file).
     *   - The command socket (FtpClient).
     * Since that's the only class that needs to see that, it is an inner class.
     */
    protected class FtpOutputStream extends FilterOutputStream {
	FtpClient ftp;
	FtpOutputStream(FtpClient cl, OutputStream fd) {
	    super(fd);
	    ftp = cl;
	}

	public void close() throws IOException {
	    super.close();
	    try {
		if (ftp != null) 
		    ftp.closeServer();
	    } catch (IOException ex) {
	    }
	}
    }

    /**
     * Creates an FtpURLConnection from a URL.
     *
     * @param	url	The <code>URL</code> to retrieve or store.
     */
    public FtpURLConnection(URL url) {
	super(url);
	host = url.getHost();
	port = url.getPort();
        String userInfo = url.getUserInfo();
        
	if (userInfo != null) { // get the user and password
	    int delimiter = userInfo.indexOf(':');
	    if (delimiter == -1) {
		user = ParseUtil.decode(userInfo);
		password = null;
	    } else {
		user = ParseUtil.decode(userInfo.substring(0, delimiter++));
		password = ParseUtil.decode(userInfo.substring(delimiter));
	    }
	}
    }

    /**
     * Connects to the FTP server and logs in.
     *
     * @throws	FtpLoginException if the login is unsuccessful
     * @throws	FtpProtocolException if an error occurs
     * @throws	UnknownHostException if trying to connect to an unknown host
     */

    public synchronized void connect() throws IOException {
	if (connected) {
	    return;
	}
	if (user == null) {
	    user = "anonymous";
	    String vers = (String) java.security.AccessController.doPrivileged(
                new GetPropertyAction("java.version"));
	    password = (String) java.security.AccessController.doPrivileged(
                new GetPropertyAction("ftp.protocol.user", 
				      "Java" + vers +"@"));
	}
	try {
	    if (port != -1) 
		ftp = new FtpClient(host, port);
	    else
		ftp = new FtpClient(host);
	} catch (UnknownHostException e) {
	    // Maybe do something smart here, like use a proxy like iftp.
	    // Just keep throwing for now.
	    throw e;
	}
	ftp.login(user, password);
	connected = true;
    }


    /*
     * Decodes the path as per the RFC-1738 specifications.
     */
    private void decodePath(String path) {
	int i = path.indexOf(";type=");
	if (i >= 0) {
	    String s1 = path.substring(i+6, path.length());
	    if ("i".equalsIgnoreCase(s1))
		type = BIN;
	    if ("a".equalsIgnoreCase(s1))
		type = ASCII;
	    if ("d".equalsIgnoreCase(s1))
		type = DIR;
	    path = path.substring(0, i);
	}
	if (path != null && path.length() > 1 &&
	    path.charAt(0) == '/')
	    path = path.substring(1);
	if (path == null || path.length() == 0)
	    path = "./";
	if (!path.endsWith("/")) {
	    i = path.lastIndexOf('/');
	    if (i > 0) {
		filename = path.substring(i+1, path.length());
		filename = ParseUtil.decode(filename);
		pathname = path.substring(0, i);
	    } else {
		filename = ParseUtil.decode(path);
		pathname = null;
	    }
	} else {
	    pathname = path.substring(0, path.length() - 1);
	    filename = null;
	}
	if (pathname != null)
	    fullpath = pathname + "/" + (filename != null ? filename : "");
	else
	    fullpath = filename;
    }

    /*
     * As part of RFC-1738 it is specified that the path should be
     * interpreted as a series of FTP CWD commands.
     * This is because, '/' is not necessarly the directory delimiter
     * on every systems.
     */

    private void cd(String path) throws IOException {
	if (path == null || "".equals(path))
	    return;
	if (path.indexOf('/') == -1) {
	    ftp.cd(ParseUtil.decode(path));
	    return;
	}

	StringTokenizer token = new StringTokenizer(path,"/");
	while (token.hasMoreTokens())
	    ftp.cd(ParseUtil.decode(token.nextToken()));
    }

    /**
     * Get the InputStream to retreive the remote file. It will issue the
     * "get" (or "dir") command to the ftp server.
     *
     * @return	the <code>InputStream</code> to the connection.
     *
     * @throws	IOException if already opened for output
     * @throws	FtpProtocolException if errors occur during the transfert.
     */
    public InputStream getInputStream() throws IOException {
	if (!connected) {
	    connect();
	}

	if (os != null)
	    throw new IOException("Already opened for output");

	if (is != null) {
	    return is;
	}

	MessageHeader msgh = new MessageHeader();

	try {
            decodePath(url.getPath());
	    if (filename == null || type == DIR) {
		ftp.ascii();
		cd(pathname);
		if (filename == null)
		    is = new FtpInputStream(ftp, ftp.list());
		else
		    is = new FtpInputStream(ftp, ftp.nameList(filename));
	    } else {
		if (type == ASCII)
		    ftp.ascii();
		else
		    ftp.binary();
		cd(pathname);
		is = new FtpInputStream(ftp, ftp.get(filename));
	    }

	    /* Try to get the size of the file in bytes.  If that is
	       successful, then create a MeteredStream. */
	    try {
		String response = ftp.getResponseString();
		int offset;

		if ((offset = response.indexOf(" bytes)")) != -1) {
		    int i = offset;
		    int c;

		    while (--i >= 0 && ((c = response.charAt(i)) >= '0'
					&& c <= '9'))
			;
		    i = Integer.parseInt(response.substring(i + 1, offset));
		    msgh.add("content-length", ""+i);
		    if (i > 0) {
			ProgressEntry pe = new ProgressEntry(fullpath, null);
			pe.update(0, i);
			ProgressData.pdata.register(pe);
			is = new MeteredStream(is, pe);
		    }
		}
	    } catch (Exception e) {
		e.printStackTrace();
		/* do nothing, since all we were doing was trying to
		   get the size in bytes of the file */
	    }

	    String type = guessContentTypeFromName(fullpath);
	    if (type == null && is.markSupported()) {
		type = guessContentTypeFromStream(is);
	    }
	    if (type != null) {
		msgh.add("content-type", type);
	    }
	} catch (FileNotFoundException e) {
	    try {
		cd(fullpath);
		/* if that worked, then make a directory listing
		   and build an html stream with all the files in
		   the directory */
		ftp.ascii();
		
		is = ftp.list();
		msgh.add("content-type", "text/plain");
	    } catch (IOException ex) {
		throw new FileNotFoundException(fullpath);
	    }
	}
	setProperties(msgh);
	return is;
    }

    /**
     * Get the OutputStream to store the remote file. It will issue the
     * "put" command to the ftp server.
     *
     * @return	the <code>OutputStream</code> to the connection.
     *
     * @throws	IOException if already opened for input or the URL
     *		points to a directory
     * @throws	FtpProtocolException if errors occur during the transfert.
     */
    public OutputStream getOutputStream() throws IOException {
	if (!connected) {
	    connect();
	}

	if (is != null)
	    throw new IOException("Already opened for input");

	if (os != null) {
	    return os;
	}

	decodePath(url.getPath());
	if (filename == null || filename.length() == 0)
	    throw new IOException("illegal filename for a PUT");
	if (pathname != null)
	    cd(pathname);
	if (type == ASCII)
	    ftp.ascii();
	else
	    ftp.binary();
	os = new FtpOutputStream(ftp, ftp.put(filename));
	return os;
    }

    String guessContentTypeFromFilename(String fname) {
	return guessContentTypeFromName(fname);
    }

    /**
     * Gets the <code>Permission</code> associated with the host & port.
     *
     * @return	The <code>Permission</code> object.
     */
    public Permission getPermission() {
	if (permission == null) {
	    int port = url.getPort();
	    port = port < 0 ? FtpClient.FTP_PORT : port;
	    String host = this.host + ":" + port;
	    permission = new SocketPermission(host, "connect");
	}
	return permission;
    }

    /**
     * Sets the general request property. If a property with the key already
     * exists, overwrite its value with the new value.
     *
     * @param   key     the keyword by which the request is known
     *                  (e.g., "<code>accept</code>").
     * @param   value   the value associated with it.
     * @throws IllegalStateException if already connected
     * @see #getRequestProperty(java.lang.String)
     */
    public void setRequestProperty(String key, String value) {
	super.setRequestProperty (key, value);
	if ("type".equals (key)) {
            if ("i".equalsIgnoreCase(value))
                type = BIN;
            else if ("a".equalsIgnoreCase(value))
                type = ASCII;
	    else
		if ("d".equalsIgnoreCase(value))
		    type = DIR;
            else
                throw new IllegalArgumentException(
		    "Value of '" + key + 
		    "' request property was '" + value + 
		    "' when it must be either 'i', 'a' or 'd'");
        }
    }
    
    /**
     * Returns the value of the named general request property for this
     * connection.
     *
     * @param key the keyword by which the request is known (e.g., "accept").
     * @return  the value of the named general request property for this
     *           connection.
     * @throws IllegalStateException if already connected
     * @see #setRequestProperty(java.lang.String, java.lang.String)
     */
    public String getRequestProperty(String key) {
	String value = super.getRequestProperty (key);
	
        if (value == null) {
            if ("type".equals (key))
                value = (type == ASCII ? "a" : type == DIR ? "d" : "i");
        }
	
        return value;
    }
}
