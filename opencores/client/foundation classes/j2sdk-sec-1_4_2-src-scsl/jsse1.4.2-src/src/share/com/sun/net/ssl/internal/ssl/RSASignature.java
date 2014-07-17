/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.util.Arrays;
import java.math.BigInteger;

import java.security.*;
import java.security.interfaces.*;

import javax.crypto.Cipher;

import COM.rsa.jsafe.*;

/**
 * Signature implementation for the SSL/TLS RSA Signature variant with both
 * MD5 and SHA-1 MessageDigests. Used for explicit RSA server authentication 
 * (RSA signed server key exchange for RSA_EXPORT and DHE_RSA) and RSA client
 * authentication (RSA signed certificate verify message).
 *
 * It conforms to the standard JCA Signature API. Note that it extends the
 * Signature class directly rather than the SignatureSpi. This works because
 * the Signature class was originally designed this way in JDK 1.1 days and
 * allows us to avoid creating an internal provider.
 *
 * The class should be instantiated via the getInstance() method in this class,
 * which returns an implementation from an external JCA provider, if possible, 
 * or the internal implementation otherwise. The internal implementation uses 
 * a JCE RSA implementation or JSSE internal JSafe, see the JsseJce class.
 *
 * This class is not thread safe.
 *
 * @version 1.29, 06/24/03
 */
final class RSASignature extends Signature {

    final static boolean rsaBlinding =
	Debug.getBooleanProperty("com.sun.crypto.rsa.blinding", true);

    private Cipher rsaCipher;
    private MessageDigest md5, sha;

    // flag indicating if the MessageDigests are in reset state
    private boolean isReset;

    private JSAFE_Signature jsafeRsa;

    private RSASignature() throws NoSuchAlgorithmException {
	super(JsseJce.SIGNATURE_RSA);
	try {
    	    rsaCipher = JsseJce.getCipher(JsseJce.CIPHER_RSA_PKCS1);
	} catch (GeneralSecurityException e) {
	    try {
		jsafeRsa = JSAFE_Signature.getInstance
				("NoDigest/RSA/PKCS1Block01Pad", "Java");
		if (rsaBlinding) {
		    jsafeRsa.setBlinding(JSAFE_AsymmetricCipher.BLINDING);
		}
	    } catch (Exception e2) {
		throw new NoSuchAlgorithmException("RSA not available");
	    }
	}
	md5 = JsseJce.getMD5();
	sha = JsseJce.getSHA();
	isReset = true;
    }
    
    /**
     * Get an implementation for the RSA signature. JCA provider
     * implementation if possible, internal implementation otherwise.
     */
    static Signature getInstance() throws NoSuchAlgorithmException {
        try {
	    return JsseJce.getSignature(JsseJce.SIGNATURE_RSA);
	} catch (Exception e) {
	    return new RSASignature();
	}
    }

    /**
     * Get an internal implementation for the RSA signature. Used for RSA
     * client authentication, which needs the ability to set the digests
     * to externally provided values via the setHashes() method.
     */
    static Signature getInternalInstance() throws NoSuchAlgorithmException {
	return new RSASignature();
    }
    
    /**
     * Set the MD5 and SHA hashes to the provided objects.
     */
    void setHashes(MessageDigest md5, MessageDigest sha) {
	this.md5 = md5;
	this.sha = sha;
    }
    
    /**
     * Reset the MessageDigests unless they are already reset.
     */
    private void reset() {
	if (isReset == false) {
	    md5.reset();
	    sha.reset();
	    isReset = true;
	}
    }

    protected void engineInitVerify(PublicKey publicKey) 
	    throws InvalidKeyException {
	reset();
	if (rsaCipher != null) {
	    rsaCipher.init(Cipher.DECRYPT_MODE, publicKey);
	} else {
	    jsafeInitVerify(publicKey);
	}
    }

    protected void engineInitSign(PrivateKey privateKey) 
	    throws InvalidKeyException {
	reset();
	if (rsaCipher != null) {
	    rsaCipher.init(Cipher.ENCRYPT_MODE, privateKey);
	} else {
	    jsafeInitSign(privateKey, null);
	}
    }

    protected void engineInitSign(PrivateKey privateKey, SecureRandom random) 
	    throws InvalidKeyException {
	reset();
	if (rsaCipher != null) {
	    rsaCipher.init(Cipher.ENCRYPT_MODE, privateKey, random);
	} else {
	    jsafeInitSign(privateKey, random);
	}
    }

    protected void engineUpdate(byte b) {
	isReset = false;
	md5.update(b);
	sha.update(b);
    }

    protected void engineUpdate(byte[] b, int off, int len) {
	isReset = false;
	md5.update(b, off, len);
	sha.update(b, off, len);
    }

    private byte[] getDigest() throws DigestException {
	byte[] data = new byte[36];
	md5.digest(data, 0, 16);
	sha.digest(data, 16, 20);
	isReset = true;
	return data;
    }

