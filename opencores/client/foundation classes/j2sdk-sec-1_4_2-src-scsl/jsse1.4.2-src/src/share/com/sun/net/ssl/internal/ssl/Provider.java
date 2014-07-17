/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.security.*;

/**
 * The JSSE provider.
 *
 * Due to licensing restrictions, this provider must not register general RSA
 * signature or encryption support. If it did, that functionality would be
 * made available to other applications, thereby violating our licensing
 * agreement with RSA Data Security, Inc.
 * However, it needs to register its RSA key factory, so that keystores
 * containing RSA public and private keys (in encoded format)
 * can be parsed and the corresponding Key objects be generated (from their
 * encoding).
 */

public final class Provider extends java.security.Provider {

    private static String info = "Sun JSSE provider" +
	"(implements RSA Signatures, PKCS12, SunX509 key/trust factories, " +
	"SSLv3, TLSv1)";

    /**
     * Installs the JSSE provider.
     */
    public static synchronized void install()
    {
	/* nop. Remove this method in the future. */
    }

    public Provider()
    {
	super("SunJSSE", 1.42, info);

        AccessController.doPrivileged(new java.security.PrivilegedAction() {
            public Object run() {

		put("KeyFactory.RSA",
		    "com.sun.net.ssl.internal.ssl.JSA_RSAKeyFactory");
                put("Alg.Alias.KeyFactory.1.2.840.113549.1.1", "RSA");
                put("Alg.Alias.KeyFactory.OID.1.2.840.113549.1.1", "RSA");
		    
		put("KeyPairGenerator.RSA",
		    "com.sun.net.ssl.internal.ssl.JSA_RSAKeyPairGenerator");
                put("Alg.Alias.KeyPairGenerator.1.2.840.113549.1.1", "RSA");
                put("Alg.Alias.KeyPairGenerator.OID.1.2.840.113549.1.1", "RSA");
		    
		put("Signature.MD2withRSA",
		    "com.sun.net.ssl.internal.ssl.JSA_MD2RSASignature");
                put("Alg.Alias.Signature.1.2.840.113549.1.1.2", "MD2withRSA");
                put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.2", 
                    "MD2withRSA");

		put("Signature.MD5withRSA",
		    "com.sun.net.ssl.internal.ssl.JSA_MD5RSASignature");
                put("Alg.Alias.Signature.1.2.840.113549.1.1.4", "MD5withRSA");
                put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.4",
                    "MD5withRSA");
		    
		put("Signature.SHA1withRSA",
		    "com.sun.net.ssl.internal.ssl.JSA_SHA1RSASignature");
                put("Alg.Alias.Signature.1.2.840.113549.1.1.5", "SHA1withRSA");
                put("Alg.Alias.Signature.OID.1.2.840.113549.1.1.5",
                    "SHA1withRSA");
                put("Alg.Alias.Signature.1.3.14.3.2.29", "SHA1withRSA");
                put("Alg.Alias.Signature.OID.1.3.14.3.2.29", "SHA1withRSA");
		    
		put("KeyManagerFactory.SunX509",
		    "com.sun.net.ssl.internal.ssl.KeyManagerFactoryImpl");
		put("TrustManagerFactory.SunX509",
		    "com.sun.net.ssl.internal.ssl.TrustManagerFactoryImpl$SimpleFactory");
		put("TrustManagerFactory.SunPKIX",
		    "com.sun.net.ssl.internal.ssl.TrustManagerFactoryImpl$PKIXFactory");
		put("SSLContext.SSL",
		    "com.sun.net.ssl.internal.ssl.SSLContextImpl");
		put("SSLContext.SSLv3",
		    "com.sun.net.ssl.internal.ssl.SSLContextImpl");
		put("SSLContext.TLS",
		    "com.sun.net.ssl.internal.ssl.SSLContextImpl");
		put("SSLContext.TLSv1",
		    "com.sun.net.ssl.internal.ssl.SSLContextImpl");

		/*
		 * KeyStore
		 */
		put("KeyStore.PKCS12",
		    "com.sun.net.ssl.internal.ssl.PKCS12KeyStore");
                return null;
            }
        });
	
    }
}
