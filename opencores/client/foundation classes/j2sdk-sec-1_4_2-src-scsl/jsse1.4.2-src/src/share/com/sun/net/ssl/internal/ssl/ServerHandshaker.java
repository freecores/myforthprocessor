/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.util.*;
import java.security.*;
import java.security.cert.*;
import java.security.interfaces.*;

import javax.net.ssl.*;

import com.sun.net.ssl.internal.ssl.HandshakeMessage.*;
import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * ServerHandshaker does the protocol handshaking from the point
 * of view of a server.  It is driven asychronously by handshake messages
 * as delivered by the parent Handshaker class, and also uses
 * common functionality (e.g. key generation) that is provided there.
 *
 * @version 1.121 06/24/03
 * @author David Brownell
 */
final class ServerHandshaker extends Handshaker {

    // is the server going to require the client to authenticate?
    private byte		doClientAuth;

    // our authentication info
    private X509Certificate[]	certs;
    private PrivateKey		privateKey;

    // flag to check for clientCertificateVerify message
    private boolean		needClientVerify = false;

    /*
     * For exportable ciphersuites using non-exportable key sizes, we use 
     * ephemeral RSA keys. We could also do anonymous RSA in the same way
     * but there are no such ciphersuites currently defined.
     */
    private PrivateKey		tempPrivateKey;
    private PublicKey	        tempPublicKey;

    /*
     * For anonymous and ephemeral Diffie-Hellman key exchange, we use
     * ephemeral Diffie-Hellman keys.
     */
    private DHKeyExchange dh;
    
    // version request by the client in its ClientHello
    // we remember it for the RSA premaster secret version check
    private ProtocolVersion clientRequestedVersion;
    
    /*
     * Constructor ... use the keys found in the auth context.
     */
    ServerHandshaker(SSLSocketImpl c, SSLContextImpl context, 
	    ProtocolList enabledProtocols, byte clientAuth) {
	super(c, context, enabledProtocols, 
			(clientAuth != SSLSocketImpl.clauth_none), false);
	doClientAuth = clientAuth;
    }

    /*
     * As long as handshaking has not started, we can change
     * whether client authentication is required.  Otherwise,
     * we will need to wait for the next handshake.
     */
    void setClientAuth(byte clientAuth) {
	doClientAuth = clientAuth;
    }

    /*
     * Get some "ephemeral" RSA keys for this context. This means
     * generating them if it's not already been done. 
     *
     * Note that we currently do not implement any ciphersuites that use
     * strong ephemeral RSA. (We do not support the EXPORT1024 ciphersuites
     * and standard RSA ciphersuites prohibit ephemeral mode for some reason)
     * This means that export is always true and 512 bit keys are generated.
     */
    private boolean getEphemeralRSAKeys(boolean export) {
	KeyPair kp = sslContext.getEphemeralKeyManager().
			getRSAKeyPair(export, sslContext.getSecureRandom());
	if (kp == null) {
	    return false;
	} else {
	    tempPublicKey = (RSAPublicKey)kp.getPublic();
	    tempPrivateKey = kp.getPrivate();
	    return true;
	}
    }
    
    /*
     * Acquire some "ephemeral" Diffie-Hellman  keys for this handshake.
     * We don't reuse these, for improved forward secrecy.
     */
    private void getEphemeralDHKeys(boolean export) {
	/*
	 * Diffie-Hellman keys ... we use 768 bit private keys due
	 * to the "use twice as many key bits as bits you want secret"
	 * rule of thumb, assuming we want the same size premaster
	 * secret with Diffie-Hellman and RSA key exchanges.  Except
	 * that exportable ciphers max out at 512 bits modulus values.
	 */
	boolean useStrong = hasStrongCrypto && !export;
	dh = new DHKeyExchange(useStrong);
	dh.generateKeyPair(sslContext.getSecureRandom(), useStrong ? 768 : 512);
    }


