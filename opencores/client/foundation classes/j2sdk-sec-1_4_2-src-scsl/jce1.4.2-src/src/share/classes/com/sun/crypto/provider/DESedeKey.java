/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.security.InvalidKeyException;
import javax.crypto.SecretKey;
import javax.crypto.spec.DESedeKeySpec;

/**
 * This class represents a DES-EDE key.
 *
 * @author Jan Luehe
 *
 * @version 1.22, 06/24/03
 */

final class DESedeKey implements SecretKey {

    static final long serialVersionUID = 2463986565756745178L;

    private byte[] key;

    /**
     * Creates a DES-EDE key from a given key.
     *
     * @param key the given key
     *
     * @exception InvalidKeyException if the given key has a wrong size
     */
    DESedeKey(byte[] key) throws InvalidKeyException {
	this(key, 0);
    }

    /**
     * Uses the first 24 bytes in <code>key</code>, beginning at
     * <code>offset</code>, as the DES-EDE key
     *
     * @param key the buffer with the DES-EDE key
     * @param offset the offset in <code>key</code>, where the DES-EDE key
     * starts
     *
     * @exception InvalidKeyException if the given key has a wrong size
     */
    DESedeKey(byte[] key, int offset) throws InvalidKeyException {

	if (key==null || ((key.length-offset)<DESedeKeySpec.DES_EDE_KEY_LEN)) {
	    throw new InvalidKeyException("Wrong key size");
	}
	this.key = new byte[DESedeKeySpec.DES_EDE_KEY_LEN];
	DESKeyGenerator.setParityBit(key, offset);
	DESKeyGenerator.setParityBit(key, offset + 8);
	DESKeyGenerator.setParityBit(key, offset + 16);
	System.arraycopy(key, offset, this.key, 0,
			 DESedeKeySpec.DES_EDE_KEY_LEN);
    }

    public byte[] getEncoded() {
        return (byte[])this.key.clone();
    }

    public String getAlgorithm() {
	return "DESede";
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
        return(retval ^= "desede".hashCode());
    }

    public boolean equals(Object obj) {
        if (this == obj)
            return true;

        if (!(obj instanceof SecretKey))
            return false;

	String thatAlg = ((SecretKey)obj).getAlgorithm();
	if (!(thatAlg.equalsIgnoreCase("DESede"))
	    && !(thatAlg.equalsIgnoreCase("TripleDES")))
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

