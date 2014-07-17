/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.security.DigestInputStream;
import java.security.DigestOutputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.PrivateKey;
import java.security.KeyStoreSpi;
import java.security.KeyStoreException;
import java.security.UnrecoverableKeyException;
import java.security.SecureRandom;
import java.security.cert.Certificate;
import java.security.cert.CertificateFactory;
import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;
import java.security.spec.PKCS8EncodedKeySpec;
import java.util.*;
import java.math.*;

import javax.security.auth.x500.X500Principal;

import sun.security.util.DerInputStream;
import sun.security.util.DerOutputStream;
import sun.security.util.DerValue;
import sun.security.util.ObjectIdentifier;
import sun.security.pkcs.ContentInfo;

import COM.rsa.jsafe.*;


/**
 * This class provides the keystore implementation referred to as "PKCS12".
 *
 * @author Jeff Nisewanger
 * @author Jan Luehe
 *
 * @see KeyProtector
 * @see java.security.KeyStoreSpi
 * @see KeyTool
 *
 * @version 1.19, 06/24/03
 *
 */
public final class PKCS12KeyStore extends KeyStoreSpi {

    public static final int VERSION_3 = 3;

    // PKCS 12 defines that MAC keys are 160 bits which is 20 bytes.
    private static final int MAC_KEY_LENGTH = 20;

    private static final int keyBag[]     = {1, 2, 840, 113549, 1, 12, 10, 1, 2};
    private static final int certBag[]    = {1, 2, 840, 113549, 1, 12, 10, 1, 3};
    private static final int pkcs9Name[]  = {1, 2, 840, 113549, 1, 9, 20};
    private static final int pkcs9KeyId[] = {1, 2, 840, 113549, 1, 9, 21};

    private static ObjectIdentifier PKCS8ShroudedKeyBag_OID;
    private static ObjectIdentifier CertBag_OID;
    private static ObjectIdentifier PKCS9FriendlyName_OID;
    private static ObjectIdentifier PKCS9LocalKeyId_OID;

    private static volatile int counter = 0;

    static {
        try {
            PKCS8ShroudedKeyBag_OID = new ObjectIdentifier(keyBag);
            CertBag_OID = new ObjectIdentifier(certBag);
            PKCS9FriendlyName_OID = new ObjectIdentifier(pkcs9Name);
            PKCS9LocalKeyId_OID = new ObjectIdentifier(pkcs9KeyId);
        } catch (IOException ioe) {
            // should not happen
        }
    }

    // Private keys and their supporting certificate chains
    class KeyEntry {
	Date date; // the creation date of this entry
	byte[] protectedPrivKey;
	Certificate chain[];
	byte[] keyId;
	String alias;
    };

    // Trusted certificates
    class TrustedCertEntry {
	Date date; // the creation date of this entry
	Certificate cert;
    };

    class KeyId {
	byte[] keyId;

	KeyId(byte[] keyId) {
	    this.keyId = keyId;
	}
	public int hashCode() {
	    int hash = 0;

	    for (int i=0; i < keyId.length; i++)
		hash += keyId[i];
	    return hash;
	}
	public boolean equals(Object obj) {
	    if (!(obj instanceof KeyId))
		return false;
	    KeyId that = (KeyId)obj;
	    if (this.keyId == that.keyId)
		return true;
	    if (this.keyId.length != that.keyId.length)
		return false;
	    for (int i=0; i < this.keyId.length; i++)
		if (this.keyId[i] != that.keyId[i])
		    return false;
		
	    return true;
	}
    }

    /**
     * Private keys and certificates are stored in a hashtable.
     * Hash entries are keyed by alias names.
     */
    private Hashtable entries = new Hashtable();

    private ArrayList keyList = new ArrayList();
    private HashMap certs = new HashMap();
    private int caCounter = 1;

