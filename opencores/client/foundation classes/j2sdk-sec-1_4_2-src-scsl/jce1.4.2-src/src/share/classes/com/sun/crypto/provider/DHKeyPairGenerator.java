/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.math.BigInteger;
import java.security.*;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import javax.crypto.spec.DHParameterSpec;
import javax.crypto.spec.DHGenParameterSpec;

/**
 * This class represents the key pair generator for Diffie-Hellman key pairs.
 *
 * <p>This key pair generator may be initialized in two different ways:
 *
 * <ul>
 * <li>By providing the size in bits of the prime modulus -
 * This will be used to create a prime modulus and base generator, which will
 * then be used to create the Diffie-Hellman key pair. The default size of the
 * prime modulus is 1024 bits.
 * <li>By providing a prime modulus and base generator
 * </ul>
 *
 * @author Jan Luehe
 *
 * @version 1.22, 06/24/03
 *
 * @see java.security.KeyPairGenerator
 */
public final class DHKeyPairGenerator extends KeyPairGeneratorSpi {

    // The public value
    private BigInteger y = null;

    // The private value
    private BigInteger x = null;

    // The prime modulus
    private BigInteger p = null;

    // The base generator
    private BigInteger g = null;

    // The size in bits of the prime modulus
    private int pSize = 1024;

    // The size in bits of the random exponent (private value)
    private int l = 0;

    // The source of randomness
    private SecureRandom random = null;

    /**
     * Initializes this key pair generator for a certain keysize and source of
     * randomness.
     * The keysize is specified as the size in bits of the prime modulus.
     *
     * @param keysize the keysize (size of prime modulus) in bits
     * @param random the source of randomness
     */
    public void initialize(int keysize, SecureRandom random) {
	if ((keysize < 512) || (keysize > 1024) || (keysize % 64 != 0)) {
	    throw new InvalidParameterException("Keysize must be multiple "
						+ "of 64, and can only range "
						+ "from 512 to 1024 "
						+ "(inclusive)");
	}
	this.pSize = keysize;
	this.l = 0;
	this.random = random;
    }

    /**
     * Initializes this key pair generator for the specified parameter
     * set and source of randomness.
     *
     * <p>The given parameter set contains the prime modulus, the base
     * generator, and optionally the requested size in bits of the random
     * exponent (private value).
     *
     * @param params the parameter set used to generate the key pair
     * @param random the source of randomness
     *
     * @exception InvalidAlgorithmParameterException if the given parameters
     * are inappropriate for this key pair generator
     */
    public void initialize(AlgorithmParameterSpec params,
		           SecureRandom random)
	throws InvalidAlgorithmParameterException {
	    if (!(params instanceof DHParameterSpec)){
		throw new InvalidAlgorithmParameterException
		    ("Inappropriate parameter type");
	    }

	    this.p = ((DHParameterSpec)params).getP();
	    this.pSize = this.p.bitLength();
	    if ((this.pSize < 512) || (this.pSize > 1024) ||
		(this.pSize % 64 != 0)) {
		throw new InvalidAlgorithmParameterException
		    ("Prime size must be multiple of 64, and can only range "
		     + "from 512 to 1024 (inclusive)");
	    }

	    this.g = ((DHParameterSpec)params).getG();

	    // exponent size is optional, could be 0
	    this.l = ((DHParameterSpec)params).getL();

	    this.random = random;

	    // Require exponentSize < primeSize
	    if ((this.l != 0) && (this.l > this.pSize)) {
		throw new InvalidAlgorithmParameterException
		    ("Exponent value must be less than (modulus value -1)");
	    }
    }

    /**
     * Generates a key pair.
     *
     * @return the new key pair
     */
    public KeyPair generateKeyPair() {
	KeyPair pair = null;

	if (random == null) {
	    random = SunJCE.RANDOM;
	}

	if (this.l <= 0) {
	    /* 
             * We can choose the size of the random exponent (private value) 
             * ourselves. For simplicity, pick a private value with a shorter
	     * size than the modulus's.
             */ 
	    this.x = new BigInteger(this.pSize-1, this.random).add(
		BigInteger.ONE);
	    this.l = this.x.bitLength();
	} else {
	    this.x = new BigInteger(this.l, random).setBit(this.l - 1);
	}

        if (this.l == this.pSize) {
            // make sure x < p-1
            BigInteger pMinus1 = p.subtract(BigInteger.ONE);
            while (this.x.compareTo(pMinus1) != -1) {
                this.x = new BigInteger(this.l, this.random).setBit(this.l - 1);
            }
	}

	try {

	    if (this.p == null || this.g == null) {
		// We have to create the prime and base parameters first
		DHGenParameterSpec genParamSpec;
		DHParameterSpec paramSpec;
		AlgorithmParameters algParams;
		genParamSpec = new DHGenParameterSpec(this.pSize,
						      this.l);
		DHParameterGenerator paramGen = new DHParameterGenerator();
		paramGen.engineInit(genParamSpec, null);
		algParams = paramGen.engineGenerateParameters();
		paramSpec = (DHParameterSpec)algParams.getParameterSpec
		    (DHParameterSpec.class);
		this.p = paramSpec.getP();
		this.g = paramSpec.getG();
	    }

	    this.y = this.g.modPow(this.x, this.p);

	    DHPublicKey pubKey = new DHPublicKey(this.y, this.p, this.g,
						 this.l);
	    DHPrivateKey privKey = new DHPrivateKey(this.x, this.p, this.g,
						    this.l);
	    pair = new KeyPair(pubKey, privKey);
        } catch (InvalidAlgorithmParameterException e) {
            // this should never happen, because we create genParamSpec
            throw new RuntimeException(e.getMessage());
        } catch (InvalidParameterSpecException e) {
            // this should never happen
            throw new RuntimeException(e.getMessage());
	} catch (InvalidKeyException e) {
	    // this should never happen
            throw new RuntimeException(e.getMessage());
	}

	return pair;
    }
}

