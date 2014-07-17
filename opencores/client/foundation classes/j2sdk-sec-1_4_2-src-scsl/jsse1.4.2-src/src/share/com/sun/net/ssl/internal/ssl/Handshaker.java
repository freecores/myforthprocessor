/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.util.*;
import java.security.Key;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLException;
import javax.net.ssl.SSLProtocolException;
import sun.misc.HexDumpEncoder;

import com.sun.net.ssl.internal.ssl.HandshakeMessage.*;
import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * Handshaker ... processes handshake records from an SSL V3.0
 * data stream, handling all the details of the handshake protocol.
 *
 * Note that the real protocol work is done in two subclasses, the  base
 * class just provides the control flow and key generation framework.
 *
 * @version 1.106, 06/24/03
 * @author David Brownell
 */
abstract class Handshaker implements ExportControl, CipherSuiteConstants {

    // current protocol version
    ProtocolVersion protocolVersion;

    // list of enabled protocols
    ProtocolList enabledProtocols;

    final boolean	isClient;

    SSLSocketImpl	conn;
    HandshakeHash	handshakeHash;
    HandshakeInStream	input;
    HandshakeOutStream	output;
    int			state;
    SSLContextImpl	sslContext;
    RandomCookie	clnt_random, svr_random;
    SSLSessionImpl	session;

    // Temporary MD5 and SHA message digests. Must always be left
    // in reset state after use.
    private MessageDigest md5Tmp, shaTmp;

    // list of enabled CipherSuites
    CipherSuiteList     enabledCipherSuites;

    // current CipherSuite. Never null, initially SSL_NULL_WITH_NULL_NULL
    CipherSuite		cipherSuite;

    // current key exchange. Never null, initially K_NULL
    KeyExchange		keyExchange;

    /* True if this session is being resumed (fast handshake) */
    boolean		resumingSession;

    /* True if it's OK to start a new SSL session */
    boolean		enableNewSession;

    // Temporary storage for the individual keys. Set by
    // calculateConnectionKeys() and cleared once the ciphers are
    // activated.
    private byte[] clntWriteKey, svrWriteKey;
    private byte[] clntWriteIV, svrWriteIV;
    private byte[] clntMacSecret, svrMacSecret;

    /* Class and subclass dynamic debugging support */
    static final Debug debug = Debug.getInstance("ssl");

    Handshaker(SSLSocketImpl c,	SSLContextImpl context,
	    ProtocolList enabledProtocols, boolean needCertVerify,
	    boolean isClient) {
	this.sslContext = context;
	this.conn = c;
	this.isClient = isClient;
	enableNewSession = true;

	setCipherSuite(C_NULL);

	md5Tmp = JsseJce.getMD5();
	shaTmp = JsseJce.getSHA();

	//
	// We accumulate digests of the handshake messages so that
	// we can read/write CertificateVerify and Finished messages,
	// getting assurance against some particular active attacks.
	//
	handshakeHash = new HandshakeHash(needCertVerify);

	setEnabledProtocols(enabledProtocols);

	c.getAppInputStream().r.setHandshakeHash(handshakeHash);

	//
	// In addition to the connection state machine, controlling
	// how the connection deals with the different sorts of records
	// that get sent (notably handshake transitions!), there's
	// also a handshaking state machine that controls message
	// sequencing.
	//
	// It's a convenient artifact of the protocol that this can,
	// with only a couple of minor exceptions, be driven by the
	// type constant for the last message seen:  except for the
	// client's cert verify, those constants are in a convenient
	// order to drastically simplify state machine checking.
	//
	state = -1;
    }

    /**
     * Set the active protocol version and propagate it to the SSLSocket
     * and our handshake streams. Called from ClientHandshaker
     * and ServerHandshaker with the negotiated protocol version.
     */
    void setVersion(ProtocolVersion protocolVersion) {
	this.protocolVersion = protocolVersion;
	conn.setVersion(protocolVersion);
	output.r.setVersion(protocolVersion);
    }

    /**
     * Set the enabled protocols. Called from the constructor or
     * SSLSocketImpl.setEnabledProtocols() (if the handshake is not yet
     * in progress).
     */
    void setEnabledProtocols(ProtocolList enabledProtocols) {
	this.enabledProtocols = enabledProtocols;

	// temporary protocol version until the actual protocol version
	// is negotiated in the Hello exchange. This affects the record
	// version we sent with the ClientHello. Using max() as the record
	// version is not really correct but some implementations fail to
	// correctly negotiate TLS otherwise.
	protocolVersion = enabledProtocols.max;

	ProtocolVersion helloVersion = enabledProtocols.helloVersion;

	output = new HandshakeOutStream(protocolVersion, helloVersion,
					handshakeHash, conn);

	input = new HandshakeInStream(handshakeHash);

	conn.getAppInputStream().r.setHelloVersion(helloVersion);
    }