    /**
     * Returns the key associated with the given alias, using the given
     * password to recover it.
     *
     * @param alias the alias name
     * @param password the password for recovering the key
     *
     * @return the requested key, or null if the given alias does not exist
     * or does not identify a <i>key entry</i>.
     *
     * @exception NoSuchAlgorithmException if the algorithm for recovering the
     * key cannot be found
     * @exception UnrecoverableKeyException if the key cannot be recovered
     * (e.g., the given password is wrong).
     */
    public Key engineGetKey(String alias, char[] password)
	throws NoSuchAlgorithmException, UnrecoverableKeyException
    {
	Object entry = entries.get(alias.toLowerCase());
	Key key = null;

	if (entry == null || !(entry instanceof KeyEntry)) {
	    return null;
	}

	byte[] encrBytes = ((KeyEntry)entry).protectedPrivKey;
	try {
            DerValue val = new DerValue(encrBytes);
            DerValue[] seq = new DerValue[2];
	    byte[] encryptedData;

            seq[0] = val.data.getDerValue();
            seq[1] = val.data.getDerValue();
 
            if (val.data.available() != 0) {
                throw new IOException("overrun, bytes = " + val.data.available());
            }
	    encryptedData = seq[1].getOctetString();

	    /*
	     * Get Crypto-J to parse the algorithm parameters for us.
	     */
            JSAFE_SymmetricCipher c;
            c = JSAFE_SymmetricCipher.getInstance(encrBytes, 0, "Java");
            JSAFE_SecretKey k = c.getBlankKey();
            k.setPassword(password, 0, password.length);
            c.decryptInit(k);
	    if (!("3DES_EDE".equals(c.getEncryptionAlgorithm())))
		throw new NoSuchAlgorithmException("3DES_EDE only for shrouded keys");

	    /*
	     * Crypto-J has an overflow bug in their PKCS12 key generator.
	     * Use our own instead.
	     */
	    byte[] myKey = deriveSHACipherKey(password, c.getSalt(), c.getPBEParameters()[0],
		(c.getPBEParameters()[1] * c.getBlockSize()));
	    byte[] myIv = deriveSHACipherIv(password, c.getSalt(), c.getPBEParameters()[0],
		c.getBlockSize());
	    String alg = c.getEncryptionAlgorithm() + "/" + c.getFeedbackMode() +
		"/" + c.getPaddingScheme();
	    c = JSAFE_SymmetricCipher.getInstance(alg, "Java");
	    c.setIV(myIv, 0, myIv.length);
	    k = c.getBlankKey();
	    k.setSecretKeyData(myKey, 0, myKey.length);
	    c.decryptInit(k);

	    byte[] privateKeyInfo = doDecryptFinal(c, encryptedData);
	    PKCS8EncodedKeySpec kspec = new PKCS8EncodedKeySpec(privateKeyInfo);

	    /*
	     * Use Crypto-J to parse the key algorithm and then use a JCA
	     * key factory to create the private key.
	     */
	    JSAFE_PrivateKey jkey = JSAFE_PrivateKey.getInstance(privateKeyInfo,
		0, "Java");
	    KeyFactory kfac = KeyFactory.getInstance(jkey.getAlgorithm());
	    key =  kfac.generatePrivate(kspec);
	} catch(Exception e) {
	    throw new UnrecoverableKeyException(e.getMessage());
	}
	return key;
    }

    /**
     * Returns the certificate chain associated with the given alias.
     *
     * @param alias the alias name
     *
     * @return the certificate chain (ordered with the user's certificate first
     * and the root certificate authority last), or null if the given alias
     * does not exist or does not contain a certificate chain (i.e., the given 
     * alias identifies either a <i>trusted certificate entry</i> or a
     * <i>key entry</i> without a certificate chain).
     */
    public Certificate[] engineGetCertificateChain(String alias) {
	Object entry = entries.get(alias.toLowerCase());

	if (entry != null && entry instanceof KeyEntry) {
	    if (((KeyEntry)entry).chain == null) {
		return null;
	    } else {
		return (Certificate[])((KeyEntry)entry).chain.clone();
	    }
	} else {
	    return null;
	}
    }

