/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import java.security.*;
import javax.crypto.BadPaddingException;
import javax.crypto.ShortBufferException;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.IvParameterSpec;

// explicit imports to override the classes with same name in this package
import javax.crypto.Cipher;

import com.sun.net.ssl.internal.ssl.CipherSuite.*;

import sun.misc.HexDumpEncoder;


/**
 * This class handles bulk data enciphering/deciphering for each SSLv3
 * message.  This provides data confidentiality.  Stream ciphers (such
 * as RC4) don't need to do padding; block ciphers (e.g. DES) need it.
 *
 * Invididual instances are obtained by calling the static method
 * newCipherBox(), which should only be invoked by BulkCipher.newCipher().
 *
 * This kind of functionality has considerable export/import sensitivity.
 *
 * NOTE that any ciphering involved in key exchange (e.g. with RSA) is
 * handled separately.
 *
 * The inheritance hierarchy for CipherBoxes looks like this:<pre>
 *
 * CipherBox (base class)
 *    |   
 *    + JCECipherBox   (wraps external JCE ciphers in CipherBox format)
 *    |   
 *    + NullCipher   (internal JSSE impl)
 *    |   
 *    + BlockCipherBox
 *    |     |
 *    |     + AcmeCBC
 *    |           |
 *    |           + CipherDES_40  (internal JSSE impl)
 *    |           |
 *    |           + CipherDES   (internal JSSE impl)
 *    |           |
 *    |           + Cipher3DES   (internal JSSE impl)
 *    |
 *    + CipherRC4_40   (internal JSSE impl)
 *    |
 *    + CipherRC4   (internal JSSE impl)
 *
 * </pre>The source for all these classes except CipherRC4 and CipherRC4_40 can
 * be found in this file. Note that the rest of the JSSE code only deals with 
 * the CipherBox class and is not aware of the underlying classes (exception 
 * are the CipherRC4*.hasRC4() calls).
 *
 * @version 1.30 06/24/03
 * @author David Brownell
 */
abstract class CipherBox implements ExportControl, CipherSuiteConstants
{
    // A CipherBox that implements the identity operation
    final static CipherBox NULL = CipherNULL.newCipherBox();

    /* Class and subclass dynamic debugging support */
    static final Debug debug = Debug.getInstance("ssl");
    
    // the protocol version this cipher conforms to
    final ProtocolVersion protocolVersion;
    
    CipherBox(ProtocolVersion protocolVersion) {
	this.protocolVersion = protocolVersion;
    }

    /*
     * Encrypts a block of data, returning the size of the
     * resulting block if padding was required.
     */
    abstract int encrypt(byte buf [], int offset, int len);

    /*
     * Decrypts a block of data, returning the size of the
     * resulting block if padding was required.
     */
    abstract int decrypt(byte buf [], int offset, int len) 
	    throws BadPaddingException;
    
    /*
     * Lets a cipher box (usually an uninitialized one) act as a factory
     * for cipher boxes of the same type.
     */
    static CipherBox newCipherBox(ProtocolVersion version, 
	    BulkCipher cipher, byte key[], byte iv[], boolean encrypt) 
	    throws NoSuchAlgorithmException {
	if (cipher.allowed == false) {
	    throw new NoSuchAlgorithmException("Unsupported cipher " + cipher);
	}
	if (cipher == B_NULL) {
	    return CipherNULL.newCipherBox();
	} else if (cipher == B_RC4_40) {
	    return CipherRC4_40.newCipherBox(version, key, iv, encrypt);
	} else if (cipher == B_DES_40) {
	    return CipherDES_40.newCipherBox(version, key, iv, encrypt);
	} else if (ExportControl.hasStrongCrypto) {
	    if (cipher == B_DES) {
		return CipherDES.newCipherBox(version, key, iv, encrypt);
	    } if (cipher == B_3DES) {
		return Cipher3DES.newCipherBox(version, key, iv, encrypt);
	    } if (cipher == B_RC4_128) {
	        return CipherRC4.newCipherBox(version, key, iv, encrypt);
	    } if ((cipher == B_AES_128) || (cipher == B_AES_256)) {
		return JCECipherBox.newCipherBox(version, cipher.name, 
				JsseJce.CIPHER_AES , key, iv, encrypt);
	    }
	}
	throw new NoSuchAlgorithmException("Unsupported cipher " + cipher);
    }
    
//
// NOTE:  The rest of these classes are NESTED within this one
// and some may be accessed via "import" statements.
//

/**
 * The JCECipherBox class provides an adapter layer between the JCE Cipher
 * API and the CipherBox API that JSSE uses. It supports both JCE block and
 * stream ciphers.
 *
 * To use it, first call the constructor and then initCipher(). If no JCE
 * Cipher is available, the constructor throws a NoSuchAlgorithmException.
 * If initialization fails for whatever reason (including JCE keylength
 * restrictions), you get an exception there.
 *
 * This class is used by the various CipherBox implementations (which revert
 * back to the internal ciphers if this class fails) and the CipherBox class 
 * itself (for the AES cipher, which does not have an internal implementation).
 */
static class JCECipherBox extends CipherBox {