    /*
     * This routine handles all the server side handshake messages, one at
     * a time.  Given the message type (and in some cases the pending cipher
     * spec) it parses the type-specific message.  Then it calls a function
     * that handles that specific message.
     *
     * It updates the state machine as each message is processed, and writes
     * responses as needed using the connection in the constructor.
     */
    void processMessage(byte type, int message_len)
	    throws IOException {
	//
	// In SSLv3 and TLS, messages follow strictly increasing
	// numerical order _except_ for one annoying special case.
	//
	if ((state > type)
		&& (state != HandshakeMessage.ht_client_key_exchange
		    && type != HandshakeMessage.ht_certificate_verify)) {
	    throw new SSLProtocolException(
		    "Handshake message sequence violation, state = " + state
		    + ", type = " + type);
	}

	switch (type) {
	    case HandshakeMessage.ht_client_hello:
		/*
		 * As per RFC2246:7.4.1.2, for forward compatibility,
		 * we may need to skip over extra stuff in
		 * clientHello that we don't know
		 * how to process yet.  We still need to hash
		 * it properly.
		 *
		 * Mark it, read the data, reset, then skip.
		 */
		input.mark(message_len);
		ClientHello ch = new ClientHello(input);
		input.reset();
		input.skip(message_len);

		/*
		 * send it off for processing.
		 */
		this.clientHello(ch);
		break;

	    case HandshakeMessage.ht_certificate:
		if (doClientAuth == SSLSocketImpl.clauth_none) {
		    conn.fatal(SSLSocketImpl.alert_unexpected_message,
				"client sent unsolicited cert chain");
		    // NOTREACHED
		}
		this.clientCertificate(new CertificateMsg(input));
		break;

	    case HandshakeMessage.ht_client_key_exchange:
		byte preMasterSecret[];
		if ((keyExchange == K_RSA) || (keyExchange == K_RSA_EXPORT)) {
		    /*
		     * The client's pre-master secret is decrypted using
		     * either the server's normal private RSA key, or the
		     * temporary one used for non-export or signing-only
		     * certificates/keys.
		     */
		    PreMasterSecret pms = new PreMasterSecret
			(protocolVersion, clientRequestedVersion,
			sslContext.getSecureRandom(), input, 
			message_len, privateKey);
		    preMasterSecret = this.clientKeyExchange(pms);
		} else { // must be DH
		    /*
		     * The pre-master secret is derived using the normal
		     * Diffie-Hellman calculation.   Note that the main
		     * protocol difference in these five flavors is in how
		     * the ServerKeyExchange message was constructed!
		     */
		    preMasterSecret = this.clientKeyExchange(
			    new ClientDiffieHellmanPublic(input));
		}

		//
		// All keys are calculated from the premaster secret
		// and the exchanged nonces in the same way; then we
		// obliterate this premaster secret, and keep only
		// the master secret we calculated.  This helps lower
		// risks associated with a server or client breakin.
		//
		calculateKeys(preMasterSecret);
		Arrays.fill(preMasterSecret, (byte)0);
		break;

	    case HandshakeMessage.ht_certificate_verify:
		this.clientCertificateVerify(new CertificateVerify(input));
		break;

	    case HandshakeMessage.ht_finished:
		this.clientFinished(new Finished(protocolVersion, input));
		break;

	    default:
		throw new SSLProtocolException(
			"Illegal server handshake msg, " + type);
	}

	//
	// Move the state machine forward except for that annoying
	// special case.  This means that clients could send extra
	// cert verify messages; not a problem so long as all of
	// them actually check out.
	//
	if (state < type && type != HandshakeMessage.ht_certificate_verify) {
	    state = type;
	}
    }


