/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.io.*;
import java.security.AlgorithmParameters;
import java.security.NoSuchAlgorithmException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.security.spec.InvalidKeySpecException;
import sun.security.x509.AlgorithmId;
import sun.security.util.DerValue;
import sun.security.util.DerOutputStream;

/**
 * This class implements the <code>EncryptedPrivateKeyInfo</code> type
 * as defined in PKCS #8.
 * <p>Its ASN.1 definition is as follows:
 *
 * <pre>
 * EncryptedPrivateKeyInfo ::=  SEQUENCE {
 *     encryptionAlgorithm   AlgorithmIdentifier,
 *     encryptedData   OCTET STRING }
 * 
 * AlgorithmIdentifier  ::=  SEQUENCE  {
 *     algorithm              OBJECT IDENTIFIER,
 *     parameters             ANY DEFINED BY algorithm OPTIONAL  }
 * </pre>
 *
 * @author Valerie Peng
 *
 * @version 1.5, 03/06/24
 * 
 * @see java.security.spec.PKCS8EncodedKeySpec
 *
 * @since 1.4
 */

public class EncryptedPrivateKeyInfo {

    // the "encryptionAlgorithm" field
    private AlgorithmId algid;

    // the "encryptedData" field
    private byte[] encryptedData;

    // the ASN.1 encoded contents of this class
    private byte[] encoded = null;

    /**
     * Constructs (i.e., parses) an <code>EncryptedPrivateKeyInfo</code> from
     * its ASN.1 encoding.
     * @param encoded the ASN.1 encoding of this object.
     * @exception NullPointerException if the <code>encoded</code> is null.
     * @exception IOException if error occurs when parsing the ASN.1 encoding.
     */
    public EncryptedPrivateKeyInfo(byte[] encoded)
	throws IOException {
	if (encoded == null) {
	    throw new NullPointerException("the encoded parameter " + 
					   "must be non-null");
	}

	DerValue val = new DerValue(encoded);

        DerValue[] seq = new DerValue[2];

        seq[0] = val.data.getDerValue();
        seq[1] = val.data.getDerValue();

        if (val.data.available() != 0) {
            throw new IOException("overrun, bytes = " + val.data.available());
        }

        this.algid = AlgorithmId.parse(seq[0]);
        if (seq[0].data.available() != 0) {
            throw new IOException("encryptionAlgorithm field overrun");
        }

        this.encryptedData = seq[1].getOctetString();
        if (seq[1].data.available() != 0) {
            throw new IOException("encryptedData field overrun");
	}
	this.encoded = (byte[])encoded.clone();
    }

    /**
     * Constructs an <code>EncryptedPrivateKeyInfo</code> from the
     * encryption algorithm name and the encrypted data.
     * <p>Note: the <code>encrypedData</code> is cloned when constructing
     * this object.
     *
     * If encryption algorithm has associated parameters use the constructor
     * with AlgorithmParameters as the parameter.
     *
     * @param algName algorithm name.
     * @param encryptedData encrypted data.
     * @exception NullPointerException if <code>algName</code> or
     * <code>encryptedData</code> is null.
     * @exception IllegalArgumentException if <code>encryptedData</code>
     * is empty, i.e. 0-length.
     * @exception NoSuchAlgorithmException if the specified algName is
     * not supported.
     */
    public EncryptedPrivateKeyInfo(String algName, byte[] encryptedData) 
	throws NoSuchAlgorithmException {

	if (algName == null)
		throw new NullPointerException("the algName parameter " + 
					       "must be non-null");
	this.algid = AlgorithmId.get(algName);

	if (encryptedData == null) {
	    throw new NullPointerException("the encryptedData " + 
					   "parameter must be non-null");
	} else if (encryptedData.length == 0) {
	    throw new IllegalArgumentException("the encryptedData " +
						"parameter must not be empty");
	} else {
	    this.encryptedData = (byte[])encryptedData.clone();
	}
	// delay the generation of ASN.1 encoding until 
	// getEncoded() is called
	this.encoded = null;
    }