    /**
     * Set cipherSuite and keyExchange to the given CipherSuite.
     * Does not perform any verification that this is a valid selection,
     * this must be done before calling this method.
     */
    void setCipherSuite(CipherSuite s) {
	this.cipherSuite = s;
	this.keyExchange = s.keyExchange;
    }

    /**
     * Check if the given ciphersuite is enabled and available.
     * (Enabled ciphersuites are always available unless the status has
     * changed due to change in JCE providers since it was enabled).
     * Does not check if the required server certificates are available.
     */
    boolean isEnabled(CipherSuite s) {
	return enabledCipherSuites.contains(s) && s.isAvailable();
    }

    /**
     * As long as handshaking has not started, we can
     * change whether session creations are allowed.
     *
     * Callers should do their own checking if handshaking
     * has started.
     */
    void setEnableSessionCreation(boolean newSessions) {
	enableNewSession = newSessions;
    }

    /**
     * Create a new read cipher and return it to caller.
     */
    CipherBox newReadCipher() throws NoSuchAlgorithmException {
	BulkCipher cipher = cipherSuite.cipher;
	CipherBox box;
	if (isClient) {
	    box = cipher.newCipher(protocolVersion, svrWriteKey, svrWriteIV,
				   false);
	    svrWriteKey = null;
	    svrWriteIV = null;
	} else {
	    box = cipher.newCipher(protocolVersion, clntWriteKey, clntWriteIV,
				   false);
	    clntWriteKey = null;
	    clntWriteIV = null;
	}
	return box;
    }

    /**
     * Create a new write cipher and return it to caller.
     */
    CipherBox newWriteCipher() throws NoSuchAlgorithmException {
	BulkCipher cipher = cipherSuite.cipher;
	CipherBox box;
	if (isClient) {
	    box = cipher.newCipher(protocolVersion, clntWriteKey, clntWriteIV,
				   true);
	    clntWriteKey = null;
	    clntWriteIV = null;
	} else {
	    box = cipher.newCipher(protocolVersion, svrWriteKey, svrWriteIV,
				   true);
	    svrWriteKey = null;
	    svrWriteIV = null;
	}
	return box;
    }

    /**
     * Create a new read MAC and return it to caller.
     */
    MAC newReadMAC() throws NoSuchAlgorithmException {
	MacAlg macAlg = cipherSuite.macAlg;
	MAC mac;
	if (isClient) {
	    mac = macAlg.newMac(protocolVersion, svrMacSecret);
	    svrMacSecret = null;
	} else {
	    mac = macAlg.newMac(protocolVersion, clntMacSecret);
	    clntMacSecret = null;
	}
	return mac;
    }

    /**
     * Create a new write MAC and return it to caller.
     */
    MAC newWriteMAC() throws NoSuchAlgorithmException {
	MacAlg macAlg = cipherSuite.macAlg;
	MAC mac;
	if (isClient) {
	    mac = macAlg.newMac(protocolVersion, clntMacSecret);
	    clntMacSecret = null;
	} else {
	    mac = macAlg.newMac(protocolVersion, svrMacSecret);
	    svrMacSecret = null;
	}
	return mac;
    }

    /*
     * Returns true iff the handshake sequence is done, so that
     * this freshly created session can become the current one.
     */
    boolean isDone() {
	return state == HandshakeMessage.ht_finished;
    }


    /*
     * Returns the session which was created through this
     * handshake sequence ... should be called after isDone()
     * returns true.
     */
    SSLSessionImpl getSession()	{
	return session;
    }

