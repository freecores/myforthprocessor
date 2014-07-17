/*
 * @(#)Handler.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 *	HTTP stream opener,
 */

package sun.plugin.net.protocol.http;

import java.io.IOException;
import java.net.URL;
import sun.plugin.net.protocol.http.HttpURLConnection;


/**
 * Open an http input stream given a URL 
 */
public class Handler extends sun.net.www.protocol.http.Handler {

    /*
     * <p>
     * We use our protocol handler for JDK 1.2 to open the connection for 
     * the specified URL
     * </p>
     * 
     * @param URL the url to open
     */
    protected java.net.URLConnection openConnection(URL u) throws IOException {
        return new HttpURLConnection(u, this);
    }
}