    /*
     * ClientHello presents the server with a bunch of options, to which the
     * server replies with a ServerHello listing the ones which this session
     * will use.  If needed, it also writes its Certificate plus in some cases
     * a ServerKeyExchange message.  It may also write a CertificateRequest,
     * to elicit a client certificate.
     *
     * All these messages are terminated by a ServerHelloDone message.  In
     * most cases, all this can be sent in a single Record.
     */
    private void clientHello(ClientHello mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	/*
	 * Always make sure this entire record has been digested before we
	 * start emitting output, to ensure correct digesting order.
	 */
	input.digestNow();

	/*
	 * FIRST, construct the ServerHello using the options and priorities
	 * from the ClientHello.  Update the (pending) cipher spec as we do
	 * so, and save the client's version to protect against rollback
	 * attacks.
	 *
	 * There are a bunch of minor tasks here, and one major one: deciding
	 * if the short or the full handshake sequence will be used.
	 */
	ServerHello m1 = new ServerHello();
	
	clientRequestedVersion = mesg.protocolVersion;
	
	// check if clientVersion is recent enough for us
	if (clientRequestedVersion.v < enabledProtocols.min.v) {
	    throw new SSLHandshakeException
	        ("Client requested protocol " + clientRequestedVersion +
		 " not enabled or not supported");
	}
	
	// now we know we have an acceptable version
	// use the lower of our max and the client requested version
	ProtocolVersion selectedVersion;
	if (clientRequestedVersion.v <= enabledProtocols.max.v) {
	    selectedVersion = clientRequestedVersion;
	} else {
	    selectedVersion = enabledProtocols.max;
	}
	setVersion(selectedVersion);

	m1.protocolVersion = protocolVersion;

	//
	// random ... save client and server values for later use
	// in computing the master secret (from pre-master secret)
	// and thence the other crypto keys.
	//
	// NOTE:  this use of three inputs to generating _each_ set
	// of ciphers slows things down, but it does increase the
	// security since each connection in the session can hold
	// its own authenticated (and strong) keys.  One could make
	// creation of a session a rare thing...
	//
	clnt_random = mesg.clnt_random;
	svr_random = new RandomCookie(sslContext.getSecureRandom());
	m1.svr_random = svr_random;

	session = null; // forget about the current session
	//
	// Here we go down either of two paths:  (a) the fast one, where
	// the client's asked to rejoin an existing session, and the server
	// permits this; (b) the other one, where a new session is created.
	//
	if (mesg.sessionId.length() != 0) {
	    // client is trying to resume a session, let's see...

	    SSLSessionImpl previous = ((SSLSessionContextImpl)sslContext
			.engineGetServerSessionContext())
			.get(mesg.sessionId.getId());
	    //
	    // Check if we can use the fast path, resuming a session.  We
	    // can do so iff we have a valid record for that session, and
	    // the cipher suite for that session was on the list which the
	    // client requested, and if we're not forgetting any needed
	    // authentication on the part of the client.
	    //
	    if (previous != null) {
		resumingSession = previous.isRejoinable();

		if (resumingSession) {
		    ProtocolVersion oldVersion = previous.getProtocolVersion();
		    // cannot resume session with different version
		    if (oldVersion != protocolVersion) {
			resumingSession = false;
		    }
		}

		if (resumingSession &&
			(doClientAuth != SSLSocketImpl.clauth_none)) {
		    try {
			previous.getPeerCertificates();
		    } catch (SSLPeerUnverifiedException e) {
			resumingSession = false;
		    }
		}

		if (resumingSession) {
		    CipherSuite suite = previous.getSuite();
		    // verify that the ciphersuite from the cached session
		    // is in the list of client requested ciphersuites and
		    // we have it enabled
		    if ((isEnabled(suite) == false) ||
		    	    (mesg.cipherSuites.contains(suite) == false)) {
			resumingSession = false;
		    } else {
			// everything looks ok, set the ciphersuite
			// this should be done last when we are sure we
			// will resume
			setCipherSuite(suite);
		    }
		}
		
		if (resumingSession) {
		    session = previous;
		    if (debug != null &&
			(Debug.isOn("handshake") || Debug.isOn("session"))) {
			System.out.println("%% Resuming " + session);
		    }
		}
	    }
	} // else client did not try to resume

	//
	// If client hasn't specified a session we can resume, start a
	// new one and choose its cipher suite and compression options.
	// Unless new session creation is disabled for this connection!
	//
	if (session == null) {
	    if (!enableNewSession) {
		throw new SSLException("Client did not resume a session");
	    }
	    chooseCipherSuite(mesg);
	    session = new SSLSessionImpl(protocolVersion, cipherSuite, 
	    	sslContext.getSecureRandom(),
		conn.getInetAddress().getHostAddress(), conn.getPort());
	    // chooseCompression(mesg);
	}

	m1.cipherSuite = cipherSuite;
	m1.sessionId = session.getSessionId();
	m1.compression_method = session.getCompression();

	if (debug != null && Debug.isOn("handshake")) {
	    m1.print(System.out);
	    System.out.println("Cipher suite:  " + session.getSuite());
	}
	m1.write(output);

	//
	// If we are resuming a session, we finish writing handshake
	// messages right now and then finish.
	//
	if (resumingSession) {
	    calculateConnectionKeys(session.getMasterSecret());
	    sendChangeCipherAndFinish(true);
	    return;
	}


	/*
	 * SECOND, write the server Certificate(s) if we need to.
	 *
	 * NOTE:  while an "anonymous RSA" mode is explicitly allowed by
	 * the protocol, we can't support it since all of the SSL flavors
	 * defined in the protocol spec are explicitly stated to require
	 * using RSA certificates.
	 */
	if (keyExchange != K_DH_ANON) {
	    if (certs == null) {
		throw new RuntimeException("no certificates");
	    }

	    CertificateMsg m2 = new CertificateMsg(certs);

	    /*
	     * Set local certs in the SSLSession, output
	     * debug info, and then actually write to the client.
	     */
	    session.setLocalCertificates(certs);
	    if (debug != null && Debug.isOn("handshake")) {
		m2.print(System.out);
	    }
	    m2.write(output);
	    
	    // XXX has some side effects with OS TCP buffering,
	    // leave it out for now
	    
	    // let client verify chain in the meantime...
	    // output.flush();
	} else {
	    if (certs != null) {
		throw new RuntimeException("anonymous keyexchange with certs");
	    }
	}

	/*
	 * THIRD, the ServerKeyExchange message ... iff it's needed.
	 *
	 * It's usually needed unless there's an encryption-capable
	 * RSA cert, or a D-H cert.  The notable exception is that
	 * exportable ciphers used with big RSA keys need to downgrade
	 * to use short RSA keys, even when the key/cert encrypts OK.
	 */

	ServerKeyExchange m3;
	if (keyExchange == K_RSA) {
	    // no server key exchange
	    m3 = null;
	} else if (keyExchange == K_RSA_EXPORT) {
	    if (((RSAPrivateKey)privateKey).getModulus().bitLength() > 512) {
		try {
		    m3 = new RSA_ServerKeyExchange(
			tempPublicKey, privateKey,
			clnt_random, svr_random,
			sslContext.getSecureRandom());
		    privateKey = tempPrivateKey;
		} catch (GeneralSecurityException e) {
		    throwSSLException
		    	("Error generating RSA server key exchange", e);
		    m3 = null; // make compiler happy
		}
	    } else {
		// RSA_EXPORT with short key, don't need ServerKeyExchange
		m3 = null;
	    }
	} else if ((keyExchange == K_DHE_DSS) || (keyExchange == K_DHE_RSA)) {
	    try {
		m3 = new DH_ServerKeyExchange(dh,
		    privateKey,
		    clnt_random.random_bytes,
		    svr_random.random_bytes,
		    sslContext.getSecureRandom());
	    } catch (GeneralSecurityException e) {
		throwSSLException("Error generating DH server key exchange", e);
	        m3 = null; // make compiler happy
	    }
	} else if (keyExchange == K_DH_ANON) {
	    m3 = new DH_ServerKeyExchange(dh);
	} else {
	    throw new RuntimeException("internal error: " + keyExchange);
	}
	if (m3 != null) {
	    if (debug != null && Debug.isOn("handshake")) {
		m3.print(System.out);
	    }
	    m3.write(output);
	}

	//
	// FOURTH, the CertificateRequest message.  The details of
	// the message can be affected by the key exchange algorithm
	// in use.  For example, certs with fixed Diffie-Hellman keys
	// are only useful with the DH_DSS and DH_RSA key exchange
	// algorithms.
	//
	// Needed only if server requires client to authenticate self.
	// Illegal for anonymous flavors, so we need to check that.
	//
	if (doClientAuth != SSLSocketImpl.clauth_none &&
		keyExchange != K_DH_ANON) {
	    CertificateRequest m4;
	    X509Certificate caCerts[];

	    caCerts = sslContext.getX509TrustManager().getAcceptedIssuers();
	    m4 = new CertificateRequest(caCerts, keyExchange);

	    if (debug != null && Debug.isOn("handshake")) {
		m4.print(System.out);
	    }
	    m4.write(output);
	}

	/*
	 * FIFTH, say ServerHelloDone.
	 */
	ServerHelloDone m5 = new ServerHelloDone();

	if (debug != null && Debug.isOn("handshake")) {
	    m5.print(System.out);
	}
	m5.write(output);

	/*
	 * Flush any buffered messages so the client will see them.
	 * Ideally, all the messages above go in a single network level
	 * message to the client.  Without big Certificate chains, it's
	 * going to be the common case.
	 */
	output.flush();
    }

