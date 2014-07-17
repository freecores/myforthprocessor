/*
 * @(#)PluginHttpsURLConnection.java	1.48 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.https;

import java.io.IOException;
import java.net.URL;
import sun.net.www.protocol.https.HttpsURLConnectionImpl;

public final class PluginHttpsURLConnection extends HttpsURLConnectionImpl
{
    /**
     * Construct a PluginHttpsURLConnection object.
     * 
     * @param u URL of the connection.
     * @param handler Protocol handler.
     */
    public PluginHttpsURLConnection(URL u, Handler handler) throws IOException 
    {
	super(u);
	delegate = new PluginDelegateHttpsURLConnection(u, handler, this);
    }
}


