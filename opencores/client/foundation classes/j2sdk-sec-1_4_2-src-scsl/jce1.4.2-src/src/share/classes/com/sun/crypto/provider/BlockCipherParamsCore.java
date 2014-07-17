/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.util.*;
import java.io.*;
import sun.security.util.*;
import sun.misc.HexDumpEncoder;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import javax.crypto.spec.IvParameterSpec;

/**
 * This class implements the parameter (IV) used with Block Ciphers
 * in feedback-mode. IV is defined in the standards as follows:
 *
 * <pre>
 * IV ::= OCTET STRING  -- length depends on the block size of the
 * block ciphers
 * </pre>
 *
 * @author Valerie Peng
 *
 * @version 1.3, 06/24/03
 */
final class BlockCipherParamsCore {
    private int block_size = 0;
    private byte[] iv = null;

    BlockCipherParamsCore(int blksize) {
	block_size = blksize;	
    }

    void init(AlgorithmParameterSpec paramSpec) 
	throws InvalidParameterSpecException {
	if (!(paramSpec instanceof IvParameterSpec)) {
	    throw new InvalidParameterSpecException
		("Inappropriate parameter specification");
	}
	byte[] tmpIv = ((IvParameterSpec)paramSpec).getIV();
	if (tmpIv.length != block_size) {
	    throw new InvalidParameterSpecException("IV not " + 
			block_size + " bytes long");
	}	    
	iv = (byte[]) tmpIv.clone();
    }

    void init(byte[] encoded) throws IOException {
	DerInputStream der = new DerInputStream(encoded);

	byte[] tmpIv = der.getOctetString();
	if (der.available() != 0) {
	    throw new IOException("IV parsing error: extra data");
	}
	if (tmpIv.length != block_size) {
	    throw new IOException("IV not " + block_size + 
		" bytes long");
	}
	iv = tmpIv;
    }

    void init(byte[] encoded, String decodingMethod)
	throws IOException {
	if ((decodingMethod != null) && 
	    (!decodingMethod.equalsIgnoreCase("ASN.1"))) {
	    throw new IllegalArgumentException("Only support ASN.1 format");
	}
	init(encoded);
    }

    AlgorithmParameterSpec getParameterSpec(Class paramSpec)
	throws InvalidParameterSpecException
    {    
	if (IvParameterSpec.class.isAssignableFrom(paramSpec)) {
	    return new IvParameterSpec(this.iv);
	} else {
	    throw new InvalidParameterSpecException
	        ("Inappropriate parameter specification");
	}
    }

    byte[] getEncoded() throws IOException {
	DerOutputStream	out = new DerOutputStream();
	out.putOctetString(this.iv);
	return out.toByteArray();
    }

    byte[] getEncoded(String encodingMethod)
	throws IOException {
	return getEncoded();
    }

    /*
     * Returns a formatted string describing the parameters.
     */
    public String toString() {
        String LINE_SEP = System.getProperty("line.separator");

        String ivString = LINE_SEP + "    iv:" + LINE_SEP + "[";
        HexDumpEncoder encoder = new HexDumpEncoder();
        ivString += encoder.encodeBuffer(this.iv);
        ivString += "]" + LINE_SEP;
	return ivString;
    }
}
