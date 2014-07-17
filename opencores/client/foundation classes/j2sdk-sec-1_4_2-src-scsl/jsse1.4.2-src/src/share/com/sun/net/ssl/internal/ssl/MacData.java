/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.security.*;

import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;
import sun.security.util.DerValue;
import sun.security.x509.AlgorithmId;
import sun.security.pkcs.ParsingException;


/**
 * A MacData type, as defined in PKCS#12.
 *
 * @version 1.8, 06/24/03
 * @author Sharon Liu
 */

class MacData {

    private String digestAlgorithmName;
    private AlgorithmParameters digestAlgorithmParams;
    private byte[] digest;
    private byte[] macSalt;
    private int iterations;

    /**
     * Parses a PKCS#12 MAC data.
     */
    MacData(DerInputStream derin)
	throws IOException, ParsingException
    {
	DerValue[] macData = derin.getSequence(2);

	// Parse the digest info
	DerInputStream digestIn = new DerInputStream(macData[0].toByteArray());
	DerValue[] digestInfo = digestIn.getSequence(2);
	
	// Parse the DigestAlgorithmIdentifier.
	AlgorithmId digestAlgorithmId = AlgorithmId.parse(digestInfo[0]);
	this.digestAlgorithmName = digestAlgorithmId.getName();
	this.digestAlgorithmParams = digestAlgorithmId.getParameters();
	// Get the digest.
	this.digest = digestInfo[1].getOctetString();

	// Get the salt.
	this.macSalt = macData[1].getOctetString();

	// Iterations is optional. The default value is 1.
	if (macData.length > 2) {
	    this.iterations = macData[2].getInteger();
	} else {
	    this.iterations = 1;
	}
    }

    String getDigestAlgName() {
	return digestAlgorithmName;
    }

    byte[] getSalt() {
	return macSalt;
    }

    int getIterations() {
	return iterations;
    }

    byte[] getDigest() {
	return digest;
    }

    void encode(DerOutputStream out) throws IOException {
    }
}