    /*
     * This routine is fed SSL handshake records when they become available,
     * and processes messages found therein.  On input, we to hash messages
     * one at a time since servers may need to access an intermediate hash
     * to validate a CertificateVerify message.
     *
     * Note that many handshake messages can come in one record (and often
     * do, to reduce network resource utilization), and one message can also
     * require multiple records (e.g. very large Certificate messages).
     */
    void process_record(InputRecord r) throws IOException {
	/*
	 * Store the incoming handshake data, then see if we can
	 * now process any completed handshake messages
	 */
	input.incomingRecord(r);
	while (input.available() > 0) {
	    byte messageType;
	    int messageLen;

	    /*
	     * See if we can read the handshake message header, and
	     * then the entire handshake message.  If not, wait till
	     * we can read and process an entire message.
	     */
	    input.mark(4);

	    messageType = (byte)input.getInt8();
	    messageLen = input.getInt24();

	    if (input.available() < messageLen) {
		input.reset();
		return;
	    }

	    /*
	     * Process the messsage.  We require
	     * that processMessage() consumes the entire message.  In
	     * lieu of explicit error checks (how?!) we assume that the
	     * data will look like garbage on encoding/processing errors,
	     * and that other protocol code will detect such errors.
	     *
	     * Note that digesting is normally deferred till after the
	     * message has been processed, though to process at least the
	     * client's Finished message (i.e. send the server's) we need
	     * to acccelerate that digesting.
	     *
	     * Also, note that hello request messages are never hashed;
	     * that includes the hello request header, too.
	     */
	    if (messageType == HandshakeMessage.ht_hello_request) {
		input.reset();
		processMessage(messageType, messageLen);
		input.ignore(4 + messageLen);
	    } else {
		input.mark(messageLen);
		processMessage(messageType, messageLen);
		input.digestNow();
	    }
	}
    }


    /**
     * Returns true iff the handshaker has sent any messages.
     * Server kickstarting is not as neat as it should be; we
     * need to create a new handshaker, this method lets us
     * know if we should.
     */
    boolean started() {
	return state >= 0;
    }


    /*
     * Used to kickstart the negotiation ... either writing a
     * ClientHello or a HelloRequest as appropriate, whichever
     * the subclass returns.  NOP if handshaking's already started.
     */
    void kickstart() throws IOException {
	if (state >= 0) {
	    return;
	}
	HandshakeMessage m = getKickstartMessage();

	if (debug != null && Debug.isOn("handshake")) {
	    m.print(System.out);
	}
	m.write(output);
	output.flush();

	state = m.messageType();
    }

    /**
     * Both client and server modes can start handshaking; but the
     * message they send to do so is different.
     */
    abstract HandshakeMessage getKickstartMessage() throws SSLException;

    /*
     * Client and Server side protocols are each driven though this
     * call, which processes a single message and drives the appropriate
     * side of the protocol state machine (depending on the subclass).
     */
    abstract void processMessage(byte messageType, int messageLen)
	throws IOException;

    /*
     * Most alerts in the protocol relate to handshaking problems.
     * Alerts are detected as the connection reads data.
     */
    abstract void handshakeAlert(byte description) throws SSLProtocolException;

    /*
     * Sends a change cipher spec message and updates the write side
     * cipher state so that future messages use the just-negotiated spec.
     */
    void sendChangeCipherSpec(Finished mesg) throws IOException {
	output.flush();	// i.e. handshake data

	/*
	 * The write cipher state is protected by the connection write lock
	 * so we must grab it while making the change. We also
	 * make sure no writes occur between sending the ChangeCipherSpec
	 * message, installing the new cipher state, and sending the
	 * Finished message.
	 */
	synchronized (conn.writeLock) {
	    OutputRecord r = new OutputRecord(Record.ct_change_cipher_spec);
	    r.setVersion(protocolVersion);
	    r.write(1);	// single byte of data
	    conn.writeRecord(r);
	    conn.changeWriteCiphers();
	    if (debug != null && Debug.isOn("handshake")) {
		mesg.print(System.out);
	    }
	    mesg.write(output);
	    output.flush();
	}
    }


    /*
     * Single access point to key calculation logic.  Given the
     * pre-master secret and the nonces from client and server,
     * produce all the keying material to be used.
     */
    void calculateKeys(byte[] preMasterSecret) {
	byte[] master = calculateMasterSecret(preMasterSecret);
	session.setMasterSecret(master);
	calculateConnectionKeys(master);
    }


    /*
     * Calculate the master secret from its various components.  This is
     * used for key exchange by all cipher suites.
     *
     * The master secret is the catenation of three MD5 hashes, each
     * consisting of the pre-master secret and a SHA1 hash.  Those three
     * SHA1 hashes are of (different) constant strings, the pre-master
     * secret, and the nonces provided by the client and the server.
     */
    private byte[] calculateMasterSecret(byte[] preMasterSecret) {
	byte[] master = new byte[3 * 16];

	if (debug != null && Debug.isOn("keygen")) {
	    try {
		HexDumpEncoder	dump = new HexDumpEncoder();

		System.out.println("SESSION KEYGEN:");

		System.out.println("PreMaster Secret:");
		dump.encodeBuffer(preMasterSecret, System.out);

		// Nonces are dumped with connection keygen, no
		// benefit to doing it twice

	    } catch (IOException e) {
		// ignore
	    }
	}

	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    // TLS
	    doPRF(preMasterSecret, PRF.LABEL_MASTER_SECRET, master);
	} else {
	    // SSL
	    for (int i = 0; i < 3; i++) {
		byte b = (byte)(65 + i);	// 'A', 'B', 'C'
		for (int j = 0; j <= i; j++) {
		    shaTmp.update(b);
		}
		shaTmp.update(preMasterSecret);
		shaTmp.update(clnt_random.random_bytes);
		shaTmp.update(svr_random.random_bytes);

		md5Tmp.update(preMasterSecret);
		md5Tmp.update(shaTmp.digest());

		System.arraycopy(md5Tmp.digest(), 0, master, 16 * i, 16);
	    }
	}

