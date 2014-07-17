/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.security.AccessController;
import java.security.Provider;
import java.security.PrivilegedAction;
import java.security.cert.*;
import java.net.URL;
import java.io.ByteArrayInputStream;
import java.security.CodeSource;
import java.security.SecureRandom;


/**
 * The "SunJCE" Cryptographic Service Provider.
 *
 * @author Jan Luehe
 * @author Sharon Liu
 *
 * @version 1.51, 06/24/03
 */

/**
 * Defines the "SunJCE" provider.
 *
 * Supported algorithms and their names:
 *
 * - DES
 *
 * - DES-EDE
 *
 * - AES
 *
 * - Blowfish
 *
 * - Cipher modes ECB, CBC, CFB, OFB, PCBC, and CTR for all block ciphers
 *
 * - Password-based Encryption (PBE)
 *
 * - Diffie-Hellman Key Agreement
 *
 * - HMAC-MD5, HMAC-SHA1
 *
 * - PKCS5Padding
 */

public final class SunJCE extends Provider {
    
    private static final String info = "SunJCE Provider " + 
    "(implements DES, Triple DES, AES, Blowfish, PBE, Diffie-Hellman, "
    + "HMAC-MD5, HMAC-SHA1)";

    /* Are we debugging? -- for developers */
    static final boolean debug = false;

    static final SecureRandom RANDOM = new SecureRandom();

    // After the SunJCE passed self-integrity checking,
    // verifiedSelfIntegrity will be set to true.
    private static boolean verifiedSelfIntegrity = false;

