/*
 * @(#)IExplorerCookieHandler.java	1.4 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.cookie;

import java.net.URL;

/**
 * <p> IExplorerCookieHandler is a class that encapsulates the mechanism for
 * obtaining the cookie value of a particular URL in Internet Explorer.
 * </p>
 */
public final class IExplorerCookieHandler implements CookieHandler 
{
    /**
     * <p> Sets the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @param String contains the corresponding cookie value.
     */
    public native void setCookieInfo(String url, String value);

    /**
     * <p> Returns the corresponding cookie value with respect to the given URL.
     * </p>
     *
     * @param url URL
     * @returns String contains the corresponding cookie value.
     */
    public native String getCookieInfo(String url);
}


