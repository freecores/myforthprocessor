/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.util.Arrays;
import java.util.StringTokenizer;
import java.util.NoSuchElementException;
import java.util.Set;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.AlgorithmParameters;
import java.security.Provider;
import java.security.Key;
import java.security.PublicKey;
import java.security.SecureRandom;
import java.security.Security;
import java.security.NoSuchAlgorithmException;
import java.security.NoSuchProviderException;
import java.security.InvalidKeyException;
import java.security.InvalidParameterException;
import java.security.InvalidAlgorithmParameterException;
import java.security.spec.AlgorithmParameterSpec;
import java.security.spec.InvalidParameterSpecException;
import java.security.cert.Certificate;
import java.security.cert.X509Certificate;

import javax.crypto.spec.*;
 
/**
 * This class provides the functionality of a cryptographic cipher for
 * encryption and decryption. It forms the core of the Java Cryptographic
 * Extension (JCE) framework.
 *
 * <p>In order to create a Cipher object, the application calls the
 * Cipher's <code>getInstance</code> method, and passes the name of the
 * requested <i>transformation</i> to it. Optionally, the name of a provider
 * may be specified.
 *
 * <p>A <i>transformation</i> is a string that describes the operation (or
 * set of operations) to be performed on the given input, to produce some
 * output. A transformation always includes the name of a cryptographic
 * algorithm (e.g., <i>DES</i>), and may be followed by a feedback mode and
 * padding scheme.
 *
 * <p> A transformation is of the form:<p>
 *
 * <ul>
 * <li>"<i>algorithm/mode/padding</i>" or
 * <p>
 * <li>"<i>algorithm</i>"
 * </ul>
 *
 * <P> (in the latter case,
 * provider-specific default values for the mode and padding scheme are used).
 * For example, the following is a valid transformation:<p>
 *
 * <pre>
 *     Cipher c = Cipher.getInstance("<i>DES/CBC/PKCS5Padding</i>");
 * </pre>
 *
 * <p>When requesting a block cipher in stream cipher mode (e.g.,
 * <code>DES</code> in <code>CFB</code> or <code>OFB</code> mode), the user may
 * optionally specify the number of bits to be
 * processed at a time, by appending this number to the mode name as shown in
 * the "<i>DES/CFB8/NoPadding</i>" and "<i>DES/OFB32/PKCS5Padding</i>"
 * transformations. If no such number is specified, a provider-specific default
 * is used. (For example, the "SunJCE" provider uses a default of 64 bits.)
 * 
 * @author Jan Luehe
 *
 * @version 1.111, 06/24/03
 *
 * @see KeyGenerator
 * @see SecretKey
 * @since 1.4
 */

public class Cipher {

    /**
     * Constant used to initialize cipher to encryption mode.
     */
    public static final int ENCRYPT_MODE = 1;

    /**
     * Constant used to initialize cipher to decryption mode.
     */
    public static final int DECRYPT_MODE = 2;

    /**
     * Constant used to initialize cipher to key-wrapping mode.
     */
    public static final int WRAP_MODE = 3;

    /**
     * Constant used to initialize cipher to key-unwrapping mode.
     */
    public static final int UNWRAP_MODE = 4;

    /**
     * Constant used to indicate the to-be-unwrapped key is a "public key".
     */
    public static final int PUBLIC_KEY = 1;

    /**
     * Constant used to indicate the to-be-unwrapped key is a "private key". 
     */
    public static final int PRIVATE_KEY = 2;

    /**
     * Constant used to indicate the to-be-unwrapped key is a "secret key".
     */
    public static final int SECRET_KEY = 3;

    // The provider
    private Provider provider;

    // The provider implementation (delegate)
    private CipherSpi cipherSpi;

    // The transformation
    private String transformation;

    // Crypto permission representing the maximum allowable cryptographic
    // strength that this Cipher object can be used for. (The cryptographic
    // strength is a function of the keysize and algorithm parameters encoded
    // in the crypto permission.)
    private CryptoPermission cryptoPerm;

    // The exemption mechanism that needs to be enforced
    private ExemptionMechanism exmech;

    // Flag which indicates whether or not this cipher has been initialized
    private boolean initialized = false;

    // The operation mode - store the operation mode after the
    // cipher has been initialized.
    private int opmode = 0;

    // The OID for the KeyUsage extension in an X.509 v3 certificate
    private static final String KEY_USAGE_EXTENSION_OID = "2.5.29.15";

    /**
     * Creates a Cipher object.
     *
     * @param cipherSpi the delegate
     * @param provider the provider
     * @param transformation the transformation
     */
    protected Cipher(CipherSpi cipherSpi,
		     Provider provider,
		     String transformation) {
	this.cipherSpi = cipherSpi;
	this.provider = provider;
	this.transformation = transformation;
    }

    /**
     * Creates a Cipher object.
     *
     * @param cipherSpi the delegate
     * @param provider the provider
     * @param cryptoPerm the crypto permission
     * @param exmech the exemption mechanism to be enforced
     * @param transformation the transformation
     * @param restricted flag indicating whether or not any export restrictions
     *        are enforced
     */
    Cipher(CipherSpi cipherSpi,
	   Provider provider,
	   CryptoPermission cryptoPerm,
	   ExemptionMechanism exmech,
	   Boolean restricted,
	   String transformation) {
	this(cipherSpi, provider, transformation); 
	this.cryptoPerm = cryptoPerm;
	this.exmech = exmech;
	// 'restricted' is no longer used now that we have the 
	// approval to remove the code which prevents "double-encryption" 
    }

