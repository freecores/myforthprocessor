/*
 * @(#)SessionCertificateStore.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.security;

import java.io.IOException;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;

/**
 * SessionCertificateStore is a class that represents the session certificate 
 * store which contains all the certificates that Java Plug-in recognizes. The
 * certificates store in this certificate store is only valid in the current
 * browser session. It is used in the certification verification process 
 * when signed applet is encountered.
 */
public class SessionCertificateStore implements CertificateStore
{
    // Collection of JPI certificates
    private Collection _certs = new HashSet();

    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	// Do nothing
	if (Globals.TraceHttpsCert) {
	    Debug.println("sessioncertstore.cert.loading");	 

	    Debug.println("sessioncertstore.cert.loaded");
	}
    }


    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException
    {
	// Do nothing
	if (Globals.TraceHttpsCert) {
	    Debug.println("sessioncertstore.cert.saving");

	    Debug.println("sessioncertstore.cert.saved");
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
	    Debug.println("sessioncertstore.cert.adding");
	}
	
	_certs.add(cert);
	
	if (Globals.TraceHttpsCert) {
	    Debug.println("sessioncertstore.cert.added");
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
	    Debug.println("sessioncertstore.cert.removing");
	}

	_certs.remove(cert);

	if (Globals.TraceHttpsCert) {
	    Debug.println("sessioncertstore.cert.removed");
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
	    Debug.println("sessioncertstore.cert.instore");
	}
	return _certs.contains(cert);
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
	    Debug.println("sessioncertstore.cert.canverify");
	}
	// Session Certificate store is not intended to be used for verification.
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
	    Debug.println("sessioncertstore.cert.iterator");
	}
	return _certs.iterator();
    }
}
