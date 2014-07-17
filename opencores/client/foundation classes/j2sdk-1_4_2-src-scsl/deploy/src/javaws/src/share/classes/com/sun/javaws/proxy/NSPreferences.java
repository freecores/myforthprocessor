/*
 * @(#)NSPreferences.java	1.14 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.proxy;

import java.util.*;
import java.io.*;
import java.net.*;
import com.sun.javaws.debug.Debug;


/**
 * Configuration information about Netscape 4.x, notably the proxy addresses,
 * is stored in a JavaScript format file on both Windows and Unix.  Each
 * line in the file is a call to the user_pref function, here's an
 * example:
 * <pre>
 * user_pref("network.proxy.http", "webcache-cup.eng");
 * </pre>
 * This class supports extracting the proxy related entries from this file,
 * and initializing an InternetProxyInfo object.  It could be used like this
 * to print the current NS proxy settings on Unix:
 * <pre>
 * InternetProxyInfo info = new InternetProxyInfo();
 * NSPreferences.parse(new File("/home/foo/.netscape/preferences.js"), info);
 * System.out.println(info);
 *
 * @version 1.5, 02/09/01
 */
public class NSPreferences
{
    /**
     * Each line in the preferences file should look like this:
     * user_pref("foo.bar.baz", <value>);
     * We extract the value part of the line here; assuming that
     * the "," isn't legal as part of the (foo.bar.baz) path.
     */
    private static String parseValue(String line) {
        int i1 = line.indexOf(",");
        if (i1 != -1) {
            int i2 = line.lastIndexOf(")");
            if ((i2 != -1) && ((i1 + 1) < i2)) {
                return line.substring(i1 + 1, i2).trim();
            }
        }
        return null;
    }
    
    
    /**
     * Return the value part of the line with the leading and trailing
     * quotes removed.
     */
    private static String parseString(String line) {
        String value = parseValue(line);
        if ((value != null) && (value.length() > 1) && value.startsWith("\"") && value.endsWith("\"")) {
            return value.substring(1, value.length() - 1);
        }
        else {
            return null;
        }
    }
    
    
    /**
     * The value part of the line is assumed to be an unadorned base 10
     * integer. Return it or return -1 if the value can't be parsed.
     */
    private static int parseInt(String line) {
        String value = parseValue(line);
        if (value != null) {
            try {
                return Integer.parseInt(value);
            }
            catch (NumberFormatException e) {
            }
        }
        return -1;
    }
    
    
    /**
     * The value part of the line is assumed to be a string that contains
     * a comma separated list off tokens.
     */
    private static List parseList(String line) {
        StringTokenizer st = new StringTokenizer(parseString(line), ", ");
        ArrayList list = new ArrayList();
        while (st.hasMoreTokens()) {
            list.add(st.nextToken());
        }
        return list;
    }
    
    
    /**
     * Each line in the preferences file should look like this:
     * user_pref("keyword", <value>);
     * Return true if keyword matches the keyword on this line.  We're
     * assuming that there's no space between the open paren and the
     * open quote on the left side of the keyword.
     */
    private static boolean isKeyword(String line, String keyword) {
        int i = line.indexOf("(");
        return (i != -1) && line.substring(i+1, line.length()).startsWith("\"" + keyword + "\"");
    }
    
    
    /**
     * Extract the proxy information from the specified "prefs.js" JavaScript
     * file ("preferences.js" on Unix) and return an InternetProxyInfo object
     * that contains whatever information was found.
     */
    static void parseFile(File file, InternetProxyInfo info, float version) {
        BufferedReader in = null;
        try {
	    in = new BufferedReader(new InputStreamReader(new FileInputStream(file), "ISO-8859-1"));           
            String line;
            int httpPort=-1;
	    int proxyType = -1;
            List httpOverrides=null;
            String httpHost=null, autoConfigURL=null;
            while((line = in.readLine()) != null){	
                if (!line.startsWith("user_pref")) {
                    continue;
                }
                else if (isKeyword(line, "network.proxy.type")) {
		    proxyType = parseInt(line);
                    info.setType(proxyType);
                }
                else if (isKeyword(line, "network.proxy.http")) {
                    //info.setHTTPHost(parseString(line));
		    String rawHost = parseString(line);
		    URL u = null;
		    try {
			u = new URL(rawHost);
			httpHost = new String(u.getHost());
		    } catch (MalformedURLException mue) {
			httpHost = new String(rawHost);
		    }
                }
                else if (isKeyword(line, "network.proxy.http_port")) {
                    //info.setHTTPPort(parseInt(line));
                    httpPort = parseInt(line);
                }
                else if (isKeyword(line, "network.proxy.no_proxies_on")) {
                    //info.setOverrides(parseList(line));
                    httpOverrides = parseList(line);
                }
                else if (isKeyword(line, "network.proxy.autoconfig_url")) {
                    //info.setAutoConfigURL(parseString(line));
                    autoConfigURL = parseString(line) ;
                }
            }
            in.close();	   

	    // No special cases here for Solaris because we can't tell
	    // the difference between 4.76 and 4.79 and the proxyType
	    // settings are going to be different between those.
	    // NS 6 & 7 proxy detection should behave correctly on
	    // Solaris but 4.76 and 4.79 are going to show the proxy
	    // dialog even if NONE is selected in the browser.

	    // Linux and netscape 4 and netscape 6.2
	    // proxy type can be -1 (None), 1 (manual) and 2 (auto)
	    // on linux, proxyType == -1 implies NONE
	    if (proxyType == -1 && System.getProperty("os.name").equals("Linux") && version >= 4) {
		info.setType(InternetProxyInfo.NONE);
		return;
	    }
	    
	    // Windows and netscape 4.x
	    // type can be -1 (none), 1 (manual), 2 (auto)
	    if (proxyType == -1 && System.getProperty("os.name").indexOf("Windows") != -1 && version >= 4 && version < 5) {
		info.setType(InternetProxyInfo.NONE);
		return;
	    }
	    
	    // Windows and netscape 6.2 & 7
	    // type can be 0 (none), -1 (manual), 2 (auto)
	    if (httpHost != null && httpPort == -1 && System.getProperty("os.name").indexOf("Windows") != -1 && version >= 6) {
		// netscape 6.2 (windows) defaults to port 8080 if nothing specified
		httpPort = 8080;
	    }
	    if (httpHost != null && httpPort != -1 && proxyType == -1 && System.getProperty("os.name").indexOf("Windows") != -1 && version >= 6) {
		// in netscape 6.2 (windows), if it did NOT specify proxy type
		// and there IS proxy server info, it means MANUAL
		proxyType = 1;
		info.setType(InternetProxyInfo.MANUAL);
	    }	

	    // work around a bug in Netscape 4.79, 6, 7 and who knows
	    // what other versions of netscape.  If we stil have not figured
	    // out what the proxy is supposed to be, and the autoconfig file
	    // exists, make the type autoconfig.  This way, the proxy dialog
	    // will popup with hints from the autoconfig file.
	    if (proxyType == -1 && autoConfigURL != null) {
		proxyType = com.sun.javaws.proxy.InternetProxyInfo.AUTO;
		info.setType(proxyType);
	    }

	    // if we still haven't figured out the proxy type for sure,
	    // set the info to -1 so that we will popup the proxy dialog
	    if (proxyType == -1) {
		info.setType(proxyType);
	    }

	    if (info.getType() !=com.sun.javaws.proxy.InternetProxyInfo.AUTO) {
		info.setHTTPHost(httpHost);
		info.setHTTPPort(httpPort);
		info.setOverrides(httpOverrides);
	    } else {
		if (com.sun.javaws.debug.Globals.TraceProxies == true) {
		    Debug.println("autoConfigURL: " + autoConfigURL);
		}
		// check if it's a regular filename first
		File f = new File(autoConfigURL);
		if (f != null & f.exists()) {
		    autoConfigURL = "file:" + autoConfigURL;
		}
                info.setAutoConfigURL(autoConfigURL);
		URL u = null;
		try {
		    u = new URL(autoConfigURL);
		} catch (MalformedURLException me) {
		    try {
			u = new URL("http://" + autoConfigURL);
		    } catch (MalformedURLException me2) {
			Debug.ignoredException(me2);
		    }
		}
		if (u != null) {
		    String s = AutoConfigProxy.getAutoConfigProxy(u);
		    if (s != null) {
			InternetProxyInfo temp = new InternetProxyInfo();
			AutoConfigProxy.parseProxyAddress(s, temp);
			if (temp.getHTTPHost() != null) {
			    info.setInvalidHTTPHost(temp.getHTTPHost());
			    info.setInvalidHTTPPort(temp.getHTTPPort());
			}
			info.setOverrides(httpOverrides);
		    }
		}
	    }
	}
        catch (IOException exc1) {
            if (in != null) {
                try {
                    in.close();
                }
                catch (IOException exc2) {
                }
            }
        }
    }
}

