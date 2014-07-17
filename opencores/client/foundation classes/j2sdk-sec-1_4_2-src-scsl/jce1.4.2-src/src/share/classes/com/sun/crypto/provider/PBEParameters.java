/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.util.*;
import java.io.*;
import java.math.BigInteger;
import java.security.AlgorithmParametersSpi;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import javax.crypto.spec.PBEParameterSpec;
import sun.misc.HexDumpEncoder;
import sun.security.util.*;


/**
 * This class implements the parameter set used with password-based
 * encryption, which is defined in PKCS#5 as follows:
 *
 * <pre>
 * PBEParameter ::=  SEQUENCE {
 *     salt   OCTET STRING SIZE(8),
 *     iterationCount   INTEGER }
 * </pre>
 *
 * @author Jan Luehe
 *
 * @version 1.9, 06/24/03
 */

public final class PBEParameters extends AlgorithmParametersSpi {

    // the salt
    private byte[] salt = null;

    // the iteration count
    private int iCount = 0;

    protected void engineInit(AlgorithmParameterSpec paramSpec) 
	throws InvalidParameterSpecException
   {
       if (!(paramSpec instanceof PBEParameterSpec)) {
	   throw new InvalidParameterSpecException
	       ("Inappropriate parameter specification");
       }
       this.salt = (byte[])((PBEParameterSpec)paramSpec).getSalt().clone();
       this.iCount = ((PBEParameterSpec)paramSpec).getIterationCount();
    }

    protected void engineInit(byte[] encoded)
	throws IOException
    {
	try {
	    DerValue val = new DerValue(encoded);
	    if (val.tag != DerValue.tag_Sequence) {
		throw new IOException("PBE parameter parsing error: "
				      + "not a sequence");
	    }
	    val.data.reset();

	    this.salt = val.data.getOctetString();
	    this.iCount = val.data.getInteger();

	    if (val.data.available() != 0) {
		throw new IOException
		    ("PBE parameter parsing error: extra data");
	    }
	} catch (NumberFormatException e) {
	    throw new IOException("iteration count too big");
	}
    }

    protected void engineInit(byte[] encoded, String decodingMethod)
	throws IOException
    {
	engineInit(encoded);
    }

    protected AlgorithmParameterSpec engineGetParameterSpec(Class paramSpec)
	throws InvalidParameterSpecException
    {    
	if (PBEParameterSpec.class.isAssignableFrom(paramSpec)) {
	    return new PBEParameterSpec(this.salt, this.iCount);
	} else {
	    throw new InvalidParameterSpecException
	        ("Inappropriate parameter specification");
	}
    }

    protected byte[] engineGetEncoded() throws IOException {
	DerOutputStream	out = new DerOutputStream();
	DerOutputStream	bytes = new DerOutputStream();

	bytes.putOctetString(this.salt);
	bytes.putInteger(this.iCount);

	out.write(DerValue.tag_Sequence, bytes);
	return out.toByteArray();
    }

    protected byte[] engineGetEncoded(String encodingMethod)
	throws IOException
    {
	return engineGetEncoded();
    }

    /*
     * Returns a formatted string describing the parameters.
     */
    protected String engineToString() {
	String LINE_SEP = System.getProperty("line.separator");
        String saltString = LINE_SEP + "    salt:" + LINE_SEP + "[";
        HexDumpEncoder encoder = new HexDumpEncoder();
        saltString += encoder.encodeBuffer(salt);
        saltString += "]";

	return saltString + LINE_SEP + "    iterationCount:"
	    + LINE_SEP + Debug.toHexString(BigInteger.valueOf(iCount))
	    + LINE_SEP;
    }
}
