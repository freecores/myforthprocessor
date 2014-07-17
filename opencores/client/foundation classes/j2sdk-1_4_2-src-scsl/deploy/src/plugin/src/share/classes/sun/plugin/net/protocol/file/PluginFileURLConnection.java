/*
 * @(#)PluginFileURLConnection.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.net.protocol.file;

import java.net.URL;
import java.io.File;
import java.io.InputStream;
import sun.plugin.util.ProgressInputStream;


public class PluginFileURLConnection extends sun.net.www.protocol.file.FileURLConnection 
{
    PluginFileURLConnection(URL u, File f)
    {
        super(u, f);        
    }

    /*
     * This method will return the ProgressInputStream to track progress
     * while loading classes from file url if progress bar should be 
     * displayed. 
     */
    protected InputStream chainDecorator(InputStream s) 
    {
    	return new ProgressInputStream(s, this);
    }
}