    /*
     * Choose cipher suite from among those supported by client. Sets
     * the cipherSuite and keyExchange variables.
     */
    private void chooseCipherSuite(ClientHello mesg) throws IOException {
	for (Iterator t = mesg.cipherSuites.iterator(); t.hasNext(); ) {
	    CipherSuite suite = (CipherSuite)t.next();
	    if (isEnabled(suite) == false) {
		continue;
	    }
	    if ((doClientAuth == SSLSocketImpl.clauth_required) &&
		    (suite.keyExchange == K_DH_ANON)) {
		continue;
	    }
	    if (trySetCipherSuite(suite) == false) {
		continue;
	    }
	    return;
	}
	conn.fatal(SSLSocketImpl.alert_handshake_failure,
		    "no cipher suites in common");
    }

    /**
     * Set the given CipherSuite, if possible. Return the result.
     * The call succeeds if the CipherSuite is available and we have
     * the necessary certificates to complete the handshake. We don't
     * check if the CipherSuite is actually enabled.
     *
     * If successful, this method also generates ephemeral keys if
     * required for this ciphersuite. This may take some time, so this
     * method should only be called if you really want to use the
     * CipherSuite.
     *
     * This method is called from chooseCipherSuite() in this class
     * and SSLServerSocketImpl.checkEnabledSuites() (as a sanity check).
     */
    boolean trySetCipherSuite(CipherSuite suite) {
	/*
	 * If we're resuming a session we know we can
	 * support this key exchange algorithm and in fact
	 * have already cached the result of it in
	 * the session state.
	 */
	if (resumingSession) {
	    return true;
	}
	
	if (suite.isAvailable() == false) {
	    return false;
	}
	
	KeyExchange keyExchange = suite.keyExchange;
	
	// null out any existing references
	privateKey = null;
	certs = null;
	dh = null;
	tempPrivateKey = null;
	tempPublicKey = null;
	
	if ((keyExchange == K_RSA) || (keyExchange == K_RSA_EXPORT)
		|| (keyExchange == K_DHE_RSA)) {
	    // need RSA certs for authentication
	    if (getPrivateKeyAndChain("RSA") == false) {
		return false;
	    }
	    
	    if (keyExchange == K_RSA_EXPORT) {
		PublicKey publKey = certs[0].getPublicKey();
		if (!(publKey instanceof RSAPublicKey)) {
		    return false;
		}
		RSAPublicKey rsaKey = (RSAPublicKey)publKey;	
		if (rsaKey.getModulus().bitLength() > 512) {
		    if (!getEphemeralRSAKeys(suite.exportable)) {
			return false;
		    }
		}
	    } else if (keyExchange == K_DHE_RSA) {
		getEphemeralDHKeys(suite.exportable);
	    } // else nothing more to do for K_RSA 
	} else if (keyExchange == K_DHE_DSS) {
	    // need DSS certs for authentication
	    if (getPrivateKeyAndChain("DSA") == false) {
		return false;
	    }
	    getEphemeralDHKeys(suite.exportable);
	} else if (keyExchange == K_DH_ANON) {
	    // no certs needed for anonymous
	    getEphemeralDHKeys(suite.exportable);
	} else {
	    // internal error, unknown key exchange
	    throw new RuntimeException("CipherSuite: " + suite);
	}
	setCipherSuite(suite);
	return true;
    }
    
