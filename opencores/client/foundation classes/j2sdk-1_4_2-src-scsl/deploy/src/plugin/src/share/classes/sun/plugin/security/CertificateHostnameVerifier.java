/*
 * @(#)CertificateHostnameVerifier.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.util.HashSet;
import java.util.Iterator;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;
import java.text.MessageFormat;
import javax.net.ssl.SSLSession;
import javax.net.ssl.SSLPeerUnverifiedException;
import sun.plugin.resources.ResourceHandler;
import sun.plugin.util.DialogFactory;
import sun.plugin.util.Trace;
import javax.swing.JOptionPane;

/**
 * CertificateHostnameVerifier is a callback mechanism so that
 * we may supply a policy for handling the case 
 * where the host to connect to and the server name 
 * from the certificate mismatch.
 */

public final class CertificateHostnameVerifier implements javax.net.ssl.HostnameVerifier 
{
    // HashSet to store hostname/certHostname pair
    private static HashSet hashSet = new HashSet();

    public CertificateHostnameVerifier()
    {
	// no-op
    }


    /**
     * Verify that the hostname is an acceptable match with the 
     * server's received certificate(s).
     *
     * @param hostname the host name
     * @param session SSLSession used on the connection to host
     * @return true if the certificate host name is acceptable
     */
    public boolean verify(String hostname, SSLSession session)
    {
	// By default set the host name as unknown
	String certHostname = ResourceHandler.getMessage("https_dialog.unknown.host");

	try {
            // get the subject's certificate
            Certificate [] cert = session.getPeerCertificates();

            X509Certificate peerCert;
            if (cert[0] instanceof java.security.cert.X509Certificate)
                peerCert = (java.security.cert.X509Certificate) cert[0];
            else
                throw new SSLPeerUnverifiedException("");

	    // Obtain HTTPS server name
	    String certDN = peerCert.getSubjectDN().getName();
	    if(certDN != null) {
	      int cn = certDN.toUpperCase().indexOf("CN=");
	      if(cn != -1) {
		int end = certDN.indexOf(",", cn);
		if (end != -1)
		  certHostname = certDN.substring(cn+3, end);
		else
		  certHostname = certDN.substring(cn+3);
	      }
	    }

	} catch (SSLPeerUnverifiedException e) {
	    return false;
	}

	if (hostname.equalsIgnoreCase(certHostname) || certHostname.equals("*"))
	    return true;
	    
	// Check if we have seen this pair before
	//	    
	for (Iterator iter = hashSet.iterator(); iter.hasNext();)
	{
	    Object[] elements = (Object[]) iter.next();

	    if (elements[0].toString().equalsIgnoreCase(hostname)
		&& elements[1].toString().equalsIgnoreCase(certHostname))
		return true;
	}

	// No, we haven't seen it, so popup dialog
	return showHostnameMismatchDialog(hostname, certHostname);
    }


    /**
     * Ask the user to see if the hostname is an acceptable match 
     * with the value from the common name entry in the server 
     * certificate's distinguished name.
     *
     * @param hostname the host name
     * @param certHostname the common name entry from the certificate
     * @return true if the certificate host name is acceptable
     */
    private boolean showHostnameMismatchDialog(String hostname, String certHostname) {

	String title = ResourceHandler.getMessage("https_dialog.caption");

	// Extract message format
	MessageFormat mf = new MessageFormat(ResourceHandler.getMessage("https_dialog.text"));
	Object[] args = { hostname, certHostname };

	// Format message
	String message = mf.format(args);

	int result = JOptionPane.NO_OPTION;
	
	// Check if automation is enabled
	if (Trace.isAutomationEnabled() == false)
	{
	    result = DialogFactory.showConfirmDialog(message, title);
	}
	else
	{
	    Trace.msgSecurityPrintln("hostnameverifier.automation.ignoremismatch");
	    result = JOptionPane.OK_OPTION;
	}

	// Store result
	if (result == JOptionPane.OK_OPTION)
	{   
	    Object[] elements = new Object[2];
	    elements[0] = hostname;
	    elements[1] = certHostname;

	    hashSet.add(elements);
	}

	return (result == JOptionPane.OK_OPTION);
    }

    /** 
     * Reset certificate hostname verifier cache
     */
    public static void reset()
    {
	hashSet.clear();
    }
}
