/*
 * @(#)Handler.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	FTP stream opener
 */

package sun.plugin.net.protocol.ftp;

import java.io.IOException;
import java.net.URL;
import java.util.Map;
import sun.plugin.net.protocol.http.HttpURLConnection;
import sun.plugin.net.proxy.ProxyInfo;
import sun.plugin.net.proxy.PluginProxyManager;
import sun.net.ftp.FtpClient;
import sun.net.www.protocol.ftp.*;

/**
 * Open an ftp connection given a URL 
 */
public class Handler extends java.net.URLStreamHandler {


    protected java.net.URLConnection openConnection(URL u) {
	/* if set for proxy usage then go through the gopher code to get
	 * the url connection.
         */
        ProxyInfo pinfo = PluginProxyManager.getProxyInfo(u);

	try  {
	    if (pinfo != null && pinfo.isProxyUsed())
	    {
		return new sun.plugin.net.protocol.http.HttpURLConnection(u, pinfo.getProxy(), pinfo.getPort());
	    }
	    else 
	    {
  		/* make a direct gopher connection */
		return superOpenConnection(u);
	    }
        } catch(IOException e)  {
	    return superOpenConnection(u);
	}
    }


    protected java.net.URLConnection superOpenConnection(URL u) {
	/* if set for proxy usage then go through the http code to get */
	/* the url connection. Bad things will happen if a user and
	 * password are specified in the ftp url */
	try  {
	    if (FtpClient.getUseFtpProxy()) {
		String host = FtpClient.getFtpProxyHost();
		if (host != null &&
		    host.length() > 0) {
		    return new sun.plugin.net.protocol.http.HttpURLConnection(u, host,
					         FtpClient.getFtpProxyPort());
		}
	    }
	} catch(IOException e)  {
	}

	/* make a direct ftp connection */
	return new FtpURLConnection(u);
    }

    // From sun.net.www.protocol.ftp.Handler
    protected int getDefaultPort() {
        return 21;
    }

    // From sun.net.www.protocol.ftp.Handler
    protected boolean equals(URL u1, URL u2) {
	String userInfo1 = u1.getUserInfo();
	String userInfo2 = u2.getUserInfo();
	return super.equals(u1, u2) && 
	    (userInfo1 == null? userInfo2 == null: userInfo1.equals(userInfo2));
    }
}
