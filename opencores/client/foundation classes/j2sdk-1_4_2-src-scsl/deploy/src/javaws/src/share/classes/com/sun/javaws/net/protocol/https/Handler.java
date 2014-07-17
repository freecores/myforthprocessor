/*
 * @(#)Handler.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	HTTP stream opener
 */

package com.sun.javaws.net.protocol.https;

import java.io.IOException;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.SecureRandom;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import com.sun.javaws.BrowserSupport;


//import sun.plugin.services.*;
//import sun.net.www.protocol.https.PluginHttpsURLConnection;


/** open an http input stream given a URL */
public class Handler extends sun.net.www.protocol.https.Handler 
{
    static 
    {
	try {	  

	    // Get the platform dependent Random generator
	    final SecureRandom sr = BrowserSupport.getSecureRandom();
	    
	    sr.nextInt();

	    // Hook up TrustManager
	    java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
	    {
		public Object run() throws Exception
		{
		    // Hostname verifier
		    HostnameVerifier verifier = null;
		 
		    // Create hostname verifier			
		    verifier = new com.sun.javaws.security.CertificateHostnameVerifier();
		  
		    javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(verifier);

		    // Get SSL context
		    SSLContext sslContext = SSLContext.getInstance("SSL");

		    // Create custom trust manager
		    TrustManager trustManager = new com.sun.javaws.security.X509JavawsTrustManager();

		    TrustManager[] trustManagerArray = new TrustManager[1];
		    trustManagerArray[0] = trustManager; 

		    // Set custom trust manager in SSL context	
		    sslContext.init(null, trustManagerArray, sr);

		    javax.net.ssl.HttpsURLConnection.setDefaultSSLSocketFactory(sslContext.getSocketFactory());

		    return null;
		}
	    });
	} catch (PrivilegedActionException e) {
	    e.printStackTrace();
	}
    }
}
