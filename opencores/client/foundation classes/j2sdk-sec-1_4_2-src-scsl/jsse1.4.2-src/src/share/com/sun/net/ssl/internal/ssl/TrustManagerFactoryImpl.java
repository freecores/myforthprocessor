/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.util.*;
import java.io.*;
import java.math.*;
import java.security.*;
import javax.net.ssl.*;
import java.security.spec.AlgorithmParameterSpec;

import sun.security.validator.Validator;

abstract class TrustManagerFactoryImpl extends TrustManagerFactorySpi {

    private static final Debug debug = Debug.getInstance("ssl");
    private X509TrustManager trustManager = null;
    private boolean isInitialized = false;
    
    TrustManagerFactoryImpl() {
	// empty
    }

    protected void engineInit(KeyStore ks) throws KeyStoreException {
	if (ks == null) {
	    try {
		ks = getCacertsKeyStore("trustmanager");
	    } catch (SecurityException se) {
		// eat security exceptions but report other throwables
		if (debug != null && Debug.isOn("trustmanager")) {
		    System.out.println(
			"SunX509: skip default keystore: " + se);
		}
	    } catch (Error err) {
		if (debug != null && Debug.isOn("trustmanager")) {
		    System.out.println(
			"SunX509: skip default keystore: " + err);
		}
		throw err;
	    } catch (RuntimeException re) {
		if (debug != null && Debug.isOn("trustmanager")) {
		    System.out.println(
			"SunX509: skip default keystore: " + re);
		}
		throw re;
	    } catch (Exception e) {
		if (debug != null && Debug.isOn("trustmanager")) {
		    System.out.println(
			"SunX509: skip default keystore: " + e);
		}
		throw new KeyStoreException(
		    "problem accessing trust store" + e);
	    }
	}
	trustManager = getInstance(ks);
	isInitialized = true;
    }
    
    abstract X509TrustManager getInstance(KeyStore ks) throws KeyStoreException;

    protected void engineInit(ManagerFactoryParameters spec) throws
	    InvalidAlgorithmParameterException {
	throw new InvalidAlgorithmParameterException(
	    "SunJSSE does not use ManagerFactoryParameters");
    }

    /**
     * Returns one trust manager for each type of trust material.
     */
    protected TrustManager[] engineGetTrustManagers() {
	if (!isInitialized) {
	    throw new IllegalStateException(
			"TrustManagerFactoryImpl is not initialized");
	}
	return new TrustManager[] { trustManager };
    }

    /*
     * Try to get an InputStream based on the file we pass in.
     */
    static FileInputStream getFileInputStream(final File file)
	    throws Exception {
	return (FileInputStream) AccessController.doPrivileged(
		new PrivilegedExceptionAction() {
		    public Object run() throws Exception {
			try {
			    if (file.exists()) {
				return new FileInputStream(file);
			    } else {
				return null;
			    }
			} catch (FileNotFoundException e) {
			    // couldn't find it, oh well.
			    return null;
			}
		    }
		});
    }

    /**
     * Returns the keystore with the configured CA certificates.
     */
    static KeyStore getCacertsKeyStore(String dbgname) throws Exception
    {
	String storeFileName = null;
	File storeFile = null;
	FileInputStream fis = null;
	String defaultTrustStoreType;
	final HashMap props = new HashMap();
	final String sep = File.separator;
	KeyStore ks = null;

	AccessController.doPrivileged(new PrivilegedExceptionAction() {
	    public Object run() throws Exception {
		props.put("trustStore", System.getProperty(
				"javax.net.ssl.trustStore"));
		props.put("javaHome", System.getProperty(
					"java.home"));
		props.put("trustStoreType", System.getProperty(
				"javax.net.ssl.trustStoreType",
				KeyStore.getDefaultType()));
		props.put("trustStorePasswd", System.getProperty(
				"javax.net.ssl.trustStorePassword", ""));
		return null;
	    }
	});

	/*
	 * Try:
	 *	javax.net.ssl.trustStore  (if this variable exists, stop)
	 *	jssecacerts
	 *	cacerts
	 *
	 * If none exists, we use an empty keystore.
	 */
	if ((storeFileName = (String) props.get("trustStore")) != null) {
	    storeFile = new File(storeFileName);
	    fis = getFileInputStream(storeFile);
	} else {
	    String javaHome = (String) props.get("javaHome");
	    storeFile = new File(javaHome + sep + "lib" + sep
						+ "security" + sep +
						"jssecacerts");
	    if ((fis = getFileInputStream(storeFile)) == null) {
		storeFile = new File(javaHome + sep + "lib" + sep
						+ "security" + sep +
						"cacerts");
		fis = getFileInputStream(storeFile);
	    }
	}

	if (fis != null) {
	    storeFileName = storeFile.getPath();
	} else {
	    storeFileName = "No File Available, using empty keystore.";
	}

	defaultTrustStoreType = (String) props.get("trustStoreType");
	if (debug != null && Debug.isOn(dbgname)) {
	    System.out.println("trustStore is: " + storeFileName);
	    System.out.println("trustStore type is : " +
				defaultTrustStoreType);
	}

	/*
	 * Try to initialize trust store.
	 */
	if (defaultTrustStoreType.length() != 0) {
	    if (debug != null && Debug.isOn(dbgname)) {
		System.out.println("init truststore");
	    }
	    ks = KeyStore.getInstance(defaultTrustStoreType);
	    char[] passwd = null;
	    String defaultTrustStorePassword = (String) props.get(
					"trustStorePasswd");
	    if (defaultTrustStorePassword.length() != 0)
		passwd = defaultTrustStorePassword.toCharArray();
	    ks.load(fis, passwd);

	    // Zero out the temporary password storage
	    if (passwd != null) {
		for (int i = 0; i < passwd.length; i++) {
		    passwd[i] = (char)0;
		}
	    }
	}

	if (fis != null) {
	    fis.close();
	}

	return ks;
    }
    
    public static final class SimpleFactory extends TrustManagerFactoryImpl {
	X509TrustManager getInstance(KeyStore ks) throws KeyStoreException {
	    return new X509TrustManagerImpl(Validator.TYPE_SIMPLE, ks);
	}
    }

    public static final class PKIXFactory extends TrustManagerFactoryImpl {
	X509TrustManager getInstance(KeyStore ks) throws KeyStoreException {
	    return new X509TrustManagerImpl(Validator.TYPE_PKIX, ks);
	}
    }
}
