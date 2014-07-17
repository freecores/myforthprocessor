/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.security.SecureRandom;
import java.security.InvalidParameterException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.spec.AlgorithmParameterSpec;
import javax.crypto.KeyGeneratorSpi;
import javax.crypto.SecretKey;
import javax.crypto.spec.DESKeySpec;

/**
 * This class generates a DES key.
 *
 * @author Jan Luehe
 *
 * @version 1.26, 06/24/03
 */

public final class DESKeyGenerator extends KeyGeneratorSpi {
    
    private SecureRandom random = null;

    /* Mask to check for parity adjustment */
    private static final byte[] PARITY_BIT_MASK = {
	(byte)0x80, (byte)0x40, (byte)0x20, (byte)0x10,
	(byte)0x08, (byte)0x04, (byte)0x02
    };

    /**
     * Verify the SunJCE provider in the constructor.
     * 
     * @exception SecurityException if fails to verify
     * its own integrity
     */
    public DESKeyGenerator() {
        if (!SunJCE.verifySelfIntegrity(this.getClass())) {
	    throw new SecurityException("The SunJCE provider may have " +
					"been tampered.");
	}
    }

    /**
     * Initializes this key generator.
     * 
     * @param random the source of randomness for this generator
     */
    protected void engineInit(SecureRandom random) {
	this.random = random;
    }

    /**
     * Initializes this key generator with the specified parameter
     * set and a user-provided source of randomness.
     *
     * @param params the key generation parameters
     * @param random the source of randomness for this key generator
     *
     * @exception InvalidAlgorithmParameterException if <code>params</code> is
     * inappropriate for this key generator
     */
    protected void engineInit(AlgorithmParameterSpec params,
			      SecureRandom random)
	throws InvalidAlgorithmParameterException {
	    throw new InvalidAlgorithmParameterException
		("DES key generation does not take any parameters");
    }

    /**
     * Initializes this key generator for a certain keysize, using the given
     * source of randomness.
     *
     * @param keysize the keysize. This is an algorithm-specific
     * metric specified in number of bits.
     * @param random the source of randomness for this key generator
     */
    protected void engineInit(int keysize, SecureRandom random) {
	if (keysize != 56) {
	    throw new InvalidParameterException("Wrong keysize: must "
						+ "be equal to 56");
	}
	this.engineInit(random);
    }

    /**
     * Generates the DES key.
     *
     * @return the new DES key
     */
    protected SecretKey engineGenerateKey() {
	DESKey desKey = null;

	if (this.random == null) {
	    this.random = SunJCE.RANDOM;
	}

	try {
	    byte[] key = new byte[8];
	    do {
		this.random.nextBytes(key);
		setParityBit(key, 0);
	    } while (DESKeySpec.isWeak(key, 0));
	    desKey = new DESKey(key);
	} catch (InvalidKeyException e) {
	    // this is never thrown
	}

	return desKey;
    }

    /*
     * Does parity adjustment, using bit in position 8 as the parity bit,
     * for 8 key bytes, starting at <code>offset</code>.
     *
     * The 8 parity bits of a DES key are only used for sanity-checking
     * of the key, to see if the key could actually be a key. If you check
     * the parity of the quantity, and it winds up not having the correct
     * parity, then you'll know something went wrong.
     *
     * A key that is not parity adjusted (e.g. e4e4e4e4e4e4e4e4) produces the
     * same output as a key that is parity adjusted (e.g. e5e5e5e5e5e5e5e5),
     * because it is the 56 bits of the DES key that are cryptographically
     * significant/"effective" -- the other 8 bits are just used for parity
     * checking.
     */
    static void setParityBit(byte[] key, int offset) {
	if (key == null)
	    return;

	for (int i = 0; i < 8; i++) {
	    int bitCount = 0;
	    for (int maskIndex = 0;
		 maskIndex < PARITY_BIT_MASK.length; maskIndex++) {
		if ((key[i+offset] & PARITY_BIT_MASK[maskIndex])
		    == PARITY_BIT_MASK[maskIndex]) {
		    bitCount++;
		}
	    }
	    if ((bitCount & 0x01) == 1) {
		// Odd number of 1 bits in the top 7 bits. Set parity bit to 0
		key[i+offset] = (byte)(key[i+offset] & (byte)0xfe);
	    } else {
		// Even number of 1 bits in the top 7 bits. Set parity bit to 1
		key[i+offset] = (byte)(key[i+offset] | 1);
	    }
	}
    }
}
