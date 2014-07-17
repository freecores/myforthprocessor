/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.math.BigInteger;

import java.security.*;
import java.security.Provider;
import java.security.interfaces.*;

import javax.crypto.*;
import javax.crypto.Cipher;

import javax.net.ssl.SSLException;

import COM.rsa.jsafe.*;

/**
 * This class defines an interface for RSA PKCS#1 block type 02 encryption.
 * With the JSSE/JCE integration has become an abstract class with one
 * implementation providing an adapter to the JSAFE API and another one
 * functioning as adapter to the JCE layer.
 *
 * @author  Jan Luehe
 * @version 1.8 06/24/03
 */
abstract class RSACipher {

    RSACipher() {
        // empty
    }
    
    abstract void encryptInit(RSAPublicKey pubKey, SecureRandom random)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException;

    abstract void decryptInit(RSAPrivateKey privKey)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException;

    abstract byte[] doEncrypt(byte[] in, int offset, int len)
	throws SSLException;

    abstract byte[] doDecrypt(byte[] in, int offset, int len)
	throws SSLException;

    /**
     * Return an RSACipher implementation. Use JCE implementation if
     * possible, JSAFE otherwise.
     */
    static RSACipher getInstance() throws NoSuchAlgorithmException {
        try {
	    return new JCE_RSACipher();
	} catch (Exception e) {
	    // empty
	} catch (ExceptionInInitializerError e) {
	    // empty
	}
	return new JSAFE_RSACipher();
    }
}

final class JCE_RSACipher extends RSACipher {

    private Cipher cipher;

    JCE_RSACipher() throws NoSuchAlgorithmException {
        cipher = JsseJce.getCipher(JsseJce.CIPHER_RSA_PKCS1);
    }

    void encryptInit(RSAPublicKey pubKey, SecureRandom random)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException {

        cipher.init(Cipher.ENCRYPT_MODE, pubKey, random);
    }


    void decryptInit(RSAPrivateKey privKey)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException {

        cipher.init(Cipher.DECRYPT_MODE, privKey);
    }

    byte[] doEncrypt(byte[] in, int offset, int len) throws SSLException {
        try {
            byte[] enc = cipher.doFinal(in, offset, len);
    	    return enc;
	} catch (Exception e) {
	    throw new SSLException(e.toString());
	}
    }

    byte[] doDecrypt(byte[] in, int offset, int len) throws SSLException {
        try {
            byte[] dec = cipher.doFinal(in, offset, len);
    	    return dec;
	} catch (Exception e) {
	    throw new SSLException(e.toString());
	}
    }
}

final class JSAFE_RSACipher extends RSACipher {

    private JSAFE_AsymmetricCipher cipher;

    JSAFE_RSACipher()
	throws NoSuchAlgorithmException
    {
	try {
	    cipher = JSAFE_AsymmetricCipher.getInstance("RSA/PKCS1Block02Pad",
							"Java");
	    if (RSASignature.rsaBlinding) {
	        cipher.setBlinding(JSAFE_AsymmetricCipher.BLINDING);
	    }
	} catch (JSAFE_UnimplementedException ue) {
	    throw new NoSuchAlgorithmException(ue.getMessage());
	} catch (JSAFE_InvalidParameterException ipe) {
	    throw new IllegalArgumentException(ipe.getMessage());
	}
    }

    void encryptInit(RSAPublicKey pubKey, SecureRandom random)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException
    {
	try {
	    JSAFE_PublicKey jsafeKey = JSAFE_PublicKey.getInstance("RSA",
								   "Java");
	    byte[][] pubKeyData = {
		pubKey.getModulus().toByteArray(),
		pubKey.getPublicExponent().toByteArray()
	    };
	    jsafeKey.setKeyData(pubKeyData);
	    
	    cipher.encryptInit(jsafeKey, random);
	} catch (JSAFE_UnimplementedException ue) {
	    throw new NoSuchAlgorithmException(ue.getMessage());
	} catch (JSAFE_InvalidKeyException ike) {
	    throw new InvalidKeyException(ike.getMessage());
	} catch (Exception e) {
	    throw new SSLException(e.getMessage());
	}
    }

    void decryptInit(RSAPrivateKey privKey)
	throws InvalidKeyException, NoSuchAlgorithmException, SSLException
    {
	try {
	    JSAFE_PrivateKey jsafeKey = JSAFE_PrivateKey.getInstance("RSA",
								     "Java");
	    if (privKey instanceof RSAPrivateCrtKey) {
		RSAPrivateCrtKey crtKey = (RSAPrivateCrtKey)privKey;
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
		jsafeKey.setKeyData("RSAPrivateKeyCRT", privKeyData);
	    } else {
	        byte[][] privKeyData = {
		    privKey.getModulus().toByteArray(),
		    privKey.getPrivateExponent().toByteArray()
	        };
	        jsafeKey.setKeyData("RSAPrivateKey", privKeyData);
	    }
	    cipher.decryptInit(jsafeKey);
	} catch (JSAFE_UnimplementedException ue) {
	    throw new NoSuchAlgorithmException(ue.getMessage());
	} catch (JSAFE_InvalidKeyException ike) {
	    throw new InvalidKeyException(ike.getMessage());
	} catch (Exception e) {
	    throw new SSLException(e.getMessage());
	}
    }

    byte[] doEncrypt(byte[] in, int offset, int len)
	throws SSLException
    {
	int outputLenEncrypt = 0;
	byte[] out = new byte[cipher.getOutputBufferSize(in.length)];

	try {
	    outputLenEncrypt = cipher.encryptUpdate(in, 0, in.length,
						    out, 0);
	    outputLenEncrypt += cipher.encryptFinal(out, outputLenEncrypt);
	} catch (Exception e) {
	    throw new SSLException(e.getMessage());
	}

	// adjust length if necessary
	if (out.length != outputLenEncrypt) {
	    byte[] tmpOut = new byte[outputLenEncrypt];
	    System.arraycopy(out, 0, tmpOut, 0, outputLenEncrypt);
	    out = tmpOut;
	}

	return out;
    }

    byte[] doDecrypt(byte[] in, int offset, int len)
	throws SSLException
    {
	int outputLenDecrypt = 0;
	byte[] out = new byte[in.length];

	try {
	    outputLenDecrypt = cipher.decryptUpdate(in, 0, in.length,
						    out, 0);
	    outputLenDecrypt += cipher.decryptFinal(out, outputLenDecrypt);
	} catch (Exception e) {
	    throw new SSLException(e.getMessage());
	}

	// adjust length if necessary
	if (out.length != outputLenDecrypt) {
	    byte[] tmpOut = new byte[outputLenDecrypt];
	    System.arraycopy(out, 0, tmpOut, 0, outputLenDecrypt);
	    out = tmpOut;
	}

	return out;
    }
}