    public SunJCE() {
	/* We are the "SunJCE" provider */
	super("SunJCE", 1.42, info);

        AccessController.doPrivileged(new java.security.PrivilegedAction() {
            public Object run() {

		/*
		 * Cipher engines 
		 */
		put("Cipher.DES", "com.sun.crypto.provider.DESCipher");

		put("Cipher.DESede", "com.sun.crypto.provider.DESedeCipher");
		put("Alg.Alias.Cipher.TripleDES", "DESede");

		put("Cipher.PBEWithMD5AndDES",
		    "com.sun.crypto.provider.PBEWithMD5AndDESCipher");
		put("Cipher.PBEWithMD5AndTripleDES",
		    "com.sun.crypto.provider.PBEWithMD5AndTripleDESCipher");

		put("Cipher.Blowfish",
		    "com.sun.crypto.provider.BlowfishCipher");

		put("Cipher.AES", "com.sun.crypto.provider.AESCipher");
		put("Alg.Alias.Cipher.Rijndael", "AES");

		/*
		 *  Key(pair) Generator engines 
		 */
		put("KeyGenerator.DES", 
		    "com.sun.crypto.provider.DESKeyGenerator");

		put("KeyGenerator.DESede", 
		    "com.sun.crypto.provider.DESedeKeyGenerator");
		put("Alg.Alias.KeyGenerator.TripleDES", "DESede");

		put("KeyGenerator.Blowfish", 
		    "com.sun.crypto.provider.BlowfishKeyGenerator");

                put("KeyGenerator.AES",
                    "com.sun.crypto.provider.AESKeyGenerator");
		put("Alg.Alias.KeyGenerator.Rijndael", "AES");

		put("KeyGenerator.HmacMD5", 
		    "com.sun.crypto.provider.HmacMD5KeyGenerator");

		put("KeyGenerator.HmacSHA1", 
		    "com.sun.crypto.provider.HmacSHA1KeyGenerator");

		put("KeyPairGenerator.DiffieHellman", 
		    "com.sun.crypto.provider.DHKeyPairGenerator");
		put("Alg.Alias.KeyPairGenerator.DH", "DiffieHellman");

		/*
		 * Algorithm parameter generation engines
		 */
		put("AlgorithmParameterGenerator.DiffieHellman",
		    "com.sun.crypto.provider.DHParameterGenerator");
		put("Alg.Alias.AlgorithmParameterGenerator.DH",
		    "DiffieHellman");

		/* 
		 * Key Agreement engines 
		 */
		put("KeyAgreement.DiffieHellman",
		    "com.sun.crypto.provider.DHKeyAgreement");
		put("Alg.Alias.KeyAgreement.DH", "DiffieHellman");

		/* 
		 * Algorithm Parameter engines 
		 */
		put("AlgorithmParameters.DiffieHellman",
		    "com.sun.crypto.provider.DHParameters");
		put("Alg.Alias.AlgorithmParameters.DH", "DiffieHellman");

		put("AlgorithmParameters.DES",
		    "com.sun.crypto.provider.DESParameters");

		put("AlgorithmParameters.DESede",
		    "com.sun.crypto.provider.DESedeParameters");
		put("Alg.Alias.AlgorithmParameters.TripleDES", "DESede");

		put("AlgorithmParameters.PBE",
		    "com.sun.crypto.provider.PBEParameters");
		put("Alg.Alias.AlgorithmParameters.PBEWithMD5AndDES", "PBE");
		put("Alg.Alias.AlgorithmParameters.PBEWithMD5AndTripleDES", 
		    "PBE");

		put("AlgorithmParameters.Blowfish",
		    "com.sun.crypto.provider.BlowfishParameters");

		put("AlgorithmParameters.AES",
                    "com.sun.crypto.provider.AESParameters");
                put("Alg.Alias.AlgorithmParameters.Rijndael", "AES");

		/*
		 * Key factories
		 */
		put("KeyFactory.DiffieHellman",
		    "com.sun.crypto.provider.DHKeyFactory");
		put("Alg.Alias.KeyFactory.DH", "DiffieHellman");

		/*
		 * Secret-key factories
		 */
		put("SecretKeyFactory.DES", 
		    "com.sun.crypto.provider.DESKeyFactory");

		put("SecretKeyFactory.DESede",
		    "com.sun.crypto.provider.DESedeKeyFactory");
		put("Alg.Alias.SecretKeyFactory.TripleDES", "DESede");

		put("SecretKeyFactory.PBEWithMD5AndDES",
		    "com.sun.crypto.provider.PBEKeyFactory");
		put("Alg.Alias.SecretKeyFactory.PBEWithMD5AndTripleDES", 
		    "PBEWithMD5AndDES");

		/*
		 * MAC
		 */
		put("Mac.HmacMD5", "com.sun.crypto.provider.HmacMD5");
		put("Mac.HmacSHA1", "com.sun.crypto.provider.HmacSHA1");

		/*
		 * KeyStore
		 */
		put("KeyStore.JCEKS", "com.sun.crypto.provider.JceKeyStore");

		return null;
	    }
	});
    }

    static final boolean verifySelfIntegrity(Class c) {
	if (verifiedSelfIntegrity) {
	    return true;
	}
	
        return doSelfVerification(c);
    }

