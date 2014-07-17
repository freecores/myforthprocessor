/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.DigestException;

class PRF {

    // constants for the various labels used in the TLS PRF
    // these are defined as byte arrays to avoid problems with
    // character encodings. these constants are referenced from
    // HandShaker and HandshakeMessage

    // NOTE that the final modifier does not guarantee that the array
    // contents remain unmodified. However, these are package private
    // so no "bad" code will ever be able to access them.

    final static byte[] LABEL_MASTER_SECRET = // "master secret"
        { 109, 97, 115, 116, 101, 114, 32, 115, 101, 99, 114, 101, 116 };

    final static byte[] LABEL_KEY_EXPANSION = // "key expansion"
        { 107, 101, 121, 32, 101, 120, 112, 97, 110, 115, 105, 111, 110 };

    final static byte[] LABEL_CLIENT_WRITE_KEY = // "client write key"
        { 99, 108, 105, 101, 110, 116, 32, 119, 114, 105, 116, 101, 32, 
	  107, 101, 121 };

    final static byte[] LABEL_SERVER_WRITE_KEY = // "server write key"
        { 115, 101, 114, 118, 101, 114, 32, 119, 114, 105, 116, 101, 32, 
	  107, 101, 121 };

    final static byte[] LABEL_IV_BLOCK = // "IV block"
        { 73, 86, 32, 98, 108, 111, 99, 107 };

    final static byte[] LABEL_CLIENT_FINISHED = // "client finished"
        { 99, 108, 105, 101, 110, 116, 32, 
	  102, 105, 110, 105, 115, 104, 101, 100 };

    final static byte[] LABEL_SERVER_FINISHED = // "server finished"
        { 115, 101, 114, 118, 101, 114, 32, 
	  102, 105, 110, 105, 115, 104, 101, 100 };

    /*
     * @param secret the secret
     * @param label the label
     * @param seed1 first part of the seed
     * @param seed2 second part of the seec
     * @param output the output buffer where the result is stored
     */
    static final void compute(MessageDigest md5Digest, MessageDigest shaDigest,
			      byte[] secret,
			      byte[] labelBytes,
			      byte[] seed1,
			      byte[] seed2,
			      byte[] output)
    {
	/*
	 * Split the secret into two halves S1 and S2 of same length.
	 * S1 is taken from the first half of the secret, S2 from the
	 * second half.
	 * Their length is created by rounding up the length of the
	 * overall secret divided by two; thus, if the original secret
	 * is an odd number of bytes long, the last byte of S1 will be
	 * the same as the first byte of S2.
	 *
	 * Note: Instead of creating S1 and S2, we determine the offset into
	 * the overall secret where S2 starts.
	 */
	int seclen = 0;
	int off = 0;
	if (secret != null) {
	    seclen = off = (secret.length / 2);
	    if ((secret.length % 2) != 0) {
		// odd length
		seclen++;
	    }
	}

	// P_MD5(S1, label + seed)
	byte[] expanded_1 = expand(md5Digest, 16, output.length, secret, 0,
				   seclen, labelBytes, seed1, seed2);

	// P_SHA-1(S2, label + seed)
	byte[] expanded_2 = expand(shaDigest, 20, output.length, secret, off,
				   seclen, labelBytes, seed1, seed2);

	for (int i = 0; i < output.length; i++)
	    expanded_1[i] ^= expanded_2[i];

	System.arraycopy(expanded_1, 0, output, 0, output.length);
    }

    /*
     * @param digest the MessageDigest to produce the HMAC
     * @param hmacSize the HMAC size
     * @param targetSize the required quantity of data that needs to be
     *        produced
     * @param secret the secret
     * @param secOff the offset into the secret
     * @param secLen the secret length
     * @param label the label
     * @param seed1 first part of the seed
     * @param seed2 second part of the seed
     */
    private static final byte[] expand(MessageDigest digest,
				       int hmacSize,
				       int targetSize,
				       byte[] secret,
				       int secOff,
				       int secLen,
				       byte[] label,
				       byte[] seed1,
				       byte[] seed2)
    {
	/*
	 * modify the padding used, by XORing the key into our copy of that
	 * padding.  That's to avoid doing that for each HMAC computation.
	 */
	byte[] pad1 = (byte[])MAC.HMAC_ipad.clone();
	byte[] pad2 = (byte[])MAC.HMAC_opad.clone();

	for (int i = 0; i < secLen; i++) {
	    pad1[i] ^= secret[i+secOff]; // XXX XOR with 0x00 id op?
	    pad2[i] ^= secret[i+secOff];
	}

	// determine number of HMAC rounds
	int rounds = (targetSize / hmacSize);
	if ((targetSize % hmacSize) != 0)
	    rounds++;

	byte[] aBytes = null;
	byte[] ret = new byte[rounds*hmacSize];
	int retOff = 0;

	/*
	 * compute:
	 *
	 *     P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
	 *                            HMAC_hash(secret, A(2) + seed) +
	 *                            HMAC_hash(secret, A(3) + seed) + ...
	 * A() is defined as:
	 * 
	 *     A(0) = seed
	 *     A(i) = HMAC_hash(secret, A(i-1))
	 */
	for (int i = 0; i < rounds; i++) {
	    /*
	     * compute A() ...
	     */
	    // inner digest
	    digest.update(pad1);
	    byte[] hash1;
	    if (aBytes == null) {
		digest.update(label);
		digest.update(seed1);  // XXX check if both provided
		hash1 = digest.digest(seed2);
	    } else {
		hash1 = digest.digest(aBytes);
	    }
	 
	    // outer digest
	    digest.update(pad2);
	    aBytes = digest.digest(hash1);
	
	    /*
	     * compute HMAC_hash() ...
	     */
	    // inner digest
	    digest.update(pad1);
	    digest.update(aBytes);
	    digest.update(label);
	    digest.update(seed1); // XXX check if both seeds provided
	    hash1 = digest.digest(seed2);
	    
	    // outer digest
	    digest.update(pad2);
	    digest.update(hash1);
	    try {
		digest.digest(ret, retOff, ret.length - retOff);
	    } catch (DigestException de) {
		// this should never happen
		throw new IllegalArgumentException("buffer for expanded " +
						   "data too small");
	    }
	    retOff += hmacSize;
	}

	return ret;
    }
}
