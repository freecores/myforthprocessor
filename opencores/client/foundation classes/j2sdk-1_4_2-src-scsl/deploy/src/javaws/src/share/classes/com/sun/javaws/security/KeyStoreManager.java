/*
 * @(#)KeyStoreManager.java	1.19 03/01/23
 * 
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.security;

import java.security.KeyStore;
import java.security.KeyStoreException;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.NoSuchProviderException;
import java.security.SignatureException;
import java.security.PermissionCollection;
import java.security.CodeSource;
import java.security.cert.Certificate;
import java.security.cert.CertificateException;
import java.util.*;
import java.io.File;
import java.io.BufferedOutputStream;
import java.io.FileOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URL;
import java.net.MalformedURLException;
import com.sun.javaws.ConfigProperties;
import com.sun.javaws.debug.Debug;
import com.sun.javaws.debug.Globals;

/** Class to wrap the KeyStore
 *
 *  Provides utility methods for handling the cacerts shipped with JavaWS
 *
 */
public class KeyStoreManager {
        
    static private KeyStore     _cacerts = null;            // Source of trusted certificates
    static private boolean      _initialized = false;
    static private final String ksLocation = ConfigProperties.getApplicationHome() + File.separator + "cacerts";
    
    
    /** Initialize KeyStore database, e.g., loads the cacerts. Throws a
     *   KeyStore exception if loading failed
     */
    static public void initialize() throws KeyStoreException {
        // Load certificate database
        FileInputStream fis = null;
        try {
            // The keystore type is always supposed to be of type 'JKS' - no matter
            // what JDK version we are using
            _cacerts = KeyStore.getInstance("JKS");
            fis = new FileInputStream(ksLocation);
            _cacerts.load(fis, null);
            _initialized = true;
        } catch(CertificateException ce) {
            throw new KeyStoreException("Error loading keystore");
        } catch(NoSuchAlgorithmException nsae) {
            throw new KeyStoreException("Error loading keystore");
        } catch(IOException ioe) {
            Debug.ignoredException(ioe);
            // Ignore if cacert database is not present
            // Should this be logged??
        } finally {
            try {
		if (fis != null) {
                    fis.close();
		}
            } catch (IOException e) {
                Debug.ignoredException(e);
            }
        }
    }
    
    static KeyStore getKeyStore() throws KeyStoreException {
        if (_initialized) {
            return _cacerts;
        } else {
            initialize();
            if (_initialized) {
                return _cacerts;
            }
        }
        return null;
    }
    
    static void saveKeyStore(KeyStore ks, String password) throws Exception {
        if (_initialized) {
            BufferedOutputStream bos = null;
            try {
                bos = new BufferedOutputStream(new FileOutputStream(new File(ksLocation)));
                ks.store(bos, password.toCharArray());
                _cacerts = ks;
            } catch (Exception e) {
                throw e;
            } finally {
                if (bos != null) bos.close();
            }
        }
    }
    
    //
    // Query interface
    //
    
    /** Check if the certificate chain has a self-signed certificate at the end, and
     *  if not, try to append one if a trusted one is found in the cacerts. This works
     *  around a problem with Netscape signtool to include the complete chain.
     */
    static Certificate[] makeTrusted(Certificate[] chain) {
        java.security.cert.Certificate cert = chain[chain.length - 1] ;
        Certificate[] orgChain = chain;
        
        // Check if it is self-signed
        if (isSigner(cert, cert)) {
            if (chain.length == 1 || isCertificateTrusted(cert)) {
                // The signer is trusted - so everything is ok
                return chain;
            } else {                
                // Strip off self-signed certificate to see if one of the
                // stored certificate can be used to verify the chain
                chain = new Certificate[chain.length - 1];
                System.arraycopy(orgChain, 0, chain, 0, chain.length);
                cert = chain[chain.length - 1];
            }
        }
        
        // Not self-signed. Check if we can find a match
        try {
            Enumeration aliases = _cacerts.aliases();
            while(aliases.hasMoreElements()) {
                String alias = (String)aliases.nextElement();
                java.security.cert.Certificate trustedCert = (java.security.cert.Certificate)_cacerts.getCertificate(alias);
                if (isSigner(cert, trustedCert)) {
                    Certificate[] newChain = new Certificate[chain.length + 1];
                    System.arraycopy(chain, 0, newChain, 0, chain.length);
                    newChain[chain.length] = trustedCert;
                    return newChain;
                }
            }
        } catch(KeyStoreException ks) {
            Debug.ignoredException(ks);
        }

        // Did do anything. Return original
        return orgChain;
    }
    
    static boolean isSigner(Certificate check, Certificate signer) {
        try {
            check.verify(signer.getPublicKey());
            return true;
        } catch(InvalidKeyException ike) {
            return false;
        } catch(NoSuchProviderException nsp) {
            return false;
        } catch(NoSuchAlgorithmException nsae) {
            return false;
        } catch(SignatureException se) {
            return false;
        } catch(CertificateException ce) {
            return false;
        }
    }
    
    /**
     *   Check if a certificate can be trusted. (in cacerts)
     */  
    static boolean isCertificateTrusted(Certificate cert) {
        if (_cacerts != null) {
            try {
                String alias = _cacerts.getCertificateAlias(cert);
                if (alias != null) {
                    return _cacerts.isCertificateEntry(alias);
                }
            } catch (KeyStoreException e) { /* Ignore */}
        }
        return false;
    }
}