    private static String[] tokenizeTransformation(String transformation) 
	throws NoSuchAlgorithmException {
	if (transformation == null) {
	    throw new NoSuchAlgorithmException("No transformation given");
	} 
	/*
	 * array containing the components of a Cipher transformation:
	 *
	 * index 0: algorithm component (e.g., DES)
	 * index 1: feedback component (e.g., CFB)
	 * index 2: padding component (e.g., PKCS5Padding)
	 */
	String[] parts = new String[3];
	int count = 0;
	StringTokenizer parser = new StringTokenizer(transformation, "/");
	try {
	    while (parser.hasMoreTokens() && count < 3) {
	        parts[count++] = parser.nextToken().trim();
	    }
	    if (count == 0 || count == 2 || parser.hasMoreTokens()) {
	        throw new NoSuchAlgorithmException("Invalid transformation"
					       + " format:" + 
					       transformation);
	    }
	} catch (NoSuchElementException e) {
	    throw new NoSuchAlgorithmException("Invalid transformation " + 
					   "format:" + transformation);
	}
	if ((parts[0]==null) || (parts[0].equals(""))) {
	    throw new NoSuchAlgorithmException("Invalid transformation:" +
				   "algorithm not specified-"
				   + transformation);
	}
        return parts;
    }
    
    private static Cipher findCipherInProvider(String[] parts, 
					       String transformation, 
					       Provider provider) 
    throws NoSuchAlgorithmException, NoSuchPaddingException {
	boolean setMode = false;
	boolean setPadding = false;
	Object[] objs = null;
	
	int type = 4;
	if ((parts[1]==null) || (parts[1].equals(""))) type -= 2;
	if ((parts[2]==null) || (parts[2].equals(""))) type -= 1;
	    
	/*
	 * If there are 3 components in the transformation name, we
	 * first check if the provider has an entry in its
	 * properties list with those 3 components (e.g.,
	 * "DES/CBC/PKCS5Padding").
	 * If this is not the case, we check for the first 2
	 * components (e.g., "DES/CBC"), and then set the padding
	 * mechanism using the 3rd component.
	 * If this still does not work, we check if there is an
	 * entry for the 1st and 3rd components (e.g.,
	 * "DES//PKCS5Padding"), and then set the mode using the
	 * 2nd component.
	 * If this still does not work, we check if there is an
	 * entry for the first component, and then set mode and
	 * padding mechanism using the 2nd and 3rd component,
	 * respectively.
	 * If this still does not work, throw a
	 * NoSuchAlgorithmException.
	 */
	switch(type) {
	case 4: // Algo, Mode & Padding
	    try {
		objs = JceSecurity.getImpl(parts[0]+"/"+parts[1]+"/"+parts[2], 
					   "Cipher", provider);
	    } catch (NoSuchAlgorithmException e) {
		try {
		    objs = JceSecurity.getImpl(parts[0]+"/"+parts[1],
					       "Cipher", provider);
		    setPadding = true;
		} catch (NoSuchAlgorithmException ee) {
		    try {
			objs = JceSecurity.getImpl(parts[0] + "//" + parts[2],
						   "Cipher", provider);
			setMode = true;
		    } catch (NoSuchAlgorithmException eee) {
			objs = JceSecurity.getImpl(parts[0], "Cipher",
						   provider);
			setMode = true;
			setPadding = true;
		    }
		}
	    }
	    break;
	case 3: // Algo & Mode
	    try {
		objs = JceSecurity.getImpl(parts[0]+"/"+parts[1], 
					   "Cipher", provider);
	    } catch (NoSuchAlgorithmException e) {
		objs = JceSecurity.getImpl(parts[0],
					   "Cipher", provider);
		setMode = true;
	    }
	    break;
	case 2: // Algo & Padding
	    try {
		objs = JceSecurity.getImpl(parts[0]+"//"+parts[2], 
					   "Cipher", provider);
	    } catch (NoSuchAlgorithmException e) {
		objs = JceSecurity.getImpl(parts[0],
					   "Cipher", provider);
		setPadding = true;
	    }
	    break;
	case 1: // Algo only
	    objs = JceSecurity.getImpl(parts[0], "Cipher", provider);
	    break;
	}
	
	Cipher cipher = new Cipher((CipherSpi)objs[0],
				   (Provider)objs[1],
				   (CryptoPermission)objs[2],
				   (ExemptionMechanism)objs[3],
				   (Boolean)objs[4],
				   transformation);

	if (setMode) cipher.cipherSpi.engineSetMode(parts[1]);
	if (setPadding) cipher.cipherSpi.engineSetPadding(parts[2]);
	
	return cipher;	
    }

