/*
 * @(#)Handler.java	1.47 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.file;

import java.net.InetAddress;
import java.net.URLConnection;
import java.net.URL;
import java.net.MalformedURLException;
import java.net.URLStreamHandler;
import java.io.InputStream;
import java.io.IOException;
import sun.net.www.ParseUtil;
import java.io.File;

/**
 * Open an file input stream given a URL.
 * @author	James Gosling
 * @version 	1.47, 03/01/23
 */
public class Handler extends URLStreamHandler {

    private String getHost(URL url) {
	String host = url.getHost();
	if (host == null)
	    host = "";
	return host;
    }


    protected void parseURL(URL u, String spec, int start, int limit) {
	/*
	 * Ugly backwards compatibility. Flip any file separator
	 * characters to be forward slashes. This is a nop on Unix
	 * and "fixes" win32 file paths. According to RFC 2396,
	 * only forward slashes may be used to represent hierarchy
	 * separation in a URL but previous releases unfortunately
	 * performed this "fixup" behavior in the file URL parsing code
	 * rather than forcing this to be fixed in the caller of the URL
	 * class where it belongs. Since backslash is an "unwise"
	 * character that would normally be encoded if literally intended
	 * as a non-seperator character the damage of veering away from the
	 * specification is presumably limited.
	 */
	super.parseURL(u, spec.replace(File.separatorChar, '/'), start, limit);
    }

    public synchronized URLConnection openConnection(URL u)
           throws IOException {
	String host = u.getHost();
	if (host == null || host.equals("") || host.equals("~") ||
	    host.equals("localhost")) {
	    File file = new File(ParseUtil.decode(u.getPath()));
	    return createFileURLConnection(u, file);
	} 

	/* If you reach here, it implies that you have a hostname
	   so attempt an ftp connection.
	 */
	URLConnection uc;
	URL ru;

	try {
	    ru = new URL("ftp", host, u.getFile() +
                             (u.getRef() == null ? "": "#" + u.getRef()));
	    uc = ru.openConnection();
	} catch (IOException e) {
	    uc = null;
	}
	if (uc == null) {
	    throw new IOException("Unable to connect to: " +
                                                       u.toExternalForm());
	}
	return uc;
    }

    // Template method to be overriden by Java Plug-in. [stanleyh]
    //
    protected URLConnection createFileURLConnection(URL u, File file)
    {
	return new FileURLConnection(u, file);
    }
}