    private static final synchronized boolean doSelfVerification(Class c) {
	if (verifiedSelfIntegrity) {
	    return true;
	}

/* CERTIFICATE USED TO SIGN SUNJCE_PROVIDER.JAR
Owner: CN=Sun Microsystems Inc, OU=Java Software Code Signing, O=Sun Microsystems Inc
Issuer: CN=JCE Code Signing CA, OU=Java Software Code Signing, O=Sun Microsystems Inc, L=Palo Alto, ST=CA, C=US
Serial number: 104
Valid from: Fri Oct 19 16:04:31 PDT 2001 until: Mon Oct 23 16:04:31 PDT 2006
Certificate fingerprints:
         MD5:  3F:B9:8C:64:7A:0D:9F:3B:C9:F3:1A:36:9E:89:39:09
         SHA1: C2:E7:9B:2C:98:43:E1:AB:AD:FB:35:3B:81:F6:AB:E8:02:3C:18:21
*/
	final String PROVIDERCERT =
"-----BEGIN CERTIFICATE-----\n" +
"MIIDtDCCA3KgAwIBAgICAQQwCwYHKoZIzjgEAwUAMIGQMQswCQYDVQQGEwJVUzELMAkGA1UECBMC" +
"Q0ExEjAQBgNVBAcTCVBhbG8gQWx0bzEdMBsGA1UEChMUU3VuIE1pY3Jvc3lzdGVtcyBJbmMxIzAh" +
"BgNVBAsTGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMRwwGgYDVQQDExNKQ0UgQ29kZSBTaWdu" +
"aW5nIENBMB4XDTAxMTAxOTIzMDQzMVoXDTA2MTAyMzIzMDQzMVowYzEdMBsGA1UECgwUU3VuIE1p" +
"Y3Jvc3lzdGVtcyBJbmMxIzAhBgNVBAsMGkphdmEgU29mdHdhcmUgQ29kZSBTaWduaW5nMR0wGwYD" +
"VQQDDBRTdW4gTWljcm9zeXN0ZW1zIEluYzCCAbUwggEqBgUrDgMCDDCCAR8CgYEA/X9TgR11EilS" +
"30qcLuzk5/YRt1I870QAwx4/gLZRJmlFXUAiUftZPY1Y+r/F9bow9subVWzXgTuAHTRv8mZgt2uZ" +
"UKWkn5/oBHsQIsJPu6nX/rfGG/g7V+fGqKYVDwT7g/bTxR7DAjVUE1oWkTL2dfOuK2HXKu/yIgMZ" +
"ndFIAccCFQCXYFCPFSMLzLKSuYKi64QL8Fgc9QKBgQD34aCF1ps93su8q1w2uFe5eZSvu/o66oL5" +
"V0wLPQeCZ1FZV4661FlP5nEHEIGAtEkWcSPoTCgWE7fPCTKMyKbhPBZ6i1R8jSjgo64eK7OmdZFu" +
"o38L+iE1YvH7YnoBJDvMpPG+qFGQiaiD3+Fa5Z8GkotmXoB7VSVkAUw7/s9JKgOBhAACgYAHzPY4" +
"Os3TWJmQD3GvqtADJzt04WQ4Eb/6t78s57unki8Izif4tP3YFB2jlbsDFqa6vDXAzfn1bKeUWyMB" +
"+a71yeCBeujkaev49YAlBCyRc5ZZtAaDF7JQrE/rnVElPffusCQlDv60MqHEDrNmQeBXzp2+My6T" +
"msl6V9zNiGCnzqOBiDCBhTARBglghkgBhvhCAQEEBAMCBBAwDgYDVR0PAQH/BAQDAgXgMB0GA1Ud" +
"DgQWBBRVjR8qBaubzoYQrjtd9ro/IsVqyjAfBgNVHSMEGDAWgBRl4vSGydNO8JFOWKJq9dh4WprB" +
"pjAgBgNVHREEGTAXgRV5dS1jaGluZy5wZW5nQHN1bi5jb20wCwYHKoZIzjgEAwUAAy8AMCwCFHVL" +
"6CE3eHkK0LXcfjZ1ueQUtdBGAhRqUdy6bRprXBgjavHKIYp3wgUWQg==\n" +
"-----END CERTIFICATE-----";
	
	X509Certificate providerCert;
	try {
	    CertificateFactory certificateFactory = 
	    	CertificateFactory.getInstance("X.509");
	    byte[] b = PROVIDERCERT.getBytes("UTF8");
	    providerCert = (X509Certificate)certificateFactory.generateCertificate
	    					(new ByteArrayInputStream(b));
	} catch (Exception e) {
	    if (debug) {
		e.printStackTrace();
	    }
	    return false;
	}
	
	final Class cc = c;
	URL url = (URL)AccessController.doPrivileged(
					  new PrivilegedAction() {
	    public Object run() {
		CodeSource s1 = cc.getProtectionDomain().getCodeSource();
		return s1.getLocation();
	    }
	});
	if (url == null) {
	   return false;
	}

	JarVerifier jv = new JarVerifier(url);
	try {
	    jv.verify(providerCert);
	} catch (Exception e) {
	    return false;
	}
	
	verifiedSelfIntegrity = true;

	return true;
    }
}
