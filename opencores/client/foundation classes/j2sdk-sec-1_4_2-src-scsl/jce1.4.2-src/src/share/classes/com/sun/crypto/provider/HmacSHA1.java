/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import javax.crypto.MacSpi;
import javax.crypto.SecretKey;
import java.security.*;
import java.security.spec.*;

/**
 * This is an implementation of the HMAC-SHA1 algorithm.
 *
 * @author Jan Luehe
 * @version 1.12, 06/24/03
 */
public final class HmacSHA1 extends MacSpi implements Cloneable {

    private HmacCore hmac = null;
    private static final int SHA1_BLOCK_LENGTH = 64;

    /**
     * Standard constructor, creates a new HmacSHA1 instance.
     * Verify the SunJCE provider in the constructor.
     * 
     * @exception SecurityException if fails to verify
     * its own integrity
     */
    public HmacSHA1() throws NoSuchAlgorithmException {
        if (!SunJCE.verifySelfIntegrity(this.getClass())) {
	    throw new SecurityException("The SunJCE provider may have " +
					"been tampered.");
	}
	this.hmac = new HmacCore(MessageDigest.getInstance("SHA1"),
				 SHA1_BLOCK_LENGTH);
    }

    /** 
     * Returns the length of the HMAC in bytes.
     *
     * @return the HMAC length in bytes.
     */
    protected int engineGetMacLength() {
	return hmac.getDigestLength();
    }

    /**
     * Initializes the HMAC with the given secret key and algorithm parameters.
     *
     * @param key the secret key.
     * @param params the algorithm parameters.
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this MAC.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this MAC.
     */
    protected void engineInit(Key key, AlgorithmParameterSpec params)
	throws InvalidKeyException, InvalidAlgorithmParameterException {
	hmac.init(key, params);
    }

    /**
     * Processes the given byte.    
     * 
     * @param input the input byte to be processed.
     */
    protected void engineUpdate(byte input) {
	hmac.update(input);
    }

    /**
     * Processes the first <code>len</code> bytes in <code>input</code>,
     * starting at <code>offset</code>.
     * 
     * @param input the input buffer.
     * @param offset the offset in <code>input</code> where the input starts.
     * @param len the number of bytes to process.
     */
    protected void engineUpdate(byte input[], int offset, int len) {
	hmac.update(input, offset, len);
    }

    /**
     * Completes the HMAC computation and resets the HMAC for further use,
     * maintaining the secret key that the HMAC was initialized with.
     *
     * @return the HMAC result.
     */
    protected byte[] engineDoFinal() {
	return hmac.doFinal();
    }

    /**
     * Resets the HMAC for further use, maintaining the secret key that the
     * HMAC was initialized with.
     */
    protected void engineReset() {
	hmac.reset();
    }

    /*
     * Clones this object.
     */
    public Object clone() {
	HmacSHA1 that = null;
	try {
	    that = (HmacSHA1)super.clone();
	    that.hmac = (HmacCore)this.hmac.clone();
	} catch (CloneNotSupportedException e) {
	}
	return that;
    }
}


