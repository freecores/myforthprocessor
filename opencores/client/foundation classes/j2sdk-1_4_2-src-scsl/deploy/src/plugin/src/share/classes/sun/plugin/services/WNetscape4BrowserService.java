/*
 * @(#)WNetscape4BrowserService.java	1.25 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;


/** 
 * WNetscape4BrowserService is a class that encapsulates the browser service
 * in Netscape 4 on Win32.
 */
public final class WNetscape4BrowserService implements BrowserService
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
	return new sun.plugin.net.proxy.WNetscape4ProxyConfig();
    }

    /**
     * Return applet context
     */
    public sun.plugin.viewer.context.PluginAppletContext getAppletContext()
    {
	return new sun.plugin.viewer.context.NetscapeAppletContext();
    }

    /**
     * Return beans context
     */
    public sun.plugin.viewer.context.PluginBeansContext getBeansContext()
    {
	sun.plugin.viewer.context.PluginBeansContext pbc = new sun.plugin.viewer.context.PluginBeansContext();
	pbc.setPluginAppletContext(new sun.plugin.viewer.context.NetscapeAppletContext());

	return pbc;
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
	return false;
    }

    public native boolean installBrowserEventListener();
    
    /**
     * Browser Authenticator
     * @since 1.4.2
     */
    public BrowserAuthenticator getBrowserAuthenticator() {
	return null;
    }
}

