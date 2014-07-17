/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.math.BigInteger;

/**
 * This class specifies a Diffie-Hellman private key with its associated
 * parameters.
 *
 * @author Jan Luehe
 *
 * @version 1.14, 06/24/03
 *
 * @see DHPublicKeySpec
 * @since 1.4
 */
public class DHPrivateKeySpec implements java.security.spec.KeySpec {

    // The private value
    private BigInteger x;

    // The prime modulus
    private BigInteger p;

    // The base generator
    private BigInteger g;

    /**
     * Constructor that takes a private value <code>x</code>, a prime
     * modulus <code>p</code>, and a base generator <code>g</code>.
     * @param x private value x 
     * @param p prime modulus p
     * @param g base generator g
     */
    public DHPrivateKeySpec(BigInteger x, BigInteger p, BigInteger g) {
	this.x = x;
	this.p = p;
	this.g = g;
    }

    /**
     * Returns the private value <code>x</code>.
     *
     * @return the private value <code>x</code>
     */
    public BigInteger getX() {
	return this.x;
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
