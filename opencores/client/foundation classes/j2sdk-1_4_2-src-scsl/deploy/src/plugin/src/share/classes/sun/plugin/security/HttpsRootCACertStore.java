/*
 * @(#)HttpsRootCACertStore.java	1.13 03/01/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

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
import java.security.GeneralSecurityException;
import java.util.Collection;
import java.util.Iterator;
import java.util.HashSet;
import java.util.Enumeration;
import sun.plugin.util.UserProfile;
import sun.plugin.util.Trace;


/**
 * HttpsRootCACertStore is a class that represents the certificate 
 * stores which contains all the Https root CA certificates. It is used in 
 * the certification verification process when signed applet is encountered.
 */
public final class HttpsRootCACertStore implements CertificateStore
{
    private static String _filename = null;

    static
    {
	// Get Https root CA file jssecacerts 
    	_filename = UserProfile.getHttpsRootCertFile();
    }

    // Collection of Https root CA cert keystore
    private KeyStore _jssecacerts = null;

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loading", new Object[] { _filename });

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
			Trace.msgSecurityPrintln("roothttpscertstore.cert.noload", new Object[] { _filename });
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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.loaded", new Object[] {_filename});
    }

 
    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.saving", new Object[] {_filename});

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.adding");

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.removing");

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.instore");

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.canverify");

	Trace.msgSecurityPrintln("roothttpscertstore.cert.tobeverified", new Object[] {cert});
 
	// Enumerate each Https root CA certificate in the Https root store
	Enumeration enum = _jssecacerts.aliases();
 
	while (enum.hasMoreElements())
	{
	    String alias = (String) enum.nextElement();

	    Certificate rootCert = _jssecacerts.getCertificate(alias);

	    Trace.msgSecurityPrintln("roothttpscertstore.cert.tobecompared", new Object[] {rootCert});

	    try
	    {
    		cert.verify(rootCert.getPublicKey());

		Trace.msgSecurityPrintln("roothttpscertstore.cert.verify.ok");
		return true;
	    }
	    catch (GeneralSecurityException e)
	    {
		// Ignore exception
	    }
	}

	Trace.msgSecurityPrintln("roothttpscertstore.cert.verify.fail");

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
	Trace.msgSecurityPrintln("roothttpscertstore.cert.iterator");

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

    /**
     * Obtain KeyStore Object
     */
    public KeyStore getKeyStore()
    {
	Trace.msgSecurityPrintln("roothttpscertstore.cert.getkeystore");
	return _jssecacerts;
    }
}
