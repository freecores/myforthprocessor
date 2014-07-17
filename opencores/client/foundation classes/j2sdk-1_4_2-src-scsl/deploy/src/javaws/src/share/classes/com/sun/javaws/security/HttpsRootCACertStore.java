/*
 * @(#)HttpsRootCACertStore.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;
import java.util.Iterator;
import java.util.HashSet;
import java.util.Enumeration;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;

/**
 * HttpsRootCACertStore is a class that represents the certificate 
 * stores which contains all the Https root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public class HttpsRootCACertStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
	// Get Https root CA file jssecacerts 
    	_filename =  System.getProperty("java.home") + File.separator + "lib" + File.separator + "security" + File.separator + "jssecacerts";
    }

    // Collection of Https root CA cert keystore
    private KeyStore _jssecacerts = null;

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	try
	{     
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    if (_jssecacerts == null)
		    {
			// Create keystore first
			_jssecacerts = KeyStore.getInstance("JKS");

			// Initialize the keystore
			_jssecacerts.load(null, null);
		    }

		    File jsseFile = new File(_filename);

		    // Only load the jssecacerts if it exits.
		    if (jsseFile.exists())
		    {
		    	FileInputStream fis = new FileInputStream(jsseFile);		    
		    	BufferedInputStream bis = new BufferedInputStream(fis);

		    	// Initialize the keystore
		   	_jssecacerts.load(bis, null);

		    	bis.close();
		    	fis.close();
		    }
		    else
		    {
			if (Globals.TraceHttpsCert) {
			    Debug.println("roothttpscertstore.cert.noload" + new Object[] { _filename });
			}
		    }

		    return null;
		}
	    });
	}
	catch (PrivilegedActionException e)
	{
	    Exception ex = e.getException();

	    if (ex instanceof IOException)
		throw (IOException)ex;
	    else if (ex instanceof CertificateException)
		throw (CertificateException)ex;
	    else if (ex instanceof KeyStoreException)
		throw (KeyStoreException)ex;
	    else if (ex instanceof NoSuchAlgorithmException)
		throw (NoSuchAlgorithmException)ex;
	    else
		ex.printStackTrace();
	}
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.loaded" + new Object[] {_filename});
	}
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.saving" + new Object[] {_filename});
	}
	// We don't persist anything into the Https root CA store.

	// Do nothing
    }


    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.adding");
	}
	// We don't add any certificate into the Https root CA store.

	throw new KeyStoreException("Cannot add Https root CA certificate");
    }


    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public void remove(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.removing");
	}
	// We don't remove any certificate into the Https root CA store.

	throw new KeyStoreException("Cannot remove Https root CA certificate");
    }


    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.instore");
	}
	// We don't compare any certificate into the Https root CA store.

	return false;
    }


    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store. 
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */ 
    public boolean verify(Certificate cert) throws KeyStoreException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.canverify");

	    Debug.println("roothttpscertstore.cert.tobeverified" + new Object[] {cert});
	}
	// Enumerate each Https root CA certificate in the Https root store
	Enumeration enum = _jssecacerts.aliases();
 
	while (enum.hasMoreElements())
	{
	    String alias = (String) enum.nextElement();

	    Certificate rootCert = _jssecacerts.getCertificate(alias);
	    
	    if (Globals.TraceHttpsCert) {
		Debug.println("roothttpscertstore.cert.tobecompared" + new Object[] {rootCert});
	    }

	    try
	    {
    		cert.verify(rootCert.getPublicKey());
		if (Globals.TraceHttpsCert) {
		    Debug.println("roothttpscertstore.cert.verify.ok");
		}
		return true;
	    }
	    catch (Exception e)
	    {		
		// Ignore exception
		Debug.ignoredException(e);
	    }
	}
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.verify.fail");
	}
	return false;
    }


    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Iterator for iterating certificates
     */
    public Iterator iterator() throws KeyStoreException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("roothttpscertstore.cert.iterator");
	}
        HashSet certCollection = new HashSet();
        Enumeration enum = _jssecacerts.aliases();

        while (enum.hasMoreElements())
        {
            // Get certificate alias from iterator
            String alias = (String) enum.nextElement();

            // Get certificate from store
            Certificate cert = _jssecacerts.getCertificate(alias);

            // Add certificate into collection
            certCollection.add(cert);
        }              

        return certCollection.iterator();
    }
}
