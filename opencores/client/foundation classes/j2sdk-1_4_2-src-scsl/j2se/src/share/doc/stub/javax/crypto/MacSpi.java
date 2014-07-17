/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)MacSpi.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto;

import java.security.*;
import java.security.spec.*;

/** 
 * This class defines the <i>Service Provider Interface</i> (<b>SPI</b>)
 * for the <code>Mac</code> class.
 * All the abstract methods in this class must be implemented by each 
 * cryptographic service provider who wishes to supply the implementation
 * of a particular MAC algorithm.
 *
 * <p> Implementations are free to implement the Cloneable interface.
 *
 * @author Jan Luehe
 *
 * @version 1.3, 01/23/03
 * @since 1.4
 */
public abstract class MacSpi
{

    public MacSpi() { }

    /** 
     * Returns the length of the MAC in bytes.
     *
     * @return the MAC length in bytes.
     */
    protected abstract int engineGetMacLength();

    /** 
     * Initializes the MAC with the given (secret) key and algorithm
     * parameters.
     *
     * @param key the (secret) key.
     * @param params the algorithm parameters.
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this MAC.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this MAC.
     */
    protected abstract void engineInit(Key key, AlgorithmParameterSpec params)
        throws InvalidKeyException, InvalidAlgorithmParameterException;

    /** 
     * Processes the given byte.    
     * 
     * @param input the input byte to be processed.
     */
    protected abstract void engineUpdate(byte input);

    /** 
     * Processes the first <code>len</code> bytes in <code>input</code>,
     * starting at <code>offset</code> inclusive.
     * 
     * @param input the input buffer.
     * @param offset the offset in <code>input</code> where the input starts.
     * @param len the number of bytes to process.
     */
    protected abstract void engineUpdate(byte[] input, int offset, int len);

    /** 
     * Completes the MAC computation and resets the MAC for further use,
     * maintaining the secret key that the MAC was initialized with.
     *
     * @return the MAC result.
     */
    protected abstract byte[] engineDoFinal();

    /** 
     * Resets the MAC for further use, maintaining the secret key that the
     * MAC was initialized with.
     */
    protected abstract void engineReset();

    /** 
     * Returns a clone if the implementation is cloneable.
     * 
     * @return a clone if the implementation is cloneable.
     *
     * @exception CloneNotSupportedException if this is called
     * on an implementation that does not support <code>Cloneable</code>.
     */
    public Object clone() throws CloneNotSupportedException { }
}
