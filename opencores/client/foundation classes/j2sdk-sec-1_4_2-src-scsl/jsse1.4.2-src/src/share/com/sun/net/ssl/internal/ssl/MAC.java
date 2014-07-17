/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * This class computes the "Message Authentication Code" (MAC) for each
 * SSL message.  This is essentially a shared-secret signature, used to
 * provide integrity protection for SSL messages.  The MAC is actually
 * one of several keyed hashes, as associated with the cipher suite and
 * protocol version.  (SSL v3.0 uses one construct, TLS uses another.)
 *
 * <P>NOTE: MAC computation is the only place in the SSL protocol that the
 * sequence number is used.  It's also reset to zero with each change of
 * a cipher spec, so this is the only place this state is needed.
 *
 * <P>This uses a single class, rather than five or more subclasses,
 * for convenience and efficiency.
 *
 * @version 1.25, 06/24/03
 * @author David Brownell
 */
final class MAC {
    
    final static MAC NULL = new MAC(CipherSuiteConstants.M_NULL, null, null);
    
    // internal identifier for the MAC algorithm
    private MacAlg		macAlg;
    private boolean		hmac;

    // stuff defined by the kind of MAC algorithm
    private MessageDigest	digest;
    private int			macSize;

    // This data is used to construct the MAC.
    private byte		writeSecret [];
    private byte		pad1 [];
    private byte		pad2 [];
    private ProtocolVersion	protocolVersion;
    private long		sequenceNumber;

