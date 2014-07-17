/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.security.*;
import java.security.interfaces.*;

import javax.net.ssl.*;

/**
 * This is the client key exchange message (CLIENT --> SERVER) used with
 * all RSA key exchanges; it holds the RSA-encrypted pre-master secret.
 *
 * The message is encrypted using PKCS #1 block type 02 encryption with the
 * server's public key.  The padding and resulting message size is a function
 * of this server's public key modulus size, but the pre-master secret is
 * always exactly 48 bytes.
 *
 * @version 1.27 06/24/03
 */
final class PreMasterSecret extends HandshakeMessage {
    
    /**
     * The TLS spec says that the version in the RSA premaster secret must
     * be the maximum version supported by the client (i.e. the version it
     * requested in its client hello version). However, we (and other
     * implementations) used to send the active negotiated version. The
     * system property below allows to toggle the behavior.
     *
     * Default is "false" (old behavior) for compatibility reasons. This
     * will be changed in the future.
     */
    private final static String PROP_NAME = 
    				"com.sun.net.ssl.rsaPreMasterSecretFix";
    
    private final static boolean rsaPreMasterSecretFix = 
    				Debug.getBooleanProperty(PROP_NAME, false);

    int	messageType() {
        return ht_client_key_exchange;
    }

    /*
     * The following field values were encrypted with the server's public
     * key (or temp key from server key exchange msg) and are presented
     * here in DECRYPTED form.
     */
    ProtocolVersion protocolVersion; // preMaster [0,1]
    byte preMaster[];		// 48 bytes
    byte encrypted[];		// same size as public modulus


    /*
     * Client randomly creates a pre-master secret and encrypts it
     * using the server's RSA public key; only the server can decrypt
     * it, using its RSA private key.  Result is the same size as the
     * server's public key, and uses PKCS #1 block format 02.
     */
    PreMasterSecret(ProtocolVersion protocolVersion, ProtocolVersion maxVersion,
	    SecureRandom generator, PublicKey publicKey) throws IOException {
	if (!(publicKey instanceof RSAPublicKey)) {
	    throw new SSLKeyException("Public key not of type RSA");
	}
	this.protocolVersion = protocolVersion;

	preMaster = new byte[48];
	generator.nextBytes(preMaster);
	if (rsaPreMasterSecretFix) {
	    preMaster[0] = maxVersion.major;
	    preMaster[1] = maxVersion.minor;
	} else {
	    preMaster[0] = protocolVersion.major;
	    preMaster[1] = protocolVersion.minor;
	}

	try {
	    RSACipher cipher = RSACipher.getInstance();
	    cipher.encryptInit((RSAPublicKey)publicKey, generator);
	    encrypted = cipher.doEncrypt(preMaster, 0, preMaster.length);
	} catch (GeneralSecurityException e) {
	    throw (SSLKeyException)new SSLKeyException
	    			("RSA premaster secret error").initCause(e);
	}
    }

    /*
     * Server gets the PKCS #1 (block format 02) data, decrypts
     * it with its private key.
     */
    PreMasterSecret(ProtocolVersion currentVersion, ProtocolVersion clientVersion,
	    SecureRandom generator, HandshakeInStream input, int messageSize, 
	    PrivateKey privateKey) throws IOException {

	if (!(privateKey instanceof RSAPrivateKey)) {
	    throw new SSLKeyException("Private key not of type RSA");
	}
	
	if (currentVersion.v >= ProtocolVersion.TLS10.v) {
	    encrypted = input.getBytes16();
	} else {
	    encrypted = new byte [messageSize];
	    if (input.read(encrypted) != messageSize) {
	        throw new SSLProtocolException
			("SSL: read PreMasterSecret: short read");
	    }
	}

	try {
	    RSACipher cipher = RSACipher.getInstance();
	    cipher.decryptInit((RSAPrivateKey)privateKey);
	    preMaster = cipher.doDecrypt(encrypted, 0, encrypted.length);
	    protocolVersion = ProtocolVersion.valueOf(preMaster[0], preMaster[1]);
	    if (debug != null && Debug.isOn("handshake")) {
		System.out.println("RSA PreMasterSecret version: " 
			+ protocolVersion);
	    }
	} catch (Exception e) {
	    // catch exception & process below
	    preMaster = null;
	    protocolVersion = currentVersion;
	}

	// check if the premaster secret version is ok
	// the specification says that it must be the maximum version supported
	// by the client from its ClientHello message. However, many 
	// implementations send the negotiated version, so accept both
	// NOTE that we may be comparing two unsupported version numbers in
	// the second case, which is why we cannot use object references
	// equality in this special case
	boolean versionMismatch = (protocolVersion != currentVersion) && 
				  (protocolVersion.v != clientVersion.v);
	/*
	 * Bogus decrypted ClientKeyExchange? If so, conjure a
	 * a random preMaster secret that will fail later during
	 * Finished message processing. This is a countermeasure against
	 * the "interactive RSA PKCS#1 encryption envelop attack" reported
	 * in June 1998. Preserving the executation path will
	 * mitigate timing attacks and force consistent error handling
	 * that will prevent an attacking client from differentiating
	 * different kinds of decrypted ClientKeyExchange bogosities.
	 */
	 if ((preMaster == null) || (preMaster.length != 48)
		|| versionMismatch) {
	    if (debug != null && Debug.isOn("handshake")) {
		System.out.println("RSA PreMasterSecret error, "
				   + "generating random secret");
		if (preMaster != null) {
		    Debug.println(System.out, "Invalid secret", preMaster);
		}
	    }
	    preMaster = new byte[48];
	    generator.nextBytes(preMaster);
	    protocolVersion = currentVersion;
	    preMaster[0] = currentVersion.major;
	    preMaster[1] = currentVersion.minor;
	}
    }

    int messageLength() {
	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    return encrypted.length + 2;
	} else {
	    return encrypted.length;
	}
    }

    void send(HandshakeOutStream s) throws IOException {
	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    s.putBytes16(encrypted);
	} else {
	    s.write(encrypted);
	}
    }

    void print(PrintStream s) throws IOException {
	s.println("*** ClientKeyExchange, RSA PreMasterSecret, " + protocolVersion);

	if (debug != null && Debug.isOn("verbose")) {
	    Debug.println(s, "Random Secret", preMaster);
	}
    }
}

