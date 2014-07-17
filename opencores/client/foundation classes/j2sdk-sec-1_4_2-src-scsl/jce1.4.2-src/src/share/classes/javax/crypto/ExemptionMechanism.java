/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.AlgorithmParameters;
import java.security.Provider;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.InvalidKeyException;
import java.security.InvalidAlgorithmParameterException;
import java.security.spec.AlgorithmParameterSpec;

/**
 * This class provides the functionality of an exemption mechanism, examples
 * of which are <i>key recovery</i>, <i>key weakening</i>, and
 * <i>key escrow</i>.
 *
 * <p>Applications or applets that use an exemption mechanism may be granted
 * stronger encryption capabilities than those which don't.
 *
 * @version 1.9, 06/24/03
 * @since 1.4
 */

public class ExemptionMechanism {

    // The provider
    private Provider provider;

    // The provider implementation (delegate)
    private ExemptionMechanismSpi exmechSpi;

    // The name of the exemption mechanism.
    private String mechanism;

    // Flag which indicates whether this ExemptionMechanism
    // result is generated successfully.
    private boolean done = false;

    // State information
    private boolean initialized = false;

    // Store away the key at init() time for later comparison.
    private Key keyStored = null;

    /**
     * Creates a ExemptionMechanism object.
     *
     * @param exmechSpi the delegate
     * @param provider the provider
     * @param mechanism the exemption mechanism
     */
    protected ExemptionMechanism(ExemptionMechanismSpi exmechSpi,
				 Provider provider,
				 String mechanism) {
	this.exmechSpi = exmechSpi;
	this.provider = provider;
	this.mechanism = mechanism;
    }

    /**
     * Returns the exemption mechanism name of this
     * <code>ExemptionMechanism</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this
     * <code>ExemptionMechanism</code> object.
     *
     * @return the exemption mechanism name of this
     * <code>ExemptionMechanism</code> object.
     */
    public final String getName() {
	return this.mechanism;
    }

    /**
     * Generates a <code>ExemptionMechanism</code> object that implements the
     * specified exemption mechanism.
     * If the default provider package provides an implementation of the
     * requested exemption mechanism, an instance of
     * <code>ExemptionMechanism</code> containing that implementation is
     * returned.
     * If the exemption mechanism is not available in the default provider
     * package, other provider packages are searched.
     *
     * @param mechanism the standard name of the requested exemption
     * mechanism.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a>
     * for information about standard exemption mechanism names.
     *
     * @return the new <code>ExemptionMechanism</code> object
     *
     * @exception NoSuchAlgorithmException if the specified exemption
     * mechanism is not available in the default provider package or any of
     * the other provider packages that were searched.
     */
    public static final ExemptionMechanism getInstance(String mechanism)
	throws NoSuchAlgorithmException
    {
	try {
	    Object[] objs = JceSecurity.getImpl(mechanism,
						"ExemptionMechanism",
						(String)null);
	    return new ExemptionMechanism((ExemptionMechanismSpi)objs[0],
					  (Provider)objs[1],
					  mechanism);
	} catch(NoSuchProviderException e) {
	    throw new NoSuchAlgorithmException(mechanism + " not found");
	}
    }

    /**
     * Generates a <code>ExemptionMechanism</code> object for the specified
     * exemption mechanism from the specified provider.
     *
     * @param mechanism the standard name of the requested exemption mechanism.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a>
     * for information about standard exemption mechanism names.
     * @param provider the name of the provider
     *
     * @return the new <code>ExemptionMechanism</code> object
     *
     * @exception NoSuchAlgorithmException if the specified exemption
     * mechanism is not available from the specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final ExemptionMechanism getInstance(String mechanism,
						       String provider)
	throws NoSuchAlgorithmException, NoSuchProviderException
    {
	if (provider == null || provider.length() == 0)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = JceSecurity.getImpl(mechanism, "ExemptionMechanism",
					    provider);
	return new ExemptionMechanism((ExemptionMechanismSpi)objs[0],
				      (Provider)objs[1],
				      mechanism);
    }

    /**
     * Generates a <code>ExemptionMechanism</code> object for the specified
     * exemption mechanism from the specified provider. Note: the 
     * <code>provider</code> doesn't have to be registered.
     *
     * @param mechanism the standard name of the requested exemption mechanism.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a>
     * for information about standard exemption mechanism names.
     * @param provider the provider
     *
     * @return the new <code>ExemptionMechanism</code> object
     *
     * @exception NoSuchAlgorithmException if the specified exemption
     * mechanism is not available from the specified provider.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final ExemptionMechanism getInstance(String mechanism,
						       Provider provider)
	throws NoSuchAlgorithmException
    {
	if (provider == null)
	    throw new IllegalArgumentException("missing provider");
	Object[] objs = JceSecurity.getImpl(mechanism, "ExemptionMechanism",
					    provider);
	return new ExemptionMechanism((ExemptionMechanismSpi)objs[0],
				      (Provider)objs[1],
				      mechanism);
    }

    /**
     * Returns the provider of this <code>ExemptionMechanism</code> object.
     *
     * @return the provider of this <code>ExemptionMechanism</code> object.
     */
     public final Provider getProvider() {
        return this.provider;
     }

