/*
 * @(#)Handler.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.file;

import java.io.File;
import java.net.URL;
import java.net.URLConnection;
import sun.plugin.util.ThreadGroupLocal;


public class Handler extends sun.net.www.protocol.file.Handler 
{
    // Override template method.
    //
    protected URLConnection createFileURLConnection(URL url, File file)
    {
	if (url.toString().toUpperCase().endsWith(".CLASS") &&
	    (ThreadGroupLocal.get(ThreadGroupLocal.PROGRESS_TRACKING) != null))
	{
            return new PluginFileURLConnection(url, file);
	}
	else
	{
	    return super.createFileURLConnection(url, file);
	}
    }
}
