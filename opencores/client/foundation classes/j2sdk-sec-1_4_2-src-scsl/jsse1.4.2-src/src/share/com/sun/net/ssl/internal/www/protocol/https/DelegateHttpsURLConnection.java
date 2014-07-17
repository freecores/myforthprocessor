/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package com.sun.net.ssl.internal.www.protocol.https;

import java.net.URL;
import java.io.IOException;
import java.util.Collection;
import java.util.List;
import java.util.Iterator;

import java.security.Principal;
import java.security.cert.*;

import javax.security.auth.x500.X500Principal;

import sun.security.util.HostnameChecker;
import sun.security.util.DerValue;
import sun.security.x509.X500Name;

import sun.net.www.protocol.https.AbstractDelegateHttpsURLConnection;

/**
 * This class was introduced to provide an additional level of
 * abstraction between javax.net.ssl.HttpURLConnection and
 * com.sun.net.ssl.HttpURLConnection objects. <p>
 * 
 * javax.net.ssl.HttpURLConnection is used in the new sun.net version
 * of protocol implementation (this one)
 * com.sun.net.ssl.HttpURLConnection is used in the com.sun version.
 *
 * @version 1.6 06/24/03
 */
class DelegateHttpsURLConnection extends AbstractDelegateHttpsURLConnection {

    // we need a reference to the HttpsURLConnection to get
    // the properties set there
    private com.sun.net.ssl.HttpsURLConnection httpsURLConnection;

    DelegateHttpsURLConnection(URL url, 
	    sun.net.www.protocol.http.Handler handler, 
	    com.sun.net.ssl.HttpsURLConnection httpsURLConnection)
	    throws IOException {
	super(url, handler);
	this.httpsURLConnection = httpsURLConnection;
    }
    
    protected javax.net.ssl.SSLSocketFactory getSSLSocketFactory() {
        return httpsURLConnection.getSSLSocketFactory();
    }
    
    protected javax.net.ssl.HostnameVerifier getHostnameVerifier() {
        // note: getHostnameVerifier() never returns null
        return new VerifierWrapper(httpsURLConnection.getHostnameVerifier());
    }
}

class VerifierWrapper implements javax.net.ssl.HostnameVerifier {

    private com.sun.net.ssl.HostnameVerifier verifier;

    VerifierWrapper(com.sun.net.ssl.HostnameVerifier verifier) {
	this.verifier = verifier;
    }
    
    /*
     * In com.sun.net.ssl.HostnameVerifier the method is defined
     * as verify(String urlHostname, String certHostname).
     * This means we need to extract the hostname from the certificate
     * in this wrapper
     */
    public boolean verify(String hostname, javax.net.ssl.SSLSession session) {
	try {
	    Certificate[] serverChain = session.getPeerCertificates();
	    if ((serverChain == null) || (serverChain.length == 0)) {
	        return false;
	    }
	    if (serverChain[0] instanceof X509Certificate == false) {
	        return false;
	    }
	    X509Certificate serverCert = (X509Certificate)serverChain[0];
	    String serverName = getServername(serverCert);
	    if (serverName == null) {
	        return false;
	    }
	    return verifier.verify(hostname, serverName);
	} catch (javax.net.ssl.SSLPeerUnverifiedException e) {
	    return false;
	}
    }
    
    /*
     * Extract the name of the SSL server from the certificate.
     *
     * Note this code is essentially a subset of the hostname extraction
     * code in HostnameChecker.
     */
    private static String getServername(X509Certificate peerCert) {
	try {
	    // compare to subjectAltNames if dnsName is present
	    Collection subjAltNames = peerCert.getSubjectAlternativeNames();
	    if (subjAltNames != null) {
		for (Iterator itr = subjAltNames.iterator(); itr.hasNext(); ) {
		    List next = (List)itr.next();
		    if (((Integer)next.get(0)).intValue() == 2) {
			// compare dNSName with host in url
			String dnsName = ((String)next.get(1));
			return dnsName;
		    }
		}
	    }

	    // else check against common name in the subject field
	    X500Name subject = HostnameChecker.getSubjectX500Name(peerCert);

	    DerValue derValue = subject.findMostSpecificAttribute
						(X500Name.commonName_oid);
	    if (derValue != null) {
		try {
		    String name = derValue.getAsString();
		    return name;
		} catch (IOException e) {
		    // ignore
		}
	    }
	} catch (java.security.cert.CertificateException e) {
	    // ignore
	}
	return null;
    }

}
