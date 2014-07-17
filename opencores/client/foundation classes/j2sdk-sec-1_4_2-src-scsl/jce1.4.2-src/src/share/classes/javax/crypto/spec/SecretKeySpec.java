/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.spec;

import java.io.UnsupportedEncodingException;
import java.security.Key;
import java.security.spec.KeySpec;
import javax.crypto.SecretKey;

/**
 * This class specifies a secret key in a provider-independent fashion.
 *
 * <p>It can be used to construct a <code>SecretKey</code> from a byte array,
 * without having to go through a (provider-based)
 * <code>SecretKeyFactory</code>.
 *
 * <p>This class is only useful for raw secret keys that can be represented as
 * a byte array and have no key parameters associated with them, e.g., DES or
 * Triple DES keys.
 *
 * @author Jan Luehe
 *
 * @version 1.19, 06/24/03
 *
 * @see javax.crypto.SecretKey
 * @see javax.crypto.SecretKeyFactory
 * @since 1.4
 */
public class SecretKeySpec implements KeySpec, SecretKey {

    /**
     * The secret key.
     *
     * @serial
     */
    private byte[] key;

    /**
     * The name of the algorithm associated with this key.
     *
     * @serial
     */
    private String algorithm;

    /**
     * Constructs a secret key from the given byte array.
     *
     * <p>This constructor does not check if the given bytes indeed specify a
     * secret key of the specified algorithm. For example, if the algorithm is
     * DES, this constructor does not check if <code>key</code> is 8 bytes
     * long, and also does not check for weak or semi-weak keys.
     * In order for those checks to be performed, an algorithm-specific
     * <i>key specification</i> class (in this case:
     * {@link DESKeySpec DESKeySpec})
     * should be used.
     *
     * @param key the key material of the secret key.
     * @param algorithm the name of the secret-key algorithm to be associated
     * with the given key material.
     * See Appendix A in the <a href="../../../../guide/security/jce/JCERefGuide.html">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     */
    public SecretKeySpec(byte[] key, String algorithm) {
	if (key == null || algorithm == null) {
	    throw new IllegalArgumentException("Missing argument");
	}
	if (key.length == 0) {
	    throw new IllegalArgumentException("Empty key");
	}
	this.key = (byte[])key.clone();
	this.algorithm = algorithm;
    }       

    /**
     * Constructs a secret key from the given byte array, using the first
     * <code>len</code> bytes of <code>key</code>, starting at
     * <code>offset</code> inclusive.
     *
     * <p> The bytes that constitute the secret key are
     * those between <code>key[offset]</code> and
     * <code>key[offset+len-1]</code> inclusive.
     *
     * <p>This constructor does not check if the given bytes indeed specify a
     * secret key of the specified algorithm. For example, if the algorithm is
     * DES, this constructor does not check if <code>key</code> is 8 bytes
     * long, and also does not check for weak or semi-weak keys.
     * In order for those checks to be performed, an algorithm-specific key
     * specification class (in this case:
     * {@link DESKeySpec DESKeySpec})
     * must be used.
     *
     * @param key the key material of the secret key.
     * @param offset the offset in <code>key</code> where the key material
     * starts.
     * @param len the length of the key material.
     * @param algorithm the name of the secret-key algorithm to be associated
     * with the given key material.
     * See Appendix A in the <a href="../../../../guide/security/jce/JCERefGuide.html">
     * Java Cryptography Extension Reference Guide</a> 
     * for information about standard algorithm names.
     */    
    public SecretKeySpec(byte[] key, int offset, int len, String algorithm) {
	if (key == null || algorithm == null) {
	    throw new IllegalArgumentException("Missing argument");
	}
	if (key.length == 0) {
	    throw new IllegalArgumentException("Empty key");
	}
	if (key.length-offset < len) {
	    throw new IllegalArgumentException
		("Invalid offset/length combination");
	}
	this.key = new byte[len];
	System.arraycopy(key, offset, this.key, 0, len);
	this.algorithm = algorithm;
    }

    /**
     * Returns the name of the algorithm associated with this secret key.
     *
     * @return the secret key algorithm.
     */
    public String getAlgorithm() {
	return this.algorithm;
    }
    
    /**
     * Returns the name of the encoding format for this secret key.
     *
     * @return the string "RAW".
     */
    public String getFormat() {
	return "RAW";
    }

    /**
     * Returns the key material of this secret key.
     *
     * @return the key material
     */
    public byte[] getEncoded() {
	return (byte[])this.key.clone();
    }

    /**
     * Calculates a hash code value for the object.
     * Objects that are equal will also have the same hashcode.
     */
    public int hashCode() {
        int retval = 0;
        for (int i = 1; i < this.key.length; i++) {
            retval += this.key[i] * i;
        }
	if (this.algorithm.equalsIgnoreCase("TripleDES"))
	    return (retval ^= "desede".hashCode());
	else
	    return (retval ^= this.algorithm.toLowerCase().hashCode());
    }

    public boolean equals(Object obj) {
        if (this == obj)
            return true;

        if (!(obj instanceof SecretKey))
            return false;

	String thatAlg = ((SecretKey)obj).getAlgorithm();
	if (!(thatAlg.equalsIgnoreCase(this.algorithm))) {
	    if ((!(thatAlg.equalsIgnoreCase("DESede"))
		 || !(this.algorithm.equalsIgnoreCase("TripleDES")))
		&& (!(thatAlg.equalsIgnoreCase("TripleDES"))
		    || !(this.algorithm.equalsIgnoreCase("DESede"))))
	    return false;
	}

	byte[] thatKey = ((SecretKey)obj).getEncoded();	

	return java.util.Arrays.equals(this.key, thatKey);
    }
}