    /**
     * Retrieve the server key and certificate for the specified algorithm
     * from the KeyManager and set the instance variables.
     * 
     * @return true if successful, false if not available or invalid
     */
    private boolean getPrivateKeyAndChain(String algorithm) {
	X509KeyManager km = sslContext.getX509KeyManager();
	String alias = km.chooseServerAlias(algorithm, null, conn);
	if (alias == null) {
	    return false;
	}
	PrivateKey tempPrivateKey = km.getPrivateKey(alias);
	if (tempPrivateKey == null) {
	    return false;
	}
	X509Certificate[] tempCerts = km.getCertificateChain(alias);
	if ((tempCerts == null) || (tempCerts.length == 0)) {
	    return false;
	}
	PublicKey publicKey = tempCerts[0].getPublicKey();
	if ((tempPrivateKey.getAlgorithm().equals(algorithm) == false)
		|| (publicKey.getAlgorithm().equals(algorithm) == false)) {
	    return false;
	}
	this.privateKey = tempPrivateKey;
	this.certs = tempCerts;
	return true;
    }

    /*
     * Diffie Hellman key exchange is used when the server presented
     * D-H parameters in its certificate (signed using RSA or DSS/DSA),
     * or else the server presented no certificate but sent D-H params
     * in a ServerKeyExchange message.  Use of D-H is specified by the
     * cipher suite chosen.
     *
     * The message optionally contains the client's D-H public key (if
     * it wasn't not sent in a client certificate).  As always with D-H,
     * if a client and a server have each other's D-H public keys and
     * they use common algorithm parameters, they have a shared key
     * that's derived via the D-H calculation.  That key becomes the
     * pre-master secret.
     */
    private byte[] clientKeyExchange(ClientDiffieHellmanPublic mesg)
	    throws IOException {
	    
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	return dh.getAgreedSecret(mesg.getClientPublicKey());
    }