    /**
     * Constructs an <code>EncryptedPrivateKeyInfo</code> from the
     * encryption algorithm parameters and the encrypted data.
     * <p>Note: the <code>encrypedData</code> is cloned when constructing
     * this object.
     *
     * @param algParams the algorithm parameters for the encryption 
     * algorithm. <code>algParams.getEncoded()</code> should return
     * the ASN.1 encoded bytes of the <code>parameters</code> field
     * of the <code>AlgorithmIdentifer</code> component of the
     * <code>EncryptedPrivateKeyInfo</code> type. 
     * @param encryptedData encrypted data.
     * @exception NullPointerException if <code>algParams</code> or 
     * <code>encryptedData</code> is null.
     * @exception IllegalArgumentException if <code>encryptedData</code>
     * is empty, i.e. 0-length.
     * @exception NoSuchAlgorithmException if the specified algName of
     * the specified <code>algParams</code> parameter is not supported.
     */
    public EncryptedPrivateKeyInfo(AlgorithmParameters algParams,
        byte[] encryptedData) throws NoSuchAlgorithmException {

	if (algParams == null) {
	    throw new NullPointerException("algParams must be non-null");
        }
	this.algid = AlgorithmId.get(algParams);

	if (encryptedData == null) {
	    throw new NullPointerException("encryptedData must be non-null");
	} else if (encryptedData.length == 0) {
	    throw new IllegalArgumentException("the encryptedData " +
						"parameter must not be empty");
	} else {
	    this.encryptedData = (byte[])encryptedData.clone();
	}

	// delay the generation of ASN.1 encoding until 
	// getEncoded() is called
	this.encoded = null;
    }


    /**
     * Returns the encryption algorithm.
     * @return the algorithm name.	
     */
    public String getAlgName() {
	return this.algid.getName();
    }

    /**
     * Returns the algorithm parameters used by the encryption algorithm.
     * @return the algorithm parameters.
     */   
    public AlgorithmParameters getAlgParameters() {
        return this.algid.getParameters();
    }       

    /**
     * Returns a copy of the encrypted data.
     * @return a copy of the encrypted data.
     */
    public byte[] getEncryptedData() {
	return (byte[])this.encryptedData.clone();
    }

    /**
     * Extract the enclosed PKCS8EncodedKeySpec object from the 
     * encrypted data and return it.
     * @return the PKCS8EncodedKeySpec object. 
     * @exception InvalidKeySpecException if the given cipher is 
     * inappropriate for the encrypted data or the encrypted
     * data is corrupted and cannot be decrypted.
     */
    public PKCS8EncodedKeySpec getKeySpec(Cipher c) 
	throws InvalidKeySpecException { 
	byte[] encoded = null;
	try {
	    encoded = c.doFinal(encryptedData);
	} catch (IllegalBlockSizeException ibse) {
	    throw new InvalidKeySpecException(ibse.toString());
	} catch (BadPaddingException bpe) {
	    throw new InvalidKeySpecException(bpe.toString());
	}
        return new PKCS8EncodedKeySpec(encoded);
    }

    /**
     * Returns the ASN.1 encoding of this object.
     * @return the ASN.1 encoding.
     * @exception IOException if error occurs when constructing its
     * ASN.1 encoding.
     */
    public byte[] getEncoded() throws IOException {
	if (this.encoded != null) return (byte[])this.encoded.clone();

	DerOutputStream out = new DerOutputStream();
	DerOutputStream tmp = new DerOutputStream();

	// encode encryption algorithm
	algid.encode(tmp);

	// encode encrypted data
	tmp.putOctetString(encryptedData);

	// wrap everything into a SEQUENCE
	out.write(DerValue.tag_Sequence, tmp);
	this.encoded = out.toByteArray();

	return (byte[])this.encoded.clone();
    }

}
