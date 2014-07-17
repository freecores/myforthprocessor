/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package javax.crypto;

import java.security.*;
import java.security.spec.*;

/**
 * This class provides a delegate for the identity cipher - one that does not
 * tranform the plaintext.
 *
 * @author  Li Gong
 *
 * @version 1.8, 06/24/03
 *
 * @see Nullcipher
 *
 * @since 1.4
 */

final class NullCipherSpi extends CipherSpi {

    /*
     * Do not let anybody instantiate this directly (protected).
     */
    protected NullCipherSpi() {}

    public void engineSetMode(String mode) {}

    public void engineSetPadding(String padding) {}
    
    protected int engineGetBlockSize() {
	return 1;
    }
	
    protected int engineGetOutputSize(int inputLen) {
	return inputLen;
    }
	
    protected byte[] engineGetIV() {
	byte[] x = new byte[8];
	return x;
    }

    protected AlgorithmParameters engineGetParameters() {
	return null;
    }

    protected void engineInit(int mode, Key key, SecureRandom random) {}

    protected void engineInit(int mode, Key key,
			      AlgorithmParameterSpec params,
			      SecureRandom random) {}

    protected void engineInit(int mode, Key key,
			      AlgorithmParameters params,
			      SecureRandom random) {}

    protected byte[] engineUpdate(byte[] input, int inputOffset,
				  int inputLen) {
	if (input == null) return null;
	byte[] x = new byte[inputLen];
	System.arraycopy(input, inputOffset, x, 0, inputLen);
	return x;
    }
	
    protected int engineUpdate(byte[] input, int inputOffset,
			       int inputLen, byte[] output,
			       int outputOffset) {
	if (input == null) return 0;
	System.arraycopy(input, inputOffset, output, outputOffset, inputLen);
	return inputLen;
    }
	
    protected byte[] engineDoFinal(byte[] input, int inputOffset,
				   int inputLen)
    {
	return engineUpdate(input, inputOffset, inputLen);
    }
	
    protected int engineDoFinal(byte[] input, int inputOffset,
				int inputLen, byte[] output,
				int outputOffset)
    {
	return engineUpdate(input, inputOffset, inputLen,
			    output, outputOffset);
    }

    protected int engineGetKeySize(Key key) 
    {
	return 0;
    }
}