    /**
     * Returns the certificate associated with the given alias.
     *
     * <p>If the given alias name identifies a
     * <i>trusted certificate entry</i>, the certificate associated with that
     * entry is returned. If the given alias name identifies a
     * <i>key entry</i>, the first element of the certificate chain of that
     * entry is returned, or null if that entry does not have a certificate
     * chain.
     *
     * @param alias the alias name
     *
     * @return the certificate, or null if the given alias does not exist or
     * does not contain a certificate.
     */
    public Certificate engineGetCertificate(String alias) {
	Object entry = entries.get(alias.toLowerCase());

	if (entry != null) {
	    if (entry instanceof TrustedCertEntry) {
		return ((TrustedCertEntry)entry).cert;
	    } else {
		if (((KeyEntry)entry).chain == null) {
		    return null;
		} else {
		    return ((KeyEntry)entry).chain[0];
		}
	    }
	} else {
	    return null;
	}
    }	

    /**
     * Returns the creation date of the entry identified by the given alias.
     *
     * @param alias the alias name
     *
     * @return the creation date of this entry, or null if the given alias does
     * not exist
     */
    public Date engineGetCreationDate(String alias) {
	Object entry = entries.get(alias.toLowerCase());

	if (entry != null) {
	    if (entry instanceof TrustedCertEntry) {
		return new Date(((TrustedCertEntry)entry).date.getTime());
	    } else {
		return new Date(((KeyEntry)entry).date.getTime());
	    }
	} else {
	    return null;
	}	
    }

    /**
     * Assigns the given key to the given alias, protecting it with the given
     * password.
     *
     * <p>If the given key is of type <code>java.security.PrivateKey</code>,
     * it must be accompanied by a certificate chain certifying the
     * corresponding public key.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key (and possibly
     * certificate chain).
     *
     * @param alias the alias name
     * @param key the key to be associated with the alias
     * @param password the password to protect the key
     * @param chain the certificate chain for the corresponding public
     * key (only required if the given key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if the given key cannot be protected, or
     * this operation fails for some other reason
     */
    public void engineSetKeyEntry(String alias, Key key, char[] password,
				  Certificate[] chain)
	throws KeyStoreException
    {
	try {
	    synchronized(entries) {
		KeyEntry entry = new KeyEntry();
		entry.date = new Date();

		// Encrypt the private key

		// clone the chain
		if (chain != null) {
		    entry.chain = (Certificate[])chain.clone();
		}

		entries.put(alias.toLowerCase(), entry);
	    }
	} catch (Exception nsae) {
	    throw new KeyStoreException("Key protection algorithm not found");
	}
    }

    /**
     * Assigns the given key (that has already been protected) to the given
     * alias.
     * 
     * <p>If the protected key is of type
     * <code>java.security.PrivateKey</code>, it must be accompanied by a
     * certificate chain certifying the corresponding public key. If the
     * underlying keystore implementation is of type <code>jks</code>,
     * <code>key</code> must be encoded as an
     * <code>EncryptedPrivateKeyInfo</code> as defined in the PKCS #8 standard.
     *
     * <p>If the given alias already exists, the keystore information
     * associated with it is overridden by the given key (and possibly
     * certificate chain).
     *
     * @param alias the alias name
     * @param key the key (in protected format) to be associated with the alias
     * @param chain the certificate chain for the corresponding public
     * key (only useful if the protected key is of type
     * <code>java.security.PrivateKey</code>).
     *
     * @exception KeyStoreException if this operation fails.
     */
    public void engineSetKeyEntry(String alias, byte[] key,
				  Certificate[] chain)
	throws KeyStoreException
    {
	synchronized(entries) {
	    // key must be encoded as EncryptedPrivateKeyInfo as defined in
	    // PKCS#8

	    KeyEntry entry = new KeyEntry();
	    entry.date = new Date();

	    entry.protectedPrivKey = (byte[])key.clone();
	    if (chain != null) {
		entry.chain = (Certificate[])chain.clone();
	    }

	    entries.put(alias.toLowerCase(), entry);
	}
    }

