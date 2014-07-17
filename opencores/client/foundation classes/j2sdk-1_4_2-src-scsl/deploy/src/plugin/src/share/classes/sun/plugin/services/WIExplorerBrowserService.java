/*
 * @(#)WIExplorerBrowserService.java	1.17 02/03/09
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;


/** 
 * WIExplorerBrowserService is a class that encapsulates the browser service
 * in Internet Explorer
 */
public final class WIExplorerBrowserService implements BrowserService
{
    /**
     * Return cookie handler.
     */
    public sun.plugin.net.cookie.CookieHandler getCookieHandler()
    {
	return new sun.plugin.net.cookie.IExplorerCookieHandler();
    }

    /**
     * Return proxy config.
     */
    public sun.plugin.net.proxy.BrowserProxyConfig getProxyConfig()
    {
	return new sun.plugin.net.proxy.WIExplorerProxyConfig();
    }

    /**
     * Return applet context
     */
    public sun.plugin.viewer.context.PluginAppletContext getAppletContext()
    {
	return new sun.plugin.viewer.context.IExplorerAppletContext();
    }

    /**
     * Return beans context
     */
    public sun.plugin.viewer.context.PluginBeansContext getBeansContext()
    {
	sun.plugin.viewer.context.PluginBeansContext pbc = new sun.plugin.viewer.context.PluginBeansContext();
	pbc.setPluginAppletContext(new sun.plugin.viewer.context.IExplorerAppletContext());

	return pbc;
    }

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer()
    {
	return true;
    }

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape()
    {
	return false;
    }

    /**
     * Return browser version.
     */
    public float getBrowserVersion()
    {
	return 5.0f;
    }

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose()
    {
	return false;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public native boolean installBrowserEventListener();

    /**
     * Browser Authenticator 
     */
    public BrowserAuthenticator getBrowserAuthenticator() {
	return new WIExplorerBrowserAuthenticator();
    }
}


