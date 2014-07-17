/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.math.BigInteger;
import java.security.*;
import java.util.*;

import javax.net.ssl.*;

import java.security.cert.X509Certificate;
import java.security.cert.CertificateException;

import com.sun.net.ssl.internal.ssl.HandshakeMessage.*;
import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * ClientHandshaker does the protocol handshaking from the point
 * of view of a client.  It is driven asychronously by handshake messages
 * as delivered by the parent Handshaker class, and also uses
 * common functionality (e.g. key generation) that is provided there.
 *
 * @version 1.105 06/24/03
 * @author David Brownell
 */
final class ClientHandshaker extends Handshaker {

    // Public key for key exchange. From server cert or ephemeral.
    private PublicKey		serverKey;

    // if (useDHE_DSS || useDH_anon) {
    private BigInteger		serverDH;
    private DHKeyExchange	dh;
    // }

    private CertificateRequest	certRequest;

    private boolean serverKeyExchangeReceived;

    /*
     * Constructors
     */
    ClientHandshaker(SSLSocketImpl c, SSLContextImpl context, 
	    ProtocolList enabledProtocols) {
	super(c, context, enabledProtocols, true, true);
    }

    /*
     * This routine handles all the client side handshake messages, one at
     * a time.  Given the message type (and in some cases the pending cipher
     * spec) it parses the type-specific message.  Then it calls a function
     * that handles that specific message.
     *
     * It updates the state machine (need to verify it) as each message
     * is processed, and writes responses as needed using the connection
     * in the constructor.
     */
    void processMessage(byte type, int messageLen) throws IOException {
	if (state > type
		&& (type != HandshakeMessage.ht_hello_request
		    && state != HandshakeMessage.ht_client_hello)) {
	    throw new SSLProtocolException(
		    "Handshake message sequence violation, " + type);
	}

	switch (type) {
	case HandshakeMessage.ht_hello_request:
	    this.serverHelloRequest(new HelloRequest(input));
	    break;

	case HandshakeMessage.ht_server_hello:
	    this.serverHello(new ServerHello(input));
	    break;

	case HandshakeMessage.ht_certificate:
	    if (keyExchange == K_DH_ANON) {
		conn.fatal(SSLSocketImpl.alert_unexpected_message,
		    "unexpected server cert chain");
		// NOTREACHED
	    }
	    this.serverCertificate(new CertificateMsg(input));
	    serverKey =
		session.getPeerCertificates()[0].getPublicKey();
	    break;

	case HandshakeMessage.ht_server_key_exchange:
	    serverKeyExchangeReceived = true;
	    if ((keyExchange == K_RSA) || (keyExchange == K_RSA_EXPORT)) {
	        try {
		    this.serverKeyExchange(
		        new RSA_ServerKeyExchange(input, messageLen));
	        } catch (GeneralSecurityException e) {
		  throwSSLException("Server key", e);
	        }
	    } else if (keyExchange == K_DH_ANON) {
		this.serverKeyExchange(new DH_ServerKeyExchange(input));
	    } else if ((keyExchange == K_DHE_DSS) || (keyExchange == K_DHE_RSA)) {
		try {
		    this.serverKeyExchange(new DH_ServerKeyExchange(
			input, serverKey,
			clnt_random.random_bytes, svr_random.random_bytes,
			messageLen));
		} catch (GeneralSecurityException e) {
		    throwSSLException("Server key", e);
		}
	    } else {
	        throw new SSLProtocolException(
		    "unsupported key exchange algorithm = "
		    + keyExchange);
	    }
	    break;

	case HandshakeMessage.ht_certificate_request:
	    // save for later, it's handled by serverHelloDone
	    if (keyExchange == K_DH_ANON) {
		throw new SSLHandshakeException(
		    "Client authentication requested for "+
		    "anonymous cipher suite.");
	    }
	    certRequest = new CertificateRequest(input);
	    if (debug != null && Debug.isOn("handshake")) {
		certRequest.print(System.out);
	    }
	    break;

	case HandshakeMessage.ht_server_hello_done:
	    this.serverHelloDone(new ServerHelloDone(input));
	    break;

	case HandshakeMessage.ht_finished:
	    this.serverFinished(new Finished(protocolVersion, input));
	    break;

	default:
	    throw new SSLProtocolException(
		"Illegal client handshake msg, " + type);
	}

	//
	// Move state machine forward if the message handling
	// code didn't already do so
	//
	if (state < type) {
	    state = type;
	}
    }