    /*
     * Client wrote a message to verify the certificate it sent earlier.
     *
     * Note that this certificate isn't involved in key exchange.  Client
     * authentication messages are included in the checksums used to
     * validate the handshake (e.g. Finished messages).  Other than that,
     * the _exact_ identity of the client is less fundamental to protocol
     * security than its role in selecting keys via the pre-master secret.
     */
    private void clientCertificateVerify(CertificateVerify mesg)
	    throws IOException {

	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}

	try {
	    PublicKey publicKey =
		session.getPeerCertificates()[0].getPublicKey();
		
	    boolean valid = mesg.verify(protocolVersion, handshakeHash, 
		    			publicKey, session.getMasterSecret());
	    if (valid == false) {
		conn.fatal(SSLSocketImpl.alert_bad_certificate,
			    "certificate verify message signature error");
	    }
	} catch (GeneralSecurityException e) {
	    conn.fatal(SSLSocketImpl.alert_bad_certificate,
	        "certificate verify format error", e);
	}

	// reset the flag for clientCertificateVerify message
	needClientVerify = false;
    }


    /*
     * Client writes "finished" at the end of its handshake, after cipher
     * spec is changed.   We verify it and then send ours.
     *
     * When we're resuming a session, we'll have already sent our own
     * Finished message so just the verification is needed.
     */
    private void clientFinished(Finished mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	/*
	 * Verify if client did send the certificate when client
	 * authentication was required, otherwise server should not proceed
	 */
	if (doClientAuth == SSLSocketImpl.clauth_required) {
	   session.getPeerCertificates();
	}

	/*
	 * Verify if client did send clientCertificateVerify message following
	 * the client Certificate, otherwise server should not proceed
	 */
	if (needClientVerify) {
		conn.fatal(SSLSocketImpl.alert_handshake_failure,
			"client did not send certificate verify message");
	}

	/*
	 * Verify the client's message with the "before" digest of messages,
	 * and forget about continuing to use that digest.
	 */
	boolean verified = mesg.verify(protocolVersion, handshakeHash, 
				Finished.CLIENT, session.getMasterSecret());

	if (!verified) {
	    conn.fatal(SSLSocketImpl.alert_handshake_failure,
			"client 'finished' message doesn't verify");
	    // NOTREACHED
	}

	/*
	 * OK, it verified.  If we're doing the full handshake, add that
	 * "Finished" message to the hash of handshake messages, then send
	 * the change_cipher_spec and Finished message.
	 */
	if (!resumingSession) {
	    input.digestNow();
	    sendChangeCipherAndFinish(false);
	}

	/*
	 * Update the session cache only after the handshake completed, else
	 * we're open to an attack against a partially completed handshake.
	 */
	session.setLastAccessedTime(System.currentTimeMillis());
	if (!resumingSession && session.isRejoinable()) {
	    ((SSLSessionContextImpl)sslContext.engineGetServerSessionContext())
		.put(session);
	    if (debug != null && Debug.isOn("session")) {
		System.out.println(
		    "%% Cached server session: " + session);
	    }
	} else if (!resumingSession &&
		debug != null && Debug.isOn("session")) {
	    System.out.println(
		"%% Didn't cache non-resumable server session: "
		+ session);
	}
    }

    /*
     * Compute finished message with the "server" digest (and then forget
     * about that digest, it can't be used again).
     */
    private void sendChangeCipherAndFinish(boolean clientHello)
	    throws IOException {

	output.flush();

	Finished mesg = new Finished(protocolVersion, handshakeHash, 
				Finished.SERVER, session.getMasterSecret());

	/*
	 * Send the change_cipher_spec record; then our Finished handshake
	 * message will be the last handshake message.  Flush, and now we
	 * are ready for application data!!
	 */
	sendChangeCipherSpec(mesg);

	/*
	 * Update state machine so client MUST send 'finished' next
	 * The update should only take place if it is not in the fast
	 * handshake mode since the server has to wait for a finished
	 * message from the client
	 */
	if (!(clientHello && resumingSession)) {
	    state = HandshakeMessage.ht_finished;
	}
    }


    /*
     * Returns a HelloRequest message to kickstart renegotiations
     */
    HandshakeMessage getKickstartMessage() {
	return new HelloRequest();
    }


    /*
     * Fault detected during handshake.
     */
    void handshakeAlert(byte description) throws SSLProtocolException {

	String message = SSLSocketImpl.alertDescription(description);

	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("SSL -- handshake alert:  "
		+ message);
	}

	/*
	 * It's ok to get a no_certificate alert from a client of which
	 * we *requested* authentication information.
	 * However, if we *required* it, then this is not acceptable.
	 *
	 * Anyone calling getPeerCertificates() on the
	 * session will get an SSLPeerUnverifiedException.
	 */
	if ((description == SSLSocketImpl.alert_no_certificate) &&
		(doClientAuth == SSLSocketImpl.clauth_requested)) {
	    return;
	}

	throw new SSLProtocolException("handshake alert: " + message);
    }

    /*
     * RSA key exchange is normally used.  The client encrypts a "pre-master
     * secret" with the server's public key, from the Certificate (or else
     * ServerKeyExchange) message that was sent to it by the server.  That's
     * decrypted using the private key before we get here.
     */
    private byte[] clientKeyExchange(PreMasterSecret mesg) throws IOException {

	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	return mesg.preMaster;
    }

    /*
     * Verify the certificate sent by the client. We'll only get one if we
     * sent a CertificateRequest to request client authentication. If we
     * are in TLS mode, the client may send a message with no certificates
     * to indicate it does not have an appropriate chain. (In SSLv3 mode,
     * it would send a no certificate alert).
     */
    private void clientCertificate(CertificateMsg mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}

	X509Certificate[] peerCerts = mesg.getCertificateChain();

	if (peerCerts.length == 0) {
	    /*
	     * If the client authentication is only *REQUESTED* (e.g.
	     * not *REQUIRED*, this is an acceptable condition.
	     */
	    if (doClientAuth == SSLSocketImpl.clauth_requested) {
		return;
	    } else {
		conn.fatal(SSLSocketImpl.alert_bad_certificate,
		    "null cert chain");
	    }
	}
	
	// ask the trust manager to verify the chain
	X509TrustManager tm = sslContext.getX509TrustManager();

	try {	  
	    // find out the types of client authentication used
	    PublicKey key = peerCerts[0].getPublicKey();
	    String authType;
	    if (key instanceof RSAPublicKey) {
		authType = "RSA";
	    } else if (key instanceof DSAPublicKey) {
		authType = "DSA";
	    } else {
		// unknown public key type
		authType = "UNKNOWN";
	    }
	    tm.checkClientTrusted(peerCerts, authType);
	} catch (CertificateException e) {
	    // This will throw an exception, so include the original error.
	    conn.fatal(SSLSocketImpl.alert_certificate_unknown, e);
	}
	// set the flag for clientCertificateVerify message
	needClientVerify = true;

	session.setPeerCertificates(peerCerts);
    }
}
