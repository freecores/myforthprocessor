/*
 * @(#)BrowserProxyHandler.java	1.2 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.proxy;

import sun.plugin.navig.motif.Worker;

/**
 * Proxy handler for Netscape 6.
 */
final class BrowserProxyHandler extends BrowserProxyHandler_share
{
    /**
     * <p> method to obtain the proxy string when automatic proxy config
     * is used. </p>
     *
     * @param url URL.
     * @returns Proxy string.
     */
    String findProxyForURL(String url) {
        return Worker.getProxySettings(url);
    }
}
