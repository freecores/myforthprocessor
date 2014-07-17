/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.spec.KeySpec;
import java.security.spec.InvalidKeySpecException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactorySpi;
import javax.crypto.spec.PBEKeySpec;

/**
 * This class implements a key factory for PBE keys according to PKCS#5,
 * meaning that the password must consist of printable ASCII characters
 * (values 32 to 126 decimal inclusive) and only the low order 8 bits
 * of each password character are used.
 *
 * @author Jan Luehe
 *
 * @version 1.15, 06/24/03
 */
public final class PBEKeyFactory extends SecretKeyFactorySpi {

    /**
     * Verify the SunJCE provider in the constructor.
     * 
     * @exception SecurityException if fails to verify
     * its own integrity
     */
    public PBEKeyFactory() {
        if (!SunJCE.verifySelfIntegrity(this.getClass())) {
	    throw new SecurityException("The SunJCE provider may have " +
					"been tampered.");
	}
    }

    /**
     * Generates a <code>SecretKey</code> object from the provided key
     * specification (key material).
     *
     * @param keySpec the specification (key material) of the secret key
     *
     * @return the secret key
     *
     * @exception InvalidKeySpecException if the given key specification
     * is inappropriate for this key factory to produce a public key.
     */
    protected SecretKey engineGenerateSecret(KeySpec keySpec)
	throws InvalidKeySpecException
    {
	if (!(keySpec instanceof PBEKeySpec)) {
	    throw new InvalidKeySpecException("Invalid key spec");
	}
	return new PBEKey((PBEKeySpec)keySpec);
    }

    /**
     * Returns a specification (key material) of the given key
     * in the requested format.
     *
     * @param key the key 
     *
     * @param keySpec the requested format in which the key material shall be
     * returned
     *
     * @return the underlying key specification (key material) in the
     * requested format
     *
     * @exception InvalidKeySpecException if the requested key specification is
     * inappropriate for the given key, or the given key cannot be processed
     * (e.g., the given key has an unrecognized algorithm or format).
     */
    protected KeySpec engineGetKeySpec(SecretKey key, Class keySpecCl)
	throws InvalidKeySpecException {    
	if ((key instanceof SecretKey)
	    && (key.getAlgorithm().equalsIgnoreCase("PBEWithMD5AndDES"))
	    && (key.getFormat().equalsIgnoreCase("RAW"))) {
	    
	    // Check if requested key spec is amongst the valid ones
	    if ((keySpecCl != null)
		&& PBEKeySpec.class.isAssignableFrom(keySpecCl)) {
		byte[] passwdBytes = key.getEncoded();
		char[] passwdChars = new char[passwdBytes.length];
		for (int i=0; i<passwdChars.length; i++)
		    passwdChars[i] = (char) (passwdBytes[i] & 0x7f);
		PBEKeySpec ret = new PBEKeySpec(passwdChars);
		// password char[] was cloned in PBEKeySpec constructor,
		// so we can zero it out here
		java.util.Arrays.fill(passwdChars, ' ');
		java.util.Arrays.fill(passwdBytes, (byte)0x00);
		return ret;
	    } else {
		throw new InvalidKeySpecException("Invalid key spec");
	    }
	} else {
	    throw new InvalidKeySpecException("Invalid key "
					      + "format/algorithm");
	}
    }

    /**
     * Translates a <code>SecretKey</code> object, whose provider may be
     * unknown or potentially untrusted, into a corresponding
     * <code>SecretKey</code> object of this key factory.
     *
     * @param key the key whose provider is unknown or untrusted
     *
     * @return the translated key
     *
     * @exception InvalidKeyException if the given key cannot be processed by
     * this key factory.
     */
    protected SecretKey engineTranslateKey(SecretKey key)
	throws InvalidKeyException
    {
	try {
	    if ((key != null) && 
		(key.getAlgorithm().equalsIgnoreCase("PBEWithMD5AndDES")) &&
		(key.getFormat().equalsIgnoreCase("RAW"))) {
		    
		// Check if key originates from this factory
		if (key instanceof com.sun.crypto.provider.PBEKey) {
		    return key;
		}

		// Convert key to spec
		PBEKeySpec pbeKeySpec = (PBEKeySpec)engineGetKeySpec
		    (key, PBEKeySpec.class);

		// Create key from spec, and return it
		return engineGenerateSecret(pbeKeySpec);
	    } else {
		throw new InvalidKeyException("Invalid key format/algorithm");
	    }

	} catch (InvalidKeySpecException ikse) {
	    throw new InvalidKeyException("Cannot translate key: "
					  + ikse.getMessage());
	}
    }
}


