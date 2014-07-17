/*
 * @(#)NSPreferences.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.net.proxy;

import java.io.IOException;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.BufferedReader;
import java.net.URL;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.List;
import java.util.StringTokenizer;
import sun.plugin.util.Trace;


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
class NSPreferences
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
        while (st.hasMoreTokens()) 
	{
	    // Notice that there is no wildcard specified in the list,
	    // so we add it manually.
	    //	    
            list.add("*" + st.nextToken());
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
     * file ("preferences.js" on Unix) and return an BrowserProxyInfo object
     * that contains whatever information was found.
     */
    protected static void parseFile(File file, BrowserProxyInfo info) 
    {
        BufferedReader in = null;
        try 
	{
	    in = new BufferedReader(new InputStreamReader(new FileInputStream(file), "ISO-8859-1"));           
            String line;

            while((line = in.readLine()) != null)
	    {
                if (!line.startsWith("user_pref")) {
                    continue;
                }
                else if (isKeyword(line, "network.proxy.type")) {
    		    Trace.msgNetPrintln("net.proxy.browser.proxyEnable", new Object[] {new Integer(parseInt(line))});
                    info.setType(parseInt(line));
                }
                else if (isKeyword(line, "network.proxy.http")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.http=" + host);

		    info.setHttpHost(host);
                }
                else if (isKeyword(line, "network.proxy.http_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.http_port=" + port);
                    info.setHttpPort(port);
                }
                else if (isKeyword(line, "network.proxy.ssl")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.ssl=" + host);
		    info.setHttpsHost(host);
                }
                else if (isKeyword(line, "network.proxy.ssl_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.ssl_port=" + port);
                    info.setHttpsPort(port);
                }
                else if (isKeyword(line, "network.proxy.ftp")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.ftp=" + host);
		    info.setFtpHost(host);
                }
                else if (isKeyword(line, "network.proxy.ftp_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.ftp_port=" + port);
                    info.setFtpPort(port);
                }
                else if (isKeyword(line, "network.proxy.gopher")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.gopher=" + host);
		    info.setGopherHost(host);
                }
                else if (isKeyword(line, "network.proxy.gopher_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.gopher_port=" + port);
                    info.setGopherPort(port);
                }
                else if (isKeyword(line, "network.proxy.socks")) {
		    String host = parseString(line);
		    try 
		    {
			URL u = new URL(host);
			host = u.getHost();
		    } catch (MalformedURLException mue) {
		    }

    		    Trace.netPrintln("    network.proxy.socks=" + host);
		    info.setSocksHost(host);
                }
                else if (isKeyword(line, "network.proxy.socks_port")) 
		{
		    int port = parseInt(line);
    		    Trace.netPrintln("    network.proxy.socks_port=" + port);
                    info.setSocksPort(port);
                }
                else if (isKeyword(line, "network.proxy.no_proxies_on")) 
		{
		    Trace.msgNetPrintln("net.proxy.browser.proxyOverride", new Object[] {parseString(line)});
                    info.setOverrides(parseList(line));
                }
                else if (isKeyword(line, "network.proxy.autoconfig_url")) 
		{
		    String autoConfigURL = parseString(line);
		    Trace.msgNetPrintln("net.proxy.browser.autoConfigURL", new Object[] {autoConfigURL});
                    info.setAutoConfigURL(autoConfigURL);
                }
            }
            in.close();
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

    /**
     * Return the location of the "prefs.js" user profile file in the
     * netscape registry or null if we can't figure that out.  This method
     * should work with versions 6 of Navigator.
     */
     static File getNS6PrefsFile(File registryFile) throws IOException
    {
       	NSRegistry reg = new NSRegistry().open(registryFile);
	String path = null;
	String currProfileName = null;

	// Get current user profile directory
	if (reg != null) 
	{
	    currProfileName = reg.get("Common/Profiles/CurrentProfile");
	    if (currProfileName != null)
	    {
		path = reg.get("Common/Profiles/" + currProfileName + "/directory");
	    }
	    reg.close();
	}

	if (path == null)
	{
	    throw new IOException();
	}
	else
	    return new File(path, "prefs.js");
    }
}

