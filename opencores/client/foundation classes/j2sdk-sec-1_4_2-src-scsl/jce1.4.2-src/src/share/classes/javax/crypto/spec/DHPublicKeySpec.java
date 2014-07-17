/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.math.BigInteger;

/**
 * This class specifies a Diffie-Hellman public key with its associated
 * parameters.
 *
 * @author Jan Luehe
 *
 * @version 1.14, 06/24/03
 *
 * @see DHPrivateKeySpec
 * @since 1.4
 */
public class DHPublicKeySpec implements java.security.spec.KeySpec {

    // The public value
    private BigInteger y;

    // The prime modulus
    private BigInteger p;

    // The base generator
    private BigInteger g;

    /**
     * Constructor that takes a public value <code>y</code>, a prime
     * modulus <code>p</code>, and a base generator <code>g</code>.
     * @param y  public value y
     * @param p  prime modulus p
     * @param g  base generator g
     */
    public DHPublicKeySpec(BigInteger y, BigInteger p, BigInteger g) {
	this.y = y;
	this.p = p;
	this.g = g;
    }

    /**
     * Returns the public value <code>y</code>.
     *
     * @return the public value <code>y</code>
     */
    public BigInteger getY() {
	return this.y;
    }

    /**
     * Returns the prime modulus <code>p</code>.
     *
     * @return the prime modulus <code>p</code>
     */
    public BigInteger getP() {
	return this.p;
    }

    /**
     * Returns the base generator <code>g</code>.
     *
     * @return the base generator <code>g</code>
     */
    public BigInteger getG() {
	return this.g;
    }
}
