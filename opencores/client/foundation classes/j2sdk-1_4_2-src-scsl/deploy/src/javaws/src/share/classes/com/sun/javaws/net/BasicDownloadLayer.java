/*
 * @(#)BasicDownloadLayer.java	1.3 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.io.*;
import java.net.URL;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.debug.Debug;

/** Implementation class for the HttpRequest and HttpResponse
 *  interfaces.
 */
public class BasicDownloadLayer implements HttpDownload {
    // Default size of download buffer
    private static final int BUF_SIZE = 32 * 1024;
       
    private HttpRequest _httpRequest;
    
    public BasicDownloadLayer(HttpRequest httpRequest) {
	_httpRequest = httpRequest;
    }
    
    /** Download resource to the given file */
    public void download(HttpResponse hr, File file, HttpDownloadListener dl)
	throws CanceledDownloadException, IOException {
	// Tell delegate about loading
	int length = hr.getContentLength();
	if (dl != null) dl.downloadProgress(0, length);
	
	if (Globals.TraceDownload)  Debug.println("Doing download");
	
	
	InputStream in = null;
	OutputStream out = null;
	try {
	    in = hr.getInputStream();
	    out  = new FileOutputStream(file);
	    int read = 0;
	    int totalRead = 0;
	    byte[] buf = new byte[BUF_SIZE];
	    while ((read = in.read(buf)) != -1) {
		out.write(buf, 0, read);
		// Notify delegate
		totalRead += read;
		if (totalRead > length && length != 0) totalRead = length;
		if (dl != null) dl.downloadProgress(totalRead, length);
	    }
	    if (Globals.TraceDownload) {
		Debug.println("Wrote URL " + hr.getRequest() + " to file " + file);
	    }
	    in.close(); in = null;
	    out.close(); out = null;
	} catch(IOException ioe) {
	    if (Globals.TraceDownload) {
		Debug.println("Got exception while downloading resource: " + ioe);
	    }
	    // Close before calling delete - otherwise it fails
	    if (in != null)  { in.close(); in = null; }
	    if (out != null) { out.close(); out = null; }
	    if (file != null) file.delete();
	    // Rethrow exception
	    throw ioe;
	}
	
	// Inform delegate about loading has completed
	if (dl != null) dl.downloadProgress(length, length);
    }
    
    /** Download resource to the given file */
    public void download(URL url, File location, HttpDownloadListener dl)
	throws CanceledDownloadException, IOException {
	HttpResponse hr = _httpRequest.doGetRequest(url);
	download(hr, location, dl);
    }
}

