/*
 * @(#)CertificateStore.java	1.11 03/01/27
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security; 

import java.io.IOException;
import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.Iterator;


/**
 * CertificateStore is an interface that represents the certificate 
 * stores that are used in the certification verification process 
 * when signed applet is encountered.
 */
public interface CertificateStore 
{
    /**
     * Load the certificate store into memory.
     */
    public void load() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException;

    /**
     * Persist the certificate store.
     */
    public void save() throws IOException, CertificateException,
			      KeyStoreException, NoSuchAlgorithmException;

    /**
     * Add a certificate into the certificate store.
     *
     * @param cert Certificate object.
     */
    public void add(Certificate cert) throws KeyStoreException;

    /**
     * Remove a certificate from the certificate store.
     *
     * @param cert Certificate object.
     */
    public void remove(Certificate cert) throws KeyStoreException;

    /**
     * Check if a certificate is stored within the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is in the store.
     */
    public boolean contains(Certificate cert) throws KeyStoreException;

    /**
     * Verify if a certificate is issued by one of the certificate
     * in the certificate store.
     *
     * @param cert Certificate object.
     * @return true if certificate is issued by one in the store.
     */
    public boolean verify(Certificate cert) throws KeyStoreException; 

    /**
     * Obtain all the certificates that are stored in this 
     * certificate store.
     *
     * @return Iterator for iterating certificates
     */
    public Iterator iterator() throws KeyStoreException;

    public KeyStore getKeyStore() throws KeyStoreException;
}
