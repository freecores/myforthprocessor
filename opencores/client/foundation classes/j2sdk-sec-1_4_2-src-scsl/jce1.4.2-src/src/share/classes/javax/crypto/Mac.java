/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package javax.crypto;

import java.security.*;
import java.security.spec.AlgorithmParameterSpec;

/**
 * This class provides the functionality of a "Message Authentication Code"
 * (MAC) algorithm.
 *
 * <p> A MAC provides a way to check
 * the integrity of information transmitted over or stored in an unreliable
 * medium, based on a secret key. Typically, message
 * authentication codes are used between two parties that share a secret
 * key in order to validate information transmitted between these
 * parties.
 *
 * <p> A MAC mechanism that is based on cryptographic hash functions is
 * referred to as HMAC. HMAC can be used with any cryptographic hash function,
 * e.g., MD5 or SHA-1, in combination with a secret shared key. HMAC is
 * specified in RFC 2104.
 *
 * @author Jan Luehe
 *
 * @version 1.18, 06/24/03
 * @since 1.4
 */

public class Mac implements Cloneable {

    // The provider
    private Provider provider;

    // The provider implementation (delegate)
    private MacSpi macSpi;
    
    // The name of the MAC algorithm.
    private String algorithm;

    // Has this object been initialized?
    private boolean initialized = false;

    /**
     * Creates a MAC object.
     *
     * @param macSpi the delegate
     * @param provider the provider
     * @param algorithm the algorithm
     */
    protected Mac(MacSpi macSpi, Provider provider, String algorithm) {
	this.macSpi = macSpi;
	this.provider = provider;
	this.algorithm = algorithm;
    }

    /**
     * Returns the algorithm name of this <code>Mac</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>Mac</code> object.
     * 
     * @return the algorithm name of this <code>Mac</code> object. 
     */
    public final String getAlgorithm() {
	return this.algorithm;
    }

    /**
     * Generates an <code>Mac</code> object that implements the
     * specified MAC algorithm.
     * If the default provider package provides an implementation of the
     * requested MAC algorithm, an instance of
     * <code>Mac</code> containing that implementation is returned.
     * If the algorithm is not available in the default provider package,
     * other provider packages are searched.
     *
     * @param algorithm the standard name of the requested MAC algorithm. 
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     *
     * @return the new <code>Mac</code> object.
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available in the default provider package or any of the other provider
     * packages that were searched.  
     */
    public static final Mac getInstance(String algorithm)
	throws NoSuchAlgorithmException
    {
	try {
	    Object[] objs = JceSecurity.getImpl(algorithm, "Mac",
						(String)null);
	    return new Mac((MacSpi)objs[0], (Provider)objs[1], algorithm);
	} catch(NoSuchProviderException e) {
	    throw new NoSuchAlgorithmException(algorithm + " not found");
	}
    }

    /** 
     * Generates an <code>Mac</code> object for the specified MAC
     * algorithm from the specified provider.
     *
     * @param algorithm the standard name of the requested MAC algorithm.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     * @param provider the name of the provider.
     *
     * @return the new <code>Mac</code> object.
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available from the specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Mac getInstance(String algorithm, String provider) 
	throws NoSuchAlgorithmException, NoSuchProviderException
    {
	if (provider == null || provider.length() == 0)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = JceSecurity.getImpl(algorithm, "Mac", provider);
	return new Mac((MacSpi)objs[0], (Provider)objs[1], algorithm);
    }

    /** 
     * Generates an <code>Mac</code> object for the specified MAC
     * algorithm from the specified provider. Note: the <code>provider</code>
     * doesn't have to be registered.
     *
     * @param algorithm the standard name of the requested MAC algorithm.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     * @param provider the name of the provider.
     *
     * @return the new <code>Mac</code> object.
     *
     * @exception NoSuchAlgorithmException if the specified algorithm is not
     * available from the specified provider.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Mac getInstance(String algorithm, Provider provider) 
	throws NoSuchAlgorithmException
    {
	if (provider == null)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = JceSecurity.getImpl(algorithm, "Mac", provider);
	return new Mac((MacSpi)objs[0], (Provider)objs[1], algorithm);
    }

    /** 
     * Returns the provider of this <code>Mac</code> object.
     * 
     * @return the provider of this <code>Mac</code> object.
     */
    public final Provider getProvider() {
	return this.provider;
    }

    /** 
     * Returns the length of the MAC in bytes.
     *
     * @return the MAC length in bytes.
     */
    public final int getMacLength() {
	return macSpi.engineGetMacLength();
    }

    /**
     * Initializes this <code>Mac</code> object with the given key.
     *
     * @param key the key.
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this MAC.
     */
    public final void init(Key key)
	throws InvalidKeyException
    {
	try {
	    macSpi.engineInit(key, null);
	} catch (InvalidAlgorithmParameterException e) {
	    // do nothing
	}
	initialized = true;
    }