    private String name, transformation;

    private Cipher cipher;

    /**
     * Cipher blocksize, 0 for stream ciphers
     */
    private int blockSize;
    
    /**
     * Construct a new JCECipherBox using the cipher transformation.
     *
     * @exception NoSuchAlgorithmException if no appropriate JCE Cipher
     * implementation could be found.
     */
    JCECipherBox(ProtocolVersion version, String name, String transformation)
	    throws NoSuchAlgorithmException {
	super(version);
	cipher = JsseJce.getCipher(transformation);
	blockSize = cipher.getBlockSize();
	// some providers implement getBlockSize() incorrectly
	if (blockSize == 1) {
	    blockSize = 0;
	}
	this.transformation = transformation;
	this.name = name;
    }
    
    static CipherBox newCipherBox(ProtocolVersion version,
	    String name, String transformation, byte[] key, 
	    byte[] iv, boolean encrypt) throws NoSuchAlgorithmException {
	try {
	    JCECipherBox box = new JCECipherBox(version, name, transformation);
	    box.initCipher(key, iv, encrypt);
	    return box;
	} catch (Exception e) {
	    throw (NoSuchAlgorithmException)new NoSuchAlgorithmException
			    ("Could not create cipher " + name).initCause(e);
	} catch (ExceptionInInitializerError e) {
	    throw (NoSuchAlgorithmException)new NoSuchAlgorithmException
			    ("Could not create cipher " + name).initCause(e);
	}
    }
    
    public String toString() {
        return "SunJSSE_" + name;
    }
    
    /**
     * Initialize this JCECipherBox for en/decryption. The iv parameter
     * should be null for stream ciphers.
     */
    void initCipher(byte[] key, byte[] iv, boolean encrypt)
        throws InvalidKeyException, InvalidAlgorithmParameterException {
	
        int i = transformation.indexOf('/');
	if (i == -1) {
	    i = transformation.length();
	}
        String alg = transformation.substring(0, i);
	SecretKeySpec ks = new SecretKeySpec(key, alg);
	IvParameterSpec is = null;
	if (iv != null) {
	    is = new IvParameterSpec(iv);
	}
	int mode = encrypt ? Cipher.ENCRYPT_MODE : Cipher.DECRYPT_MODE;
	cipher.init(mode, ks, is);
    }
    
    int encrypt(byte[] buf, int offset, int len) {
        try {
	    if (blockSize != 0) {
		len = BlockCipherBox.addPadding(buf, offset, len, blockSize);
	    }
	    if (debug != null && Debug.isOn("plaintext")) {
		try {
		    HexDumpEncoder hd = new HexDumpEncoder();

		    System.out.println(
			"Padded plaintext before ENCRYPTION:  len = "
			+ len);
		    hd.encodeBuffer(
			new ByteArrayInputStream(buf, offset, len),
			System.out);
		} catch (IOException e) { }
	    }
	    int newLen = cipher.update(buf, offset, len, buf, offset);
	    if (newLen != len) {
	        // catch BouncyCastle buffering error
	        throw new RuntimeException("Cipher buffering error " +
		    "in JCE provider " + cipher.getProvider().getName());
	    }
	    return newLen;
	} catch (ShortBufferException e) {
	    throw new ArrayIndexOutOfBoundsException(e.toString());
	}
    }
    
    int decrypt(byte[] buf, int offset, int len) throws BadPaddingException {
        try {
            int newLen = cipher.update(buf, offset, len, buf, offset);
	    if (newLen != len) {
	        // catch BouncyCastle buffering error
	        throw new RuntimeException("Cipher buffering error " +
		    "in JCE provider " + cipher.getProvider().getName());
	    }
	    if (debug != null && Debug.isOn("plaintext")) {
		try {
		    HexDumpEncoder hd = new HexDumpEncoder();

		    System.out.println(
			"Padded plaintext after DECRYPTION:  len = "
			+ newLen);
		    hd.encodeBuffer(
			new ByteArrayInputStream(buf, offset, newLen),
			System.out);
		} catch (IOException e) { }
	    }
	    if (blockSize != 0) {
		newLen = BlockCipherBox.removePadding(buf, offset, newLen, 
	                     blockSize, protocolVersion);
	    }
            return newLen;
	} catch (ShortBufferException e) {
	    throw new ArrayIndexOutOfBoundsException(e.toString());
	}
    }
}



/*
 * NULL cipher, does no work and hence is exportable
 */
static final class CipherNULL extends CipherBox {
    
