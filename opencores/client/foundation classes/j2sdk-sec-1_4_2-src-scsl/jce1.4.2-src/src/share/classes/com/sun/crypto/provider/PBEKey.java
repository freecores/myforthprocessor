/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.crypto.provider;

import java.io.UnsupportedEncodingException;
import java.security.spec.InvalidKeySpecException;
import javax.crypto.SecretKey;
import javax.crypto.spec.PBEKeySpec;

/**
 * This class represents a PBE key.
 *
 * @author Jan Luehe
 *
 * @version 1.17, 06/24/03
 */
final class PBEKey implements SecretKey {

    static final long serialVersionUID = -2234768909660948176L;

    private byte[] key;

    /**
     * Creates a PBE key from a given PBE key specification.
     *
     * @param key the given PBE key specification
     */
    PBEKey(PBEKeySpec keySpec) throws InvalidKeySpecException {
	char[] passwd = keySpec.getPassword();
	if (passwd == null) {
	    // Should allow an empty password.
	    passwd = new char[0];
	}
	for (int i=0; i<passwd.length; i++) {
	    if ((passwd[i] < '\u0020') || (passwd[i] > '\u007E')) {
		throw new InvalidKeySpecException("Password is not ASCII");
	    }
	}
	this.key = new byte[passwd.length];
	for (int i=0; i<passwd.length; i++)
	    this.key[i] = (byte) (passwd[i] & 0x7f);
	java.util.Arrays.fill(passwd, ' ');
    }

    public byte[] getEncoded() {
        return (byte[])this.key.clone();
    }

    public String getAlgorithm() {
	return "PBEWithMD5AndDES";
    }
	    
    public String getFormat() {
	return "RAW";
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
        return(retval ^= getAlgorithm().toLowerCase().hashCode());
    }

    public boolean equals(Object obj) {
	if (obj == this)
	    return true;

	if (!(obj instanceof SecretKey))
	    return false;

	SecretKey that = (SecretKey)obj;

	if (!(that.getAlgorithm().equalsIgnoreCase("PBEWithMD5AndDES")))
	    return false;

	byte[] thatEncoded = that.getEncoded();
	boolean ret = java.util.Arrays.equals(this.key, thatEncoded);
	java.util.Arrays.fill(thatEncoded, (byte)0x00);
	return ret;
    }

    /**
     * readObject is called to restore the state of this key from
     * a stream.
     */
    private void readObject(java.io.ObjectInputStream s)
         throws java.io.IOException, ClassNotFoundException
    {
        s.defaultReadObject();
        key = (byte[])key.clone();
    }

    /**
     * Ensures that the password bytes of this key are
     * set to zero when there are no more references to it. 
     */
    protected void finalize() throws Throwable {
	try {
            if (this.key != null) {
                java.util.Arrays.fill(this.key, (byte)0x00);
                this.key = null;
            }
	} finally {
	    super.finalize();
	}
    }
}

