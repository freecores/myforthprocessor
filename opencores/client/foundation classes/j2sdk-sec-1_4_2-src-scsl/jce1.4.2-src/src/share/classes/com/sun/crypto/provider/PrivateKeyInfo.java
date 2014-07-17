/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.math.*;
import java.io.*;
import sun.security.x509.AlgorithmId;
import sun.security.util.*;


/**
 * This class implements the <code>PrivateKeyInfo</code> type,
 * which is defined in PKCS #8 as follows:
 *
 * <pre>
 * PrivateKeyInfo ::=  SEQUENCE {
 *     version   INTEGER,
 *     privateKeyAlgorithm   AlgorithmIdentifier,
 *     privateKey   OCTET STRING,
 *     attributes   [0] IMPLICIT Attributes OPTIONAL }
 * </pre>
 *
 * @author Jan Luehe
 * @version 1.8, 06/24/03
 */
final class PrivateKeyInfo {

    // the version number defined by the PKCS #8 standard
    private static final BigInteger VERSION = BigInteger.ZERO;

    // the private-key algorithm
    private AlgorithmId algid;

    // the private-key value
    private byte[] privkey;

    /**
     * Constructs a PKCS#8 PrivateKeyInfo from its ASN.1 encoding.
     */
    PrivateKeyInfo(byte[] encoded) throws IOException {
	DerValue val = new DerValue(encoded);

	if (val.tag != DerValue.tag_Sequence)
	    throw new IOException("private key parse error: not a sequence");

	// version
	BigInteger parsedVersion = val.data.getBigInteger();
	if (!parsedVersion.equals(VERSION)) {
	    throw new IOException("version mismatch: (supported: " + 
				  VERSION + ", parsed: " + parsedVersion);
	}

	// privateKeyAlgorithm
	this.algid = AlgorithmId.parse(val.data.getDerValue());

	// privateKey
	this.privkey = val.data.getOctetString();

	// OPTIONAL attributes not supported yet
    }

    /**
     * Returns the private-key algorithm.
     */
    AlgorithmId getAlgorithm() {
	return this.algid;
    }
}
