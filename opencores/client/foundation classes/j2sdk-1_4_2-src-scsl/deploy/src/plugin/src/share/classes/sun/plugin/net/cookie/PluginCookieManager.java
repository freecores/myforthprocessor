/*
 * @(#)PluginCookieManager.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;
import java.util.HashMap;
import sun.plugin.util.Trace;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.services.BrowserService;
import sun.plugin.services.ServiceProvider;
import sun.plugin.services.ServiceUnavailableException;


/**
 * <p> PluginCookieManager is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL.
 * </p>
 */
public class PluginCookieManager 
{
    /**
     * <p> Cookie cache. </p>
     */
    private static HashMap cookieTable = new HashMap();

    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param u URL
     * @param String contains the corresponding cookie value.
     */
    public static synchronized void setCookieInfo(URL u, String value)
    {
	initialize();	

	Trace.msgNetPrintln("cookiehandler.server", new Object[] {u, value});

	try
	{
	    cookieHandler.setCookieInfo(u.toString(), value);
	}
	catch (ServiceUnavailableException e)
	{
	    System.out.println(ResourceHandler.getMessage("cookiehandler.ignore.setcookie"));
	}
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param u URL
     * @returns String contains the corresponding cookie value.
     */
    public static synchronized String getCookieInfo(URL u)
    {
	initialize();

	String cookie = null;

	try
	{
	    // Find key to lookup cookie table
	    String key = u.getProtocol() + u.getHost() + u.getFile();

	    // To lookup the cookie, just the URL hostname and path without filename
	    int index = key.lastIndexOf('/');

	    if (index < 0)
		return null;

	    key = key.substring(0, index);

	    try
	    {
		cookie = cookieHandler.getCookieInfo(u.toString());

		// Store cookie into cache
		//
		if (cookie != null && !cookie.equals("") && !cookie.equals("\n") && !cookie.equals("\r\n"))
		    cookieTable.put(key, cookie);
		else  {
		    cookieTable.put(key, "");
		    cookie = null;
		}
	    }
	    catch (ServiceUnavailableException se)
	    {
		System.out.println(ResourceHandler.getMessage("cookiehandler.noservice"));

		// Obtain cookie from cache
		cookie = (String) cookieTable.get(key);
	    }

	    if (cookie != null)
	    {
		Trace.msgNetPrintln("cookiehandler.connect", new Object[] {u, cookie});
	    }
	}
	catch (Throwable e)
	{
	    e.printStackTrace();
	}

        return cookie;
    }

  
    // Default cookie handler
    private static CookieHandler cookieHandler = null;
          
    /**
     * Returns default cookie handler.
     */
    private static void initialize()
    {
	if (cookieHandler == null)
	{
	    BrowserService service = ServiceProvider.getService();
	    cookieHandler = service.getCookieHandler();
	}
    }
}