	return master;
    }

    // TLS PRF for master secret derivation, etc.
    private void doPRF(byte[] secret, byte[] label, byte[] output) {
	PRF.compute(md5Tmp, shaTmp, secret, label, clnt_random.random_bytes,
		    svr_random.random_bytes, output);
    }

    // TLS PRF for key expansion. Uses the randoms in a different order for
    // some odd reason.
    private void doKeyExpansionPRF(byte[] secret, byte[] label, byte[] output) {
	PRF.compute(md5Tmp, shaTmp, secret, label, svr_random.random_bytes,
		    clnt_random.random_bytes, output);
    }


    /*
     * Calculate the keys needed for this connection, once the session's
     * master secret has been calculated.  Uses the master key and nonces;
     * the amount of keying material generated is a function of the cipher
     * suite that's been negotiated.
     *
     * This gets called both on the "full handshake" (where we exchanged
     * a premaster secret and started a new session) as well as on the
     * "fast handshake" (where we just resumed a pre-existing session).
     */
    void calculateConnectionKeys(byte[] master) {
	/*
	 * For both the read and write sides of the protocol, we use the
	 * master to generate MAC secrets and cipher keying material.  Block
	 * ciphers need initialization vectors, which we also generate.
	 *
	 * First we figure out how much keying material is needed.
	 */
	int hashSize = cipherSuite.macAlg.size;
	boolean is_exportable = cipherSuite.exportable;
	BulkCipher cipher = cipherSuite.cipher;
	int keySize = cipher.keySize;
	int ivSize = cipher.ivSize;

	int keyBlockLen = hashSize + keySize + (is_exportable ? 0 : ivSize);
	keyBlockLen *= 2;
	byte[] keyBlock = new byte[keyBlockLen];


	/*
	 * Then we iteratively generate the keying material.
	 */
	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    // TLS
	    doKeyExpansionPRF(master, PRF.LABEL_KEY_EXPANSION, keyBlock);
	} else {
	    // SSL

	    for (int i = 0, remaining = keyBlockLen;
		 remaining > 0;
		 i++, remaining -= 16) {

		byte b = (byte)(65 + i);	// 'A', 'B', ...
		for (int j = 0; j <= i; j++) {
		    shaTmp.update(b);
		}

		shaTmp.update(master);
		shaTmp.update(svr_random.random_bytes);
		shaTmp.update(clnt_random.random_bytes);

		md5Tmp.update(master);
		md5Tmp.update(shaTmp.digest());

		System.arraycopy(md5Tmp.digest(), 0, keyBlock, i * 16,
				  Math.min(remaining, 16));

	    }
	}

	/*
	 * First, take the client and server MAC write secrets out of the
	 * key block, to initialize the read and write MAC objects.
	 */
	clntMacSecret = new byte [hashSize];
	svrMacSecret = new byte [hashSize];

	System.arraycopy(keyBlock, 0, clntMacSecret, 0, hashSize);
	System.arraycopy(keyBlock, hashSize, svrMacSecret, 0, hashSize);

	/*
	 * Then take the client and server write keys, which will be used
	 * when initializing the cipher boxes.
	 */
	clntWriteKey = new byte [keySize];
	svrWriteKey = new byte [keySize];

	System.arraycopy(keyBlock, (2 * hashSize),
		clntWriteKey, 0, keySize);
	System.arraycopy(keyBlock, (2 * hashSize) + keySize,
		svrWriteKey, 0, keySize);

	/*
	 * Finish by taking data to initialize block cipher boxes for client
	 * and server.  Note that exportable ciphers have special rules for
	 * generating these init vectors; no secrets are used.
	 */
	if (ivSize != 0) {
	    clntWriteIV = new byte [ivSize];
	    svrWriteIV = new byte [ivSize];

	    if (!is_exportable) {
		System.arraycopy(keyBlock,
			2 * (hashSize + keySize),
			clntWriteIV, 0, ivSize);
		System.arraycopy(keyBlock,
			(2 * (hashSize + keySize)) + ivSize,
			svrWriteIV, 0, ivSize);
	    }
	} else {
	    clntWriteIV = null;
	    svrWriteIV = null;
	}

	/*
	 * Exportable cipher suites require special handling for both
	 * keying data and initialization vectors.
	 */
	if (is_exportable) {
	    int expandedKeySize = cipher.expandedKeySize;
	    if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
		// TLS
		byte[] finalClntWriteKey = new byte[expandedKeySize];
		doPRF(clntWriteKey, PRF.LABEL_CLIENT_WRITE_KEY,
			    finalClntWriteKey);
		clntWriteKey = finalClntWriteKey;

		byte[] finalSvrWriteKey = new byte[expandedKeySize];
		doPRF(svrWriteKey, PRF.LABEL_SERVER_WRITE_KEY,
			    finalSvrWriteKey);
		svrWriteKey = finalSvrWriteKey;

		if (ivSize != 0) {
		    byte[] ivBlock = new byte[2 * ivSize];
		    doPRF(null, PRF.LABEL_IV_BLOCK, ivBlock);
		    System.arraycopy(ivBlock, 0, clntWriteIV, 0, ivSize);
		    System.arraycopy(ivBlock, ivSize, svrWriteIV, 0, ivSize);
		}
	    } else {
		// SSL
		/*
		 * Postprocess client and server write keys ... we only
		 * had a minimal input of "secret" data (e.g. 40 bits)
		 * and here's where we expand it to the full amount of
		 * data used by the particular cipher used.  (At least,
		 * so long as that's no more than the 128 bits produced
		 * by MD5 !!)
		 */
		md5Tmp.update(clntWriteKey);
		md5Tmp.update(clnt_random.random_bytes);
		md5Tmp.update(svr_random.random_bytes);
		clntWriteKey = new byte[expandedKeySize];
		System.arraycopy(md5Tmp.digest(), 0, clntWriteKey, 0,
				  expandedKeySize);

		md5Tmp.update(svrWriteKey);
		md5Tmp.update(svr_random.random_bytes);
		md5Tmp.update(clnt_random.random_bytes);
		svrWriteKey = new byte[expandedKeySize];
		System.arraycopy(md5Tmp.digest(), 0, svrWriteKey, 0,
				  expandedKeySize);

		/*
		 * Maybe create client and server write IVs, using only
		 * the public nonces rather than any secret-derived data.
		 */
		if (ivSize != 0) {
		    md5Tmp.update(clnt_random.random_bytes);
		    md5Tmp.update(svr_random.random_bytes);
		    System.arraycopy(md5Tmp.digest(),
				      0, clntWriteIV, 0, ivSize);

		    md5Tmp.update(svr_random.random_bytes);
		    md5Tmp.update(clnt_random.random_bytes);
		    System.arraycopy(md5Tmp.digest(),
				      0, svrWriteIV, 0, ivSize);
		}
	    }
	}

	//
	// Dump the connection keys as they're generated.
	//
	if (debug != null && Debug.isOn("keygen")) {
	    try {
		HexDumpEncoder	dump = new HexDumpEncoder();

		System.out.println("CONNECTION KEYGEN:");

		// Inputs:
		System.out.println("Client Nonce:");
		dump.encodeBuffer(clnt_random.random_bytes, System.out);
		System.out.println("Server Nonce:");
		dump.encodeBuffer(svr_random.random_bytes, System.out);
		System.out.println("Master Secret:");
		dump.encodeBuffer(master, System.out);

		// Outputs:
		System.out.println("Client MAC write Secret:");
		dump.encodeBuffer(clntMacSecret, System.out);
		System.out.println("Server MAC write Secret:");
		dump.encodeBuffer(svrMacSecret, System.out);

		System.out.println("Client write key:");
		dump.encodeBuffer(clntWriteKey, System.out);
		System.out.println("Server write key:");
		dump.encodeBuffer(svrWriteKey, System.out);

		if (clntWriteIV != null) {
		    System.out.println("Client write IV:");
		    dump.encodeBuffer(clntWriteIV, System.out);
		    System.out.println("Server write IV:");
		    dump.encodeBuffer(svrWriteIV, System.out);
		} else
		    System.out.println("... no IV for cipher");

	    } catch (IOException e) {
		// just for debugging, ignore this
	    }
	}
    }

    /**
     * Throw an SSLException with the specified message and cause.
     * Shorthand until a new SSLException constructor is added.
     * This method never returns.
     */
    static void throwSSLException(String msg, Throwable cause)
	    throws SSLException {
	SSLException e = new SSLException(msg);
	e.initCause(cause);
	throw e;
    }

}
