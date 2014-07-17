/*
 * @(#)BrowserService.java	1.20 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.services;


/** 
 * BrowserService is an interface that encapsulates the browser service.
 */
public interface BrowserService
{
    /**
     * Return cookie handler.
     */
    public sun.plugin.net.cookie.CookieHandler getCookieHandler();

    /**
     * Return proxy config.
     */
    public sun.plugin.net.proxy.BrowserProxyConfig getProxyConfig();

    /**
     * Return applet context
     */
    public sun.plugin.viewer.context.PluginAppletContext getAppletContext();

    /**
     * Return beans context
     */
    public sun.plugin.viewer.context.PluginBeansContext getBeansContext();

    /**
     * Check if browser is IE.
     */
    public boolean isIExplorer();

    /**
     * Check if browser is NS.
     */
    public boolean isNetscape();

    /**
     * Return browser version.
     */
    public float getBrowserVersion();

    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose();

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener();

    /**
     * Browser authenticator
     * @since 1.4.2
     */
    public BrowserAuthenticator getBrowserAuthenticator();
}



