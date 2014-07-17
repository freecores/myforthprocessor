/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

/*
 * @(#)EncryptionKey.java	1.13 03/06/24
 *
 * Portions Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 * 
 * ===========================================================================
 *  IBM Confidential
 *  OCO Source Materials
 *  Licensed Materials - Property of IBM
 * 
 *  (C) Copyright IBM Corp. 1999 All Rights Reserved.
 * 
 *  The source code for this program is not published or otherwise divested of
 *  its trade secrets, irrespective of what has been deposited with the U.S.
 *  Copyright Office.
 * 
 *  Copyright 1997 The Open Group Research Institute.  All rights reserved.
 * ===========================================================================
 * 
 */

package sun.security.krb5;

import sun.security.util.*;
import sun.security.krb5.internal.*;
import sun.security.krb5.internal.crypto.*;
import java.io.IOException;
import sun.security.krb5.internal.ktab.KeyTab;
import sun.security.krb5.internal.ccache.CCacheOutputStream;

/**
 * This class encapsulates the concept of an EncryptionKey. An encryption
 * key is defined in RFC 1510 as:
 *          EncryptionKey ::=   SEQUENCE {
 *                              keytype[0]    INTEGER,
 *                              keyvalue[1]   OCTET STRING
 *          }
 *
 *    keytype   This field specifies the type of encryption key that
 *             follows in the keyvalue field.  It will almost always
 *             correspond to the encryption algorithm used to generate the
 *             EncryptedData, though more than one algorithm may use the
 *             same type of key (the mapping is many to one).  This might
 *             happen, for example, if the encryption algorithm uses an
 *             alternate checksum algorithm for an integrity check, or a
 *             different chaining mechanism.
 */
