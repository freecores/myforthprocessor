/*
 * @(#)Handler.java	1.19 03/04/11
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*-
 *	HTTP stream opener
 */

package sun.plugin.net.protocol.https;

import java.io.IOException;
import java.net.URL;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.KeyStore;
import java.io.FileInputStream;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.HostnameVerifier;
import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.KeyManager;
import sun.plugin.services.*;
import sun.net.www.protocol.https.PluginHttpsURLConnection;


/** open an http input stream given a URL */
public class Handler extends sun.net.www.protocol.https.Handler 
{
    static 
    {
	try
	{
	    // Hook up TrustManager
	    java.security.AccessController.doPrivileged(new PrivilegedExceptionAction()
	    {
		public Object run() throws Exception
		{
		    // Hostname verifier
		    HostnameVerifier verifier = null;

		    // Create hostname verifier
		    verifier = new sun.plugin.security.CertificateHostnameVerifier();

		    javax.net.ssl.HttpsURLConnection.setDefaultHostnameVerifier(verifier);

		    // Get SSL context
		    SSLContext sslContext = SSLContext.getInstance("SSL");

		    // Create custom trust manager
		    TrustManager trustManager = new sun.plugin.security.X509PluginTrustManager();

		    TrustManager[] trustManagerArray = new TrustManager[1];
		    trustManagerArray[0] = trustManager; 

		    // Get keyManagers
		    KeyManager[] keyManagers = null;
		    String keyStore = System.getProperty("javax.net.ssl.keyStore");
		    if (keyStore != null)
		    {
			char[] keyPassphrase = "".toCharArray();
			String keyStorePassword = System.getProperty("javax.net.ssl.keyStorePassword");
			if (keyStorePassword != null)
			   keyPassphrase = keyStorePassword.toCharArray();

			KeyStore ks;	
			KeyManagerFactory kmf;
			kmf = KeyManagerFactory.getInstance("SunX509");

			// Get keyStoreType from env, if not specify, default to 'JKS'
			String keyStoreType = System.getProperty("javax.net.ssl.keyStoreType");
           		if (keyStoreType == null)
              		   keyStoreType = "JKS";
           		ks = KeyStore.getInstance(keyStoreType);
			ks.load(new FileInputStream(keyStore), keyPassphrase);
			kmf.init(ks, keyPassphrase);
			keyManagers = kmf.getKeyManagers();
		    }

		    // Set custom keymanager and trust manager in SSL context
		    sslContext.init(keyManagers, trustManagerArray, null);

		    javax.net.ssl.HttpsURLConnection.setDefaultSSLSocketFactory(sslContext.getSocketFactory());

		    return null;
		}
	    });
	}
	catch (PrivilegedActionException e)
	{
	    e.printStackTrace();
	}
    }


    /*
     * <p>
     * Delegate to the Https connection 
     * </p>
     */
    public java.net.URLConnection openConnection(URL u) throws IOException 
    {
	return new sun.net.www.protocol.https.PluginHttpsURLConnection(u, this);
    }
}
