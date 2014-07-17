/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.security.spec.AlgorithmParameterSpec;

/**
 * This class specifies an <i>initialization vector</i> (IV). 
 * Examples which use IVs are ciphers in feedback mode, 
 * e.g., DES in CBC mode and RSA ciphers with OAEP encoding
 * operation.
 * 
 * @author Jan Luehe
 *
 * @version 1.16, 06/24/03
 * @since 1.4
 */
public class IvParameterSpec implements AlgorithmParameterSpec {

    private byte[] iv;

    /**
     * Uses the bytes in <code>iv</code> as the IV.
     *
     * @param iv the buffer with the IV
     */
    public IvParameterSpec(byte[] iv) {
	this(iv, 0, iv.length);
    }

    /**
     * Uses the first <code>len</code> bytes in <code>iv</code>,
     * beginning at <code>offset</code> inclusive, as the IV.
     *
     * <p> The bytes that constitute the IV are those between
     * <code>iv[offset]</code> and <code>iv[offset+len-1]</code> inclusive.
     *
     * @param iv the buffer with the IV
     * @param offset the offset in <code>iv</code> where the IV
     * starts
     * @param len the number of IV bytes
     */
    public IvParameterSpec(byte[] iv, int offset, int len) {
	if (iv == null) {
            throw new IllegalArgumentException("IV missing");
        }
        if (iv.length - offset < len) {
            throw new IllegalArgumentException
		("IV buffer too short for given offset/length combination");
        }
	this.iv = new byte[len];
	System.arraycopy(iv, offset, this.iv, 0, len);
    }

    /**
     * Returns the initialization vector (IV).
     *
     * @return the initialization vector (IV)
     */
    public byte[] getIV() {
	return (byte[])this.iv.clone();
    }
}