    /**
     * Assigns the given certificate to the given alias.
     *
     * <p>If the given alias already exists in this keystore and identifies a
     * <i>trusted certificate entry</i>, the certificate associated with it is
     * overridden by the given certificate.
     *
     * @param alias the alias name
     * @param cert the certificate
     *
     * @exception KeyStoreException if the given alias already exists and does
     * not identify a <i>trusted certificate entry</i>, or this operation
     * fails for some other reason.
     */
    public void engineSetCertificateEntry(String alias, Certificate cert)
	throws KeyStoreException
    {
	synchronized(entries) {

	    Object entry = entries.get(alias.toLowerCase());
	    if ((entry != null) && (entry instanceof KeyEntry)) {
		throw new KeyStoreException
		    ("Cannot overwrite own certificate");
	    }

	    TrustedCertEntry trustedCertEntry = new TrustedCertEntry();
	    trustedCertEntry.cert = cert;
	    trustedCertEntry.date = new Date();
	    entries.put(alias.toLowerCase(), trustedCertEntry);
	}
    }

    /**
     * Deletes the entry identified by the given alias from this keystore.
     *
     * @param alias the alias name
     *
     * @exception KeyStoreException if the entry cannot be removed.
     */
    public void engineDeleteEntry(String alias)
	throws KeyStoreException
    {
	synchronized(entries) {
	    entries.remove(alias.toLowerCase());
	}
    }

    /**
     * Lists all the alias names of this keystore.
     *
     * @return enumeration of the alias names
     */
    public Enumeration engineAliases() {
	return entries.keys();
    }

    /**
     * Checks if the given alias exists in this keystore.
     *
     * @param alias the alias name
     *
     * @return true if the alias exists, false otherwise
     */
    public boolean engineContainsAlias(String alias) {
	return entries.containsKey(alias.toLowerCase());
    }