    // singleton
    private static CipherNULL NULL = new CipherNULL();

    private CipherNULL() {
	super(ProtocolVersion.DEFAULT);
    }

    public String toString() {
        return "NULL"; 
    }

    int encrypt(byte buf [], int offset, int len) {
        return len;
    }

    int decrypt(byte buf [], int offset, int len) {
        return len;
    }
    
    static CipherBox newCipherBox() {
	return NULL;
    }

}

/*
 * Base class for block ciphers.  Stream ciphers operate in units
 * of one byte; block ciphers work in larger units, so they need to
 * know how to pad.
 */
private static abstract class BlockCipherBox extends CipherBox {

    int blockSize;

    /*
     * All SSLv3 block ciphers happen to use a block size of 8 bytes.
     * We default accordingly; later subtypes can override.
     */
    BlockCipherBox(ProtocolVersion protocolVersion) {
	super(protocolVersion);
        blockSize = 8;
    }

    /*
     * Block ciphers add at least one byte after the SSLv3 MAC, to tell
     * how much padding was added.  The constraint is that the length
     * after adding the padding and the byte must be a exact multiple
     * of the cipher's block size, so that the cipher algorithm works.
     *
     * This routine adds padding per the SSLv3 protocol spec.
     *
     * NOTE:  for TLS, padding can be more than one block.
     *
     * @return padded length of the buffer, beginning at offset;
     *	note that padding is done in place.
     */
    int addPadding(byte buf [], int offset, int len) {
        return addPadding(buf, offset, len, blockSize);
    }
    
    static int addPadding(byte[] buf, int offset, int len, int blockSize) {
	int	newlen = len + 1;
	byte	pad;
	int	i;

	if ((newlen % blockSize) != 0) {
	    newlen += blockSize - 1;
	    newlen -= newlen % blockSize;
	}
	pad = (byte) (newlen - len);

	if (buf.length < (newlen + offset)) {
	    throw new IllegalArgumentException("no space to pad buffer");
	}

	/*
	 * only really need to add the final byte, but we don't 
	 * want to send potentially sensitive data.
	 */
	for (i = 0, offset += len; i < pad; i++) {
	    buf [offset++] = (byte) (pad - 1);
	}
	return newlen;
    }

    /*
     * Remove the padding that was added when encoding the buffer.
     * If an obvious error was detected, nothing is removed.
     *
     * NOTE:  for TLS, padding can be more than one block.
     *
     * @return the length of the buffer after removing padding
     */
    int removePadding(byte buf [], int offset, int len) 
	    throws BadPaddingException {
        return removePadding(buf, offset, len, blockSize, protocolVersion);
    }
    
    /*
     * Typical TLS padding format for a 64 bit block cipher is as follows:
     *   xx xx xx xx xx xx xx 00
     *   xx xx xx xx xx xx 01 01
     *   ...
     *   xx 06 06 06 06 06 06 06
     *   07 07 07 07 07 07 07 07
     * TLS also allows any amount of padding from 1 and 256 bytes as long
     * as it makes the data a multiple of the block size
     */
    static int removePadding(byte[] buf, int offset, int len, int blockSize,
	    ProtocolVersion protocolVersion) throws BadPaddingException {
	// last byte is length byte (i.e. actual padding length - 1)
	int padOffset = offset + len - 1;
	int pad = buf[padOffset] & 0x0ff;
	
	int newlen = len - (pad + 1);
	if (newlen < 0) {
	    throw new BadPaddingException("Padding length invalid: " + pad);
	}
	
	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    for (int i = 1; i <= pad; i++) {
		int val = buf[padOffset - i] & 0xff;
		if (val != pad) {
		    throw new BadPaddingException("Invalid TLS padding: " + val);
		}
	    }
	} else { // SSLv3
	    // SSLv3 requires 0 <= length byte < block size
	    // some implementations do 1 <= length byte <= block size,
	    // so accept that as well
	    // v3 does not require any particular value for the other bytes
	    if (pad > blockSize) {
		throw new BadPaddingException("Invalid SSLv3 padding: " + pad);
	    }
	}
	return newlen;
    }

}


/**
 * Utility class, to facilitate use of ACME DES implementation
 */
private static abstract class AcmeCBC extends BlockCipherBox {

    CbcBlockCipher	cbc;

