/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.security.InvalidKeyException;
import javax.crypto.SecretKey;
import javax.crypto.spec.DESKeySpec;

/**
 * This class represents a DES key.
 *
 * @author Jan Luehe
 *
 * @version 1.23, 06/24/03
 */

final class DESKey implements SecretKey {

    static final long serialVersionUID = 7724971015953279128L;

    private byte[] key;

    /**
     * Uses the first 8 bytes of the given key as the DES key.
     *
     * @param key the buffer with the DES key bytes.
     *
     * @exception InvalidKeyException if less than 8 bytes are available for
     * the key.
     */
    DESKey(byte[] key) throws InvalidKeyException {
	this(key, 0);
    }

    /**
     * Uses the first 8 bytes in <code>key</code>, beginning at
     * <code>offset</code>, as the DES key
     *
     * @param key the buffer with the DES key bytes.
     * @param offset the offset in <code>key</code>, where the DES key bytes
     * start.
     *
     * @exception InvalidKeyException if less than 8 bytes are available for
     * the key.
     */
    DESKey(byte[] key, int offset) throws InvalidKeyException {
	if (key == null || key.length - offset < DESKeySpec.DES_KEY_LEN) {
	    throw new InvalidKeyException("Wrong key size");
	}
	this.key = new byte[DESKeySpec.DES_KEY_LEN];
	DESKeyGenerator.setParityBit(key, offset);
	System.arraycopy(key, offset, this.key, 0, DESKeySpec.DES_KEY_LEN);
    }

    public byte[] getEncoded() {
	// Return a copy of the key, rather than a reference,
	// so that the key data cannot be modified from outside
        return (byte[])this.key.clone();
    }

    public String getAlgorithm() {
	return "DES";
    }
	    
    public String getFormat() {
	return "RAW";
    }

    /**
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() {
        int retval = 0;
        for (int i = 1; i < this.key.length; i++) {
            retval += this.key[i] * i;
        }
        return(retval ^= "des".hashCode());
    }

    public boolean equals(Object obj) {
        if (this == obj)
            return true;

        if (!(obj instanceof SecretKey))
            return false;

	String thatAlg = ((SecretKey)obj).getAlgorithm();
	if (!(thatAlg.equalsIgnoreCase("DES")))
	    return false;

	byte[] thatKey = ((SecretKey)obj).getEncoded();
	boolean ret = java.util.Arrays.equals(this.key, thatKey);
	java.util.Arrays.fill(thatKey, (byte)0x00);
	return ret;
    }

    /**
     * readObject is called to restore the state of this key from
     * a stream.
     */
    private void readObject(java.io.ObjectInputStream s)
         throws java.io.IOException, ClassNotFoundException
    {
        s.defaultReadObject();
        key = (byte[])key.clone();
    }

    /**
     * Ensures that the bytes of this key are
     * set to zero when there are no more references to it. 
     */
    protected void finalize() throws Throwable {
	try {
            if (this.key != null) {
                java.util.Arrays.fill(this.key, (byte)0x00);
                this.key = null;
            }
	} finally {
	    super.finalize();
	}
    }
}

