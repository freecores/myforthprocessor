/*
 * @(#)MNetscape6CookieHandler.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;
import sun.plugin.viewer.AppletPanelCache;
import sun.plugin.services.ServiceUnavailableException;


/**
 * <p> MNetscape6CookieHandler is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL in Netscape 6 on Unix.
 * </p>
 */
public final class MNetscape6CookieHandler implements CookieHandler 
{
    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @param String contains the corresponding cookie value.
     */
    public void setCookieInfo(String url, String value) 
	        throws ServiceUnavailableException
    {
	// Cookie service in Netscape 6 is only available if
	// there is at least one plugin instance exists. Otherwise,
	// accessing the cookie service without any plugin instance
	// will likely to confuse the browser and fail. Thus, it 
	// is VERY important to check if there is at least one 
	// applet instance before calling back to the browser.
	//

	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Cookie service is NOT available
	    throw new ServiceUnavailableException("Cookie service is not available for " + url);
	}

	sun.plugin.navig.motif.Worker.setCookieForURL(url, value);
    }

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public String getCookieInfo(String url)
		  throws ServiceUnavailableException
    {
	// Cookie service in Netscape 6 is only available if
	// there is at least one plugin instance exists. Otherwise,
	// accessing the cookie service without any plugin instance
	// will likely to confuse the browser and fail. Thus, it 
	// is VERY important to check if there is at least one 
	// applet instance before calling back to the browser.
	//

	if (AppletPanelCache.hasValidInstance() == false)
	{
	    // Cookie service is NOT available
	    throw new ServiceUnavailableException("Cookie service is not available for " + url);
	}

	return sun.plugin.navig.motif.Worker.findCookieForURL(url);
    }
}


