/*
 * @(#)RootCACertificateStore.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.PrivilegedExceptionAction;
import java.security.PrivilegedActionException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;

/**
 * RootCACertificateStore is a class that represents the certificate 
 * stores which contains all the root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public class RootCACertificateStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
	// Get root CA file cacerts
    	_filename = System.getProperty("java.home") + File.separator + "lib" + File.separator + "security" + File.separator + "cacerts";
    }

    // Collection of root CA cert keystore
    private KeyStore _cacerts = null;

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.loading" + new Object[] {_filename});
	}
	try
	{     
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    if (_cacerts == null)
		    {
			// Create keystore first
			_cacerts = KeyStore.getInstance("JKS");

			// Initialize the keystore
			_cacerts.load(null, null);
		    }		   

		    File rootCAFile = new File(_filename);

		    // Only load the root CA store if exists.
		    if (rootCAFile.exists())
		    {
		    	FileInputStream fis = new FileInputStream(rootCAFile);		    
		    	BufferedInputStream bis = new BufferedInputStream(fis);

			// Initialize the keystore
		    	_cacerts.load(bis, null);

		    	bis.close();
		    	fis.close();
		    }
		    else
		    {
			if (Globals.TraceHttpsCert) {
			    Debug.println("rootcertstore.cert.noload" + new Object[] {_filename});
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
		e.printStackTrace();
	}
	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.loaded" + new Object[] {_filename});
	}
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.saving" + new Object[] {_filename});
	}
	// We don't persist anything into the root CA store.

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
	    Debug.println("rootcertstore.cert.adding");
	}
	// We don't add any certificate into the root CA store.

	throw new KeyStoreException("Cannot add root CA certificate");
    }


    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public void remove(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.removing");
	}
	// We don't remove any certificate into the root CA store.

	throw new KeyStoreException("Cannot remove root CA certificate");
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
	    Debug.println("rootcertstore.cert.instore");
	}
	// We don't compare any certificate into the root CA store.

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
	    Debug.println("rootcertstore.cert.canverify");
	}

	StringBuffer sb = new StringBuffer();

	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.tobeverified" + new Object[] {cert});
	}

	// Enumerate each root CA certificate in the root store
	Enumeration enum = _cacerts.aliases();
 
	while (enum.hasMoreElements())
	{
	    String alias = (String) enum.nextElement();

	    Certificate rootCert = _cacerts.getCertificate(alias);

	    if (Globals.TraceHttpsCert) {
		Debug.println("rootcertstore.cert.tobecompared" + new Object[] {rootCert});
	    }

	    try
	    {
    		cert.verify(rootCert.getPublicKey());
		if (Globals.TraceHttpsCert) {
		    Debug.println("rootcertstore.cert.verify.ok");
		}
		return true;
	    }
	    catch (Exception e)
	    {
		// Ignore exception
	    }
	}
	if (Globals.TraceHttpsCert) {
	    Debug.println("rootcertstore.cert.verify.fail");
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
	    Debug.println("rootcertstore.cert.iterator");
	}
        HashSet certCollection = new HashSet();
        Enumeration enum = _cacerts.aliases();

        while (enum.hasMoreElements())
        {
            // Get certificate alias from iterator
            String alias = (String) enum.nextElement();

            // Get certificate from store
            Certificate cert = _cacerts.getCertificate(alias);

            // Add certificate into collection
            certCollection.add(cert);
        }              

        return certCollection.iterator();
    }
}
