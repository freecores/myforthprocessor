/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.security.spec.AlgorithmParameterSpec;

/**
 * This class specifies the parameters used with the
 * <a href="http://www.rsa.com/rsalabs/newfaq/q75.html"><i>RC2</i></a>
 * algorithm.
 *
 * <p> The parameters consist of an effective key size and optionally
 * an 8-byte initialization vector (IV) (only in feedback mode).
 *
 * <p> This class can be used to initialize a <code>Cipher</code> object that
 * implements the <i>RC2</i> algorithm.
 *
 * @author Jan Luehe
 *
 * @version 1.10, 06/24/03
 * @since 1.4
 */
public class RC2ParameterSpec implements AlgorithmParameterSpec {

    private byte[] iv = null;
    private int effectiveKeyBits;

    /**
     * Constructs a parameter set for RC2 from the given effective key size
     * (in bits).
     *
     * @param effectiveKeyBits the effective key size in bits.
     */
    public RC2ParameterSpec(int effectiveKeyBits) {
	this.effectiveKeyBits = effectiveKeyBits;
    }

    /**
     * Constructs a parameter set for RC2 from the given effective key size
     * (in bits) and an 8-byte IV.
     *
     * <p> The bytes that constitute the IV are those between
     * <code>iv[0]</code> and <code>iv[7]</code> inclusive.
     *
     * @param effectiveKeyBits the effective key size in bits.
     * @param iv the buffer with the 8-byte IV.
     */
    public RC2ParameterSpec(int effectiveKeyBits, byte[] iv) {
	this(effectiveKeyBits, iv, 0);
    }

    /**
     * Constructs a parameter set for RC2 from the given effective key size
     * (in bits) and IV.
     *
     * <p> The IV is taken from <code>iv</code>, starting at
     * <code>offset</code> inclusive.
     * The bytes that constitute the IV are those between
     * <code>iv[offset]</code> and <code>iv[offset+7]</code> inclusive.
     *
     * @param effectiveKeyBits the effective key size in bits.
     * @param iv the buffer with the IV.
     * @param offset the offset in <code>iv</code> where the 8-byte IV
     * starts.
     */
    public RC2ParameterSpec(int effectiveKeyBits, byte[] iv, int offset) {
	this.effectiveKeyBits = effectiveKeyBits;
	if (iv == null) throw new IllegalArgumentException("IV missing");
	int blockSize = 8;
        if (iv.length - offset < blockSize) {
            throw new IllegalArgumentException("IV too short");
        }
	this.iv = new byte[blockSize];
	System.arraycopy(iv, offset, this.iv, 0, blockSize);
    }

    /**
     * Returns the effective key size in bits.
     *
     * @return the effective key size in bits.
     */
    public int getEffectiveKeyBits() {
	return this.effectiveKeyBits;
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
     * object. Two RC2ParameterSpec objects are considered equal if their 
     * effective key sizes and IVs are equal.
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
	if (!(obj instanceof RC2ParameterSpec)) {
	    return false;
	}
	RC2ParameterSpec other = (RC2ParameterSpec) obj;

	return ((effectiveKeyBits == other.effectiveKeyBits) &&
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
	return (retval += effectiveKeyBits);
    }
}
