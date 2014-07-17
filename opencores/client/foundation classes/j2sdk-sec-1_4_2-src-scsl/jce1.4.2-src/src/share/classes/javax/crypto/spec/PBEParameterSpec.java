/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.math.BigInteger;
import java.security.spec.AlgorithmParameterSpec;

/**
 * This class specifies the set of parameters used with password-based
 * encryption (PBE), as defined in the
 * <a href="http://www.rsa.com/rsalabs/pubs/PKCS/html/pkcs-5.html">PKCS #5</a>
 * standard.
 * 
 * @author Jan Luehe
 *
 * @version 1.13, 06/24/03
 * @since 1.4
 */
public class PBEParameterSpec implements AlgorithmParameterSpec {

    private byte[] salt;
    private int iterationCount;

    /**
     * Constructs a parameter set for password-based encryption as defined in
     * the PKCS #5 standard.
     *
     * @param salt the salt.
     * @param iterationCount the iteration count.
     */
    public PBEParameterSpec(byte[] salt, int iterationCount) {
	this.salt = (byte[])salt.clone();
	this.iterationCount = iterationCount;
    }

    /**
     * Returns the salt.
     *
     * @return the salt
     */
    public byte[] getSalt() {
	return (byte[])this.salt.clone();
    }

    /**
     * Returns the iteration count.
     *
     * @return the iteration count
     */
    public int getIterationCount() {
	return this.iterationCount;
    }
}
