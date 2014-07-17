/*
 * @(#)KeyStores.java	1.2, 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.validator;

import java.io.*;
import java.util.*;

import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;

import sun.security.action.*;

/**
 * Collection of static utility methods related to KeyStores.
 *
 * @author Andreas Sterbenz
 * @version 1.2, 01/23/03
 */
public class KeyStores {

    private KeyStores() {
        // empty
    }
    
    // in the future, all accesses to the system cacerts keystore should
    // go through this class. but not right now.
/*
    private final static String javaHome = 
    	(String)AccessController.doPrivileged(new GetPropertyAction("java.home"));

    private final static char SEP = File.separatorChar;

    private static KeyStore caCerts;

    private static KeyStore getKeyStore(String type, String name, 
	    char[] password) throws IOException {
        if (type == null) {
            type = "JKS";
        }
        try {
            KeyStore ks = KeyStore.getInstance(type);
            FileInputStream in = (FileInputStream)AccessController.doPrivileged
	    				(new OpenFileInputStreamAction(name));
            ks.load(in, password);
            return ks;
        } catch (GeneralSecurityException e) {
            // XXX
            throw new IOException();
        } catch (PrivilegedActionException e) {
            throw (IOException)e.getCause();
        }
    }

    /**
     * Return a KeyStore with the contents of the lib/security/cacerts file.
     * The file is only opened once per JVM invocation and the contents
     * cached subsequently.
     *
    public synchronized static KeyStore getCaCerts() throws IOException {
        if (caCerts != null) {
            return caCerts;
        }
        String name = javaHome + SEP + "lib" + SEP + "security" + SEP + "cacerts";
        caCerts = getKeyStore(null, name, null);
        return caCerts;
    }
*/
    private static boolean isValid(X509Certificate cert, Date date) {
        try {
            cert.checkValidity(date);
            return true;
        } catch (CertificateExpiredException e) {
            return false;
        } catch (CertificateNotYetValidException e) {
            // may soon be valid
            return true;
        }
    }

    /**
     * Return a Set with all unexpired trusted X509Certificates contained in
     * this KeyStore. Includes all currently valid and all not yet valid
     * X509Certificates.
     */
    public static Set getTrustedCerts(KeyStore ks) {
        Set set = new HashSet();
        Date date = new Date();
        try {
            for (Enumeration e = ks.aliases(); e.hasMoreElements(); ) {
                String alias = (String)e.nextElement();
                if (ks.isCertificateEntry(alias)) {
                    Certificate cert = ks.getCertificate(alias);
                    if (cert instanceof X509Certificate) {
                        if (isValid((X509Certificate)cert, date)) {
                            set.add(cert);
                        }
                    }
                } else if (ks.isKeyEntry(alias)) {
                    Certificate[] certs = ks.getCertificateChain(alias);
                    if ((certs != null) && (certs.length > 0) &&
                            (certs[0] instanceof X509Certificate)) {
                        X509Certificate cert = (X509Certificate)certs[0];
                        if (isValid(cert, date)) {
                            set.add(cert);
                        }
                    }
                }
            }
        } catch (KeyStoreException e) {
            // ignore
        }
        return set;
    }

}
