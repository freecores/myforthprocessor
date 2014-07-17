/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;

/**
 * This class specifies the set of parameters used for generating
 * Diffie-Hellman (system) parameters for use in Diffie-Hellman key
 * agreement. This is typically done by a central
 * authority.
 * 
 * <p> The central authority, after computing the parameters, must send this
 * information to the parties looking to agree on a secret key.
 *
 * @author Jan Luehe
 *
 * @version 1.16, 06/24/03
 *
 * @see DHParameterSpec
 * @since 1.4
 */
public class DHGenParameterSpec implements AlgorithmParameterSpec {

    // The size in bits of the prime modulus
    private int primeSize;

    // The size in bits of the random exponent (private value)
    private int exponentSize;

    /**
     * Constructs a parameter set for the generation of Diffie-Hellman
     * (system) parameters. The constructed parameter set can be used to
     * initialize an
     * {@link java.security.AlgorithmParameterGenerator AlgorithmParameterGenerator}
     * object for the generation of Diffie-Hellman parameters.
     *
     * @param primeSize the size (in bits) of the prime modulus.
     * @param exponentSize the size (in bits) of the random exponent.
     */
    public DHGenParameterSpec(int primeSize, int exponentSize) {
	this.primeSize = primeSize;
	this.exponentSize = exponentSize;
    }

    /**
     * Returns the size in bits of the prime modulus.
     *
     * @return the size in bits of the prime modulus
     */
    public int getPrimeSize() {
	return this.primeSize;
    }

    /**
     * Returns the size in bits of the random exponent (private value).
     *
     * @return the size in bits of the random exponent (private value)
     */
    public int getExponentSize() {
	return this.exponentSize;
    }
}
