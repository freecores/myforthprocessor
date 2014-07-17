/*
 * @(#)Handler.java	1.17 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.gopher;

import java.net.URL;
import java.io.IOException;
import sun.plugin.net.protocol.http.HttpURLConnection;
import sun.plugin.net.proxy.ProxyInfo;
import sun.plugin.net.proxy.PluginProxyManager;


/**
 * A class to handle the gopher protocol.
 */

public class Handler extends sun.net.www.protocol.gopher.Handler {

    public java.net.URLConnection openConnection(URL u) throws IOException {
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
		return super.openConnection(u);
	    }
        } catch(IOException e)  {
	    return super.openConnection(u);
	}
    }
}