public class EncryptionKey 
    implements Cloneable {

    public static EncryptionKey NULL_KEY = new EncryptionKey(new byte[] {});

    private int keyType;
    private byte[] keyValue;
    private Integer kvno; // not part of ASN1 encoding; may be missing
    
    private static int KEYTYPE_DEFAULT;
    private static boolean DEBUG = Krb5.DEBUG; 

    /*
     * Usually, when keyType is decoded from ASN.1 it will contain a
     * value indicating what the algorithm to be used is. However, when
     * converting from a password to a key for the AS-EXCHANGE, this
     * keyType will not be available. Use
     * EncryptedData.ETYPE_DES_CBC_MD5 as the default in that case. If 
     * default_tkt_enctypes was set in the libdefaults of krb5.conf,
     * then use the first of that sequence.
     */
    static {
        try {
	    KEYTYPE_DEFAULT =
		Config.getInstance().defaultEtype("default_tkt_enctypes")[0];
	} catch (KrbException e) {
	    if (DEBUG) {
	        System.out.println("Exception in getting default ticket " +
				   "enctypes from the configuration " + 
				   "Setting default to be DES_CBC_MD5");
		e.printStackTrace();
	    }
            KEYTYPE_DEFAULT = EncryptedData.ETYPE_DES_CBC_MD5;
        }
    }

    public synchronized int getEType() {
	return keyType;
    }
    
    public final Integer getKeyVersionNumber() {
	return kvno;
    }
    
    /**
     * Returns the raw key bytes, not in any ASN.1 encoding.
     */
    public final byte[] getBytes() {
	// This method cannot be called outside sun.security, hence no
	// cloning. getEncoded() calls this method.
	return keyValue;
    }
    
    public synchronized Object clone() {
	return new EncryptionKey(keyValue, keyType, kvno);
    }
    
    /**
     * Obtains the latest version of the secret key of 
     * the principal from a keytab. 
     *
     * @param princ the principal whose secret key is desired
     * @param keytab the path to the keytab file. A value of null
     * will be accepted to indicate that the default path should be 
     * searched.
     * @returns the secret key or null if none was found.
     */
    public static EncryptionKey acquireSecretKey(PrincipalName princ,
						 String keytab) 
	throws KrbException, IOException {

	if (princ == null) {
	    throw new IllegalArgumentException(
					       "Cannot have null pricipal name to look in keytab.");
	}

	KeyTab ktab = KeyTab.getInstance(keytab);

	if (ktab == null)
	    return null;

	return ktab.readServiceKey(princ);
    }

    /**
     * Obtains all versions of the secret key of the principal from a
     * keytab.
     *
     * @Param princ the principal whose secret key is desired
     * @param keytab the path to the keytab file. A value of null
     * will be accepted to indicate that the default path should be 
     * searched.
     * @returns an array of secret keys or null if none were found.
     */
    public static EncryptionKey[] acquireSecretKeys(PrincipalName princ,
						    String keytab) 
	throws KrbException, IOException {

	    if (princ == null)
	    throw new IllegalArgumentException(
					       "Cannot have null pricipal name to look in keytab.");

	    KeyTab ktab = KeyTab.getInstance(keytab);

	    if (ktab == null)
	    return null;

	    return ktab.readServiceKeys(princ);
	}

    public EncryptionKey(byte[] keyValue,
			 int keyType,
			 Integer kvno) {
	if (keyValue != null) {
	    this.keyValue = new byte[keyValue.length];
	    System.arraycopy(keyValue, 0, this.keyValue, 0, keyValue.length);
	} else
	    throw new IllegalArgumentException("EncryptionKey: " +
					       "Key bytes cannot be null!");
	this.keyType = keyType;
	this.kvno = kvno;
    }
    
    /**
     * Constructs an EncryptionKey by using the specified key type and key
     * value.  It is used to recover the key when retrieving data from
     * credential cache file. 
     * 
     */
    public EncryptionKey(int keyType,
			 byte[] keyValue) {
	this(keyValue, keyType, null);
    }
    
    public EncryptionKey(byte[] keyValue) {
	this(keyValue, KEYTYPE_DEFAULT, null);
    }
    
    public EncryptionKey(StringBuffer password) throws KrbCryptoException {
	this(Des.string_to_key_bytes(password));
    }
    
    public EncryptionKey(StringBuffer password,
			 String salt) throws KrbCryptoException {
	this(Des.string_to_key_bytes(password.append(salt)));
    }

    public EncryptionKey(StringBuffer password,
			 String salt,
			 String algorithm) throws KrbCryptoException {
	// Currently we support only DES 
	if (algorithm != null && !algorithm.equalsIgnoreCase("DES")) {
	    throw new IllegalArgumentException ("Algorithm " + algorithm + 
						" not supported");
	}
	this.keyValue = Des.string_to_key_bytes(password.append(salt));
	this.keyType = KEYTYPE_DEFAULT;
	this.kvno = null;
    }
    
    
    /**
     * Generates a sub-sessionkey from a given session key.
     */
    public EncryptionKey(EncryptionKey key) throws KrbCryptoException {
	keyValue = (byte[]) key.keyValue.clone();
	keyType = key.keyType;
	/*
	  // TBD: Need to add code to check parity and for weak key
	  // As for now simply use the same key
	  keyValue = Confounder.bytes(key.keyValue.length);
	  for (int i = 0; i < keyValue.length; i++) {
	  keyValue[i] ^= key.keyValue[i];
	  }
	  keyType = key.keyType;
	*/
    }
    
    /**
     * Constructs an instance of EncryptionKey type.
     * @param encoding a single DER-encoded value.
     * @exception Asn1Exception if an error occurs while decoding an ASN1
     * encoded data. 
     * @exception IOException if an I/O error occurs while reading encoded
     * data. 
     *
     *
     */
    public EncryptionKey(DerValue encoding) throws Asn1Exception, IOException {
        DerValue der;
	if (encoding.getTag() != DerValue.tag_Sequence) {
            throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
	der = encoding.getData().getDerValue();
	if ((der.getTag() & (byte)0x1F) == (byte)0x00) {
	    keyType = der.getData().getBigInteger().intValue();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
        der = encoding.getData().getDerValue();		
	if ((der.getTag() & (byte)0x1F) == (byte)0x01) {
	    keyValue = der.getData().getOctetString();
	}
	else
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	if (der.getData().available() > 0) {
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	}
    }
    
    /**
     * Returns the ASN.1 encoding of this EncryptionKey.
     *
     * <xmp>
     * EncryptionKey ::=   SEQUENCE {
     *                             keytype[0]    INTEGER,
     *                             keyvalue[1]   OCTET STRING }
     * </xmp>
     *
     * <p>
     * This definition reflects the Network Working Group RFC 1510
     * specifications dated September 1993 and available at
     * <a href="http://www.ietf.org/rfc/rfc1510.txt">
     * http://www.ietf.org/rfc/rfc1510.txt</a>.
     *
     * @return byte array of encoded EncryptionKey object.
     * @exception Asn1Exception if an error occurs while decoding an ASN1
     * encoded data. 
     * @exception IOException if an I/O error occurs while reading encoded
     * data.
     *
     */
    public synchronized byte[] asn1Encode() throws Asn1Exception, IOException {
        DerOutputStream bytes = new DerOutputStream();
	DerOutputStream temp = new DerOutputStream();
	temp.putInteger(keyType);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, 
				       (byte)0x00), temp);
	temp = new DerOutputStream();
        temp.putOctetString(keyValue);
	bytes.write(DerValue.createTag(DerValue.TAG_CONTEXT, true, 
				       (byte)0x01), temp);
	temp = new DerOutputStream();
	temp.write(DerValue.tag_Sequence, bytes);
	return temp.toByteArray();
    }
    
    public synchronized void destroy() {
	if (keyValue != null)
	    for (int i = 0; i < keyValue.length; i++)
		keyValue[i] = 0;
    }
    
    
    /**
     * Parse (unmarshal) an Encryption key from a DER input stream.  This form
     * parsing might be used when expanding a value which is part of
     * a constructed sequence and uses explicitly tagged type.
     *
     * @param data the Der input stream value, which contains one or more
     * marshaled value.
     * @param explicitTag tag number.
     * @param optional indicate if this data field is optional
     * @exception Asn1Exception if an error occurs while decoding an ASN1
     * encoded data.
     * @exception IOException if an I/O error occurs while reading encoded
     * data.
     * @return an instance of EncryptionKey.
     *
     */
    public static EncryptionKey parse(DerInputStream data, byte
				      explicitTag, boolean optional) throws
				      Asn1Exception, IOException { 
	if ((optional) && (((byte)data.peekByte() & (byte)0x1F) !=
			   explicitTag)) {		
	    return null;
	}
	DerValue der = data.getDerValue();
	if (explicitTag != (der.getTag() & (byte)0x1F))  {	
	    throw new Asn1Exception(Krb5.ASN1_BAD_ID);
	} else {
	    DerValue subDer = der.getData().getDerValue();
	    return new EncryptionKey(subDer);
	}
    }
    
    /**
     * Writes key value in FCC format to a <code>CCacheOutputStream</code>.
     *
     * @param cos a <code>CCacheOutputStream</code> to be written to.
     * @exception IOException if an I/O exception occurs.
     * @see sun.security.krb5.internal.ccache.CCacheOutputStream
     *  
     */
    public synchronized void writeKey(CCacheOutputStream cos) throws IOException {
	cos.write16(keyType);
	// we use KRB5_FCC_FVNO_3	
	cos.write16(keyType); // key type is recorded twice.
	cos.write32(keyValue.length);
	for (int i = 0; i < keyValue.length; i++) {
	    cos.write8(keyValue[i]);
	}
    }    

    public String toString() {
	return new String("EncryptionKey: keyType=" + keyType
			  + " kvno=" + kvno
			  + " keyValue (hex dump)="
			  + (keyValue == null || keyValue.length == 0 ?
			     " Empty Key" : '\n' + Krb5.hexDumper.encode(keyValue)
			     + '\n'));
    }
}
