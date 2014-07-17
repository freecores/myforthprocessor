/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.util.*;
import java.io.*;
import sun.security.util.*;
import java.security.AlgorithmParametersSpi;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import javax.crypto.spec.IvParameterSpec;

/**
 * This class implements the parameter (IV) used with the Triple DES algorithm
 * in feedback-mode. IV is defined in the standards as follows:
 *
 * <pre>
 * IV ::= OCTET STRING  -- 8 octets
 * </pre>
 *
 * @author Jan Luehe
 *
 * @version 1.9, 06/24/03
 */
public final class DESedeParameters extends AlgorithmParametersSpi {

    private BlockCipherParamsCore core;

    public DESedeParameters() {
	core = new BlockCipherParamsCore(DESConstants.DES_BLOCK_SIZE);
    }

    protected void engineInit(AlgorithmParameterSpec paramSpec)
        throws InvalidParameterSpecException {
	core.init(paramSpec);
    }

    protected void engineInit(byte[] encoded)
        throws IOException {
	core.init(encoded); 
    }

    protected void engineInit(byte[] encoded, String decodingMethod)
        throws IOException {
	core.init(encoded, decodingMethod); 
    }

    protected AlgorithmParameterSpec engineGetParameterSpec(Class paramSpec)
        throws InvalidParameterSpecException {
	return core.getParameterSpec(paramSpec);
    }

    protected byte[] engineGetEncoded() throws IOException {
	return core.getEncoded();
    }

    protected byte[] engineGetEncoded(String encodingMethod)
	throws IOException {
	return core.getEncoded();
    }

    protected String engineToString() {
	return core.toString();
    }
}