    /**
     * Set up, configured for the given SSL/TLS MAC type and version.
     *
     * @param type the type of MAC to use
     * @param major major version number (usually 3)
     * @param major minor version number (0 for SSL, 1 for TLS 1.0)
     */
    MAC(MacAlg macAlg, ProtocolVersion protocolVersion, byte[] secret) {
	this.macAlg = macAlg;
	this.protocolVersion = protocolVersion;
	this.macSize = macAlg.size;
	
	if (macAlg == CipherSuiteConstants.M_NULL) {
	    digest = null;
	} else if (macAlg == CipherSuiteConstants.M_MD5) {
	    digest = JsseJce.getMD5();
	    if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
		hmac = true;
		pad1 = (byte[])HMAC_ipad.clone();
		pad2 = (byte[])HMAC_opad.clone();
	    } else {
		hmac = false;
		pad1 = MD5_pad1;
		pad2 = MD5_pad2;
	    }
	} else if (macAlg == CipherSuiteConstants.M_SHA) {
	    digest = JsseJce.getSHA();
	    if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
		hmac = true;
		pad1 = (byte[])HMAC_ipad.clone();
		pad2 = (byte[])HMAC_opad.clone();
	    } else {
		hmac = false;
		pad1 = SHA_pad1;
		pad2 = SHA_pad2;
	    }
	} else {
	    throw new RuntimeException("Unknown MAC: " + macAlg);
	}
	if (hmac) {
	    for (int i = 0; i < secret.length; i++) {
		pad1[i] ^= secret [i];
		pad2[i] ^= secret [i];
	    }
	}
	this.writeSecret = secret;
	this.sequenceNumber = 0;
    }
    
    /**
     * Returns the length of the MAC.
     */
    int MAClen() {
	return macSize;
    }

    /**
     * Computes and returns the MAC for the data supplied.
     *
     * You must have initialized this with a write key before using this
     * call, else you'll get a null pointer exception indicating your error.
     *
     * @param type record type
     * @param buf compressed record on which the MAC is computed
     * @param offset start of compressed record data
     * @param len the size of the compressed record
     */
    final byte[] compute(byte type, byte buf[], int offset, int len) {
	if (macSize == 0) {
	    return nullMAC;
	}

	if (hmac) {
	    return computeHMAC(type, buf, offset, len);
	} else {
	    return computeMAC(type, buf, offset, len);
	}
    }


    /*
     * The SSL 3.0 MAC construct doesn't include the entire header,
     * and uses a nonstandard HMAC construct
     */
    private final byte[] computeMAC(byte type, byte buf[], int offset, int len) {
	/*
	 * hash1 = hash (secret + pad1 + seq + type + len + content)
	 */

	// secret + pad1 ...
	digest.update(writeSecret);
	digest.update(pad1);

	// ... + seq ...
	digest.update((byte)(sequenceNumber >> 56));
	digest.update((byte)(sequenceNumber >> 48));
	digest.update((byte)(sequenceNumber >> 40));
	digest.update((byte)(sequenceNumber >> 32));
	digest.update((byte)(sequenceNumber >> 24));
	digest.update((byte)(sequenceNumber >> 16));
	digest.update((byte)(sequenceNumber >>  8));
	digest.update((byte)(sequenceNumber >>  0));
	sequenceNumber++;

	// ... + type ...
	digest.update(type);

	// ... + len ...
	digest.update((byte)(len >> 8));
	digest.update((byte)(len >> 0));

	// ... + content
	digest.update(buf, offset, len);

	byte[] hash1 = digest.digest();

	/*
	 * hash2 = hash (secret, pad2, hash1)
	 */
	digest.update(writeSecret);
	digest.update(pad2);
	digest.update(hash1);

	return digest.digest();
    }


    /*
     * TLS specifies use of the HMAC construct from RFC 2104, and
     * it includes all the data in the TLS record header (though
     * it's not in the order found in the TLS record).
     */
    private final byte[] computeHMAC(byte type, byte buf [], int offset, int len) {
	/*
	 * hash1 = hash (K xor HMAC_ipad, sequence, type,
	 *	version, length, content)
	 */

	// K xor HMAC_ipad 
	digest.update(pad1);

	// ... + sequence ...
	digest.update((byte)(sequenceNumber >> 56));
	digest.update((byte)(sequenceNumber >> 48));
	digest.update((byte)(sequenceNumber >> 40));
	digest.update((byte)(sequenceNumber >> 32));
	digest.update((byte)(sequenceNumber >> 24));
	digest.update((byte)(sequenceNumber >> 16));
	digest.update((byte)(sequenceNumber >>  8));
	digest.update((byte)(sequenceNumber >>  0));
	sequenceNumber++;

	// ... + type ...
	digest.update(type);

	// ... + version ...
	digest.update(protocolVersion.major);
	digest.update(protocolVersion.minor);

	// ... + length ...
	digest.update((byte)(len >> 8));
	digest.update((byte)(len >> 0));

	// ... + content
	digest.update(buf, offset, len);

	byte[] hash1 = digest.digest();

	/*
	 * hash2 = hash (K xor opad, hash1)
	 */
	digest.update(pad2);
	digest.update(hash1);

	return digest.digest();
    }

    /*
     * SSL MAC and TLS HMAC use the same padding byte values, chosen
     * to have (among other things) even number of on/off bits.
     */
    private static final byte pad1Byte = 0x36;
    private static final byte pad2Byte = 0x5c;

    /*
     * SSL MAC padding support.
     */
    static final byte MD5_pad1[] = genPad(MAC.pad1Byte, 48);
    static final byte MD5_pad2[] = genPad(MAC.pad2Byte, 48);

    static final byte SHA_pad1[] = genPad(MAC.pad1Byte, 40);
    static final byte SHA_pad2[] = genPad(MAC.pad2Byte, 40);

    /*
     * TLS HMAC "inner" and "outer" padding.  This isn't a function
     * of the digest algorithm.
     */
    static final byte HMAC_ipad[] = genPad(MAC.pad1Byte, 64);
    static final byte HMAC_opad[] = genPad(MAC.pad2Byte, 64);


    private static byte[] genPad(byte b, int count) {
	byte padding[] = new byte[count];
	while (count-- > 0) {
	    padding [count] = b;
	}
	return padding;
    }

    // Value of the null MAC is fixed
    private static final byte nullMAC[] = new byte[0];
}

