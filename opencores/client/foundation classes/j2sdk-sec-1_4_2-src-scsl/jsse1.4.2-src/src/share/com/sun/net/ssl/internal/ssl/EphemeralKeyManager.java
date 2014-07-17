/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.security.*;

/**
 * The "KeyManager" for ephemeral RSA keys. Ephemeral Diffie-Hellman keys
 * are handled by the DHKeyExchange class itself at the moment.
 *
 * @version 1.3, 06/24/03
 */
final class EphemeralKeyManager {
    
    // indices for the keys array below
    private final static int INDEX_RSA512 = 0;
    private final static int INDEX_RSA1024 = 1;
    
    /*
     * Current cached RSA KeyPairs. Elements are never null.
     * Indexed via the the constants above.
     */
    private final EphemeralKeyPair[] keys = new EphemeralKeyPair[] {
	new EphemeralKeyPair(null),
	new EphemeralKeyPair(null),
    };
    
    EphemeralKeyManager() {
	// empty
    }
	
    /*
     * Get a temporary RSA KeyPair.
     */
    KeyPair getRSAKeyPair(boolean export, SecureRandom random) {
	int length, index;
	if (ExportControl.hasStrongCrypto && !export) {
	    length = 1024;
	    index = INDEX_RSA512;
	} else {
	    length = 512;
	    index = INDEX_RSA1024;
	}
	
	KeyPair kp;
	EphemeralKeyPair ekp = keys[index];
	synchronized (ekp) {
	    kp = ekp.getKeyPair();
	    if (kp == null) {
		try {
		    KeyPairGenerator kgen = 
				    KeyPairGenerator.getInstance("RSA");
		    kgen.initialize(length, random);
		    ekp = new EphemeralKeyPair(kgen.genKeyPair());
		    kp = ekp.getKeyPair();
		    keys[index] = ekp;
		} catch (Exception e) {
		    // ignore
		}
	    }
	}
	return kp;
    }

    /**
     * Inner class to handle storage of ephemeral KeyPairs.
     */
    private static class EphemeralKeyPair {
	
	// maximum number of times a KeyPair is used
	private final static int MAX_USE = 200;
	
	// maximum time interval in which the keypair is used (1 hour in ms)
	private final static long USE_INTERVAL = 3600*1000;
	
	private KeyPair keyPair;
	private int uses;
	private long expirationTime;
	
	private EphemeralKeyPair(KeyPair keyPair) {
	    this.keyPair = keyPair;
	    expirationTime = System.currentTimeMillis() + USE_INTERVAL;
	}
	
	/*
	 * Check if the KeyPair can still be used.
	 */
	private boolean isValid() {
	    return (keyPair != null) && (uses < MAX_USE) 
		   && (System.currentTimeMillis() < expirationTime);
	}
	
	/*
	 * Return the KeyPair or null if it is invalid.
	 */
	private KeyPair getKeyPair() {
	    if (isValid() == false) {
		keyPair = null;
		return null;
	    }
	    uses++;
	    return keyPair;
	}
    }
    
}

