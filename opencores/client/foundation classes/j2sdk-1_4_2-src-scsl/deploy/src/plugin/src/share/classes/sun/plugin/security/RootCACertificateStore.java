/*
 * @(#)RootCACertificateStore.java	1.35 03/01/27
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
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.security.GeneralSecurityException;
import java.util.Collection;
import java.util.Enumeration;
import java.util.HashSet;
import java.util.Iterator;
import sun.plugin.util.Trace;
import sun.plugin.util.UserProfile;

/**
 * RootCACertificateStore is a class that represents the certificate 
 * stores which contains all the root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public final class RootCACertificateStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
	// Get root CA file cacerts
    	_filename = UserProfile.getRootCertificateFile();
    }

    // Collection of root CA cert keystore
    private KeyStore _cacerts = null;

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("rootcertstore.cert.loading", new Object[] {_filename});

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
			Trace.msgSecurityPrintln("rootcertstore.cert.noload", new Object[] {_filename});
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
	
	Trace.msgSecurityPrintln("rootcertstore.cert.loaded", new Object[] {_filename});
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
        Trace.msgSecurityPrintln("rootcertstore.cert.saving", new Object[] {_filename});

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
	Trace.msgSecurityPrintln("rootcertstore.cert.adding");

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
	Trace.msgSecurityPrintln("rootcertstore.cert.removing");

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
	Trace.msgSecurityPrintln("rootcertstore.cert.instore");

	// We compare any certificate into the root CA store.
        if (_cacerts.getCertificateAlias(cert) != null)
           return true;
        else
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
	Trace.msgSecurityPrintln("rootcertstore.cert.canverify");

	StringBuffer sb = new StringBuffer();

	Trace.msgSecurityPrintln("rootcertstore.cert.tobeverified", new Object[] {cert});
 
	// Enumerate each root CA certificate in the root store
	Enumeration enum = _cacerts.aliases();
 
	while (enum.hasMoreElements())
	{
	    String alias = (String) enum.nextElement();

	    Certificate rootCert = _cacerts.getCertificate(alias);

	    Trace.msgSecurityPrintln("rootcertstore.cert.tobecompared", new Object[] {rootCert});

	    try
	    {
    		cert.verify(rootCert.getPublicKey());

		Trace.msgSecurityPrintln("rootcertstore.cert.verify.ok");
		return true;
	    }
	    catch (GeneralSecurityException e)
	    {
		// Ignore exception
	    }
	}

	Trace.msgSecurityPrintln("rootcertstore.cert.verify.fail");

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
	Trace.msgSecurityPrintln("rootcertstore.cert.iterator");

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

   /**
    * Obtain KeyStore Object 
    */
   public KeyStore getKeyStore()
   {
	Trace.msgSecurityPrintln("rootcertstore.cert.getkeystore");
	return _cacerts;
   }
}
