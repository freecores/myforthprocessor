/*
 * @(#)PluginHttpsCertStore.java	1.14 03/01/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

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
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;

/**
 * PluginHttpsCertStore is a class that represents the permanent certificate 
 * store which contains all the certificates that Java Plug-in recognize. It 
 * is used in the certification verification process when signed applet is 
 * encountered when you use Https protocol connection.
 */
public final class PluginHttpsCertStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
		// Get Plugin Https certificate file jpihttpscerts140.
		_filename = UserProfile.getHttpsCertificateFile(); 
    }

    // Collection of JPI certificates
    private KeyStore _deploymentcerts = null; 

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.loading", new Object[]{_filename});

	try
	{ 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
			
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    if (_deploymentcerts == null)
		    {
			// Create keystore first
			_deploymentcerts = KeyStore.getInstance("JKS");
			_deploymentcerts.load(null, null);
		    }

		    File file = new File(_filename);

		    // Only load the cert store if it exists
		    if (file.exists())
		    {
			FileInputStream fis = new FileInputStream(file);
			BufferedInputStream bis = new BufferedInputStream(fis);
	    		
			// Initialize the keystore
			_deploymentcerts.load(bis, new char[0]);

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
		Trace.securityPrintException(e);
	}

	Trace.msgSecurityPrintln("jpihttpscertstore.cert.loaded", new Object[] {_filename});
    }


    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.saving", new Object[] {_filename});

	try
	{ 
	    AccessController.doPrivileged(new PrivilegedExceptionAction() {
		    
		public Object run() throws IOException, CertificateException,
					   KeyStoreException, NoSuchAlgorithmException
		{
		    FileOutputStream fos = new FileOutputStream(new File(_filename));
		    BufferedOutputStream bos = new BufferedOutputStream(fos);

	    	    _deploymentcerts.store(bos, new char[0]);

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
		Trace.securityPrintException(e);
	}

	Trace.msgSecurityPrintln("jpihttpscertstore.cert.saved", new Object[] {_filename});
    }


    /**
     * Add a certificate into the certificate store.
     * 
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.adding");

	if (contains(cert) == false)
	{
    	    // Generate a unique alias for the certificate
	    Random rand = new Random();
	    boolean found = false;

	    String alias = null;

	    // Loop until we found a unique alias that is not in the store
	    do 
	    {
		alias = "deploymentcert" + rand.nextLong();
		
    		Certificate c = _deploymentcerts.getCertificate(alias);

		if (c == null)
		    found = true;
	    }
	    while (found == false);

	    _deploymentcerts.setCertificateEntry(alias, cert);

	    Trace.msgSecurityPrintln("jpihttpscertstore.cert.added", new Object[] {alias});
	}
    }


    /**
     * Remove a certificate from the certificate store.
     * 
     * @param cert Certificate object.
     */
    public void remove(Certificate cert) throws KeyStoreException 
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.removing");

	String alias = _deploymentcerts.getCertificateAlias(cert);

	if (alias != null)
	    _deploymentcerts.deleteEntry(alias);

	Trace.msgSecurityPrintln("jpihttpscertstore.cert.removed", new Object[] {alias});
    }


    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.instore");

	// Certificate alias returned only if there is a match
	String alias = null;
	
	alias = _deploymentcerts.getCertificateAlias(cert);
 
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
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.canverify");

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
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.iterator");

	HashSet certCollection = new HashSet();
	Enumeration enum = _deploymentcerts.aliases();
	
	while (enum.hasMoreElements())
	{
	    // Get certificate alias from iterator
	    String alias = (String) enum.nextElement();

	    // Get certificate from store
    	    Certificate cert = _deploymentcerts.getCertificate(alias);

	    // Add certificate into collection
	    certCollection.add(cert);
	}		

	return certCollection.iterator();
    }

   /**
    * Obtain KeyStore Object
    */
    public KeyStore getKeyStore()
    {
	Trace.msgSecurityPrintln("jpihttpscertstore.cert.getkeystore");
	return _deploymentcerts;
    }
}