    /*
     * Used by the server to kickstart negotiations -- this requests a
     * "client hello" to renegotiate current cipher specs (e.g. maybe lots
     * of data has been encrypted with the same keys, or the server needs
     * the client to present a certificate).
     */
    private void serverHelloRequest(HelloRequest mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}

	//
	// Could be (e.g. at connection setup) that we already
	// sent the "client hello" but the server's not seen it.
	//
	if (state < HandshakeMessage.ht_client_hello) {
	    kickstart();
	}
    }


    /*
     * Server chooses session parameters given options created by the
     * client -- basically, cipher options, session id, and someday a
     * set of compression options.
     *
     * There are two branches of the state machine, decided by the
     * details of this message.  One is the "fast" handshake, where we
     * can resume the pre-existing session we asked resume.  The other
     * is a more expensive "full" handshake, with key exchange and
     * probably authentication getting done.
     */
    private void serverHello(ServerHello mesg) throws IOException {
	serverKeyExchangeReceived = false;
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	
	// check if the server selected protocol version is OK for us
	ProtocolVersion mesgVersion = mesg.protocolVersion;
	if (enabledProtocols.contains(mesgVersion) == false) {
	    throw new SSLHandshakeException
	    ("Server chose unsupported or disabled protocol: " + mesgVersion);
	}
	
	// Set protocolVersion and propagate to SSLSocket and the
	// Handshake streams
	setVersion(mesgVersion);

	//
	// Save server nonce, we always use it to compute connection
	// keys and it's also used to create the master secret if we're
	// creating a new session (i.e. in the full handshake).
	//
	svr_random = mesg.svr_random;
	
	if (isEnabled(mesg.cipherSuite) == false) {
	    conn.fatal(SSLSocketImpl.alert_illegal_parameter,
		"Server selected disabled ciphersuite " + cipherSuite);
	}
	setCipherSuite(mesg.cipherSuite);

	if (mesg.compression_method != 0) {
	    conn.fatal(SSLSocketImpl.alert_illegal_parameter,
		"compression type not supported, "
		+ mesg.compression_method);
	    // NOTREACHED
	}
	
	// so far so good, let's look at the session
	if (session != null) {
	    // we tried to resume, let's see what the server decided
	    if (session.getSessionId().equals(mesg.sessionId)) {
		// server resumed the session, let's make sure everything
		// checks out
		
		// Verify that the session ciphers are unchanged.
		CipherSuite sessionSuite = session.getSuite();
		if (cipherSuite != sessionSuite) {
		    throw new SSLProtocolException
			("Server returned wrong cipher suite for session");
		}

		// verify protocol version match
		ProtocolVersion sessionVersion = session.getProtocolVersion();
		if (protocolVersion != sessionVersion) {
		    throw new SSLProtocolException
		        ("Server resumed session with wrong protocol version");
		}

		// looks fine; resume it, and update the state machine.
		resumingSession = true;
		state = HandshakeMessage.ht_finished - 1;
		calculateConnectionKeys(session.getMasterSecret());
		if (debug != null && Debug.isOn("session")) {
		    System.out.println("%% Server resumed " + session);
		}
		return;
	    } else {
		// we wanted to resume, but the server refused
		session = null;
		if (!enableNewSession) {
		    throw new SSLException
			("New session creation is disabled");
		}
	    }
	}

	// Create a new session, we need to do the full handshake
	session = new SSLSessionImpl(protocolVersion, cipherSuite, 
			    mesg.sessionId, conn.getHost(), conn.getPort());
	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("** " + cipherSuite);
	}
    }


    /*
     * Server's own key was either a signing-only key, or was too
     * large for export rules ... this message holds an ephemeral
     * RSA key to use for key exchange.
     */
    private void serverKeyExchange(RSA_ServerKeyExchange mesg)
	    throws IOException, GeneralSecurityException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	if (!mesg.verify(serverKey, clnt_random, svr_random)) {
	    conn.fatal(SSLSocketImpl.alert_handshake_failure,
		"server key exchange invalid");
	    // NOTREACHED
	}
	serverKey = mesg.getPublicKey();
    }


    /*
     * Get the ephemeral D-H Key exchange object we'll use for this session.
     */
    private void getDHephemeral(BigInteger modulus, BigInteger base) {
	/*
	 * Generate a new DH key exchange object using keys which are
	 * as close to twice the desired premaster secret size as we're by
	 * allowed to get, according to current interpretations of the EAR.
	 */
	dh = new DHKeyExchange(modulus, base);
	int strength = hasStrongCrypto ? 768 : 512;
	dh.generateKeyPair(sslContext.getSecureRandom(), strength);
    }


    /*
     * Diffie-Hellman key exchange.  We save the server public key and
     * our own D-H algorithm object so we can defer key calculations
     * until after we've sent the client key exchange message (which
     * gives client and server some useful parallelism).
     */
    private void serverKeyExchange(DH_ServerKeyExchange mesg)
	    throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	getDHephemeral(mesg.getModulus(), mesg.getBase());
	serverDH = mesg.getServerPublicKey();
    }


    /*
     * The server's "Hello Done" message is the client's sign that
     * it's time to do all the hard work.
     */
    private void serverHelloDone(ServerHelloDone mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	/*
	 * Always make sure the input has been digested before we
	 * start emitting data, to ensure the hashes are correctly
	 * computed for the Finished and CertificateVerify messages
	 * which we send (here).
	 */
	input.digestNow();

	/*
	 * FIRST ... if requested, send an appropriate Certificate chain
	 * to authenticate the client, and remember the associated private
	 * key to sign the CertificateVerify message.
	 */
	PrivateKey signingKey = null;

	if (certRequest != null) {
	    X509KeyManager km = sslContext.getX509KeyManager();
	    String alias = null;
	    CertificateMsg m1 = null;
	    X509Certificate[] certs = null;

	    ArrayList keytypesTmp = new ArrayList(4);

	    for (int i = 0; i < certRequest.types.length; i++) {
		String typeName;

		switch (certRequest.types[i]) {
		case CertificateRequest.cct_rsa_sign:
		    typeName = "RSA";
		    break;

		case CertificateRequest.cct_dss_sign:
		    typeName = "DSA";
		    break;

		case CertificateRequest.cct_rsa_fixed_dh:
		    typeName = "DH_RSA";
		    break;

		case CertificateRequest.cct_dss_fixed_dh:
		    typeName = "DH_DSA";
		    break;

		case CertificateRequest.cct_rsa_ephemeral_dh:
		case CertificateRequest.cct_dss_ephemeral_dh:
		default:
		    typeName = null;
		    break;
		}

		if ((typeName != null) && (!keytypesTmp.contains(typeName))) {
		    keytypesTmp.add(typeName);
		}
	    }

	    int keytypesTmpSize = keytypesTmp.size();
	    if (keytypesTmpSize != 0) {
		String keytypes[] =
		    (String[])keytypesTmp.toArray(new String[keytypesTmpSize]);

		alias = km.chooseClientAlias(keytypes,
			certRequest.getAuthorities(), conn);
	    }

	    if (alias != null) {
		certs = km.getCertificateChain(alias);
		m1 = new CertificateMsg(certs);
		signingKey = km.getPrivateKey(alias);
		session.setSelfAuthenticated(true);
	    } else {
		//
		// No appropriate cert was found ... report this to the
		// server.  For SSLv3, send the no_certificate alert;
		// TLS uses an empty cert chain instead.
		//
		if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
		    m1 = new CertificateMsg(new X509Certificate [0]);
		} else {
		    conn.warning(SSLSocketImpl.alert_no_certificate);
		}
	    }

	    //
	    // At last ... send any client certificate chain.
	    //
	    if (m1 != null) {
		session.setLocalCertificates(certs);
		if (debug != null && Debug.isOn("handshake")) {
		    m1.print(System.out);
		}
		m1.write(output);
	    }
	}

	/*
	 * SECOND ... send the client key exchange message.  The
	 * procedure used is a function of the cipher suite selected;
	 * one is always needed.
	 */
	HandshakeMessage m2;
	
	if ((keyExchange == K_RSA) || (keyExchange == K_RSA_EXPORT)) {
	    /*
	     * For RSA key exchange, we randomly generate a new
	     * pre-master secret and encrypt it with the server's
	     * public key.  Then we save that pre-master secret
	     * so that we can calculate the keying data later;
	     * it's a performance speedup not to do that until
	     * the client's waiting for the server response, but
	     * more of a speedup for the D-H case.
	     */
	     m2 = new PreMasterSecret(protocolVersion, enabledProtocols.max,
	     	 		sslContext.getSecureRandom(), serverKey);
	} else if ((keyExchange == K_DH_RSA) || (keyExchange == K_DH_DSS)) {
	    /*
	     * For DH Key exchange, we only need to make sure the server
	     * knows our public key, so we calculate the same pre-master
	     * secret.
	     *
	     * For certs that had DH keys in them, we send an empty
	     * handshake message (no key) ... we flag this case by
	     * passing a null "dhPublic" value.
	     *
	     * Otherwise we send ephemeral DH keys, unsigned.
	     */
	    // if (useDH_RSA || useDH_DSS)
	    m2 = new ClientDiffieHellmanPublic();
	} else if ((keyExchange == K_DHE_RSA) || (keyExchange == K_DHE_DSS)
		   || (keyExchange == K_DH_ANON)) {
	    m2 = new ClientDiffieHellmanPublic(dh.getPublicKey());
	} else {
	    // somethings very wrong
	    throw new RuntimeException
	    			("Unsupported key exchange: " + keyExchange);
	}
	if (debug != null && Debug.isOn("handshake")) {
	    m2.print(System.out);
	}
	m2.write(output);


	/*
	 * THIRD, send a "change_cipher_spec" record followed by the
	 * "Finished" message.  We flush the messages we've queued up, to
	 * get concurrency between client and server.  The concurrency is
	 * useful as we calculate the master secret, which is needed both
	 * to compute the "Finished" message, and to compute the keys used
	 * to protect all records following the change_cipher_spec.
	 */

	output.doHashes();
	output.flush();

	/*
	 * We deferred calculating the master secret and this connection's
	 * keying data; we do it now.  Deferring this calculation is good
	 * from a performance point of view, since it lets us do it during
	 * some time that network delays and the server's own calculations
	 * would otherwise cause to be "dead" in the critical path.
	 */
	byte[] preMasterSecret;
	if ((keyExchange == K_RSA) || (keyExchange == K_RSA_EXPORT)) {
	    preMasterSecret = ((PreMasterSecret)m2).preMaster;
	} else { // DH
	    preMasterSecret = dh.getAgreedSecret(serverDH);
	}

	calculateKeys(preMasterSecret);
	Arrays.fill(preMasterSecret, (byte)0);


	/*
	 * FOURTH, if we sent a Certificate, we need to send a signed
	 * CertificateVerify (unless the key in the client's certificate
	 * was a Diffie-Hellman key).).
	 *
	 * This uses a hash of the previous handshake messages ... either
	 * a nonfinal one (if the particular implementation supports it)
	 * or else using the third element in the arrays of hashes being
	 * computed.
	 */
	if (signingKey != null) {
	    CertificateVerify m3;
	    try {
		m3 = new CertificateVerify(protocolVersion, handshakeHash, 
		    signingKey, session.getMasterSecret(),
		    sslContext.getSecureRandom());
	    } catch (GeneralSecurityException e) {
		conn.fatal(SSLSocketImpl.alert_handshake_failure,
		    "Error signing certificate verify", e);
		// NOTREACHED, make compiler happy
		m3 = null;
	    }
	    if (debug != null && Debug.isOn("handshake")) {
		m3.print(System.out);
	    }
	    m3.write(output);
	    output.doHashes();
	}

	/*
	 * OK, that's that!
	 */
	sendChangeCipherAndFinish();
    }


    /*
     * "Finished" is the last handshake message sent.  If we got this
     * far, the MAC has been validated post-decryption.  We validate
     * the two hashes here as an additional sanity check, protecting
     * the handshake against various active attacks.
     */
    private void serverFinished(Finished mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	
	boolean verified = mesg.verify(protocolVersion, handshakeHash, 
				Finished.SERVER, session.getMasterSecret());

	if (!verified) {
	    conn.fatal(SSLSocketImpl.alert_illegal_parameter,
		       "server 'finished' message doesn't verify");
	    // NOTREACHED
	}

	/*
	 * OK, it verified.  If we're doing the fast handshake, add that
	 * "Finished" message to the hash of handshake messages, then send
	 * our own change_cipher_spec and Finished message for the server
	 * to verify in turn.  These are the last handshake messages.
	 *
	 * In any case, update the session cache.  We're done handshaking,
	 * so there are no threats any more associated with partially
	 * completed handshakes.
	 */
	if (resumingSession) {
	    input.digestNow();
	    sendChangeCipherAndFinish();
	}
	session.setLastAccessedTime(System.currentTimeMillis());

	if (!resumingSession) {
	    if (session.isRejoinable()) {
	        ((SSLSessionContextImpl) sslContext
			.engineGetClientSessionContext())
			.put(session);
	        if (debug != null && Debug.isOn("session")) {
		    System.out.println("%% Cached client session: " + session);
		}
	    } else if (debug != null && Debug.isOn("session")) {
	        System.out.println(
		    "%% Didn't cache non-resumable client session: "
		    + session);
	    }
	}
    }


    /*
     * Send my change-cipher-spec and Finished message ... done as the
     * last handshake act in either the short or long sequences.  In
     * the short one, we've already seen the server's Finished; in the
     * long one, we wait for it now.
     */
    private void sendChangeCipherAndFinish() throws IOException {
	Finished mesg = new Finished(protocolVersion, handshakeHash, 
				Finished.CLIENT, session.getMasterSecret());

	/*
	 * Send the change_cipher_spec message, then the Finished message
	 * which we just calculated (and protected using the keys we just
	 * calculated).  Server responds with its Finished message, except
	 * in the "fast handshake" (resume session) case.
	 */
	sendChangeCipherSpec(mesg);

	/*
	 * Update state machine so server MUST send 'finished' next.
	 * (In "long" handshake case; in short case, we're responding
	 * to its message.)
	 */
	state = HandshakeMessage.ht_finished - 1;
    }


    /*
     * Returns a ClientHello message to kickstart renegotiations
     */
    HandshakeMessage getKickstartMessage() throws SSLException {
	ClientHello mesg = new ClientHello(sslContext.getSecureRandom(),
					protocolVersion);

	clnt_random = mesg.clnt_random;

	//
	// Try to resume an existing session.  This might be mandatory,
	// given certain API options.
	//
	session = ((SSLSessionContextImpl)sslContext
			.engineGetClientSessionContext())
			.get(conn.getHost(), conn.getPort());
	if (debug != null && Debug.isOn("session")) {
	    if (session != null) {
		System.out.println("%% Client cached "
		    + session
		    + (session.isRejoinable() ? "" : " (not rejoinable)"));
	    } else {
		System.out.println("%% No cached client session");
	    }
	}

	if (session != null) {
	    CipherSuite sessionSuite = session.getSuite();
	    ProtocolVersion sessionVersion = session.getProtocolVersion();
	    if (isEnabled(sessionSuite) == false) {
		if (debug != null && Debug.isOn("session")) {
		    System.out.println("%% can't resume, cipher disabled");
		}
		session = null;
	    }

	    if ((session != null) && 
	    		(enabledProtocols.contains(sessionVersion) == false)) {
		if (debug != null && Debug.isOn("session")) {
		    System.out.println("%% can't resume, protocol disabled");
		}
		session = null;
	    }
	    
	    if (session != null) {
		if (debug != null) {
		    if (Debug.isOn("handshake") || Debug.isOn("session")) {
			System.out.println("%% Try resuming " + session
			    + " from port " + conn.getLocalPort());
		    }
		}
		mesg.sessionId = session.getSessionId();
		
		mesg.protocolVersion = sessionVersion;
		// Update SSL version number in underlying SSL socket and
		// handshake output stream, so that the output records (at the
		// record layer) have the correct version
		setVersion(sessionVersion);
	    }

	    //
	    // don't say much beyond the obvious if we _must_ resume.
	    //
	    if (!enableNewSession) {
		if (session == null) {
		    throw new SSLException(
			"Can't reuse existing SSL client session");
		}
		mesg.cipherSuites = new CipherSuiteList(sessionSuite);
		return mesg;
	    }
	}
	if (session == null) {
	    if (enableNewSession) {
		mesg.sessionId = SSLSessionImpl.nullSession.getSessionId();
	    } else {
		throw new SSLException("No existing session to resume.");
	    }
	}
	
	//
	// All we have left to do is fill out the cipher suites.
	// (If this changes, change the 'return' above!)
	//
	mesg.cipherSuites = enabledCipherSuites;

	return mesg;
    }

    /*
     * Fault detected during handshake.
     */
    void handshakeAlert(byte description) throws SSLProtocolException {
	String message = SSLSocketImpl.alertDescription(description);

	if (debug != null && Debug.isOn("handshake")) {
	    System.out.println("SSL - handshake alert: " + message);
	}
	throw new SSLProtocolException("handshake alert:  " + message);
    }

    /*
     * Unless we are using an anonymous ciphersuite, the server always
     * sends a certificate message (for the CipherSuites we currently
     * support). The trust manager verifies the chain for us.
     */
    private void serverCertificate(CertificateMsg mesg) throws IOException {
	if (debug != null && Debug.isOn("handshake")) {
	    mesg.print(System.out);
	}
	X509Certificate[] peerCerts = mesg.getCertificateChain();
	if (peerCerts.length == 0) {
	    conn.fatal(SSLSocketImpl.alert_bad_certificate,
		"empty certificate chain");
	}
	// ask the trust manager to verify the chain
	X509TrustManager tm = sslContext.getX509TrustManager();
	try {
	    // find out the key exchange algorithm used
	    // use "RSA" for non-ephemeral "RSA_EXPORT"
	    String keyExchangeString;
	    if (keyExchange == K_RSA_EXPORT && !serverKeyExchangeReceived) {
		keyExchangeString = K_RSA.name;
	    } else {
		keyExchangeString = keyExchange.name;
	    }
	    tm.checkServerTrusted(peerCerts, keyExchangeString);
	} catch (CertificateException e) {
	    // This will throw an exception, so include the original error.
	    conn.fatal(SSLSocketImpl.alert_certificate_unknown, e);
	}
	session.setPeerCertificates(peerCerts);
    }
}