    AcmeCBC(ProtocolVersion protocolVersion, BlockCipher cipher) {
	super(protocolVersion);
	cbc = new CbcBlockCipher(cipher);
    }

    /*
     * Encrypts a block of data, returning the size of the
     * resulting block after padding was added.
     */
    int encrypt(byte buf [], int offset, int len) {
	int paddedLen = addPadding(buf, offset, len);

	if (debug != null && Debug.isOn("plaintext")) {
	    try {
		HexDumpEncoder hd = new HexDumpEncoder();

		System.out.println(
		    "Padded plaintext before ENCRYPTION:  len = "
		    + paddedLen);
		hd.encodeBuffer(
		    new ByteArrayInputStream(buf, offset, paddedLen),
		    System.out);
	    } catch (IOException e) { }
	}

	for (int i = 0; i < paddedLen; i += blockSize, offset += blockSize) {
	    cbc.encrypt(buf, offset, buf, offset);
	}
	return paddedLen;
    }

    /*
     * Decrypts a block of data, returning the size of the
     * resulting block if padding was required.
     */
    int decrypt(byte buf [], int offset, int len) throws BadPaddingException {
	int temp = offset;

	for (int i = 0; i < len; i += blockSize, temp += blockSize)
	    cbc.decrypt(buf, temp, buf, temp);

	if (debug != null && Debug.isOn("plaintext")) {
	    try {
		HexDumpEncoder hd = new HexDumpEncoder();

		System.out.println(
		    "Padded plaintext after DECRYPTION:  len = "
		    + len);
		hd.encodeBuffer(
		    new ByteArrayInputStream(buf, offset, len),
		    System.out);
	    } catch (IOException e) { }
	}

	return removePadding(buf, offset, len);
    }
}


/*
 * DES ... exportable variant
 */
private static final class CipherDES_40 extends AcmeCBC {

    private CipherDES_40(ProtocolVersion version, byte[] key, byte[] iv) {
	super(version, new DesCipher(key));
	cbc.setIv(iv);
    }

    public String toString() {
	return "DES40_CBC";
    }

    static CipherBox newCipherBox(ProtocolVersion protocolVersion,
	    byte[] key, byte[] iv, boolean encrypt)
	    throws NoSuchAlgorithmException {
        try {
	    JCECipherBox box = new JCECipherBox(protocolVersion,
	        "DES40_CBC", JsseJce.CIPHER_DES);
            box.initCipher(key, iv, encrypt);
	    return box;
	} catch (Exception e) {
	    // empty
	} catch (ExceptionInInitializerError e) {
	    // empty
	}
	// if not found/error, fall back to internal implementation
	return new CipherDES_40(protocolVersion, key, iv);
    }

}


/*
 * DES ... popular worldwide, not exportable
 */
private static final class CipherDES extends AcmeCBC {

    private CipherDES(ProtocolVersion version, byte[] key, byte[] iv) {
	super(version, new DesCipher(key));
	cbc.setIv(iv);
    }

    public String toString() {
	return "DES_CBC";
    }

    static CipherBox newCipherBox(ProtocolVersion protocolVersion,
	    byte key[], byte iv[], boolean encrypt)
	    throws NoSuchAlgorithmException {
        try {
	    JCECipherBox box = new JCECipherBox(protocolVersion,
	        "DES_CBC", JsseJce.CIPHER_DES);
            box.initCipher(key, iv, encrypt);
	    return box;
	} catch (Exception e) {
	    // empty
	} catch (ExceptionInInitializerError e) {
	    // empty
	}
	// if not found/error, fall back to internal implementation
	return new CipherDES(protocolVersion, key, iv);
    }

}


/*
 * Triple-DES ... popular world wide for financial work, not exportable
 */
private static final class Cipher3DES extends AcmeCBC {

    private Cipher3DES(ProtocolVersion version, byte[] key, byte[] iv) {
	super(version, new Des3Cipher(key));
	cbc.setIv(iv);
    }

    public String toString() { 
        return "3DES_EDE_CBC";
    }

    static CipherBox newCipherBox(ProtocolVersion protocolVersion,
	    byte[] key, byte[] iv, boolean encrypt)
	    throws NoSuchAlgorithmException {
        try {
	    JCECipherBox box = new JCECipherBox(protocolVersion,
	        "3DES_EDE_CBC", JsseJce.CIPHER_3DES);
            box.initCipher(key, iv, encrypt);
	    return box;
	} catch (Exception e) {
	    // empty
	} catch (ExceptionInInitializerError e) {
	    // empty
	}
	// if not found/error, fall back to internal implementation
	return new Cipher3DES(protocolVersion, key, iv);
    }

}


//
// END of NESTED CLASES
//

}

