/*
 * @(#)Handler.java	1.46 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	FTP stream opener
 */

package sun.net.www.protocol.ftp;

import java.io.IOException;
import java.net.URL;
import java.util.Map;
import java.util.HashMap;
import sun.net.ftp.FtpClient;
import sun.net.www.protocol.http.HttpURLConnection;

/** open an ftp connection given a URL */
public class Handler extends java.net.URLStreamHandler {

    protected int getDefaultPort() {
        return 21;
    }

    protected boolean equals(URL u1, URL u2) {
	String userInfo1 = u1.getUserInfo();
	String userInfo2 = u2.getUserInfo();
	return super.equals(u1, u2) && 
	    (userInfo1 == null? userInfo2 == null: userInfo1.equals(userInfo2));
    }

    protected java.net.URLConnection openConnection(URL u) 
    throws IOException {
	/* if set for proxy usage then go through the http code to get */
	/* the url connection. Bad things will happen if a user and
	 * password are specified in the ftp url */
	if (FtpClient.getUseFtpProxy()) {
	    String host = FtpClient.getFtpProxyHost();
	    String urlHost = u.getHost().toLowerCase();
	   
  	    // see if the host is on dontProxy list 
	    if (!FtpClient.matchNonProxyHosts(urlHost) &&
		host != null &&
	        host.length() > 0) {
		return new HttpURLConnection(u, host,
					     FtpClient.getFtpProxyPort());
	    }
	}
	/* make a direct ftp connection */
	return new FtpURLConnection(u);
    }
}








