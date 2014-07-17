/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.math.BigInteger;
import java.security.SecureRandom;

/**
 * This class implements the Diffie-Hellman key exchange algorithm.
 * D-H means combining your private key with your partners public key to
 * generate a number. The peer does the same with its private key and our
 * public key. Through the magic of Diffie-Hellman we both come up with the
 * same number. This number is secret (discounting MITM attacks) and hence 
 * called the shared secret. It has the same length as the modulus, e.g. 512 
 * or 1024 bit. Man-in-the-middle attacks are typically countered by an 
 * independent authentication step using certificates (RSA, DSA, etc.).
 *
 * The thing to note is that the shared secret is constant for two partners
 * with constant private keys. This is often not what we want, which is why
 * it is generally a good idea to create a new private key for each session.
 * Generating a private key involves one modular exponentiation assuming
 * suitable D-H parameters are available.
 *
 * General usage of this class (TLS DHE case):
 *  . create DHKeyExchange instance (selecting group parameters)
 *  . generate a ephemeral key pair
 *  . send parameters and public value to remote peer
 *  . receive peers ephemeral public key
 *  . call getAgreedSecret() to calculate the shared secret
 *
 * In TLS the server chooses the parameter values itself, the client must use 
 * those sent to it by the server.
 *
 * The use of ephemeral keys as described above also achieves what is called 
 * "forward secrecy". This means that even if the authentication keys are broken 
 * at a later date, the shared secret remains secure. The session is compromised
 * only if the authentication keys are already broken at the time the key
 * exchange takes place and an active MITM attack is used. This is in contrast
 * to straightforward encrypting RSA key exchanges.
 *
 * @version 1.20 06/24/03
 * @author David Brownell
 */
final class DHKeyExchange {

    // group parameters (prime modulus and generator)
    private BigInteger modulus;			// P (aka N)
    private BigInteger base;			// G (aka alpha)

    // private component x (random number 1 < x < modulus)
    private BigInteger privateKey;		// x
    // public component X = (g ^ x) mod p
    private BigInteger publicKey;		// X (aka y)

    /**
     * Prepares to use key exchange with a precomputed set of Diffie-Hellman
     * algorithm parameters.  These are either the SKIP 1024 bit parameters,
     * or else a set of precomputed 512 bit parameters.
     */
    DHKeyExchange(boolean useStrongCrypto) {
        if (useStrongCrypto) {
	    if (ExportControl.hasStrongCrypto == false) {
		throw new InternalError("Cannot use strong D-H");
	    }
	    modulus = skip1024Modulus;
	    base = skip1024Base;
	} else {
	    modulus = export512Modulus;
	    base = export512Base;
	}
    }


    /**
     * Prepares to use a specified set of parameters.
     *
     * @param modulus the Diffie-Hellman modulus P
     * @param base the Diffie-Hellman base G
     */
    DHKeyExchange(BigInteger modulus, BigInteger base) {
	this.modulus = modulus;
	this.base = base;
    }

    /** Returns the Diffie-Hellman modulus. */
    BigInteger getModulus() {
	return modulus;
    }

    /** Returns the Diffie-Hellman base (generator).  */
    BigInteger getBase() { 
	return base;
    }


    /**
     * Generates a Diffie-Hellman key pair according to the key parameters
     * being used.  These keys are later accessed using the getPrivateKey
     * and getPublicKey methods.
     *
     * <P> Note that there is a rule of thumb that the number of bits in
     * the private key should be at least twice the number of bits of secret
     * data that are needed ... if you need 256 bits (32 bytes) of secret
     * data, use at least a 512 bit (64 bytes) private key.
     *
     * @param seed source of random numbers
     * @param privateBits number of bits of private key to produce
     */
    void generateKeyPair(SecureRandom seed, int privateBits) {
	privateKey = new BigInteger(privateBits, seed);
	publicKey = base.modPow(privateKey, modulus);
    }

    /**
     * Gets the public key of this end of the key exchange.
     */
    BigInteger getPublicKey() {
	return publicKey;
    }

    /** Gets the private key of this end of the key exchange. */
    BigInteger getPrivateKey() {
	return privateKey;
    }

    /**
     * Get the secret data that has been agreed on through Diffie-Hellman
     * key agreement protocol.  Note that in the two party protocol, if
     * the peer keys are already known, no other data needs to be sent in
     * order to agree on a secret.  That is, a secured message may be
     * sent without any mandatory round-trip overheads.
     *
     * <P>It is illegal to call this member function if the private key
     * has not been set (or generated).
     *
     * @param peerPublicKey the peer's public key.
     * @returns the secret, which is an unsigned big-endian integer
     *	the same size as the Diffie-Hellman modulus.
     */
    byte[] getAgreedSecret(BigInteger peerPublicKey) {
	BigInteger bsecret = peerPublicKey.modPow(privateKey, modulus);
	return HandshakeMessage.toByteArray(bsecret);
    }

