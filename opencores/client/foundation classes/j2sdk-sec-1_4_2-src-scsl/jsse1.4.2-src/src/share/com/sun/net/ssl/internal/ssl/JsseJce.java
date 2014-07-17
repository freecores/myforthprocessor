/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.util.*;

import java.security.*;
import javax.crypto.*;

// explicit imports to override the classes with same name in this package
import java.security.Provider;
import javax.crypto.Cipher;

/**
 * This class contains a few static methods for interaction with the JCA/JCE
 * to obtain implementations, etc.
 *
 * @version 1.6, 06/24/03
 */
class JsseJce {

    private final static Debug debug = Debug.getInstance("ssl");

    /**
     * JCE transformation string for RSA with PKCS#1 v1.5 padding.
     * Can be used for encryption, decryption, signing, verifying.
     */
    final static String CIPHER_RSA_PKCS1 = "RSA/ECB/PKCS1Padding";
    /**
     * JCE transformation string for the stream cipher RC4.
     */
    final static String CIPHER_RC4 = "RC4";
    /**
     * JCE transformation string for DES in CBC mode without padding.
     */
    final static String CIPHER_DES = "DES/CBC/NoPadding";
    /**
     * JCE transformation string for (3-key) Triple DES in CBC mode
     * without padding.
     */
    final static String CIPHER_3DES = "DESede/CBC/NoPadding";
    /**
     * JCE transformation string for AES in CBC mode
     * without padding.
     */
    final static String CIPHER_AES = "AES/CBC/NoPadding";
    /**
     * JCA identifier string for Raw DSA, i.e. a DSA signature without
     * hashing where the application provides the SHA-1 hash of the data.
     */
    final static String SIGNATURE_RAWDSA = "RawDSA";
    /**
     * JCA identifier string for the SSL/TLS style RSA Signature. I.e.
     * an signature using RSA with PKCS#1 v1.5 padding signing a
     * concatenation of an MD5 and SHA-1 digest.
     */
    final static String SIGNATURE_RSA = "MD5andSHA1withRSA";
    
    // the names of our providers that we know currently do not support AES
    private final static Set nonAES = new HashSet(Arrays.asList(new String[] {
	"SUN", "SunJSSE", "SunRsaSign", "SunJGSS",
    }));
    
    private JsseJce() {
        // no instantiation of this class
    }

    /** 
     * Return a list with the JCE providers to use. These are all installed
     * providers up to the JSSE provider, providers with a lower priority
     * are ignored. This is to allow the user to specify that he wants to
     * prefer e.g. the JSSE internal DES implementation over some other 
     * provider. It emulates the behavior we would get if the
     * JSSE implementations would be standard JCE implementations.
     */
    private static List getProviders() {
        Provider[] providers = Security.getProviders();
	ArrayList list = new ArrayList();
	for (int i = 0; i < providers.length; i++) {
	    Provider prov = providers[i];
	    list.add(prov);
	    if (prov.getName().equals("SunJSSE") 
	            && prov.getClass().getName().equals(
		            "com.sun.net.ssl.internal.ssl.Provider")) {
	        break;
	    }
	}
	return list;
    }
    
