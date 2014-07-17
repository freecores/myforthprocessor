/*
 * @(#)SHA3.java	1.3 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

import java.security.*;
import java.math.BigInteger;

/**
 * This class implements the Secure Hash Algorithm SHA-384 developed by
 * the National Institute of Standards and Technology along with the
 * National Security Agency.
 *
 * <p>It implements java.security.MessageDigestSpi, and can be used 
 * through Java Cryptography Architecture (JCA), as a pluggable 
 * MessageDigest implementation.
 * 
 * @version     1.3 01/23/03
 * @author	Valerie Peng
 */

public class SHA3 extends SHA5 {

    private static final int LENGTH = 48;
    private static final long [] INITIAL_HASHES = {
	0xcbbb9d5dc1059ed8L, 0x629a292a367cd507L,
	0x9159015a3070dd17L, 0x152fecd8f70e5939L,
	0x67332667ffc00b31L, 0x8eb44a8768581511L,
	0xdb0c2e0d64f98fa7L, 0x47b5481dbefa4fa4L
    };

    public SHA3() {
	init();
    }

    private SHA3(SHA3 that) {
	super((SHA5)that);
    }

    void init() {
	super.init();
	setInitialHash(INITIAL_HASHES);
    }
 
    /**
     * Return the length of the digest in bytes
     */
    protected int engineGetDigestLength() {
	return (LENGTH);
    }
    
    /**
     * Computes the final hash and returns the final value as a
     * byte[48] array. The object is reset to be ready for further
     * use, as specified in the JavaSecurity MessageDigest
     * specification. 
     */
    protected byte[] engineDigest() {
	byte[] sha5hashvalue = super.engineDigest();
	byte[] hashvalue = new byte[LENGTH];
	System.arraycopy(sha5hashvalue, 0, hashvalue, 0, LENGTH);
	return hashvalue;
    }

    /**
     * Resets the buffers and hash value to start a new hash.
     */
    protected void engineReset() {
	init();
    }

    /**
     * Computes the final hash and returns the final value as a
     * byte[48] array. The object is reset to be ready for further
     * use, as specified in the JavaSecurity MessageDigest
     * specification.
     * @param hashvalue buffer to hold the digest
     * @param offset offset for storing the digest
     * @param len length of the buffer
     * @return length of the digest in bytes
     */
    protected int engineDigest(byte[] hashvalue, int offset, int len)
	throws DigestException {

	if (len < LENGTH)
		throw new DigestException("partial digests not returned");
	if (hashvalue.length - offset < LENGTH)
		throw new DigestException("insufficient space in the output " +
					"buffer to store the digest");
	super.performDigest(hashvalue, offset, LENGTH);
	return LENGTH;
    }

    /*
     * Clones this object.
     */
    public Object clone() {
        SHA3 that = null;
        that = new SHA3(this);
        return that;
    }

}