    /*
     * The 1024 bit Diffie-Hellman modulus values used by SKIP.  See
     * the SKIP RFCs (skip-06) for a full description of the procedure
     * used to generate these values.
     *
     * These values were cut/pasted from the RFC and then modified so
     * they'd compile in Java (which has no unsigned byte constants).
     */
    private static final byte skip1024ModulusBytes[] = {
	(byte)0xF4, (byte)0x88, (byte)0xFD, (byte)0x58,
	(byte)0x4E, (byte)0x49, (byte)0xDB, (byte)0xCD,
	(byte)0x20, (byte)0xB4, (byte)0x9D, (byte)0xE4,
	(byte)0x91, (byte)0x07, (byte)0x36, (byte)0x6B,
	(byte)0x33, (byte)0x6C, (byte)0x38, (byte)0x0D,
	(byte)0x45, (byte)0x1D, (byte)0x0F, (byte)0x7C,
	(byte)0x88, (byte)0xB3, (byte)0x1C, (byte)0x7C,
	(byte)0x5B, (byte)0x2D, (byte)0x8E, (byte)0xF6,
	(byte)0xF3, (byte)0xC9, (byte)0x23, (byte)0xC0,
	(byte)0x43, (byte)0xF0, (byte)0xA5, (byte)0x5B,
	(byte)0x18, (byte)0x8D, (byte)0x8E, (byte)0xBB,
	(byte)0x55, (byte)0x8C, (byte)0xB8, (byte)0x5D,
	(byte)0x38, (byte)0xD3, (byte)0x34, (byte)0xFD,
	(byte)0x7C, (byte)0x17, (byte)0x57, (byte)0x43,
	(byte)0xA3, (byte)0x1D, (byte)0x18, (byte)0x6C,
	(byte)0xDE, (byte)0x33, (byte)0x21, (byte)0x2C,
	(byte)0xB5, (byte)0x2A, (byte)0xFF, (byte)0x3C,
	(byte)0xE1, (byte)0xB1, (byte)0x29, (byte)0x40,
	(byte)0x18, (byte)0x11, (byte)0x8D, (byte)0x7C,
	(byte)0x84, (byte)0xA7, (byte)0x0A, (byte)0x72,
	(byte)0xD6, (byte)0x86, (byte)0xC4, (byte)0x03,
	(byte)0x19, (byte)0xC8, (byte)0x07, (byte)0x29,
	(byte)0x7A, (byte)0xCA, (byte)0x95, (byte)0x0C,
	(byte)0xD9, (byte)0x96, (byte)0x9F, (byte)0xAB,
	(byte)0xD0, (byte)0x0A, (byte)0x50, (byte)0x9B,
	(byte)0x02, (byte)0x46, (byte)0xD3, (byte)0x08,
	(byte)0x3D, (byte)0x66, (byte)0xA4, (byte)0x5D,
	(byte)0x41, (byte)0x9F, (byte)0x9C, (byte)0x7C,
	(byte)0xBD, (byte)0x89, (byte)0x4B, (byte)0x22,
	(byte)0x19, (byte)0x26, (byte)0xBA, (byte)0xAB,
	(byte)0xA2, (byte)0x5E, (byte)0xC3, (byte)0x55,
	(byte)0xE9, (byte)0x2F, (byte)0x78, (byte)0xC7
    };

    /** The SKIP 1024 bit modulus. */
    private static final BigInteger skip1024Modulus
	= new BigInteger(1, skip1024ModulusBytes);

    /** The base used with the SKIP 1024 bit modulus. */
    private static final BigInteger skip1024Base = BigInteger.valueOf(2);


    /*
     * A precomputed 512 bit modulus, used where US export regulations
     * prohibit use of larger keys (such as the SKIP 1024 bit modulus)
     * and where performance prohibits dynamic generation of a new
     * Diffie-Hellman modulus (which is quite expensive).
     */
    private static final byte export512ModulusBytes[] = {
	(byte) 0xac, (byte) 0xc8, (byte) 0x14, (byte) 0x9e, 
	(byte) 0x86, (byte) 0x2b, (byte) 0xb3, (byte) 0x2b, 
	(byte) 0x1e, (byte) 0xf6, (byte) 0xb9, (byte) 0x72, 
	(byte) 0x54, (byte) 0xf8, (byte) 0x43, (byte) 0xba, 
	(byte) 0x47, (byte) 0xf8, (byte) 0x91, (byte) 0x50, 
	(byte) 0x77, (byte) 0x22, (byte) 0xa1, (byte) 0x6b, 
	(byte) 0x59, (byte) 0x7c, (byte) 0x40, (byte) 0x7f, 
	(byte) 0x25, (byte) 0x76, (byte) 0x4c, (byte) 0xc4, 
	(byte) 0xf1, (byte) 0xa1, (byte) 0xd0, (byte) 0x69, 
	(byte) 0x02, (byte) 0x85, (byte) 0xdd, (byte) 0x10, 
	(byte) 0xbf, (byte) 0xe5, (byte) 0x75, (byte) 0x84, 
	(byte) 0x95, (byte) 0x6f, (byte) 0x25, (byte) 0xa9, 
	(byte) 0xd8, (byte) 0x81, (byte) 0x9d, (byte) 0xa2, 
	(byte) 0x33, (byte) 0x90, (byte) 0x25, (byte) 0xa4, 
	(byte) 0x9f, (byte) 0x9f, (byte) 0x38, (byte) 0x53, 
	(byte) 0x67, (byte) 0xec, (byte) 0x15, (byte) 0x05
    };

    /**
     * A precomputed 512 bit (exportable) modulus.
     */
    private static final BigInteger export512Modulus
	= new BigInteger(1, export512ModulusBytes);

    /** The base used with the exportable 512 bit modulus. */
    private static final BigInteger export512Base = BigInteger.valueOf(2);
}
