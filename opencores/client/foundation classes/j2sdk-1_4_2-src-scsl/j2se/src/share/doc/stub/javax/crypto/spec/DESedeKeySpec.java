/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DESedeKeySpec.java	1.4 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.spec;

import java.security.InvalidKeyException;

/** 
 * This class specifies a DES-EDE ("triple-DES") key.
 *
 * @author Jan Luehe
 *
 * @version 1.15, 07/16/01
 * @since 1.4
 */
public class DESedeKeySpec implements java.security.spec.KeySpec
{
    /** 
     * The constant which defines the length of a DESede key in bytes.
     */
    public static final int DES_EDE_KEY_LEN = 0;

    /** 
     * Uses the first 24 bytes in <code>key</code> as the DES-EDE key.
     * 
     * <p> The bytes that constitute the DES-EDE key are those between
     * <code>key[0]</code> and <code>key[23]</code> inclusive
     *
     * @param key the buffer with the DES-EDE key material.
     *
     * @exception InvalidKeyException if the given key material is shorter
     * than 24 bytes.
     */
    public DESedeKeySpec(byte[] key) throws InvalidKeyException { }

    /** 
     * Uses the first 24 bytes in <code>key</code>, beginning at
     * <code>offset</code> inclusive, as the DES-EDE key.
     *
     * <p> The bytes that constitute the DES-EDE key are those between
     * <code>key[offset]</code> and <code>key[offset+23]</code> inclusive.
     *
     * @param key the buffer with the DES-EDE key material.
     * @param offset the offset in <code>key</code>, where the DES-EDE key
     * material starts.
     *
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 24 bytes
     */
    public DESedeKeySpec(byte[] key, int offset) throws InvalidKeyException { }

    /** 
     * Returns the DES-EDE key.
     *
     * @return the DES-EDE key
     */
    public byte[] getKey() { }

    /** 
     * Checks if the given DES-EDE key, starting at <code>offset</code>
     * inclusive, is parity-adjusted.
     *
     * @param key    a byte array which holds the key value
     * @param offset the offset into the byte array 
     * @return true if the given DES-EDE key is parity-adjusted, false
     * otherwise
     *
     * @exception InvalidKeyException if the given key material, starting at
     * <code>offset</code> inclusive, is shorter than 24 bytes
     */
    public static boolean isParityAdjusted(byte[] key, int offset)
        throws InvalidKeyException
    { }
}