    protected byte[] engineSign() throws SignatureException {
	if (rsaCipher == null) {
	    return jsafeSign();
	}
	try {
	    byte[] signature = rsaCipher.doFinal(getDigest());
	    return signature;
	} catch (GeneralSecurityException e) {
	    throw (SignatureException)new SignatureException().initCause(e);
	}
    }

    protected boolean engineVerify(byte[] sigBytes) throws SignatureException {
	return engineVerify(sigBytes, 0, sigBytes.length);
    }

    protected boolean engineVerify(byte[] sigBytes, int offset, int length) 
	    throws SignatureException {
	if (rsaCipher == null) {
	    return jsafeVerify(sigBytes, offset, length);
	}
	try {
	    byte[] decryptedData = rsaCipher.doFinal(sigBytes, offset, length);
	    byte[] data = getDigest();
	    return Arrays.equals(data, decryptedData);
	} catch (GeneralSecurityException e) {
	    throw (SignatureException)new SignatureException().initCause(e);
	}
    }

    protected void engineSetParameter(String param, Object value) 
	    throws InvalidParameterException {
	throw new InvalidParameterException("Parameters not supported");
    }

    protected Object engineGetParameter(String param) 
	    throws InvalidParameterException {
	throw new InvalidParameterException("Parameters not supported");
    }
    
    private void jsafeInitSign(PrivateKey privateKey, SecureRandom sr) 
	    throws InvalidKeyException {
	try {
	    // create JSAFE private-key object 
	    JSAFE_PrivateKey jsafePrivKey = JSAFE_PrivateKey.getInstance
	    						("RSA", "Java");

	    if (privateKey instanceof RSAPrivateCrtKey) {
		RSAPrivateCrtKey crtKey = (RSAPrivateCrtKey)privateKey;
		BigInteger p = crtKey.getPrimeP();
		BigInteger q = crtKey.getPrimeQ();
		BigInteger pe = crtKey.getPrimeExponentP();
		BigInteger qe = crtKey.getPrimeExponentQ();
	        byte[][] privKeyData = {
		    crtKey.getModulus().toByteArray(),
		    crtKey.getPublicExponent().toByteArray(),
		    crtKey.getPrivateExponent().toByteArray(),
		    p.toByteArray(),
		    q.toByteArray(),
		    pe.toByteArray(),
		    qe.toByteArray(),
		    crtKey.getCrtCoefficient().toByteArray(),
		};
		jsafePrivKey.setKeyData("RSAPrivateKeyCRT", privKeyData);
	    } else {
		RSAPrivateKey rsaKey = (RSAPrivateKey)privateKey;
	        byte[][] privKeyData = {
		    rsaKey.getModulus().toByteArray(),
		    rsaKey.getPrivateExponent().toByteArray()
	        };
	        jsafePrivKey.setKeyData("RSAPrivateKey", privKeyData);
	    }

	    jsafeRsa.signInit(jsafePrivKey, sr);

 	} catch (JSAFE_Exception e) {
	    throw (InvalidKeyException)new InvalidKeyException().initCause(e);
	} catch (Exception e) {
	    // should never happen
	    throw (IllegalArgumentException)
	    			new IllegalArgumentException().initCause(e);
	}
    }
    
    private void jsafeInitVerify(PublicKey publicKey) 
	    throws InvalidKeyException {
	try {
	    // create JSAFE public-key object
	    JSAFE_PublicKey jsafePubKey = 
	    			JSAFE_PublicKey.getInstance("RSA", "Java");
	    RSAPublicKey rsaKey = (RSAPublicKey)publicKey;
	    byte[][] pubKeyData = {
		rsaKey.getModulus().toByteArray(),
		rsaKey.getPublicExponent().toByteArray()
	    };
	    jsafePubKey.setKeyData(pubKeyData);

	    jsafeRsa.verifyInit(jsafePubKey, null);

 	} catch (JSAFE_Exception e) {
	    throw (InvalidKeyException)new InvalidKeyException().initCause(e);
	} catch (Exception e) {
	    // should never happen
	    throw (IllegalArgumentException)
	    			new IllegalArgumentException().initCause(e);
	}
    }
    
    private byte[] jsafeSign() throws SignatureException {
	try {
	    byte[] digest = getDigest();
	    jsafeRsa.signUpdate(digest, 0, digest.length);
	    return jsafeRsa.signFinal();
 	} catch (JSAFE_Exception e) {
	    throw (SignatureException)new SignatureException().initCause(e);
	} catch (Exception e) {
	    // should never happen
	    throw (IllegalArgumentException)
	    			new IllegalArgumentException().initCause(e);
	}
    }
    
    private boolean jsafeVerify(byte[] sigBytes, int offset, int length) 
	    throws SignatureException {
	try {
	    byte[] digest = getDigest();
	    jsafeRsa.verifyUpdate(digest, 0, digest.length);
	    return jsafeRsa.verifyFinal(sigBytes, offset, length);
 	} catch (JSAFE_Exception e) {
	    throw (SignatureException)new SignatureException().initCause(e);
	} catch (Exception e) {
	    // should never happen
	    throw (IllegalArgumentException)
	    			new IllegalArgumentException().initCause(e);
	}
    }

}
