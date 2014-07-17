/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package sun.net.www.protocol.https;

import java.net.URL;
import java.io.IOException;

/**
 * This class was introduced to provide an additional level of
 * abstraction between javax.net.ssl.HttpURLConnection and
 * com.sun.net.ssl.HttpURLConnection objects. <p>
 * 
 * javax.net.ssl.HttpURLConnection is used in the new sun.net version
 * of protocol implementation (this one)
 * com.sun.net.ssl.HttpURLConnection is used in the com.sun version.
 *
 * @version 1.4 06/24/03
 */
class DelegateHttpsURLConnection extends AbstractDelegateHttpsURLConnection {

    // we need a reference to the HttpsURLConnection to get
    // the properties set there
    private javax.net.ssl.HttpsURLConnection httpsURLConnection;

    DelegateHttpsURLConnection(URL url, 
	    sun.net.www.protocol.http.Handler handler, 
	    javax.net.ssl.HttpsURLConnection httpsURLConnection)
	    throws IOException {
	super(url, handler);
	this.httpsURLConnection = httpsURLConnection;
    }
    
    protected javax.net.ssl.SSLSocketFactory getSSLSocketFactory() {
        return httpsURLConnection.getSSLSocketFactory();
    }
    
    protected javax.net.ssl.HostnameVerifier getHostnameVerifier() {
        return httpsURLConnection.getHostnameVerifier();
    }
}
