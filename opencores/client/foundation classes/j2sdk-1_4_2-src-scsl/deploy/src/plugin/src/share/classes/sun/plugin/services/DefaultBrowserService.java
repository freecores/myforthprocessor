/*
 * @(#)DefaultBrowserService.java	1.21 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;


/** 
 * DefaultBrowserService is a class that encapsulates the default browser service.
 */
public final class DefaultBrowserService implements BrowserService
{
    /**
     * Return cookie handler.
     */
    public sun.plugin.net.cookie.CookieHandler getCookieHandler()
    {
	return new sun.plugin.net.cookie.Netscape4CookieHandler();
    }

    /**
     * Return proxy config.
     */
    public sun.plugin.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return null;
    }

    /**
     * Return applet context
     */
    public sun.plugin.viewer.context.PluginAppletContext getAppletContext()
    {
	return null;
    }

    /**
     * Return beans context
     */
    public sun.plugin.viewer.context.PluginBeansContext getBeansContext()
    {
	return null;
    }

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer()
    {
	return false;
    }

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape()
    {
	return true;
    }

    /**
     * Return browser version.
     */
    public float getBrowserVersion()
    {
	return 4.0f;
    }

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose()
    {
	return true;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener() {
	return false;
    }

    /**
     * Browser Authenticator
     * @since 1.4.2
     */
    public BrowserAuthenticator getBrowserAuthenticator() {
	return null;
    }
}