    /**
     * Returns whether the result blob has been generated successfully by this
     * exemption mechanism.
     *
     * <p>The method also makes sure that the key passed in is the same as
     * the one this exemption mechanism used in initializing and generating
     * phases.
     *
     * @param key the key the crypto is going to use.
     *
     * @return whether the result blob of the same key has been generated
     * successfully by this exemption mechanism.
     *
     * @exception ExemptionMechanismException if problem(s) encountered
     * while determining whether the result blob has been generated successfully
     * by this exemption mechanism object.
     */
     public final boolean isCryptoAllowed(Key key)
     throws ExemptionMechanismException {
        boolean ret = false;
        if (done && (key != null)) {
            // Check if the key passed in is the same as the one
            // this exemption mechanism used.
            ret = keyStored.equals(key);
        }
        return ret;
     }

    /**
     * Returns the length in bytes that an output buffer would need to be in
     * order to hold the result of the next
     * {@link #genExemptionBlob(byte[]) genExemptionBlob}
     * operation, given the input length <code>inputLen</code> (in bytes).
     *
     * <p>The actual output length of the next
     * {@link #genExemptionBlob(byte[]) genExemptionBlob}
     * call may be smaller than the length returned by this method.
     *
     * @param inputLen the input length (in bytes)
     *
     * @return the required output buffer size (in bytes)
     *
     * @exception IllegalStateException if this exemption mechanism is in a
     * wrong state (e.g., has not yet been initialized)
     */
    public final int getOutputSize(int inputLen)
	throws IllegalStateException {
	    if (!initialized) {
	        throw new IllegalStateException
	        ("ExemptionMechanism not initialized");
	    }
	    if (inputLen < 0) {
	        throw new IllegalArgumentException
	        ("Input size must be equal to " + "or greater than zero");
	    }
	    return exmechSpi.engineGetOutputSize(inputLen);
    }

    /**
     * Initializes this exemption mechanism with a key.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * that cannot be derived from the given <code>key</code>, the
     * underlying exemption mechanism implementation is supposed to
     * generate the required parameters itself (using provider-specific
     * default values); in the case that algorithm parameters must be
     * specified by the caller, an <code>InvalidKeyException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
     public final void init(Key key)
     throws InvalidKeyException, ExemptionMechanismException {
        done = false;
        initialized = false;

        keyStored = key;
        exmechSpi.engineInit(key);
        initialized = true;
    }

    /**
     * Initializes this exemption mechanism with a key and a set of algorithm
     * parameters.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * and <code>params</code> is null, the underlying exemption
     * mechanism implementation is supposed to generate the required
     * parameters itself (using provider-specific default values); in the case
     * that algorithm parameters must be specified by the caller, an
     * <code>InvalidAlgorithmParameterException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
     public final void init(Key key, AlgorithmParameterSpec params)
     throws InvalidKeyException, InvalidAlgorithmParameterException,
     ExemptionMechanismException {
        done = false;
        initialized = false;

	keyStored = key;
        exmechSpi.engineInit(key, params);
        initialized = true;	
    }

    /**
     * Initializes this exemption mechanism with a key and a set of algorithm
     * parameters.
     *
     * <p>If this exemption mechanism requires any algorithm parameters
     * and <code>params</code> is null, the underlying exemption mechanism
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default values); in the case that algorithm
     * parameters must be specified by the caller, an
     * <code>InvalidAlgorithmParameterException</code> is raised.
     *
     * @param key the key for this exemption mechanism
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * this exemption mechanism.
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this exemption mechanism.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of initializing.
     */
     public final void init(Key key, AlgorithmParameters params)
     throws InvalidKeyException, InvalidAlgorithmParameterException,
     ExemptionMechanismException {
        done = false;
        initialized = false;

        keyStored = key;
        exmechSpi.engineInit(key, params);
        initialized = true;
    }

    /**
     * Generates the exemption mechanism key blob.
     *
     * @return the new buffer with the result key blob.
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final byte[] genExemptionBlob()
	throws IllegalStateException, ExemptionMechanismException {
	    if (!initialized) {
            throw new IllegalStateException
            ("ExemptionMechanism not initialized");
        }
        byte[] blob = exmechSpi.engineGenExemptionBlob();
        done = true;
        return blob;
    }

    /**
     * Generates the exemption mechanism key blob, and stores the result in
     * the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final int genExemptionBlob(byte[] output)
	throws IllegalStateException, ShortBufferException,
	ExemptionMechanismException {
	    if (!initialized) {
	        throw new IllegalStateException
	        ("ExemptionMechanism not initialized");
	    }
	    int n = exmechSpi.engineGenExemptionBlob(output, 0);
	    done = true;
	    return n;
    }

    /**
     * Generates the exemption mechanism key blob, and stores the result in
     * the <code>output</code> buffer, starting at <code>outputOffset</code>
     * inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this exemption mechanism is in
     * a wrong state (e.g., has not been initialized).
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result.
     * @exception ExemptionMechanismException if problem(s) encountered in the
     * process of generating.
     */
    public final int genExemptionBlob(byte[] output, int outputOffset)
	throws IllegalStateException, ShortBufferException,
	ExemptionMechanismException {
	    if (!initialized) {
	        throw new IllegalStateException
	        ("ExemptionMechanism not initialized");
	    }
	    int n = exmechSpi.engineGenExemptionBlob(output, outputOffset);
	    done = true;
	    return n;
    }

    /**
     * Ensures that the key stored away by this ExemptionMechanism
     * object will be wiped out when there are no more references to it.
     */
    protected void finalize() {
	keyStored = null;
	// Are there anything else we could do?
    }    
}

