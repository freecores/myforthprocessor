/*
 * @(#)ServiceProvider.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;

import sun.plugin.util.Trace;


/**
 * ServiceProvider is a factory class to obtain different native 
 * code implementation in each platform.
 */
public class ServiceProvider
{
    // Browser service for this platform
    private static BrowserService service = new DefaultBrowserService();

    /**
     * Returns service for a particular platform.
     *
     * @return BrowserService.
     */
    public synchronized static BrowserService getService()
    {
	return service;
    }

    /**
     * Returns service for a particular platform.
     *
     * @param service Service.
     */
    public synchronized static void setService(int type)
    {
	Class c = DefaultBrowserService.class;
	
	try
	{
    	    if (type == BrowserType.INTERNET_EXPLORER_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WIExplorerBrowserService");
	    }
	    else if (type == BrowserType.NETSCAPE4_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WNetscape4BrowserService");
	    }
	    else if (type == BrowserType.NETSCAPE6_WIN32)
	    {
		c = Class.forName("sun.plugin.services.WNetscape6BrowserService");
	    }
	    else if (type == BrowserType.NETSCAPE4_UNIX)
	    {
		c = Class.forName("sun.plugin.services.MNetscape4BrowserService");
	    }
	    else if (type == BrowserType.NETSCAPE6_UNIX)
	    {
		c = Class.forName("sun.plugin.services.MNetscape6BrowserService");
	    }
	    else if (type == BrowserType.AXBRIDGE)
	    {
		c = Class.forName("sun.plugin.services.AxBridgeBrowserService");
	    }


    	    service = (BrowserService) c.newInstance();
	}
	catch (Throwable e)
	{
	    Trace.printException(e);
	}
    }
}