    /**
     * Generates a <code>Cipher</code> object that implements the specified
     * transformation.
     *
     * <p>If the default provider package supplies an implementation of the
     * requested transformation, an instance of <code>Cipher</code> containing
     * that implementation is returned.
     * If the transformation is not available in the default provider package,
     * other provider packages are searched.
     *
     * @param transformation the name of the transformation, e.g.,
     * <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if the specified transformation is
     * not available in the default provider package or any of the other
     * provider packages that were searched.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     */
    public static final Cipher getInstance(String transformation)
	throws NoSuchAlgorithmException, NoSuchPaddingException
    {
	String[] parts = tokenizeTransformation(transformation);

	/*
	 * Go through the complete provider list starting with the 
	 * most preferred provider.
	 */
	Cipher cipher = null;
	Provider[] list = Security.getProviders();
	for (int i=0; (i<list.length) && (cipher == null); i++) {
	    try {
		cipher = findCipherInProvider(parts, transformation, list[i]);
	    } catch (Exception ex) {
		// swallow and proceed to next provider
            }
	}
	if (cipher == null) {
	    // did not find any provider supporting the specified 
	    // transformation
	    throw new NoSuchAlgorithmException("Cannot find any provider supporting " + transformation);
	}
	return cipher;
    }

    /**
     * Creates a <code>Cipher</code> object that implements the specified
     * transformation, as supplied by the specified provider.
     *
     * @param transformation the name of the transformation,
     * e.g., <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     * @param provider the name of the provider
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if no transformation was
     * specified, or if the specified transformation is
     * not available from the specified provider.
     * @exception NoSuchProviderException if the specified provider has not
     * been configured.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Cipher getInstance(String transformation,
					   String provider)
	throws NoSuchAlgorithmException, NoSuchProviderException,
	NoSuchPaddingException
    {
	String[] parts = tokenizeTransformation(transformation);
	if (provider == null || provider.trim().equals("")) {
	    throw new IllegalArgumentException("Missing provider");
	}
	Provider p = Security.getProvider(provider);
	if (p == null) {
	    throw new NoSuchProviderException("Provider '" + provider + 
					      "' not found");
	}
	return findCipherInProvider(parts, transformation, p);
    }

    /**
     * Creates a <code>Cipher</code> object that implements the specified
     * transformation, as supplied by the specified provider. Note: the 
     * <code>provider</code> doesn't have to be registered.
     *
     * @param transformation the name of the transformation,
     * e.g., <i>DES/CBC/PKCS5Padding</i>.
     * See Appendix A in the
     * <a href="../../../guide/security/jce/JCERefGuide.html#AppA">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard transformation names.
     * @param provider the provider
     *
     * @return a cipher that implements the requested transformation
     *
     * @exception NoSuchAlgorithmException if no transformation was
     * specified, or if the specified transformation is
     * not available from the specified provider.
     * @exception NoSuchPaddingException if <code>transformation</code>
     * contains a padding scheme that is not available.
     * @exception IllegalArgumentException if the <code>provider</code>
     * is null.
     */
    public static final Cipher getInstance(String transformation,
					   Provider provider)
	throws NoSuchAlgorithmException, NoSuchPaddingException
    {
	String[] parts = tokenizeTransformation(transformation);
	if (provider == null)
	    throw new IllegalArgumentException("Missing provider");
	return findCipherInProvider(parts, transformation, provider);
    }

    /** 
     * Returns the provider of this <code>Cipher</code> object.
     * 
     * @return the provider of this <code>Cipher</code> object
     */
    public final Provider getProvider() {
	return this.provider;
    }

    /**
     * Returns the algorithm name of this <code>Cipher</code> object.
     *
     * <p>This is the same name that was specified in one of the
     * <code>getInstance</code> calls that created this <code>Cipher</code>
     * object..
     *
     * @return the algorithm name of this <code>Cipher</code> object. 
     */
    public final String getAlgorithm() {
	return this.transformation;
    }

    /**
     * Returns the block size (in bytes).
     *
     * @return the block size (in bytes), or 0 if the underlying algorithm is
     * not a block cipher
     */
    public final int getBlockSize() {
	return cipherSpi.engineGetBlockSize();
    }

