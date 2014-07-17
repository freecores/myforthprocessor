/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.security.InvalidKeyException;

/**
 * This class specifies a DES key.
 *
 * @author Jan Luehe
 *
 * @version 1.18, 06/24/03
 * @since 1.4
 */
public class DESKeySpec implements java.security.spec.KeySpec {

    /** 
     * The constant which defines the length of a DES key in bytes.
     */
    public static final int DES_KEY_LEN = 8;

    private byte[] key;

    /* Mask to check for parity adjustment */
    private static final byte[] PARITY_BIT_MASK = {
	(byte)0x80, (byte)0x40, (byte)0x20, (byte)0x10,
	(byte)0x08, (byte)0x04, (byte)0x02
    };

    /* Weak and semi-weak DES keys */
    private static final byte[][] WEAK_KEYS = { 

	{ (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01, (byte)0x01,
	  (byte)0x01, (byte)0x01, (byte)0x01 },

	{ (byte)0xFE, (byte)0xFE, (byte)0xFE, (byte)0xFE, (byte)0xFE,
	  (byte)0xFE, (byte)0xFE, (byte)0xFE },

	{ (byte)0x1F, (byte)0x1F, (byte)0x1F, (byte)0x1F, (byte)0x1F,
	  (byte)0x1F, (byte)0x1F, (byte)0x1F },
	
	{ (byte)0xE0, (byte)0xE0, (byte)0xE0, (byte)0xE0, (byte)0xE0,
	  (byte)0xE0, (byte)0xE0, (byte)0xE0 },

	{ (byte)0x01, (byte)0xFE, (byte)0x01, (byte)0xFE, (byte)0x01,
	  (byte)0xFE, (byte)0x01, (byte)0xFE },

	{ (byte)0x1F, (byte)0xE0, (byte)0x1F, (byte)0xE0, (byte)0x0E,
	  (byte)0xF1, (byte)0x0E, (byte)0xF1 },

	{ (byte)0x01, (byte)0xE0, (byte)0x01, (byte)0xE0, (byte)0x01,
	  (byte)0xF1, (byte)0x01, (byte)0xF1 },

	{ (byte)0x1F, (byte)0xFE, (byte)0x1F, (byte)0xFE, (byte)0x0E,
	  (byte)0xFE, (byte)0x0E, (byte)0xFE },

	{ (byte)0x01, (byte)0x1F, (byte)0x01, (byte)0x1F, (byte)0x01,
	  (byte)0x0E, (byte)0x01, (byte)0x0E },

	{ (byte)0xE0, (byte)0xFE, (byte)0xE0, (byte)0xFE, (byte)0xF1,
	  (byte)0xFE, (byte)0xF1, (byte)0xFE },

	{ (byte)0xFE, (byte)0x01, (byte)0xFE, (byte)0x01, (byte)0xFE,
	  (byte)0x01, (byte)0xFE, (byte)0x01 },

	{ (byte)0xE0, (byte)0x1F, (byte)0xE0, (byte)0x1F, (byte)0xF1,
	  (byte)0x0E, (byte)0xF1, (byte)0x0E },

	{ (byte)0xE0, (byte)0x01, (byte)0xE0, (byte)0x01, (byte)0xF1,
	  (byte)0x01, (byte)0xF1, (byte)0x01 },

	{ (byte)0xFE, (byte)0x1F, (byte)0xFE, (byte)0x1F, (byte)0xFE,
	  (byte)0x0E, (byte)0xFE, (byte)0x0E },

	{ (byte)0x1F, (byte)0x01, (byte)0x1F, (byte)0x01, (byte)0x0E,
	  (byte)0x01, (byte)0x0E, (byte)0x01 },

	{ (byte)0xFE, (byte)0xE0, (byte)0xFE, (byte)0xE0, (byte)0xFE,
	  (byte)0xF1, (byte)0xFE, (byte)0xF1 }
    };

    /**
     * Uses the first 8 bytes in <code>key</code> as the key material for the
     * DES key.
     *
     * <p> The bytes that constitute the DES key are those between
     * <code>key[0]</code> and <code>key[7]</code> inclusive.
     *
     * @param key the buffer with the DES key material.
     *
     * @exception InvalidKeyException if the given key material is shorter
     * than 8 bytes.
     */
    public DESKeySpec(byte[] key) throws InvalidKeyException {
	this(key, 0);
    }

    /**
     * Uses the first 8 bytes in <code>key</code>, beginning at
     * <code>offset</code> inclusive, as the key material for the DES key.
     *
     * <p> The bytes that constitute the DES key are those between
     * <code>key[offset]</code> and <code>key[offset+7]</code> inclusive.
     *
     * @param key the buffer with the DES key material.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 8 bytes.
     */
    public DESKeySpec(byte[] key, int offset) throws InvalidKeyException {
	if (key.length - offset < DES_KEY_LEN) {
	    throw new InvalidKeyException("Wrong key size");
	}
	this.key = new byte[DES_KEY_LEN];
	System.arraycopy(key, offset, this.key, 0, DES_KEY_LEN);
    }

    /**
     * Returns the DES key material.
     *
     * @return the DES key material.
     */
    public byte[] getKey() {
	return (byte[])this.key.clone();
    }

    /**
     * Checks if the given DES key material, starting at <code>offset</code>
     * inclusive, is parity-adjusted.
     *
     * @param key the buffer with the DES key material.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @return true if the given DES key material is parity-adjusted, false
     * otherwise.
     *
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 8 bytes.
     */
    public static boolean isParityAdjusted(byte[] key, int offset)
	throws InvalidKeyException {
	    if (key == null) {
		throw new InvalidKeyException("null key");
	    }
	    if (key.length - offset < DES_KEY_LEN) {
		throw new InvalidKeyException("Wrong key size");
	    }
	    for (int i = 0; i < DES_KEY_LEN; i++) {
		int bitCount = 0;
		for (int maskIndex = 0; maskIndex < PARITY_BIT_MASK.length;
		     maskIndex++) {
		    if ((key[i+offset] & PARITY_BIT_MASK[maskIndex])
			== PARITY_BIT_MASK[maskIndex]) {
			bitCount++;
		    }
		}
		if ((bitCount & 0x01) == 1) {
		    /*
		     * Odd number of 1 bits in the top 7 bits.
		     * Parity bit should be 0.
		     */
		    if ((key[i+offset] & (byte)0x01) == (byte)0x01)
			return false;
		} else {
		    /*
		     * Even number of 1 bits in the top 7 bits.
		     * Parity bit should be 1.
		     */
		    if ((key[i+offset] & (byte)0x01) != (byte)0x01)
			return false;
		}
	    }
	    return true;
    }

    /**
     * Checks if the given DES key material is weak or semi-weak.
     *
     * @param key the buffer with the DES key material.
     * @param offset the offset in <code>key</code>, where the DES key
     * material starts.
     *
     * @return true if the given DES key material is weak or semi-weak, false
     * otherwise.
     *
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 8 bytes.
     */
    public static boolean isWeak(byte[] key, int offset)
	throws InvalidKeyException {
	if (key == null) {
	    throw new InvalidKeyException("null key");
	}
	if (key.length - offset < DES_KEY_LEN) {
	    throw new InvalidKeyException("Wrong key size");
	}
	for (int i = 0; i < WEAK_KEYS.length; i++) {
	    boolean found = true;
	    for (int j = 0; j < DES_KEY_LEN && found == true; j++) {
		if (WEAK_KEYS[i][j] != key[j+offset]) {
		    found = false;
		}
	    }
	    if (found == true) {
		return found;
	    }
	}
	return false;
    }
}
