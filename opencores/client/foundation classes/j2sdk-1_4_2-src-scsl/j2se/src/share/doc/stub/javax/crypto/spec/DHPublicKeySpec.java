/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DHPublicKeySpec.java	1.4 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.spec;

import java.math.BigInteger;

/** 
 * This class specifies a Diffie-Hellman public key with its associated
 * parameters.
 *
 * @author Jan Luehe
 *
 * @version 1.12, 07/16/01
 *
 * @see DHPrivateKeySpec
 * @since 1.4
 */
public class DHPublicKeySpec implements java.security.spec.KeySpec
{

    /** 
     * Constructor that takes a public value <code>y</code>, a prime
     * modulus <code>p</code>, and a base generator <code>g</code>.
     * @param y  public value y
     * @param p  prime modulus p
     * @param g  base generator g
     */
    public DHPublicKeySpec(BigInteger y, BigInteger p, BigInteger g) { }

    /** 
     * Returns the public value <code>y</code>.
     *
     * @return the public value <code>y</code>
     */
    public BigInteger getY() { }

    /** 
     * Returns the prime modulus <code>p</code>.
     *
     * @return the prime modulus <code>p</code>
     */
    public BigInteger getP() { }

    /** 
     * Returns the base generator <code>g</code>.
     *
     * @return the base generator <code>g</code>
     */
    public BigInteger getG() { }
}