    /**
     * Returns the length in bytes that an output buffer would need to be in
     * order to hold the result of the next <code>update</code> or
     * <code>doFinal</code> operation, given the input length
     * <code>inputLen</code> (in bytes).
     *
     * <p>This call takes into account any unprocessed (buffered) data from a
     * previous <code>update</code> call, and padding.
     *
     * <p>The actual output length of the next <code>update</code> or
     * <code>doFinal</code> call may be smaller than the length returned by
     * this method.
     *
     * @param inputLen the input length (in bytes)
     *
     * @return the required output buffer size (in bytes)
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not yet been initialized)
     */
    public final int getOutputSize(int inputLen)
	throws IllegalStateException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	if (inputLen < 0) {
	    throw new IllegalArgumentException("Input size must be equal to "
					       + "or greater than zero");
	}
	return cipherSpi.engineGetOutputSize(inputLen);
    }

    /**
     * Returns the initialization vector (IV) in a new buffer.
     *
     * <p>This is useful in the case where a random IV was created,
     * or in the context of password-based encryption or
     * decryption, where the IV is derived from a user-supplied password. 
     *
     * @return the initialization vector in a new buffer, or null if the
     * underlying algorithm does not use an IV, or if the IV has not yet
     * been set.
     */
    public final byte[] getIV() {
	return cipherSpi.engineGetIV();
    }

    /**
     * Returns the parameters used with this cipher.
     *
     * <p>The returned parameters may be the same that were used to initialize
     * this cipher, or may contain a combination of default and random
     * parameter values used by the underlying cipher implementation if this
     * cipher requires algorithm parameters but was not initialized with any.
     *
     * @return the parameters used with this cipher, or null if this cipher
     * does not use any parameters.
     */
    public final AlgorithmParameters getParameters() {
	return cipherSpi.engineGetParameters();
    }

    /**
     * Returns the exemption mechanism object used with this cipher.
     *
     * @return the exemption mechanism object used with this cipher, or
     * null if this cipher does not use any exemption mechanism.
     */
    public final ExemptionMechanism getExemptionMechanism() {
	return exmech;
    }

    /**
     * Initializes this cipher with a key.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the given <code>key</code>, the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of
     * the following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the key
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or if this cipher is being initialized for
     * decryption and requires algorithm parameters that cannot be
     * determined from the given key, or if the given key has a keysize that
     * exceeds the maximum allowable keysize (as determined from the
     * configured jurisdiction policy files).
     */
    public final void init(int opmode, Key key)
	throws InvalidKeyException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) && 
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm != CryptoAllPermission.INSTANCE) {
	    // Determine keysize and check if it is within legal limits
	    int keySize = cipherSpi.engineGetKeySize(key);
	    if (!(cryptoPerm.implies(new CryptoPermission(
                                         key.getAlgorithm(),
					 keySize,
					 cryptoPerm.getExemptionMechanism())))) {
		throw new SecurityException("Unsupported keysize or " +
					    "algorithm parameters");
	    }
	    if (exmech != null) {
		try {
		    if (!exmech.isCryptoAllowed(key)) {
			throw new SecurityException(exmech.getName() +
						    " isn't enforced");
		    }
		} catch (ExemptionMechanismException eme) {
		    throw new InvalidKeyException("Cannot determine whether " +
						  exmech.getName() + " has " +
						  "been enforced");
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, JceSecurity.RANDOM);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Initializes this cipher with a key and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the given <code>key</code>, the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or if this cipher is being initialized for
     * decryption and requires algorithm parameters that cannot be
     * determined from the given key, or if the given key has a keysize that
     * exceeds the maximum allowable keysize (as determined from the
     * configured jurisdiction policy files).
     */
    public final void init(int opmode, Key key, SecureRandom random)
	throws InvalidKeyException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) && 
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm != CryptoAllPermission.INSTANCE) {
	    // Determine keysize and check if it is within legal limits
	    int keySize = cipherSpi.engineGetKeySize(key);
	    if (!(cryptoPerm.implies(new CryptoPermission(
                                         key.getAlgorithm(),
					 keySize,
					 cryptoPerm.getExemptionMechanism())))) {
		throw new SecurityException("Unsupported keysize or " +
                                            "algorithm parameters");
	    }
	    if (exmech != null) {
		try {
		    if (!exmech.isCryptoAllowed(key)) {
			throw new SecurityException(exmech.getName() +
						    " isn't enforced");
		    }
		} catch (ExemptionMechanismException eme) {
		    throw new InvalidKeyException("Cannot determine whether " +
						  exmech.getName() + " has " +
						  "been enforced");
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, random);
	initialized = true;
	this.opmode = opmode;
    }
    
    /**
     * Initializes this cipher with a key and a set of algorithm
     * parameters.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameterSpec params)
	throws InvalidKeyException, InvalidAlgorithmParameterException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) &&
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm == null) {
	    // See bug 4341369 & 4334690 for more info.
	    // If the caller is trusted, then okey. 
	    // Otherwise throw a NullPointerException.
	    JceSecurityManager jsm = (JceSecurityManager)
		AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			return new JceSecurityManager();
		    }
		});
	    if (!jsm.isCallerTrusted()) {
		throw new NullPointerException();
	    }
	} else {
	    if (cryptoPerm != CryptoAllPermission.INSTANCE) {
		// Determine keysize and check if it is within legal limits
		int keySize = cipherSpi.engineGetKeySize(key);
	    				  
		if (!(cryptoPerm.implies(new CryptoPermission(
					 key.getAlgorithm(),
					 keySize,
					 params,
					 cryptoPerm.getExemptionMechanism())))) {
		    throw new SecurityException("Unsupported keysize or " +
						"algorithm parameters");
		}
		if (exmech != null) {
		    try {
			if (!exmech.isCryptoAllowed(key)) {
			    throw new SecurityException(exmech.getName() +
							" isn't enforced");
			}
		    } catch (ExemptionMechanismException eme) {
			throw new InvalidKeyException("Cannot determine whether " +
						      exmech.getName() + " has " +
						      "been enforced");
		    }
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, params, JceSecurity.RANDOM);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Initializes this cipher with a key, a set of algorithm
     * parameters, and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameterSpec params,
			   SecureRandom random)
	throws InvalidKeyException, InvalidAlgorithmParameterException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) &&
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm != CryptoAllPermission.INSTANCE) {
	    // Determine keysize and check if it is within legal limits
	    int keySize = cipherSpi.engineGetKeySize(key);
	    if (!(cryptoPerm.implies(new CryptoPermission(
                                         key.getAlgorithm(),
                                         keySize,
					 params,
					 cryptoPerm.getExemptionMechanism())))) {
		throw new SecurityException("Unsupported keysize or " +
                                            "algorithm parameters");
	    }
	    if (exmech != null) {
		try {
		    if (!exmech.isCryptoAllowed(key)) {
			throw new SecurityException(exmech.getName() +
						    " isn't enforced");
		    }
		} catch (ExemptionMechanismException eme) {
		    throw new InvalidKeyException("Cannot determine whether " +
						  exmech.getName() + " has " +
						  "been enforced");
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, params, random);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Initializes this cipher with a key and a set of algorithm
     * parameters.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the {@link SecureRandom <code>SecureRandom</code>}
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following: <code>ENCRYPT_MODE</code>,
     * <code>DECRYPT_MODE</code>, <code>WRAP_MODE</code>
     * or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameters params)
	throws InvalidKeyException, InvalidAlgorithmParameterException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) &&
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm == null) {
	    // See bug 4341369 & 4334690 for more info.
	    // If the caller is trusted, then okey.
	    // Otherwise throw a NullPointerException.
	    JceSecurityManager jsm = (JceSecurityManager)
                  AccessController.doPrivileged(new PrivilegedAction() {
		      public Object run() {
			  return new JceSecurityManager();
		      }
		  });
	    if (!jsm.isCallerTrusted()) {
		throw new NullPointerException();
	    }
	} else {
	    if (cryptoPerm != CryptoAllPermission.INSTANCE) {
		// Determine keysize and check if it is within legal limits
		int keySize = cipherSpi.engineGetKeySize(key);
		AlgorithmParameterSpec pSpec = null;
		try {
		    pSpec = getAlgorithmParameterSpec(params);
		} catch (InvalidParameterSpecException ipse) {
		    throw new InvalidAlgorithmParameterException("Failed to " +
				     "retrieve algorithm parameter specification");
		}

		if (!(cryptoPerm.implies(new CryptoPermission(
					     key.getAlgorithm(),
					     keySize,
					     pSpec,
					     cryptoPerm.getExemptionMechanism())))) {
		
		    throw new SecurityException("Unsupported keysize or " +
						"algorithm parameters");
		}
		if (exmech != null) {
		    try {
			if (!exmech.isCryptoAllowed(key)) {
			    throw new SecurityException(exmech.getName() +
						        " isn't enforced");
			}
		    } catch (ExemptionMechanismException eme) {
			throw new InvalidKeyException("Cannot determine whether " +
						      exmech.getName() + " has " +
						      "been enforced");
		    }
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, params, JceSecurity.RANDOM);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Initializes this cipher with a key, a set of algorithm
     * parameters, and a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If this cipher requires any algorithm parameters and
     * <code>params</code> is null, the underlying cipher implementation is
     * supposed to generate the required parameters itself (using
     * provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidAlgorithmParameterException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following: <code>ENCRYPT_MODE</code>, 
     * <code>DECRYPT_MODE</code>, <code>WRAP_MODE</code>
     * or <code>UNWRAP_MODE</code>)
     * @param key the encryption key
     * @param params the algorithm parameters
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the given key is inappropriate for
     * initializing this cipher, or its keysize exceeds the maximum allowable
     * keysize (as determined from the configured jurisdiction policy files).
     * @exception InvalidAlgorithmParameterException if the given algorithm
     * parameters are inappropriate for this cipher,
     * or this cipher is being initialized for decryption and requires
     * algorithm parameters and <code>params</code> is null, or the given
     * algorithm parameters imply a cryptographic strength that would exceed
     * the legal limits (as determined from the configured jurisdiction
     * policy files).
     */
    public final void init(int opmode, Key key, AlgorithmParameters params,
			   SecureRandom random)
	throws InvalidKeyException, InvalidAlgorithmParameterException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) &&
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	if (cryptoPerm != CryptoAllPermission.INSTANCE) {
	    // Determine keysize and check if it is within legal limits
	    int keySize = cipherSpi.engineGetKeySize(key);
	    AlgorithmParameterSpec pSpec = null;
	    try {
		pSpec = getAlgorithmParameterSpec(params);
	    } catch (InvalidParameterSpecException ipse) {
		throw new InvalidAlgorithmParameterException("Failed to " +
				 "retrieve algorithm parameter specification");
	    }

	    if (!(cryptoPerm.implies(new CryptoPermission(
					 key.getAlgorithm(),
					 keySize,
					 pSpec,
					 cryptoPerm.getExemptionMechanism())))) {
		
		throw new SecurityException("Unsupported keysize or " +
                                            "algorithm parameters");
	    }
	    if (exmech != null) {
		try {
		    if (!exmech.isCryptoAllowed(key)) {
			throw new SecurityException(exmech.getName() +
						    " isn't enforced");
		    }
		} catch (ExemptionMechanismException eme) {
		    throw new InvalidKeyException("Cannot determine whether " +
						  exmech.getName() + " has " +
						  "been enforced");
		}
	    }
	}

	cipherSpi.engineInit(opmode, key, params, random);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Initializes this cipher with the public key from the given certificate.
     * <p> The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping or  key unwrapping, depending
     * on the value of <code>opmode</code>.
     *
     * <p>If the certificate is of type X.509 and has a <i>key usage</i>
     * extension field marked as critical, and the value of the <i>key usage</i>
     * extension field implies that the public key in
     * the certificate and its corresponding private key are not
     * supposed to be used for the operation represented by the value 
     * of <code>opmode</code>,
     * an <code>InvalidKeyException</code>
     * is thrown.
     *
     * <p> If this cipher requires any algorithm parameters that cannot be
     * derived from the public key in the given certificate, the underlying 
     * cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or ramdom values) if it is being
     * initialized for encryption or key wrapping, and raise an <code>
     * InvalidKeyException</code> if it is being initialized for decryption or 
     * key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them using the
     * <code>SecureRandom</code>
     * implementation of the highest-priority
     * installed provider as the source of randomness.
     * (If none of the installed providers supply an implementation of
     * SecureRandom, a system-provided source of randomness will be used.)
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param certificate the certificate
     *
     * @exception InvalidKeyException if the public key in the given
     * certificate is inappropriate for initializing this cipher, or this
     * cipher is being initialized for decryption or unwrapping keys and
     * requires algorithm parameters that cannot be determined from the
     * public key in the given certificate, or the keysize of the public key
     * in the given certificate has a keysize that exceeds the maximum
     * allowable keysize (as determined by the configured jurisdiction policy
     * files).
     */
    public final void init(int opmode, Certificate certificate)
	throws InvalidKeyException
    {
	init(opmode, certificate, JceSecurity.RANDOM);
    }

     /**
     * Initializes this cipher with the public key from the given certificate
     * and 
     * a source of randomness.
     *
     * <p>The cipher is initialized for one of the following four operations:
     * encryption, decryption, key wrapping
     * or key unwrapping, depending on
     * the value of <code>opmode</code>.
     *
     * <p>If the certificate is of type X.509 and has a <i>key usage</i>
     * extension field marked as critical, and the value of the <i>key usage</i>
     * extension field implies that the public key in
     * the certificate and its corresponding private key are not
     * supposed to be used for the operation represented by the value of
     * <code>opmode</code>,
     * an <code>InvalidKeyException</code>
     * is thrown.
     *
     * <p>If this cipher requires any algorithm parameters that cannot be
     * derived from the public key in the given <code>certificate</code>,
     * the underlying cipher
     * implementation is supposed to generate the required parameters itself
     * (using provider-specific default or random values) if it is being
     * initialized for encryption or key wrapping, and raise an
     * <code>InvalidKeyException</code> if it is being
     * initialized for decryption or key unwrapping.
     * The generated parameters can be retrieved using
     * {@link #getParameters() getParameters} or
     * {@link #getIV() getIV} (if the parameter is an IV).
     *
     * <p>If this cipher (including its underlying feedback or padding scheme)
     * requires any random bytes (e.g., for parameter generation), it will get
     * them from <code>random</code>.
     *
     * <p>Note that when a Cipher object is initialized, it loses all 
     * previously-acquired state. In other words, initializing a Cipher is 
     * equivalent to creating a new instance of that Cipher and initializing 
     * it.
     *
     * @param opmode the operation mode of this cipher (this is one of the
     * following:
     * <code>ENCRYPT_MODE</code>, <code>DECRYPT_MODE</code>,
     * <code>WRAP_MODE</code> or <code>UNWRAP_MODE</code>)
     * @param certificate the certificate
     * @param random the source of randomness
     *
     * @exception InvalidKeyException if the public key in the given
     * certificate is inappropriate for initializing this cipher, or this
     * cipher is being initialized for decryption or unwrapping keys and
     * requires algorithm parameters that cannot be determined from the
     * public key in the given certificate, or the keysize of the public key
     * in the given certificate has a keysize that exceeds the maximum
     * allowable keysize (as determined by the configured jurisdiction policy
     * files).
     */
    public final void init(int opmode, Certificate certificate, 
			   SecureRandom random)
	throws InvalidKeyException
    {
	if ((opmode != Cipher.ENCRYPT_MODE) &&
	    (opmode != Cipher.DECRYPT_MODE) &&
	    (opmode != Cipher.WRAP_MODE) &&
	    (opmode != Cipher.UNWRAP_MODE))
	    throw new InvalidParameterException("Invalid operation mode");

	// Check key usage if the certificate is of
	// type X.509.
	if (certificate instanceof java.security.cert.X509Certificate) {
	    // Check whether the cert has a key usage extension
	    // marked as a critical extension.
	    X509Certificate cert = (X509Certificate)certificate;
	    Set critSet = cert.getCriticalExtensionOIDs();

	    if (critSet != null && !critSet.isEmpty()
		&& critSet.contains(KEY_USAGE_EXTENSION_OID)) {
		boolean[] keyUsageInfo = cert.getKeyUsage();
		// keyUsageInfo[2] is for keyEncipherment;
		// keyUsageInfo[3] is for dataEncipherment.
		if ((keyUsageInfo != null) && 
		    (((opmode == Cipher.ENCRYPT_MODE) &&
		      (keyUsageInfo.length > 3) &&
		      (keyUsageInfo[3] == false)) ||
		     ((opmode == Cipher.WRAP_MODE) &&
		      (keyUsageInfo.length > 2) &&
		      (keyUsageInfo[2] == false)))) {
		    throw new InvalidKeyException("Wrong key usage");
		}
	    }
	}

	PublicKey publicKey = certificate.getPublicKey();

	if (cryptoPerm != CryptoAllPermission.INSTANCE) {
	    // Determine keysize and check if it is within legal limits
	    int keySize = cipherSpi.engineGetKeySize(publicKey);
	    if (!(cryptoPerm.implies(new CryptoPermission(
					       publicKey.getAlgorithm(),
					       keySize)))) {
		throw new SecurityException("Unsupported keysize or " +
                                            "algorithm parameters");
	    }
	}

	cipherSpi.engineInit(opmode, publicKey, random);
	initialized = true;
	this.opmode = opmode;
    }

    /**
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The bytes in the <code>input</code> buffer are processed, and the
     * result is stored in a new buffer.
     *
     * <p>If <code>input</code> has a length of zero, this method returns
     * <code>null</code>.
     *
     * @param input the input buffer
     *
     * @return the new buffer with the result, or null if the underlying
     * cipher is a block cipher and the input data is too short to result in a
     * new block.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     */
    public final byte[] update(byte[] input)
	throws IllegalStateException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null) {
	    throw new IllegalArgumentException("Null input buffer");
	}
	if (input.length == 0) {
	    return null;
	}
       
	return cipherSpi.engineUpdate(input, 0, input.length);
    }

    /**
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in a new buffer.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * <code>null</code>.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     *
     * @return the new buffer with the result, or null if the underlying
     * cipher is a block cipher and the input data is too short to result in a
     * new block.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     */
    public final byte[] update(byte[] input, int inputOffset, int inputLen)
	throws IllegalStateException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	if (inputLen == 0) {
	    return null;
	}
	return cipherSpi.engineUpdate(input, inputOffset, inputLen);
    }

    /**
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * a length of zero.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     */
    public final int update(byte[] input, int inputOffset, int inputLen,
			    byte[] output)
	throws IllegalStateException, ShortBufferException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	if (inputLen == 0) {
	    return 0;
	}
	return cipherSpi.engineUpdate(input, inputOffset, inputLen, 
				      output, 0);
    }

    /**
     * Continues a multiple-part encryption or decryption operation
     * (depending on how this cipher was initialized), processing another data
     * part.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, are processed,
     * and the result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>If <code>inputLen</code> is zero, this method returns
     * a length of zero.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     */
    public final int update(byte[] input, int inputOffset, int inputLen,
			    byte[] output, int outputOffset)
	throws IllegalStateException, ShortBufferException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0
	    || outputOffset < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	if (inputLen == 0) {
	    return 0;
	}
	return cipherSpi.engineUpdate(input, inputOffset, inputLen, 
				      output, outputOffset);
    }

    /**
     * Finishes a multiple-part encryption or decryption operation, depending
     * on how this cipher was initialized.
     *
     * <p>Input data that may have been buffered during a previous
     * <code>update</code> operation is processed, with padding (if requested)
     * being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again. 
     * 
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal() 
	throws IllegalStateException, IllegalBlockSizeException, 
	    BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	return cipherSpi.engineDoFinal(null, 0, 0);
    }
    
    /**
     * Finishes a multiple-part encryption or decryption operation, depending
     * on how this cipher was initialized.
     *
     * <p>Input data that may have been buffered during a previous
     * <code>update</code> operation is processed, with padding (if requested)
     * being applied.
     * The result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */ 
    public final int doFinal(byte[] output, int outputOffset) 
	throws IllegalStateException, IllegalBlockSizeException,
	    ShortBufferException, BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if ((output == null) || (outputOffset < 0)) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	return cipherSpi.engineDoFinal(null, 0, 0, output, outputOffset);
    }

    /**
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The bytes in the <code>input</code> buffer, and any input bytes that
     * may have been buffered during a previous <code>update</code> operation,
     * are processed, with padding (if requested) being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param input the input buffer
     *
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal(byte[] input) 
	throws IllegalStateException, IllegalBlockSizeException,
	    BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null) {
	    throw new IllegalArgumentException("Null input buffer");
	}
	return cipherSpi.engineDoFinal(input, 0, input.length);
    }

    /**
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous <code>update</code>
     * operation, are processed, with padding (if requested) being applied.
     * The result is stored in a new buffer.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data. 
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     *
     * @return the new buffer with the result
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final byte[] doFinal(byte[] input, int inputOffset, int inputLen)
	throws IllegalStateException, IllegalBlockSizeException,
	    BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	return cipherSpi.engineDoFinal(input, inputOffset, inputLen);
    }

    /**
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous <code>update</code>
     * operation, are processed, with padding (if requested) being applied.
     * The result is stored in the <code>output</code> buffer.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data.
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final int doFinal(byte[] input, int inputOffset, int inputLen,
			     byte[] output)
	throws IllegalStateException, ShortBufferException,
	       IllegalBlockSizeException, BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	return cipherSpi.engineDoFinal(input, inputOffset, inputLen, 
				       output, 0);
    }

    /**
     * Encrypts or decrypts data in a single-part operation, or finishes a
     * multiple-part operation. The data is encrypted or decrypted,
     * depending on how this cipher was initialized.
     *
     * <p>The first <code>inputLen</code> bytes in the <code>input</code>
     * buffer, starting at <code>inputOffset</code> inclusive, and any input
     * bytes that may have been buffered during a previous
     * <code>update</code> operation, are processed, with padding
     * (if requested) being applied.
     * The result is stored in the <code>output</code> buffer, starting at
     * <code>outputOffset</code> inclusive.
     *
     * <p>If the <code>output</code> buffer is too small to hold the result,
     * a <code>ShortBufferException</code> is thrown. In this case, repeat this
     * call with a larger output buffer. Use 
     * {@link #getOutputSize(int) getOutputSize} to determine how big
     * the output buffer should be.
     *
     * <p>Upon finishing, this method resets this cipher object to the state 
     * it was in when previously initialized via a call to <code>init</code>.
     * That is, the object is reset and available to encrypt or decrypt
     * (depending on the operation mode that was specified in the call to
     * <code>init</code>) more data.
     *
     * <p>Note: if any exception is thrown, this cipher object may need to
     * be reset before it can be used again.
     *
     * <p>Note: this method should be copy-safe, which means the
     * <code>input</code> and <code>output</code> buffers can reference
     * the same byte array and no unprocessed input data is overwritten
     * when the result is copied into the output buffer.
     *
     * @param input the input buffer
     * @param inputOffset the offset in <code>input</code> where the input
     * starts
     * @param inputLen the input length
     * @param output the buffer for the result
     * @param outputOffset the offset in <code>output</code> where the result
     * is stored
     *
     * @return the number of bytes stored in <code>output</code>
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized)
     * @exception IllegalBlockSizeException if this cipher is a block cipher,
     * no padding has been requested (only in encryption mode), and the total
     * input length of the data processed by this cipher is not a multiple of
     * block size
     * @exception ShortBufferException if the given output buffer is too small
     * to hold the result
     * @exception BadPaddingException if this cipher is in decryption mode,
     * and (un)padding has been requested, but the decrypted data is not
     * bounded by the appropriate padding bytes
     */
    public final int doFinal(byte[] input, int inputOffset, int inputLen,
			     byte[] output, int outputOffset)
	throws IllegalStateException, ShortBufferException,
				 IllegalBlockSizeException, BadPaddingException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	    throw new IllegalStateException("Cipher not initialized");
	}
	// Input sanity check
	if (input == null || inputOffset < 0
	    || inputLen > (input.length - inputOffset) || inputLen < 0
	    || outputOffset < 0) {
	    throw new IllegalArgumentException("Bad arguments");
	}
	return cipherSpi.engineDoFinal(input, inputOffset, inputLen, 
				       output, outputOffset);
    }

    /**
     * Wrap a key.
     *
     * @param key the key to be wrapped.
     *
     * @return the wrapped key.
     *
     * @exception IllegalStateException if this cipher is in a wrong
     * state (e.g., has not been initialized).
     *
     * @exception IllegalBlockSizeException if this cipher is a block 
     * cipher, no padding has been requested, and the length of the 
     * encoding of the key to be wrapped is not a
     * multiple of the block size.
     *
     * @exception InvalidKeyException if it is impossible or unsafe to
     * wrap the key with this cipher (e.g., a hardware protected key is 
     * being passed to a software-only cipher).
     */
     public final byte[] wrap(Key key)
	 throws IllegalStateException, IllegalBlockSizeException,
	        InvalidKeyException
     {
	 if (!initialized && !(this instanceof NullCipher)) {
	     throw new IllegalStateException("Cipher not initialized");
	 }
	 if (opmode != Cipher.WRAP_MODE) {
	     throw new IllegalStateException("Cipher not initialized for " +
					     "wrapping keys");
	 }

	 return cipherSpi.engineWrap(key);
     }

    /**
     * Unwrap a previously wrapped key.
     *
     * @param wrappedKey the key to be unwrapped.
     *
     * @param wrappedKeyAlgorithm the algorithm associated with the wrapped
     * key.
     *
     * @param wrappedKeyType the type of the wrapped key. This must be one of
     * <code>SECRET_KEY</code>, <code>PRIVATE_KEY</code>, or
     * <code>PUBLIC_KEY</code>.
     *
     * @return the unwrapped key.
     *
     * @exception IllegalStateException if this cipher is in a wrong state
     * (e.g., has not been initialized).
     *
     * @exception NoSuchAlgorithmException if no installed providers
     * can create keys of type <code>wrappedKeyType</code> for the
     * <code>wrappedKeyAlgorithm</code>.
     *
     * @exception InvalidKeyException if <code>wrappedKey</code> does not
     * represent a wrapped key of type <code>wrappedKeyType</code> for
     * the <code>wrappedKeyAlgorithm</code>.
     */
    public final Key unwrap(byte[] wrappedKey,
			    String wrappedKeyAlgorithm,
			    int wrappedKeyType)
	throws IllegalStateException, InvalidKeyException,
	       NoSuchAlgorithmException
    {
	if (!initialized && !(this instanceof NullCipher)) {
	     throw new IllegalStateException("Cipher not initialized");
	}
	if (opmode != Cipher.UNWRAP_MODE) {
	    throw new IllegalStateException("Cipher not initialized for " +
	       			            "unwrapping keys");
	}
	if ((wrappedKeyType != SECRET_KEY) &&
	    (wrappedKeyType != PRIVATE_KEY) &&
	    (wrappedKeyType != PUBLIC_KEY)) {
	    throw new InvalidParameterException("Invalid key type");
	}
	return cipherSpi.engineUnwrap(wrappedKey,
				      wrappedKeyAlgorithm,
				      wrappedKeyType);
    }

    private AlgorithmParameterSpec getAlgorithmParameterSpec(
                                      AlgorithmParameters params)
        throws InvalidParameterSpecException {
	if (params == null) {
	    return null;
	}

	String alg = params.getAlgorithm().toUpperCase();

	if (alg.equalsIgnoreCase("RC2")) {
	    return params.getParameterSpec(RC2ParameterSpec.class);
	}

	if (alg.equalsIgnoreCase("RC5")) {
	    return params.getParameterSpec(RC5ParameterSpec.class);
	}

	if (alg.startsWith("PBE")) {
	    return params.getParameterSpec(PBEParameterSpec.class);
	}

	if (alg.startsWith("DES")) {
	    return params.getParameterSpec(IvParameterSpec.class);
	}

	return null;
    }
	    
}
