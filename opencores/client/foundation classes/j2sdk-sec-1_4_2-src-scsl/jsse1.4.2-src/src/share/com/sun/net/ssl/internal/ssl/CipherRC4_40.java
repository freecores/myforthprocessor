/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.security.NoSuchAlgorithmException;

import sun.misc.HexDumpEncoder;

import COM.rsa.jsafe.*;


/*
 * RC4_40 stream cipher.
 *
 * @version 1.14 06/24/03
 * @author David Brownell
 */
final class CipherRC4_40 extends CipherBox {

    static boolean canDo;

    static boolean hasRC4() { 
        if (canDo) {
	    return true;
	}
	// JCE providers may change dynamically, do not cache result
	try {
	    JsseJce.getCipher(JsseJce.CIPHER_RC4);
	    return true;
	} catch (NoSuchAlgorithmException e) {
	    return false;
	}
    }

    static {
        try {
            JSAFE_SymmetricCipher.getInstance("RC4", "Java");
            canDo = true;
        } catch (Exception e) {
        }
    }
    
    private CipherRC4_40(ProtocolVersion protocolVersion) {
	super(protocolVersion);
    }

    private JSAFE_SymmetricCipher encryptor;

    int decrypt(byte buf [], int offset, int len) {
	if ((buf.length - offset) < len) {
	    throw new IllegalArgumentException();
	}

	try {
	    // RC4 encryption and decryption are the same
	    encryptor.encryptUpdate(buf, offset, len, buf, offset);
	} catch (Exception e) {
	    throw new IllegalArgumentException(e.getMessage());
	}

	if (debug != null && Debug.isOn("plaintext")) {
	    try {
		HexDumpEncoder hd = new HexDumpEncoder();

		System.out.println(
		    "Plaintext after DECRYPTION:  len = " + len);
		hd.encodeBuffer(
		    new ByteArrayInputStream(buf, offset, len),
		    System.out);
	    } catch (IOException e) { }
	}

	return len;
    }

    int encrypt(byte buf [], int offset, int len) {
	if ((buf.length - offset) < len) {
	    throw new IllegalArgumentException();
	}

	if (debug != null && Debug.isOn("plaintext")) {
	    try {
		HexDumpEncoder hd = new HexDumpEncoder();

		System.out.println(
		    "Plaintext before ENCRYPTION:  len = " + len);
		hd.encodeBuffer(
		    new ByteArrayInputStream(buf, offset, len),
		    System.out);
	    } catch (IOException e) { }
	}

	try {
	    encryptor.encryptUpdate(buf, offset, len, buf, offset);
	} catch (Exception e) {
	    throw new IllegalArgumentException(e.getMessage());
	}

	return len;
    }

    public String toString() {
	return "RC4_40";
    }

    void init(byte[] key) throws NoSuchAlgorithmException {
	try {
	    JSAFE_SecretKey secretKey = JSAFE_SecretKey.getInstance("RC4",
								    "Java");
	    secretKey.setSecretKeyData(key, 0, key.length);
	    encryptor = JSAFE_SymmetricCipher.getInstance("RC4",
							  "Java");
	    encryptor.encryptInit(secretKey);
	} catch (JSAFE_UnimplementedException ue) {
	    throw new NoSuchAlgorithmException(ue.getMessage());
	} catch (Exception e) {
	    // should never happen
	    throw new IllegalArgumentException(e.getMessage());
	}
    }

    static CipherBox newCipherBox(ProtocolVersion protocolVersion,
	    byte[] key, byte[] iv, boolean encrypt)
	    throws NoSuchAlgorithmException {
        try {
	    JCECipherBox box = new JCECipherBox(protocolVersion,
	        "RC4_40", JsseJce.CIPHER_RC4);
            box.initCipher(key, iv, encrypt);
	    return box;
	} catch (Exception e) {
	    // empty
	} catch (ExceptionInInitializerError e) {
	    // empty
	}

	if (!canDo) {
	    throw new NoSuchAlgorithmException("RC4_40 stream cipher not available");
	}
	if (iv != null && iv.length != 0) {
	    throw new IllegalArgumentException();
	}
        if (key == null) {
            throw new IllegalArgumentException();
	}

	CipherRC4_40 retval = new CipherRC4_40(protocolVersion);
	retval.init(key);

	return retval;
    }

}

