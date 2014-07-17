/*
 * @(#)AutoConfigProxy.java	1.8 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;

import java.util.*;
import java.io.*;
import java.net.*;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.net.*;
import com.sun.javaws.JavawsFactory;


public class AutoConfigProxy {
    private static String _proxyFromConfigScript = null;
     
    static synchronized String getAutoConfigProxy(URL u) {
	if (_proxyFromConfigScript != null) {
	    return _proxyFromConfigScript;
	}
	try {
	    HttpRequest httpreq = JavawsFactory.getHttpRequestImpl(); 
	    HttpResponse response = httpreq.doGetRequest(u);
	    BufferedReader bf =
		new BufferedReader(new RemoveCommentReader(new InputStreamReader(response.getInputStream())));
	    StringWriter sw = new StringWriter();
	    char [] buffer = new char[4096];
	    int numchars;
	    while ((numchars = bf.read(buffer)) != -1) {
		sw.write(buffer, 0, numchars);
	    }
	    try { bf.close(); } catch (IOException ioe) { }
	    try { sw.close(); } catch (IOException ioe2) { }
       
	    String line = sw.toString();
	    if (line != null) {
		StringTokenizer st = new StringTokenizer(line, ";", false);
		while (st.hasMoreTokens()) {
		    String pattern = st.nextToken();
		    int i = pattern.indexOf("PROXY");
		    int j = pattern.lastIndexOf("\"");
		    if (i >= 0) {
			if (j <= i) {
			    _proxyFromConfigScript =
				new String(pattern.substring(i+6));
			} else {
			    _proxyFromConfigScript =
				new String(pattern.substring(i+6, j));
			}
			break;
		    }
		}
	    }
	} catch (IOException ioe) {
	    Debug.ignoredException(ioe);
	}
	return _proxyFromConfigScript;
    }
    
    //
    // Parse proxy string of the form "protocol://host:port" or "host:port"
    //
    static void parseProxyAddress(String adString, InternetProxyInfo info) {
	URL u;
	String addressString = null;
	try {
	    u = new URL(adString);
	    addressString = new String(u.getHost() + ":" + u.getPort());
	} catch (MalformedURLException mue) {
	    addressString = new String(adString);
	}
	StringTokenizer st = new StringTokenizer(addressString, ":");
	if (st.hasMoreTokens()) {
	    info.setHTTPHost(st.nextToken());
	}
	if (st.hasMoreTokens()) {
	    try {
		info.setHTTPPort(Integer.parseInt(st.nextToken()));
	    } catch (NumberFormatException exc) { }
	}
    }
}
