/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.security.spec.AlgorithmParameterSpec;

/**
 * This class specifies the parameters used with the
 * <a href="http://www.rsa.com/rsalabs/newfaq/q76.html"><i>RC5</i></a>
 * algorithm.
 *
 * <p> The parameters consist of a version number, a rounds count, a word
 * size, and optionally an initialization vector (IV) (only in feedback mode).
 *
 * <p> This class can be used to initialize a <code>Cipher</code> object that
 * implements the <i>RC5</i> algorithm as supplied by
 * <a href="http://www.rsa.com">RSA Data Security, Inc.</a> (RSA DSI),
 * or any parties authorized by RSA DSI.
 *
 * @author Jan Luehe
 *
 * @version 1.11, 06/24/03
 * @since 1.4
 */
public class RC5ParameterSpec implements AlgorithmParameterSpec {

    private byte[] iv = null;
    private int version;
    private int rounds;
    private int wordSize; // the word size in bits

    /**
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds and word size (in bits).
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize) {
	this.version = version;
	this.rounds = rounds;
	this.wordSize = wordSize;
    }

    /**
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds, word size (in bits), and IV.
     *
     * <p> Note that the size of the IV (block size) must be twice the word
     * size. The bytes that constitute the IV are those between
     * <code>iv[0]</code> and <code>iv[2*(wordSize/8)-1]</code> inclusive.
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     * @param iv the buffer with the IV.
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize, byte[] iv) {
	this(version, rounds, wordSize, iv, 0);
    }

    /**
     * Constructs a parameter set for RC5 from the given version, number of
     * rounds, word size (in bits), and IV.
     *
     * <p> The IV is taken from <code>iv</code>, starting at
     * <code>offset</code> inclusive.
     * Note that the size of the IV (block size), starting at
     * <code>offset</code> inclusive, must be twice the word size.
     * The bytes that constitute the IV are those between
     * <code>iv[offset]</code> and <code>iv[offset+2*(wordSize/8)-1]</code>
     * inclusive.
     *
     * @param version the version.
     * @param rounds the number of rounds.
     * @param wordSize the word size in bits.
     * @param iv the buffer with the IV.
     * @param offset the offset in <code>iv</code> where the IV starts.
     */
    public RC5ParameterSpec(int version, int rounds, int wordSize,
			    byte[] iv, int offset) {
	this.version = version;
	this.rounds = rounds;
	this.wordSize = wordSize;
	if (iv == null) throw new IllegalArgumentException("IV missing");
	int blockSize = (wordSize / 8) * 2;
        if (iv.length - offset < blockSize) {
            throw new IllegalArgumentException("IV too short");
        }
	this.iv = new byte[blockSize];
	System.arraycopy(iv, offset, this.iv, 0, blockSize);
    }

    /**
     * Returns the version.
     *
     * @return the version.
     */
    public int getVersion() {
	return this.version;
    }

    /**
     * Returns the number of rounds.
     *
     * @return the number of rounds.
     */
    public int getRounds() {
	return this.rounds;
    }

    /**
     * Returns the word size in bits.
     *
     * @return the word size in bits.
     */
    public int getWordSize() {
	return this.wordSize;
    }

    /**
     * Returns the IV or null if this parameter set does not contain an IV.
     *
     * @return the IV or null if this parameter set does not contain an IV.
     */
    public byte[] getIV() {
	return (iv == null? null:(byte[])iv.clone());
    }

   /**
     * Tests for equality between the specified object and this
     * object. Two RC5ParameterSpec objects are considered equal if their 
     * version numbers, number of rounds, word sizes, and IVs are equal.
     * (Two IV references are considered equal if both are <tt>null</tt>.)
     * 
     * @param obj the object to test for equality with this object.
     * 
     * @return true if the objects are considered equal, false otherwise.
     */
    public boolean equals(Object obj) {
	if (obj == this) {
	    return true;
	}
	if (!(obj instanceof RC5ParameterSpec)) {
	    return false;
	}
	RC5ParameterSpec other = (RC5ParameterSpec) obj;

	return ((version == other.version) &&
		(rounds == other.rounds) &&
		(wordSize == other.wordSize) &&
		java.util.Arrays.equals(iv, other.iv));
    }

    /**
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() {
        int retval = 0;
	if (iv != null) {
	    for (int i = 1; i < iv.length; i++) {
		retval += iv[i] * i;
	    }
	}
	retval += (version + rounds + wordSize);
	return retval;
    }
}
