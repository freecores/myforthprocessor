/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)RC2ParameterSpec.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
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
 * @version 1.3, 01/23/03
 * @since 1.4
 */
public class RC2ParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Constructs a parameter set for RC2 from the given effective key size
     * (in bits).
     *
     * @param effectiveKeyBits the effective key size in bits.
     */
    public RC2ParameterSpec(int effectiveKeyBits) { }

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
    public RC2ParameterSpec(int effectiveKeyBits, byte[] iv) { }

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
    public RC2ParameterSpec(int effectiveKeyBits, byte[] iv, int offset) { }

    /** 
     * Returns the effective key size in bits.
     *
     * @return the effective key size in bits.
     */
    public int getEffectiveKeyBits() { }

    /** 
     * Returns the IV or null if this parameter set does not contain an IV.
     *
     * @return the IV or null if this parameter set does not contain an IV.
     */
    public byte[] getIV() { }

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
    public boolean equals(Object obj) { }

    /** 
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() { }
}
