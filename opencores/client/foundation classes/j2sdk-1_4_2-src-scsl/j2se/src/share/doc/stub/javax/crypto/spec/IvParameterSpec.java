/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)IvParameterSpec.java	1.4 03/01/23
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
 * This class specifies an <i>initialization vector</i> (IV). 
 * Examples which use IVs are ciphers in feedback mode, 
 * e.g., DES in CBC mode and RSA ciphers with OAEP encoding
 * operation.
 * 
 * @author Jan Luehe
 *
 * @version 1.14, 09/13/01
 * @since 1.4
 */
public class IvParameterSpec implements AlgorithmParameterSpec
{

    /** 
     * Uses the bytes in <code>iv</code> as the IV.
     *
     * @param iv the buffer with the IV
     */
    public IvParameterSpec(byte[] iv) { }

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
    public IvParameterSpec(byte[] iv, int offset, int len) { }

    /** 
     * Returns the initialization vector (IV).
     *
     * @return the initialization vector (IV)
     */
    public byte[] getIV() { }
}
