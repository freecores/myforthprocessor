/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import javax.crypto.MacSpi;
import javax.crypto.SecretKey;
import java.security.*;
import java.security.spec.*;

/**
 * This class constitutes the core of HMAC-<MD> algorithms, where
 * <MD> can be SHA1 or MD5, etc.
 *
 * @author Jan Luehe
 * @version 1.3, 06/24/03
 */
final class HmacCore implements Cloneable {

    private MessageDigest md;
    private byte[] k_ipad; // inner padding - key XORd with ipad
    private byte[] k_opad; // outer padding - key XORd with opad
    private boolean first = true; // Is this the first data to be processed?

    private int blockLen = 0;

    /**
     * Standard constructor, creates a new HmacCore instance.
     */
    HmacCore(MessageDigest md, int bl) throws NoSuchAlgorithmException {
	this.md = md;
	this.blockLen = bl;
	this.k_ipad = new byte[blockLen];
	this.k_opad = new byte[blockLen];
    }

    /** 
     * Returns the length of the HMAC in bytes.
     *
     * @return the HMAC length in bytes.
     */
    int getDigestLength() {
	return this.md.getDigestLength();
    }

    /**
     * Initializes the HMAC with the given secret key and algorithm parameters.
     *
     * @param key the secret key.
     * @param params the algorithm parameters.
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this MAC.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this MAC.
     */
    void init(Key key, AlgorithmParameterSpec params)
	throws InvalidKeyException, InvalidAlgorithmParameterException {

	int i;

	if (!(key instanceof SecretKey)) {
	    throw new InvalidKeyException("Secret key expected");
	}

	byte[] secret = key.getEncoded();
	if (secret == null || secret.length == 0) {
	    throw new InvalidKeyException("Missing key data");
	}
	    
	// if key is longer than the block length, reset it using
	// the message digest object.
	if (secret.length > blockLen) {
	    secret = md.digest(secret);
	}

	byte[] k = null;
	if (secret.length == blockLen) {
	    k = secret;
	} else {
	    // append 0x00 bytes to the end of key, to make it
	    // blockLen bytes long
	    k = new byte[blockLen];
	    System.arraycopy(secret, 0, k, 0, secret.length);
	    for (i=secret.length; i<blockLen; i++) {
		k[i] = (byte)0x00;
	    }
	}

	// XOR k with ipad and opad, respectively
	for (i=0; i<blockLen; i++) {
	    k_ipad[i] = (byte)(k[i] ^ (byte)0x36);
	    k_opad[i] = (byte)(k[i] ^ (byte)0x5c);
	}

	// now erase the secret
	for (i=0; i<secret.length; i++) {
	    secret[i] = (byte)0x00;
	}
	if (k != secret) {
	    for (i=0; i<k.length; i++) {
		k[i] = (byte)0x00;
	    }
	}
	
	// help out the garbage collector
	secret = null;
	k = null;
    }

    /**
     * Processes the given byte.    
     * 
     * @param input the input byte to be processed.
     */
    void update(byte input) {
	if (first == true) {
	    // compute digest for 1st pass; start with inner pad
	    md.update(k_ipad);
	    first = false;
	}

	// add the passed byte to the inner digest
	md.update(input);
    }

    /**
     * Processes the first <code>len</code> bytes in <code>input</code>,
     * starting at <code>offset</code>.
     * 
     * @param input the input buffer.
     * @param offset the offset in <code>input</code> where the input starts.
     * @param len the number of bytes to process.
     */
    void update(byte input[], int offset, int len) {
	if (first == true) {
	    // compute digest for 1st pass; start with inner pad
	    md.update(k_ipad);
	    first = false;
	}

	// add the selected part of an array of bytes to the inner digest
	md.update(input, offset, len);
    }

    /**
     * Completes the HMAC computation and resets the HMAC for further use,
     * maintaining the secret key that the HMAC was initialized with.
     *
     * @return the HMAC result.
     */
    byte[] doFinal() {
	if (first == true) {
	    // compute digest for 1st pass; start with inner pad
	    md.update(k_ipad);
	} else {
	    first = true;
	}

	// finish the inner digest
	byte[] innerDigest = md.digest();

	// compute digest for 2nd pass; start with outer pad
	md.update(k_opad);

	// add result of 1st hash
	return md.digest(innerDigest);
    }

    /**
     * Resets the HMAC for further use, maintaining the secret key that the
     * HMAC was initialized with.
     */
    void reset() {
	md.reset();
	first = true;
    }

    /*
     * Clones this object.
     */
    public Object clone() {
	HmacCore that = null;
	try {
	    that = (HmacCore)super.clone();
	    that.md = (MessageDigest)this.md.clone();
	    that.k_ipad = (byte[])this.k_ipad.clone();
	    that.k_opad = (byte[])this.k_opad.clone();
	    that.first = this.first;
	} catch (CloneNotSupportedException e) {
	}
	return that;
    }
}