    /**
     * Return an JCE cipher implementation for the specified algorithm.
     * The providers returned by getProviders() are searched.
     * The transformation string should be one of the constants defined
     * in this class. 
     */
    static Cipher getCipher(String transformation) 
            throws NoSuchAlgorithmException {
	if (transformation.equals(CIPHER_AES)) {
	    return getCipherAES();
	}
        for (Iterator t = getProviders().iterator(); t.hasNext(); ) {
	    Provider prov = (Provider)t.next();
	    String providerName = prov.getName();
	    if (providerName.equals("SUN") || providerName.equals("SunJSSE")) {
	        // XXX ignore the Sun and SunJSSE providers, we know they do
		// not implement any ciphers. This is a performance
		// optimisation to avoid accessing the JCE classes and the
		// associated startup delay. Remove when/if this is no longer
		// an issue.
	        continue;
	    }
	    try {
	        Cipher c = Cipher.getInstance(transformation, prov);
		if (debug != null && Debug.isOn("handshake")) {
		    System.out.println("JsseJce: Using cipher " + 
		        transformation + " from provider " + prov.getName());
		}
		return c;
	    } catch (Exception e) {
	        // JSafe does not support the standard PKCS1 transformation
		// string, provide a workaround for them.
	        if (transformation.equals(CIPHER_RSA_PKCS1) &&
	        	prov.getName().equals("JsafeJCE")) {
		    try {
		        Cipher c = Cipher.getInstance("RSA", prov);
			if (debug != null && Debug.isOn("handshake")) {
			    System.out.println("JsseJce: Using cipher RSA" +
			        " from provider " + prov.getName());
			}
			return c;
		    } catch (Exception ee) {
		        // ignore
		    }
		}
	    }
	}
	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("JsseJCE: Using JSSE internal implementation " +
	    	"for cipher " + transformation);
	}
	String msg = "No provider implementation for " + transformation
	            + " found.";
        throw new NoSuchAlgorithmException(msg);
    }

    /**
     * Return a JCE cipher for the AES algorithm. AES is a special case in
     * that it is currently the only cipher we support that does not have
     * an internal implementation. This means we look through all providers
     * rather than just all providers with a higher preference than SunJSSE.
     *
     * We do some trickery similar to getCipher() to avoid initializing the 
     * JCE classes and the corresponding startup penalty if possible.
     */
    private static Cipher getCipherAES() throws NoSuchAlgorithmException {
	Provider[] providers = Security.getProviders();
	for (int i = 0; i < providers.length; i++) {
	    Provider prov = providers[i];
	    if (nonAES.contains(prov.getName())) {
		continue;
	    }
	    try {
		Cipher c = Cipher.getInstance(CIPHER_AES, prov);
		if (debug != null && Debug.isOn("handshake")) {
		    System.out.println("JsseJce: Using cipher " + 
		        CIPHER_AES + " from provider " + prov.getName());
		}
		return c;
	    } catch (Exception e) {
		// ignore NoSuchAlgorithm, NoSuchPaddingException, etc.
		// and try next provider
	    }
	}
	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("JsseJce: No implementation for " +
	    	CIPHER_AES + " found in JCE providers");
	}
	throw new NoSuchAlgorithmException("No implementation for " +
		CIPHER_AES + " found");
    }
    
    /**
     * Return an JCA signature implementation for the specified algorithm.
     * The providers returned by getProviders() are searched.
     * The algorithm string should be one of the constants defined
     * in this class. 
     */
    static Signature getSignature(String algorithm) 
            throws NoSuchAlgorithmException {
        for (Iterator t = getProviders().iterator(); t.hasNext(); ) {
	    Provider prov = (Provider)t.next();
	    try {
	        Signature s = Signature.getInstance(algorithm, prov);
		if (debug != null && Debug.isOn("handshake")) {
		    System.out.println("JsseJce: Using signature " + algorithm +
		        " from provider " + prov.getName());
		}
		return s;
	    } catch (Exception e) {
	        // ignore
	    }
	}
	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("JsseJce: Using JSSE internal implementation " +
	    	"for signature " + algorithm);
	}
	String msg = "No provider implementation for " + algorithm + " found.";
        throw new NoSuchAlgorithmException(msg);
    }
    
    static MessageDigest getMD5() {
	return getMessageDigest("MD5");
    }
    
    static MessageDigest getSHA() {
	return getMessageDigest("SHA");
    }
    
    private static MessageDigest getMessageDigest(String algorithm) {
	try {
	    return MessageDigest.getInstance(algorithm);
	} catch (NoSuchAlgorithmException e) {
	    throw new RuntimeException
	    		("Algorithm " + algorithm + " not available", e);
	}
    }

}