    /**
     * Initializes this <code>Mac</code> object with the given key and
     * algorithm parameters.
     *
     * @param key the key.
     * @param params the algorithm parameters.
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this MAC.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this MAC.
     */
    public final void init(Key key, AlgorithmParameterSpec params)
	throws InvalidKeyException, InvalidAlgorithmParameterException
    {
	macSpi.engineInit(key, params);
	initialized = true;
    }

    /**
     * Processes the given byte.    
     * 
     * @param input the input byte to be processed.
     *
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final void update(byte input)
	throws IllegalStateException
    {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}
	macSpi.engineUpdate(input);
    }

    /**
     * Processes the given array of bytes.
     * 
     * @param input the array of bytes to be processed.
     *
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final void update(byte[] input)
	throws IllegalStateException
    {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}
	if (input != null) {
	    macSpi.engineUpdate(input, 0, input.length);
	}
    }

    /**
     * Processes the first <code>len</code> bytes in <code>input</code>,
     * starting at <code>offset</code> inclusive.
     * 
     * @param input the input buffer.
     * @param offset the offset in <code>input</code> where the input starts.
     * @param len the number of bytes to process.
     *
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final void update(byte[] input, int offset, int len)
	throws IllegalStateException
    {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}

	if (input != null) {
	    if ((offset < 0) || (len > (input.length - offset)) || (len < 0))
		throw new IllegalArgumentException("Bad arguments");
	    macSpi.engineUpdate(input, offset, len);
	}
    }

    /**
     * Finishes the MAC operation.
     *
     * <p>A call to this method resets this <code>Mac</code> object to the
     * state it was in when previously initialized via a call to
     * <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     * That is, the object is reset and available to generate another MAC from
     * the same key, if desired, via new calls to <code>update</code> and 
     * <code>doFinal</code>.     
     * (In order to reuse this <code>Mac</code> object with a different key,
     * it must be reinitialized via a call to <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     *
     * @return the MAC result.
     *
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final byte[] doFinal() throws IllegalStateException {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}
	byte[] mac = macSpi.engineDoFinal();
	macSpi.engineReset();
	return mac;
    }

    /**
     * Finishes the MAC operation.
     *
     * <p>A call to this method resets this <code>Mac</code> object to the
     * state it was in when previously initialized via a call to
     * <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     * That is, the object is reset and available to generate another MAC from
     * the same key, if desired, via new calls to <code>update</code> and 
     * <code>doFinal</code>.     
     * (In order to reuse this <code>Mac</code> object with a different key,
     * it must be reinitialized via a call to <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     *
     * <p>The MAC result is stored in <code>output</code>, starting at
     * <code>outOffset</code> inclusive.
     *
     * @param output the buffer where the MAC result is stored
     * @param outOffset the offset in <code>output</code> where the MAC is
     * stored
     *
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final void doFinal(byte[] output, int outOffset)
	throws ShortBufferException, IllegalStateException
    {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}
	int macLen = getMacLength();
	if (output == null || output.length-outOffset < macLen) {
	    throw new ShortBufferException
		("Cannot store MAC in output buffer");
	}
	byte[] mac = doFinal();
	System.arraycopy(mac, 0, output, outOffset, macLen);
	return;
    }

    /**
     * Processes the given array of bytes and finishes the MAC operation.
     *
     * <p>A call to this method resets this <code>Mac</code> object to the
     * state it was in when previously initialized via a call to
     * <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     * That is, the object is reset and available to generate another MAC from
     * the same key, if desired, via new calls to <code>update</code> and 
     * <code>doFinal</code>.     
     * (In order to reuse this <code>Mac</code> object with a different key,
     * it must be reinitialized via a call to <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     *
     * @param input data in bytes
     * @return the MAC result.
     *
     * @exception IllegalStateException if this <code>Mac</code> has not been
     * initialized.
     */
    public final byte[] doFinal(byte[] input) throws IllegalStateException
    {
	if (initialized == false) {
	    throw new IllegalStateException("MAC not initialized");
	}
	update(input);
	return doFinal();
    }

    /**
     * Resets this <code>Mac</code> object.
     *
     * <p>A call to this method resets this <code>Mac</code> object to the
     * state it was in when previously initialized via a call to
     * <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     * That is, the object is reset and available to generate another MAC from
     * the same key, if desired, via new calls to <code>update</code> and 
     * <code>doFinal</code>.     
     * (In order to reuse this <code>Mac</code> object with a different key,
     * it must be reinitialized via a call to <code>init(Key)</code> or
     * <code>init(Key, AlgorithmParameterSpec)</code>.
     */
    public final void reset() {
        macSpi.engineReset();
    }

    /**
     * Returns a clone if the provider implementation is cloneable.    
     * 
     * @return a clone if the provider implementation is cloneable.
     *
     * @exception CloneNotSupportedException if this is called on a
     * delegate that does not support <code>Cloneable</code>.
     */
    public final Object clone() throws CloneNotSupportedException {
	Mac that = (Mac)super.clone();
	that.macSpi = (MacSpi)this.macSpi.clone();
	return that;
    }
}

