/*
 * @(#)JavawsHttpsCertStore.java	1.5 03/01/23
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
import java.security.cert.X509Certificate;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Random;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;
import com.sun.javaws.ConfigProperties;

/**
 * JavawsHttpsCertStore is a class that represents the permanent certificate 
 * store which contains all the certificates that JWS recognize. It 
 * is used in the certification verification process when signed applet is 
 * encountered when you use Https protocol connection.
 */
public class JavawsHttpsCertStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
	// Get JWS Https certificate file jwshttpscerts.
	//_filename = UserProfile.getHttpsCertificateFile(); 
	_filename = ConfigProperties.getInstance().getCacheDir()  +
	    File.separator + "javawshttpscerts";
    }

    public static String getJavawsHttpsFilename() {
	return _filename;
    }

    public static void removeJavawsHttpsCert() {
	new File(_filename).delete();
	_jwscerts = null;
    }

    // Collection of JWS certificates
    private static KeyStore _jwscerts = null; 

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.loading" + new Object[]{_filename});
	}

	try
	{ 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    if (_jwscerts == null)
		    {
			// Create keystore first
			_jwscerts = KeyStore.getInstance("JKS");
			_jwscerts.load(null, null);
		    }
		   
		    File file = new File(_filename);

		    // Only load the cert store if it exists
		    if (file.exists())
		    {		
			FileInputStream fis = new FileInputStream(file);
			BufferedInputStream bis = new BufferedInputStream(fis);
	    		
			// Initialize the keystore
			_jwscerts.load(bis, new char[0]);

			bis.close();
			fis.close();
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
		Debug.ignoredException(e);
	}
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.loaded" + new Object[] {_filename});
	}
    }


    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.saving" + new Object[] {_filename});
	}
	try
	{ 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{		    		    		    
		    File cacheDir = new File(ConfigProperties.getInstance().getCacheDir());
		    //cacheDir must exist before creating javawshttpscerts
		    cacheDir.mkdirs();

		    File file = new File(_filename);

		    FileOutputStream fos = new FileOutputStream(file);
		    BufferedOutputStream bos = new BufferedOutputStream(fos);

	    	    _jwscerts.store(bos, new char[0]);

		    bos.close();
		    fos.close();
		
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
	    Debug.println("jwshttpscertstore.cert.saved" + new Object[] {_filename});
	}
    }


    /**
     * Add a certificate into the certificate store.
     * 
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.adding");
	}
	if (contains(cert) == false)
	{
    	    // Generate a unique alias for the certificate
	    Random rand = new Random();
	    boolean found = false;

	    String alias = null;

	    // Loop until we found a unique alias that is not in the store
	    do 
	    {
		alias = "jwscert" + rand.nextLong();
		
    		Certificate c = _jwscerts.getCertificate(alias);

		if (c == null)
		    found = true;
	    }
	    while (found == false);

	    _jwscerts.setCertificateEntry(alias, cert);

	    if (Globals.TraceHttpsCert) {
		Debug.println("jwshttpscertstore.cert.added" + new Object[] {alias});
	    }
	}
    }


    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public void remove(Certificate cert) throws KeyStoreException 
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.removing");
	}

	String alias = _jwscerts.getCertificateAlias(cert);

	if (alias != null)
	    _jwscerts.deleteEntry(alias);

	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.removed" + new Object[] {alias});
	}
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
	    Debug.println("jwshttpscertstore.cert.instore");
	}

	// Certificate alias returned only if there is a match
	String alias = null;
	
	alias = _jwscerts.getCertificateAlias(cert);
 
	return (alias != null);
    }


    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */
    public boolean verify(Certificate cert)
    {
	if (Globals.TraceHttpsCert) {
	    Debug.println("jwshttpscertstore.cert.canverify");
	}

	// Plug-in Certificate store is not intended to be used for verification.
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
	    Debug.println("jwshttpscertstore.cert.iterator");
	}

	HashSet certCollection = new HashSet();
	Enumeration enum = _jwscerts.aliases();
	
	while (enum.hasMoreElements())
	{
	    // Get certificate alias from iterator
	    String alias = (String) enum.nextElement();	 
	    // Get certificate from store
    	    Certificate cert = _jwscerts.getCertificate(alias);

	    // Add certificate into collection
	    certCollection.add(cert);
	}		

	return certCollection.iterator();
    }
}
