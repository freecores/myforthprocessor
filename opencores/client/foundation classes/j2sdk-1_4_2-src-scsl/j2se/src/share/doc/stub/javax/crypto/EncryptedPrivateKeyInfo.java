/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)EncryptedPrivateKeyInfo.java	1.7 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
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
 * @version 1.3, 01/09/25
 * 
 * @see java.security.spec.PKCS8EncodedKeySpec
 *
 * @since 1.4
 */
public class EncryptedPrivateKeyInfo
{

    /** 
     * Constructs (i.e., parses) an <code>EncryptedPrivateKeyInfo</code> from
     * its ASN.1 encoding.
     * @param encoded the ASN.1 encoding of this object.
     * @exception NullPointerException if the <code>encoded</code> is null.
     * @exception IOException if error occurs when parsing the ASN.1 encoding.
     */
    public EncryptedPrivateKeyInfo(byte[] encoded) throws IOException { }

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
        throws NoSuchAlgorithmException
    { }

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
    public EncryptedPrivateKeyInfo(AlgorithmParameters algParams, byte[]
        encryptedData) throws NoSuchAlgorithmException
    { }

    /** 
     * Returns the encryption algorithm.
     * @return the algorithm name.	
     */
    public String getAlgName() { }

    /** 
     * Returns the algorithm parameters used by the encryption algorithm.
     * @return the algorithm parameters.
     */
    public AlgorithmParameters getAlgParameters() { }

    /** 
     * Returns a copy of the encrypted data.
     * @return a copy of the encrypted data.
     */
    public byte[] getEncryptedData() { }

    /** 
     * Extract the enclosed PKCS8EncodedKeySpec object from the 
     * encrypted data and return it.
     * @return the PKCS8EncodedKeySpec object. 
     * @exception InvalidKeySpecException if the given cipher is 
     * inappropriate for the encrypted data or the encrypted
     * data is corrupted and cannot be decrypted.
     */
    public PKCS8EncodedKeySpec getKeySpec(Cipher c)
        throws InvalidKeySpecException
    { }

    /** 
     * Returns the ASN.1 encoding of this object.
     * @return the ASN.1 encoding.
     * @exception IOException if error occurs when constructing its
     * ASN.1 encoding.
     */
    public byte[] getEncoded() throws IOException { }
}