    /**
     * Retrieves the number of entries in this keystore.
     *
     * @return the number of entries in this keystore
     */
    public int engineSize() {
	return entries.size();
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>key entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>key entry</i>, false otherwise.
     */
    public boolean engineIsKeyEntry(String alias) {
	Object entry = entries.get(alias.toLowerCase());
	if ((entry != null) && (entry instanceof KeyEntry)) {
	    return true;
	} else {
	    return false;
	}
    }

    /**
     * Returns true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, and false otherwise.
     *
     * @return true if the entry identified by the given alias is a
     * <i>trusted certificate entry</i>, false otherwise.
     */
    public boolean engineIsCertificateEntry(String alias) {
	Object entry = entries.get(alias.toLowerCase());
	if ((entry != null) && (entry instanceof TrustedCertEntry)) {
	    return true;
	} else {
	    return false;
	}
    }

    /**
     * Returns the (alias) name of the first keystore entry whose certificate
     * matches the given certificate.
     *
     * <p>This method attempts to match the given certificate with each
     * keystore entry. If the entry being considered
     * is a <i>trusted certificate entry</i>, the given certificate is
     * compared to that entry's certificate. If the entry being considered is
     * a <i>key entry</i>, the given certificate is compared to the first
     * element of that entry's certificate chain (if a chain exists).
     *
     * @param cert the certificate to match with.
     *
     * @return the (alias) name of the first entry with matching certificate,
     * or null if no such entry exists in this keystore.
     */
    public String engineGetCertificateAlias(Certificate cert) {
	Certificate certElem;

	for (Enumeration e = entries.keys(); e.hasMoreElements(); ) {
	    String alias = (String)e.nextElement();
	    Object entry = entries.get(alias);
	    if (entry instanceof TrustedCertEntry) {
		certElem = ((TrustedCertEntry)entry).cert;
	    } else if (((KeyEntry)entry).chain != null) {
		certElem = ((KeyEntry)entry).chain[0];
	    } else {
		continue;
	    }
	    if (certElem.equals(cert)) {
		return alias;
	    }
	}
	return null;
    }

    /**
     * Stores this keystore to the given output stream, and protects its
     * integrity with the given password.
     *
     * @param stream the output stream to which this keystore is written.
     * @param password the password to generate the keystore integrity check
     *
     * @exception IOException if there was an I/O problem with data
     * @exception NoSuchAlgorithmException if the appropriate data integrity
     * algorithm could not be found
     * @exception CertificateException if any of the certificates included in
     * the keystore data could not be stored
     */
    public void engineStore(OutputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	synchronized(entries) {
	    throw new IOException("PKCS 12 storing not implemented");
	}
    }

    /**
     * Loads the keystore from the given input stream.
     *
     * <p>If a password is given, it is used to check the integrity of the
     * keystore data. Otherwise, the integrity of the keystore is not checked.
     *
     * @param stream the input stream from which the keystore is loaded
     * @param password the (optional) password used to check the integrity of
     * the keystore.
     *
     * @exception IOException if there is an I/O or format problem with the
     * keystore data
     * @exception NoSuchAlgorithmException if the algorithm used to check
     * the integrity of the keystore cannot be found
     * @exception CertificateException if any of the certificates in the
     * keystore could not be loaded
     */
    public void engineLoad(InputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	synchronized(entries) {
	    DataInputStream dis;
	    MessageDigest md = null;
	    CertificateFactory cf = null;
	    Hashtable cfs = null;
	    ByteArrayInputStream bais = null;
	    byte[] encoded = null;

	    if (stream == null)
		return;

	    DerValue val = new DerValue(stream);
	    DerInputStream s = val.toDerInputStream();
	    int version = s.getInteger();

	    if (version != VERSION_3) {
		throw new IOException("PKCS12 keystore not in version 3 format");
	    }

	    cfs = new Hashtable(3);
	    entries.clear();

	    /* 
	     * Read the authSafe.
	     */
	    byte[] authSafeData;
	    ContentInfo authSafe = new ContentInfo(s);
	    ObjectIdentifier contentType = authSafe.getContentType();

            if (contentType.equals(ContentInfo.DATA_OID)) {
		authSafeData = authSafe.getData();
	    } else /* signed data */ {
		throw new IOException("public key protected PKCS12 not supported");
	    }

            DerInputStream as = new DerInputStream(authSafeData);

	    DerValue[] safeContentsArray = as.getSequence(2);
	    int count = safeContentsArray.length;

	    /*
	     * Spin over the ContentInfos.
	     */
	    for (int i = 0; i < count; i++) {
	        byte[] safeContentsData;
	        ContentInfo safeContents;
		DerInputStream sci;
		byte[] eAlgId = null;

		sci = new DerInputStream(safeContentsArray[i].toByteArray());
		safeContents = new ContentInfo(sci);
	        contentType = safeContents.getContentType();
	        safeContentsData = null;
                if (contentType.equals(ContentInfo.DATA_OID)) {
	            safeContentsData = safeContents.getData();
	        } else if (contentType.equals(ContentInfo.ENCRYPTED_DATA_OID)) {
		    if (password == null) {
			continue;
		    }
		    DerInputStream edi = safeContents.getContent().toDerInputStream();
		    int edVersion = edi.getInteger();
		    DerValue[] seq = edi.getSequence(2);
		    ObjectIdentifier edContentType = seq[0].getOID();
		    eAlgId = seq[1].toByteArray();
		    if (!seq[2].isContextSpecific((byte)0)) {
		        throw new IOException("encrypted content not present!");
		    }
		    byte newTag = DerValue.tag_OctetString;
		    if (seq[2].isConstructed())
			newTag |= 0x20;
		    seq[2].resetTag(newTag);
		    safeContentsData = seq[2].getOctetString();

		  try {
		    JSAFE_SymmetricCipher c;
		    c = JSAFE_SymmetricCipher.getInstance(eAlgId, 0, "Java");
		    JSAFE_SecretKey k = c.getBlankKey();
		    k.setPassword(password, 0, password.length);
		    c.decryptInit(k);

		    /*
		     * Workaround for J-Crypto bug. It insists on using 128 effective
		     * key length for PKCS12 RC2 PBE even though the key length
		     * parameter is 40 bits. Since RC2 effective key length doesn't
		     * matter to the PKCS12 PBE key generation algorithm we use
		     * the above to parse the algorithm parameters and generate
		     * the key and IV and then create an explicit RC2 cipher with
		     * 40-bit effective and plug back the relevant info.
		     */
		    if (!("RC2".equals(c.getEncryptionAlgorithm())))
			throw new NoSuchAlgorithmException("RC2 Only for safeContents");
		    byte[] keyStuff = k.getSecretKeyData();
		    byte[] ivStuff = c.getIV();
		    int[] pbeParams= c.getPBEParameters();
		    String alg = "RC2-" + pbeParams[1] + "/CBC/PKCS5Padding";
		    c = JSAFE_SymmetricCipher.getInstance(alg, "Java");
		    c.setIV(ivStuff, 0, ivStuff.length);
		    k = c.getBlankKey();
		    k.setSecretKeyData(keyStuff, 0, keyStuff.length);
		    c.decryptInit(k);

		    safeContentsData = doDecryptFinal(c, safeContentsData);
		  } catch (Exception e) {
		    throw new IOException("failed to decrypt safe contents entry");
		  }
	        } else {
		    throw new IOException("public key protected PKCS12 not supported");
	        }
		DerInputStream sc = new DerInputStream(safeContentsData);
	        loadSafeContents(sc, password);
	    }
	
	    // The MacData is optional.
	    if (password != null && s.available() > 0) {
		MacData macData = new MacData(s);
		try{
		    String algName = macData.getDigestAlgName().toUpperCase();

		    // JSAFE supports only HMAC/SHA1
		    if (algName.equals("SHA")  || 
			algName.equals("SHA1") || 
			algName.equals("SHA-1"))
			algName = "SHA1";

		    JSAFE_MAC mac = JSAFE_MAC.getInstance("HMAC/" + algName,
							  "Java");
		    byte[] myKey = deriveSHAMacKey(password, macData.getSalt(),
						   macData.getIterations(),
						   MAC_KEY_LENGTH); 
		    JSAFE_SecretKey k = mac.getBlankKey();
		    k.setSecretKeyData(myKey, 0, myKey.length);
		    mac.macInit(k, new SecureRandom());
		   
		    mac.macUpdate(authSafeData, 0, authSafeData.length);
		    byte[] macResult = mac.macFinal();

		    if (!Arrays.equals(macData.getDigest(), 
						 macResult)) {
			throw new SecurityException("Failed PKCS12 " +
						    "integrity checking");
		    }
		} catch (Exception e) {
		    throw new IOException("integrity check: " + e);
		}
	    }
	    
	    /*
	     * Match up private keys with certificate chains.
	     */
	    Object[] list = keyList.toArray();
	    for (int m=0; m < list.length; m++) {
		KeyEntry entry = (KeyEntry)list[m];
		if (entry.keyId != null) {
		    ArrayList chain = new ArrayList();
		    X509Certificate cert = (X509Certificate)certs.get(new KeyId(entry.keyId));
		    while(cert != null) {
			chain.add(cert);
			X500Principal issuerDN = cert.getIssuerX500Principal();
			if (issuerDN.equals(cert.getSubjectX500Principal())) {
			    break;
			}
			cert = (X509Certificate)certs.get(issuerDN);
		    }
		    /* Update existing KeyEntry in entries table */
		    if (chain.size() > 0)
		        entry.chain = (Certificate[])chain.toArray(new Certificate[chain.size()]);
		}
	    }
	    certs.clear();
	    keyList.clear();
	}
    }

    private static byte[] doDecryptFinal(JSAFE_SymmetricCipher c, byte[] ciphertext)
        throws Exception {
	int outLen = c.getOutputBufferSize(ciphertext.length);
	byte[] output = new byte[outLen];
	byte[] out;
	int off = 0;

	off = c.decryptUpdate(ciphertext, 0, ciphertext.length, output, 0);
	off = off + c.decryptFinal(output, off);
	if (off < output.length) {
	    out = new byte[off];
	    System.arraycopy(output, 0, out, 0, out.length);
	} else {
	    out = output;
	}
	return out;
    }

    private void loadSafeContents(DerInputStream stream, char[] password)
	throws IOException, NoSuchAlgorithmException, CertificateException
    {
	DerValue[] safeBags = stream.getSequence(2);
	int count = safeBags.length;

	/*
	 * Spin over the SafeBags.
	 */
	for (int i = 0; i < count; i++) {
	    ObjectIdentifier bagId;
	    DerInputStream sbi;
	    DerValue bagValue;
	    Object bagItem = null;

	    sbi = safeBags[i].toDerInputStream();
	    bagId = sbi.getOID();
	    bagValue = sbi.getDerValue();
	    if (!bagValue.isContextSpecific((byte)0)) {
		throw new IOException("unsupported PKCS12 bag value type " + bagValue.tag);
	    }
	    bagValue = bagValue.data.getDerValue();
            if (bagId.equals(PKCS8ShroudedKeyBag_OID)) {
		KeyEntry kEntry = new KeyEntry();
		kEntry.protectedPrivKey = bagValue.toByteArray();
		kEntry.date = new Date();
		bagItem = kEntry;
            } else if (bagId.equals(CertBag_OID)) {
		DerInputStream cs = new DerInputStream(bagValue.toByteArray());
		DerValue[] certValues = cs.getSequence(2);
		ObjectIdentifier certId = certValues[0].getOID();
		if (!certValues[1].isContextSpecific((byte)0)) {
		    throw new IOException("unsupported PKCS12 cert value type " + certValues[1].tag);
		}
		DerValue certValue = certValues[1].data.getDerValue();
		CertificateFactory cf = CertificateFactory.getInstance("X509");
		X509Certificate cert;
		cert = (X509Certificate)cf.generateCertificate(new ByteArrayInputStream(certValue.getOctetString()));
		bagItem = cert;
	    } else {
		throw new IOException("unsupported PKCS12 bag type " + bagId);
	    }
	    DerValue[] attrSet = sbi.getSet(2);

	    String alias = null;
	    byte[] keyId = null;

	    for (int j=0; j < attrSet.length; j++) {
		DerInputStream as = new DerInputStream(attrSet[j].toByteArray());
		DerValue[] attrSeq = as.getSequence(2);
		ObjectIdentifier attrId = attrSeq[0].getOID();
		DerInputStream vs = new DerInputStream(attrSeq[1].toByteArray());
		DerValue[] valSet = vs.getSet(2);
		if (attrId.equals(PKCS9FriendlyName_OID)) {
		    alias = valSet[0].getBMPString();
		} else if (attrId.equals(PKCS9LocalKeyId_OID)) {
		    keyId = valSet[0].getOctetString();
		} else {
                    System.out.println("unknown attr" + attrId);
		}
	    }

	   /*
	    * As per PKCS12 v1.0 friendlyname (alias) and localKeyId (keyId)
	    * are optional PKCS12 bagAttributes. But entries in the keyStore
	    * are identified by their alias. Hence we need to have an
	    * Unfriendlyname in the alias, if alias is null. The keyId 
	    * attribute is required to match the private key with the
	    * certificate. If we get a bagItem of type KeyEntry with a
	    * null keyId, we should skip it entirely.
	    */
	    if (bagItem instanceof KeyEntry) {
		KeyEntry entry = (KeyEntry)bagItem;
		if (keyId == null)
		   continue;
		entry.keyId = keyId;
		keyList.add(entry);
	        if (alias == null)
		   alias = getUnfriendlyName();
		entries.put(alias.toLowerCase(), entry);
	    } else if (bagItem instanceof X509Certificate) {
		X509Certificate cert = (X509Certificate)bagItem;
		if (keyId != null)
		    certs.put(new KeyId(keyId), bagItem);
		if (alias != null)
		    certs.put(alias, bagItem);
		X500Principal subjectDN = cert.getSubjectX500Principal();
		if (subjectDN != null)
		    certs.put(subjectDN, bagItem);
	    }
	}
    }

    private String getUnfriendlyName() {
	counter++;
	return (String.valueOf(counter));
    }

    // The parameter length is the length of the expected key in bytes.
    static byte[] deriveSHACipherKey(char[] passwd, byte[] salt, int iterations, int length) {
	return deriveSHA(passwd, salt, iterations, length, 1);
    }
    static byte[] deriveSHACipherIv(char[] passwd, byte[] salt, int iterations, int length) {
	return deriveSHA(passwd, salt, iterations, length, 2);
    }
    static byte[] deriveSHAMacKey(char[] passwd, byte[] salt, int iterations, int length) {
	return deriveSHA(passwd, salt, iterations, length, 3);
    }

    private static byte[] deriveSHA(char[] chars, byte[] salt, int iterations, int n, int id) {

	    byte[] passwd;

	/* Add in trailing NULL terminator. */
	int length = chars.length;
	length *= 2;
	if (length > 0)
	    length += 2;

	passwd = new byte[length];
        for (int i=0, j=0; i < chars.length; i++, j+=2) {
	    passwd[j] = (byte) ((chars[i] >>> 8) & 0xFF);
	    passwd[j+1] = (byte) (chars[i] & 0xFF);
	}


	    int v = 512 / 8;
	    int u = 160 / 8;
	    int c = roundup(n, u) / u;
	    byte[] D = new byte[v];
	    int s = roundup(salt.length, v);
	    int p = roundup(passwd.length, v);
	    byte[] I = new byte[s + p];
	    byte[] key = new byte[n];

	    Arrays.fill(D, (byte)id);
	    concat(salt, I, 0, s);
	    concat(passwd, I, s, p);

	try {
	    MessageDigest sha = MessageDigest.getInstance("SHA1");
	    byte[] Ai;
	    byte[] B = new byte[v];
	    byte[] tmp = new byte[v];

	    int i = 0;
	    for(;; i++, n -= u) {
	        sha.update(D);
	        sha.update(I);
	        Ai = sha.digest();
	        for (int r=1; r < iterations; r++)
		    Ai = sha.digest(Ai);
		System.arraycopy(Ai, 0, key, u * i, Math.min(n, u));
		if (i + 1 == c)
		    break;
		concat(Ai, B, 0, B.length);
		BigInteger B1;
		B1 = new BigInteger(1, B).add(BigInteger.ONE);

		for(int j=0; j < I.length; j+=v) {
		    BigInteger Ij;
		    int trunc;

	    	    if (tmp.length != v)
			tmp = new byte[v];
		    System.arraycopy(I, j, tmp, 0, v);
		    Ij = new BigInteger(1, tmp);
		    Ij = Ij.add(B1);
		    tmp = Ij.toByteArray();
		    trunc = tmp.length - v;
		    if (trunc >= 0) {
		        System.arraycopy(tmp, trunc, I, j, v);
		    } else if (trunc < 0) {
			Arrays.fill(I, j, j + (-trunc), (byte)0);
		        System.arraycopy(tmp, 0, I, j + (-trunc), tmp.length);
		    }
		}
	    }
	} catch (Exception e) {
	    throw new RuntimeException("internal error: " + e);
	}
	return key;
    }

    private static int roundup(int x, int y) {
	return ((x + (y - 1)) / y) * y;
    }

    private static void concat(byte[] src, byte[] dst, int start, int len) {
	int loop = len / src.length;
	int off, i;
	for(i=0, off=0; i < loop; i++, off += src.length)
	    System.arraycopy(src, 0, dst, off + start, src.length);
	System.arraycopy(src, 0, dst, off + start, len - off);
    }
}
