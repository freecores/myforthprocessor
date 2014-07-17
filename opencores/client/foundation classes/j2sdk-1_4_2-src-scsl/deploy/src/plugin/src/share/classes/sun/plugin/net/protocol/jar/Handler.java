/*
 * @(#)Handler.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.jar;

import java.net.URL;
import java.io.IOException;

/*
 * Jar URL Handler for plugin
 */
public class Handler extends sun.net.www.protocol.jar.Handler 
{
    protected java.net.URLConnection openConnection(URL u)
    throws IOException {
        return new CachedJarURLConnection(u, this);
    }
}
