/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.math.BigInteger;
import java.security.*;
import java.security.interfaces.*;
import java.security.spec.*;
import java.security.cert.*;
import java.util.*;

import javax.security.auth.x500.X500Principal;

import javax.net.ssl.SSLException;
import javax.net.ssl.SSLProtocolException;
import javax.net.ssl.SSLKeyException;

import sun.security.action.GetPropertyAction;

import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * Many data structures are involved in the handshake messages.  These
 * classes are used as structures, with public data members.  They are
 * not visible outside the SSL package.
 *
 * Handshake messages all have a common header format, and they are all
 * encoded in a "handshake data" SSL record substream.  The base class
 * here (HandshakeMessage) provides a common framework and records the
 * SSL record type of the particular handshake message.
 *
 * This file contains subclasses for all the basic handshake messages.
 * All handshake messages know how to encode and decode themselves on
 * SSL streams; this facilitates using the same code on SSL client and
 * server sides, although they don't send and receive the same messages.
 *
 * Messages also know how to print themselves, which is quite handy
 * for debugging.  They always identify their type, and can optionally
 * dump all of their content.
 *
 * @version 1.89, 06/24/03
 * @author David Brownell
 */
abstract class HandshakeMessage implements CipherSuiteConstants {

    HandshakeMessage() { }

    // enum HandshakeType:
    static final byte	ht_hello_request = 0;
    static final byte	ht_client_hello = 1;
    static final byte	ht_server_hello = 2;

    static final byte	ht_certificate = 11;
    static final byte	ht_server_key_exchange = 12;
    static final byte	ht_certificate_request = 13;
    static final byte	ht_server_hello_done = 14;
    static final byte	ht_certificate_verify = 15;
    static final byte	ht_client_key_exchange = 16;

    static final byte	ht_finished = 20;

    /* Class and subclass dynamic debugging support */
    static final Debug debug = Debug.getInstance("ssl");
    
    /**
     * Utility method to convert a BigInteger to a byte array in unsigned
     * format as needed in the handshake messages. BigInteger uses
     * 2's complement format, i.e. it prepends an extra zero if the MSB
     * is set. We remove that.
     */
    static byte[] toByteArray(BigInteger bi) {
	byte[] b = bi.toByteArray();
	if ((b.length > 1) && (b[0] == 0)) {
	    int n = b.length - 1;
	    byte[] newarray = new byte[n];
	    System.arraycopy(b, 1, newarray, 0, n);
	    b = newarray;
	}
	return b;
    }

    /*
     * Write a handshake message on the (handshake) output stream.
     * This is just a four byte header followed by the data.
     *
     * NOTE that huge messages -- notably, ones with huge cert
     * chains -- are handled correctly.
     */
    public final void write(HandshakeOutStream s) throws IOException
    {
	int	len = messageLength();

	if (len > (1 << 24))
	    throw new SSLException("Handshake message too big"
		+ ", type = " + messageType()
		+ ", len = " + len);

	s.write(messageType());
	s.putInt24(messageLength());
	send(s);
    }

    /*
     * Subclasses implement these methods so those kinds of
     * messages can be emitted.  Base class delegates to subclass.
     */
    abstract int  messageType();
    abstract int  messageLength();
    abstract void send(HandshakeOutStream s) throws IOException;

    /*
     * Write a descriptive message on the output stream; for debugging.
     */
    abstract void print(PrintStream p) throws IOException;

//
// NOTE:  the rest of these classes are nested within this one, and are
// imported by other classes in this package.  There are a few other
// handshake message classes, not neatly nested here because of current
// licensing requirement for native (RSA) methods.  They belong here,
// but those native methods complicate things a lot!
//


/*
 * HelloRequest ... SERVER --> CLIENT
 *
 * Server can ask the client to initiate a new handshake, e.g. to change
 * session parameters after a connection has been (re)established.
 */
static final
class HelloRequest extends HandshakeMessage
{
    int messageType() { return ht_hello_request; }

    HelloRequest() { }

    HelloRequest(HandshakeInStream in) throws IOException
    {
	// nothing in this message
    }

    int messageLength() { return 0; }

    void send(HandshakeOutStream out) throws IOException
    {
	// nothing in this messaage
    }

    void print(PrintStream out) throws IOException
    {
	out.println("*** HelloRequest (empty)");
    }

}


/*
 * ClientHello ... CLIENT --> SERVER
 *
 * Client initiates handshake by telling server what it wants, and what it
 * can support (prioritized by what's first in the ciphe suite list).
 *
 * By RFC2246:7.4.1.2 it's explicitly anticipated that this message
 * will have more data added at the end ... e.g. what CAs the client trusts.
 * Until we know how to parse it, we will just read what we know
 * about, and let our caller handle the jumps over unknown data.
 */
static final
class ClientHello extends HandshakeMessage
{
    int messageType() { return ht_client_hello; }

    ProtocolVersion	protocolVersion;
    RandomCookie	clnt_random;
    SessionId		sessionId;
    CipherSuiteList     cipherSuites;
    byte[]		compression_methods;
    
    private final static byte[]	NULL_COMPRESSION = new byte[] {0};

    ClientHello(SecureRandom generator, ProtocolVersion protocolVersion) {
	this.protocolVersion = protocolVersion;
	clnt_random = new RandomCookie(generator);
	compression_methods = NULL_COMPRESSION;
	// sessionId, cipher_suites TBS later
    }

    int messageLength() {
	/*
	 * Add fixed size parts of each field...
	 * version + random + session + cipher + compress
	 */
	return (2 + 32 + 1 + 2 + 1
	    + sessionId.length()		/* ... + variable parts */
	    + (cipherSuites.size() * 2)
	    + compression_methods.length);
    }

    ClientHello(HandshakeInStream s) throws IOException {
	protocolVersion = ProtocolVersion.valueOf(s.getInt8(), s.getInt8());
	clnt_random = new RandomCookie(s);
	sessionId = new SessionId(s.getBytes8());
	cipherSuites = new CipherSuiteList(s);
	compression_methods = s.getBytes8();
    }

    void send(HandshakeOutStream s) throws IOException {
	s.putInt8(protocolVersion.major);
	s.putInt8(protocolVersion.minor);
	clnt_random.send(s);
	s.putBytes8(sessionId.getId());
	cipherSuites.send(s);
	s.putBytes8(compression_methods);
    }

    void print(PrintStream s) throws IOException {
	s.println("*** ClientHello, " + protocolVersion);

	if (debug != null && Debug.isOn("verbose")) {
	    s.print   ("RandomCookie:  "); clnt_random.print(s);

	    s.print("Session ID:  ");
	    s.println(sessionId);

	    s.println("Cipher Suites: " + cipherSuites);
	    
	    Debug.println(s, "Compression Methods", compression_methods);
	    s.println("***");
	}
    }
}


/*
 * ServerHello ... SERVER --> CLIENT
 *
 * Server chooses protocol options from among those it supports and the
 * client supports.  Then it sends the basic session descriptive parameters
 * back to the client.
 */
static final
class ServerHello extends HandshakeMessage
{
    int messageType() { return ht_server_hello; }
    
    ProtocolVersion	protocolVersion;
    RandomCookie	svr_random;
    SessionId           sessionId;
    CipherSuite		cipherSuite;
    byte		compression_method;

    ServerHello() { }

    ServerHello(HandshakeInStream input) throws IOException {
	protocolVersion = ProtocolVersion.valueOf(input.getInt8(), 
						  input.getInt8());
	svr_random = new RandomCookie(input);
	sessionId = new SessionId(input.getBytes8());
	cipherSuite = CipherSuite.valueOf(input.getInt8(), input.getInt8());
	compression_method = (byte)input.getInt8();
    }

    int messageLength()
    {
	// almost fixed size, except session ID:
	//	major + minor = 2
	//	random = 32
	//	session ID len field = 1
	//	cipher suite + compression = 3
	return 38 + sessionId.length();
    }

    void send(HandshakeOutStream s) throws IOException
    {
	s.putInt8(protocolVersion.major);
	s.putInt8(protocolVersion.minor);
	svr_random.send(s);
	s.putBytes8(sessionId.getId());
	s.putInt8(cipherSuite.id >> 8);
	s.putInt8(cipherSuite.id & 0xff);
	s.putInt8(compression_method);
    }

    void print(PrintStream s) throws IOException
    {
	s.println("*** ServerHello, " + protocolVersion);

	if (debug != null && Debug.isOn("verbose")) {
	    s.print   ("RandomCookie:  "); svr_random.print(s);

	    int i;

	    s.print("Session ID:  ");
	    s.println(sessionId);

	    s.println("Cipher Suite: " + cipherSuite);
	    s.println("Compression Method: " + compression_method);
	    s.println("***");
	}
    }
}


/*
 * CertificateMsg ... send by both CLIENT and SERVER
 *
 * Each end of a connection may need to pass its certificate chain to
 * the other end.  Such chains are intended to validate an identity with
 * reference to some certifying authority.  Examples include companies
 * like Verisign, or financial institutions.  There's some control over
 * the certifying authorities which are sent.
 *
 * NOTE: that these messages might be huge, taking many handshake records.
 * Up to 2^48 bytes of certificate may be sent, in records of at most 2^14
 * bytes each ... up to 2^32 records sent on the output stream.
 */
static final
class CertificateMsg extends HandshakeMessage
{
    int messageType() { return ht_certificate; }

    private X509Certificate chain [];

    CertificateMsg(X509Certificate certs [])
    {
	chain = certs;
    }

    CertificateMsg(HandshakeInStream input) throws IOException
    {
	int		chainLen = input.getInt24();
	Vector		v = new Vector(3);

	while (chainLen > 0) {
	    int		certLen = input.getInt24();
	    byte	cert [] = new byte [certLen];
	    int		offset = 0;

	    chainLen -= (3 + certLen);
	    while (offset != certLen) {
		int	temp = input.read(cert, offset, certLen - offset);

		if (temp < 0) {	// EOF
		    throw new SSLProtocolException("short read of certificates");
		}
		offset += temp;
	    }

	    try {
		CertificateFactory cf = CertificateFactory.getInstance("X.509");
		v.addElement(cf.generateCertificate(
		    new ByteArrayInputStream(cert)));
	    } catch (CertificateException e) {
		throw (SSLProtocolException)new SSLProtocolException
			(e.getMessage()).initCause(e);
	    }
	}

	chain = new X509Certificate[v.size()];

	for (int i = 0; i < chain.length; i++)
	    chain[i] = (X509Certificate) v.elementAt(i);
    }

    int messageLength()
    {
	int	i, length = 3;

	try {
	    for (i = chain.length - 1; i >= 0; i--)
		length += 3 + chain[i].getEncoded().length;
	} catch (CertificateException e) {
	    // Eat exception for now... it will be rethrown & caught later
	}
	return length;
    }

    void send(HandshakeOutStream s) throws IOException
    {
	try {
	    s.putInt24(messageLength() - 3);
	    for (int i = 0; i < chain.length; i++)
		s.putBytes24(chain[i].getEncoded());
	} catch (CertificateException e) {
	    throw (SSLProtocolException)new SSLProtocolException
		    (e.getMessage()).initCause(e);
	}
    }

    void print(PrintStream s) throws IOException
    {
	s.println("*** Certificate chain");

	if (debug != null && Debug.isOn("verbose")) {
	    for (int i = 0; i < chain.length; i++)
		s.println("chain [" + i + "] = " + chain[i]);
	    s.println("***");
	}
    }

    public X509Certificate [] getCertificateChain() { return chain; }
}


/*
 * ServerKeyExchange ... SERVER --> CLIENT
 *
 * The cipher suite selected, when combined with the certificate exchanged,
 * implies one of several different kinds of key exchange.  Most current
 * cipher suites require the server to send more than its certificate.
 *
 * The primary exceptions are when a server sends an encryption-capable
 * RSA public key in its cert, to be used with RSA (or RSA_export) key
 * exchange; and when a server sends its Diffie-Hellman cert.  Those kinds
 * of key exchange do not require a ServerKeyExchange message.
 *
 * Key exchange can be viewed as having three modes, which are explicit
 * for the Diffie-Hellman flavors and poorly specified for RSA ones:
 *
 *	- "Ephemeral" keys.  Here, a "temporary" key is allocated by the
 *	  server, and signed.  Diffie-Hellman keys signed using RSA or
 *	  DSS are ephemeral (DHE flavor).  RSA keys get used to do the same
 *	  thing, to cut the key size down to 512 bits (export restrictions)
 *	  or for signing-only RSA certificates.
 *
 *	- Anonymity.  Here no server certificate is sent, only the public
 *	  key of the server.  This case is subject to man-in-the-middle
 *	  attacks.  This can be done with Diffie-Hellman keys (DH_anon) or
 *	  with RSA keys, but is only used in SSLv3 for DH_anon.
 *
 *	- "Normal" case.  Here a server certificate is sent, and the public
 *	  key there is used directly in exchanging the premaster secret.
 *	  For example, Diffie-Hellman "DH" flavor, and any RSA flavor with
 *	  only 512 bit keys.
 *
 * If a server certificate is sent, there is no anonymity.  However,
 * when a certificate is sent, ephemeral keys may still be used to
 * exchange the premaster secret.  That's how RSA_EXPORT often works,
 * as well as how the DHE_* flavors work.
 */
static abstract class ServerKeyExchange extends HandshakeMessage
{
    int messageType() { return ht_server_key_exchange; }
}


/*
 * Using RSA for Key Exchange:  exchange a session key that's not as big
 * as the signing-only key.  Used for export applications, since exported
 * RSA encryption keys can't be bigger than 512 bytes.
 *
 * This is never used when keys are 512 bits or smaller, and isn't used
 * on "US Domestic" ciphers in any case.
 */
static final
class RSA_ServerKeyExchange extends ServerKeyExchange
{
    private byte rsa_modulus[];     // 1 to 2^16 - 1 bytes
    private byte rsa_exponent[];    // 1 to 2^16 - 1 bytes

    private Signature signature;
    private byte[] signatureBytes;

    /*
     * Hash the nonces and the ephemeral RSA public key.
     */
    private void updateSignature(byte clntNonce[], byte svrNonce[]) 
	    throws SignatureException {
	int tmp;

	signature.update(clntNonce);
	signature.update(svrNonce);

	tmp = rsa_modulus.length;
	signature.update((byte)(tmp >> 8));
	signature.update((byte)(tmp & 0x0ff));
	signature.update(rsa_modulus);

	tmp = rsa_exponent.length;
	signature.update((byte)(tmp >> 8));
	signature.update((byte)(tmp & 0x0ff));
	signature.update(rsa_exponent);
    }


    /*
     * Construct an RSA server key exchange message, using data
     * known _only_ to the server.
     *
     * The client knows the public key corresponding to this private
     * key, from the Certificate message sent previously.  To comply
     * with US export regulations we use short RSA keys ... either
     * long term ones in the server's X509 cert, or else ephemeral
     * ones sent using this message.
     */
    RSA_ServerKeyExchange(PublicKey ephemeralKey, PrivateKey privateKey,
    	    RandomCookie clntNonce, RandomCookie svrNonce, SecureRandom	sr)
	    throws GeneralSecurityException {
	RSAPublicKey rsaKey = (RSAPublicKey)ephemeralKey;
	rsa_modulus = toByteArray(rsaKey.getModulus());
	rsa_exponent = toByteArray(rsaKey.getPublicExponent());
	signature = RSASignature.getInstance();
	signature.initSign(privateKey, sr);
	updateSignature(clntNonce.random_bytes, svrNonce.random_bytes);
	signatureBytes = signature.sign();
    }


    /*
     * Parse an RSA server key exchange message, using data known
     * to the client (and, in some situations, eavesdroppers).
     */
    RSA_ServerKeyExchange(HandshakeInStream input, int message_size)
    	    throws IOException, NoSuchAlgorithmException {
	signature = RSASignature.getInstance();
	rsa_modulus = input.getBytes16();
	rsa_exponent = input.getBytes16();
	signatureBytes = input.getBytes16();
    }

    /*
     * Get the ephemeral RSA public key that will be used in this
     * SSL connection.
     */
    PublicKey getPublicKey() {
	try {
	    KeyFactory kfac = KeyFactory.getInstance("RSA");
	    // modulus and exponent are always positive
	    RSAPublicKeySpec kspec = new RSAPublicKeySpec
					 (new BigInteger(1, rsa_modulus),
					  new BigInteger(1, rsa_exponent));
	    return kfac.generatePublic(kspec);
	} catch (Exception e) {
	    throw new RuntimeException(e);
	}
    }

    /*
     * Verify the signed temporary key using the hashes computed
     * from it and the two nonces.  This is called by clients
     * with "exportable" RSA flavors.
     */
    boolean verify(PublicKey certifiedKey, RandomCookie clntNonce, 
	    RandomCookie svrNonce) throws GeneralSecurityException { 
	signature.initVerify(certifiedKey);
	updateSignature(clntNonce.random_bytes, svrNonce.random_bytes);
	return signature.verify(signatureBytes);
    }

    int messageLength() {
	return 6 + rsa_modulus.length + rsa_exponent.length
	       + signatureBytes.length;
    }

    void send(HandshakeOutStream s) throws IOException {
	s.putBytes16(rsa_modulus);
	s.putBytes16(rsa_exponent);
	s.putBytes16(signatureBytes);
    }

    void print(PrintStream s) throws IOException {
	s.println("*** RSA ServerKeyExchange");

	if (debug != null && Debug.isOn("verbose")) {
	    Debug.println(s, "RSA Modulus", rsa_modulus);
	    Debug.println(s, "RSA Public Exponent", rsa_exponent);
	}
    }
}


/*
 * Using Diffie-Hellman algorithm for key exchange.  All we really need to
 * do is securely get Diffie-Hellman keys (using the same P, G parameters)
 * to our peer, then we automatically have a shared secret without need
 * to exchange any more data.  (D-H only solutions, such as SKIP, could
 * eliminate key exchange negotiations and get faster connection setup.
 * But they still need a signature algorithm like DSS/DSA to support the
 * trusted distribution of keys without relying on unscalable physical
 * key distribution systems.)
 *
 * This class supports several DH-based key exchange algorithms, though
 * perhaps eventually each deserves its own class.  Notably, this has
 * basic support for DH_anon and its DHE_DSS and DHE_RSA signed variants.
 */
static final
class DH_ServerKeyExchange extends ServerKeyExchange
{
    // Fix message encoding, see 4348279
    private final static boolean dhKeyExchangeFix = 
    	Debug.getBooleanProperty("com.sun.net.ssl.dhKeyExchangeFix", true);

    private byte		dh_p [];	// 1 to 2^16 - 1 bytes
    private byte		dh_g [];	// 1 to 2^16 - 1 bytes
    private byte		dh_Ys [];	// 1 to 2^16 - 1 bytes

    private byte		signature [];

    /* Return the Diffie-Hellman modulus */
    public BigInteger getModulus() {
    	return new BigInteger(1, dh_p);
    }

    /* Return the Diffie-Hellman base/generator */
    public BigInteger getBase() {
    	return new BigInteger(1, dh_g);
    }

    /* Return the server's Diffie-Hellman public key */
    public BigInteger getServerPublicKey() {
    	return new BigInteger(1, dh_Ys);
    }

    /*
     * Update sig with nonces and Diffie-Hellman public key.
     */
    private void updateSignature(Signature sig, byte clntNonce[], 
    	    byte svrNonce[]) throws SignatureException {
	int tmp;

	sig.update(clntNonce);
	sig.update(svrNonce);

	tmp = dh_p.length;
	sig.update((byte)(tmp >> 8));
	sig.update((byte)(tmp & 0x0ff));
	sig.update(dh_p);

	tmp = dh_g.length;
	sig.update((byte)(tmp >> 8));
	sig.update((byte)(tmp & 0x0ff));
	sig.update(dh_g);

	tmp = dh_Ys.length;
	sig.update((byte)(tmp >> 8));
	sig.update((byte)(tmp & 0x0ff));
	sig.update(dh_Ys);
    }

    /*
     * Construct from initialized DH key object, for DH_anon
     * key exchange.
     */
    DH_ServerKeyExchange(DHKeyExchange obj) {
	getValues(obj);
	signature = null;
    }

    /*
     * Construct from initialized DH key object and the key associated
     * with the cert chain which was sent ... for DHE_DSS and DHE_RSA
     * key exchange.  (Constructor called by server.)
     */
    DH_ServerKeyExchange(DHKeyExchange obj, PrivateKey key, byte clntNonce[],
	    byte svrNonce[], SecureRandom sr) throws GeneralSecurityException {

	getValues(obj);

	Signature sig;
	if (key instanceof DSAPrivateKey) {
	    sig = Signature.getInstance("DSA");
	} else {
	    sig = RSASignature.getInstance();
	}
	sig.initSign(key, sr);
	updateSignature(sig, clntNonce, svrNonce);
	signature = sig.sign();
    }

    private void getValues(DHKeyExchange obj) {
	dh_p = toByteArray(obj.getModulus());
	dh_g = toByteArray(obj.getBase());
	dh_Ys = toByteArray(obj.getPublicKey());
    }

    /*
     * Construct a DH_ServerKeyExchange message from an input
     * stream, as if sent from server to client for use with
     * DH_anon key exchange
     */
    DH_ServerKeyExchange(HandshakeInStream input) throws IOException {
	dh_p = input.getBytes16();
	dh_g = input.getBytes16();
	dh_Ys = input.getBytes16();
	signature = null;
    }

    /*
     * Construct a DH_ServerKeyExchange message from an input stream
     * and a certificate, as if sent from server to client for use with
     * DHE_DSS or DHE_RSA key exchange.  (Called by client.)
     */
    DH_ServerKeyExchange(HandshakeInStream input, PublicKey publicKey,
    	    byte clntNonce[], byte svrNonce[], int messageSize) 
	    throws IOException, GeneralSecurityException {

	dh_p = input.getBytes16();
	dh_g = input.getBytes16();
	dh_Ys = input.getBytes16();

	byte signature[];
	if (dhKeyExchangeFix) {
	    signature = input.getBytes16();
	} else {
	    messageSize -= (dh_p.length + 2);
	    messageSize -= (dh_g.length + 2);
	    messageSize -= (dh_Ys.length + 2);

	    signature = new byte[messageSize];
	    input.read(signature);
	}
	
	Signature sig;
	if (publicKey instanceof DSAPublicKey) {
	    sig = Signature.getInstance("DSA");
	} else if (publicKey instanceof RSAPublicKey) {
	    sig = RSASignature.getInstance();
	} else {
	    throw new SSLKeyException("neither an RSA or a DSA key");
	}

	sig.initVerify(publicKey);
	updateSignature(sig, clntNonce, svrNonce);

	if (sig.verify(signature) == false ) {
	    throw new SSLKeyException("Server D-H key verification failed");
	}
    }

    int messageLength() {
	int temp = 6;	// overhead for p, g, y(s) values.

	temp += dh_p.length;
	temp += dh_g.length;
	temp += dh_Ys.length;
	if (signature != null) {
	    temp += signature.length;
	    if (dhKeyExchangeFix) {
		temp += 2;
	    }
	}
	return temp;
    }

    void send(HandshakeOutStream s) throws IOException {
	s.putBytes16(dh_p);
	s.putBytes16(dh_g);
	s.putBytes16(dh_Ys);
	if (signature != null) {
	    if (dhKeyExchangeFix) {
		s.putBytes16(signature);
	    } else {
		s.write(signature);
	    }
	}
    }

    void print(PrintStream s) throws IOException {
	s.println("*** Diffie-Hellman ServerKeyExchange");

	if (debug != null && Debug.isOn("verbose")) {
	    Debug.println(s, "DH Modulus", dh_p);
	    Debug.println(s, "DH Base", dh_g);
	    Debug.println(s, "Server DH Public Key", dh_Ys);

	    if (signature == null) {
		s.println("Anonymous");
	    } else {
		s.println("Signed with a DSA or RSA public key");
	    }
	}
    }
}

static final class DistinguishedName {

    /* 
     * DER encoded distinguished name.
     * TLS requires that its not longer than 65535 bytes.
     */ 
    byte name[];

    DistinguishedName(HandshakeInStream input) throws IOException {
	name = input.getBytes16();
    }

    DistinguishedName(X500Principal dn) {
        name = dn.getEncoded();
    }
    
    X500Principal getX500Principal() throws IOException {
        try {
            return new X500Principal(name);
	} catch (IllegalArgumentException e) {
	    throw (SSLProtocolException)new SSLProtocolException
		    (e.getMessage()).initCause(e);
	}
    }

    int length() { 
        return 2 + name.length;
    }

    void send(HandshakeOutStream output) throws IOException {
	output.putBytes16(name);
    }

    void print(PrintStream output) throws IOException {
        X500Principal principal = new X500Principal(name);
	output.println("<" + principal.toString() + ">");
    }
}

/*
 * CertificateRequest ... SERVER --> CLIENT
 *
 * Authenticated servers may ask clients to authenticate themselves
 * in turn, using this message.
 */
static final
class CertificateRequest extends HandshakeMessage
{
    int messageType() { return ht_certificate_request; }

    // enum ClientCertificateType
    static final int   cct_rsa_sign = 1;
    static final int   cct_dss_sign = 2;
    static final int   cct_rsa_fixed_dh = 3;
    static final int   cct_dss_fixed_dh = 4;
    static final int   cct_rsa_ephemeral_dh = 5;
    static final int   cct_dss_ephemeral_dh = 6;

    private final static byte[] TYPES = { cct_rsa_sign, cct_dss_sign };
    
    byte		types [];		// 1 to 255 types
    DistinguishedName	authorities [];		// 3 to 2^16 - 1
	// ... "3" because that's the smallest DER-encoded X500 DN
    
    CertificateRequest(X509Certificate ca[], KeyExchange keyExchange) 
	    throws IOException {
	// always use X500Principal
	authorities = new DistinguishedName[ca.length];
	for (int i = 0; i < ca.length; i++) {
	    X500Principal x500Principal = ca[i].getSubjectX500Principal();
	    authorities[i] = new DistinguishedName(x500Principal);
	}
	// we only support RSA and DSS client authentication and they
	// can be used with all ciphersuites. If this changes, the code
	// needs to be adapted to take keyExchange into account.
	this.types = TYPES;
    }

    CertificateRequest(HandshakeInStream input) throws IOException {
	types = input.getBytes8();
	int len = input.getInt16();
	ArrayList v = new ArrayList();
	while (len >= 3) {
	    DistinguishedName dn = new DistinguishedName(input);
	    v.add(dn);
	    len -= dn.length();
	}

	if (len != 0) {
	    throw new SSLProtocolException("Bad CertificateRequest DN length");
	}
	
	authorities = (DistinguishedName[])v.toArray(
					    new DistinguishedName[v.size()]);
    }

    X500Principal[] getAuthorities() throws IOException {
	X500Principal[] ret = new X500Principal[authorities.length];
	for (int i = 0; i < authorities.length; i++) {
	    ret[i] = authorities[i].getX500Principal();
	}
	return ret;
    }

    int messageLength()
    {
	int len;

	len = 1 + types.length + 2;
	for (int i = 0; i < authorities.length; i++)
	    len += authorities[i].length();
	return len;
    }

    void send(HandshakeOutStream output) throws IOException
    {
	int	len = 0;

	for (int i = 0; i < authorities.length; i++)
	    len += authorities[i].length();

	output.putBytes8(types);
	output.putInt16(len);
	for (int i = 0; i < authorities.length; i++)
	    authorities[i].send(output);
    }

    void print(PrintStream s) throws IOException
    {
	s.println("*** CertificateRequest");

	if (debug != null && Debug.isOn("verbose")) {
	    s.print("Cert Types: ");
	    for (int i = 0; i < types.length; i++) {
		switch (types[i]) {
		  case cct_rsa_sign:
		    s.print("RSA"); break;
		  case cct_dss_sign:
		    s.print("DSS"); break;
		  case cct_rsa_fixed_dh:
		    s.print("Fixed DH (RSA sig)"); break;
		  case cct_dss_fixed_dh:
		    s.print("Fixed DH (DSS sig)"); break;
		  case cct_rsa_ephemeral_dh:
		    s.print("Ephemeral DH (RSA sig)"); break;
		  case cct_dss_ephemeral_dh:
		    s.print("Ephemeral DH (DSS sig)"); break;
		  default:
		    s.print("Type-" + types[i]); break;
		}
		if (i != types.length)
		    s.print(", ");
	    }
	    s.println();

	    s.println("Cert Authorities:");
	    for (int i = 0; i < authorities.length; i++)
		authorities[i].print(s);
	}
    }
}


/*
 * ServerHelloDone ... SERVER --> CLIENT
 *
 * When server's done sending its messages in response to the client's
 * "hello" (e.g. its own hello, certificate, key exchange message, perhaps
 * client certificate request) it sends this message to flag that it's
 * done that part of the handshake.
 */
static final
class ServerHelloDone extends HandshakeMessage
{
    int messageType() { return ht_server_hello_done; }

    ServerHelloDone() { }

    ServerHelloDone(HandshakeInStream input)
    {
	// nothing to do
    }

    int messageLength()
    {
	return 0;
    }

    void send(HandshakeOutStream s) throws IOException
    {
	// nothing to send
    }

    void print(PrintStream s) throws IOException
    {
	s.println("*** ServerHelloDone");
    }
}


/*
 * CertificateVerify ... CLIENT --> SERVER
 *
 * Sent after client sends signature-capable certificates (e.g. not
 * Diffie-Hellman) to verify.
 */
static final class CertificateVerify extends HandshakeMessage {

    int messageType() { return ht_certificate_verify; }

    private byte[] signature;

    /*
     * Create an RSA or DSA signed certificate verify message.
     */
    CertificateVerify(ProtocolVersion protocolVersion, HandshakeHash 
	    handshakeHash, PrivateKey privateKey, byte[] masterSecret, 
	    SecureRandom sr) throws GeneralSecurityException {
	String algorithm = privateKey.getAlgorithm();
	Signature sig = getSignature(protocolVersion, algorithm);
	sig.initSign(privateKey, sr);
	updateSignature(sig, protocolVersion, handshakeHash, algorithm, 
			masterSecret);
	signature = sig.sign();
    }

    //
    // Unmarshal the signed data from the input stream.
    //
    CertificateVerify(HandshakeInStream input) throws IOException  {
	signature = input.getBytes16();
    }
    
    /*
     * Verify a certificate verify message. Return the result of verification,
     * if there is a problem throw a GeneralSecurityException.
     */
    boolean verify(ProtocolVersion protocolVersion, 
    	    HandshakeHash handshakeHash, PublicKey publicKey, 
	    byte[] masterSecret) throws GeneralSecurityException {
	String algorithm = publicKey.getAlgorithm();
	Signature sig = getSignature(protocolVersion, algorithm);
	sig.initVerify(publicKey);
	updateSignature(sig, protocolVersion, handshakeHash, algorithm, 
			masterSecret);
	return sig.verify(signature);
    }

    /*
     * Get the Signature object appropriate for verification using the
     * given signature algorithm and protocol version.
     */
    private static Signature getSignature(ProtocolVersion protocolVersion,
    	    String algorithm) throws GeneralSecurityException {
	if (algorithm.equals("RSA")) {
	    return RSASignature.getInternalInstance();
	} else if (algorithm.equals("DSA")) {
	    if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
		return RawDSA.getInstance();
	    } else {
		return Signature.getInstance("DSA");
	    }
	} else {
	    throw new SignatureException("Unrecognized algorithm: "
	    	+ algorithm);
	}
    }
    
    /*
     * Update the Signature with the data appropriate for the given
     * signature algorithm and protocol version so that the object is
     * ready for signing or verifying.
     */
    private static void updateSignature(Signature sig, 
    	    ProtocolVersion protocolVersion,
	    HandshakeHash handshakeHash, String algorithm, byte[] masterSecret)
	    throws SignatureException {
	MessageDigest md5Clone = handshakeHash.getMD5Clone();
	MessageDigest shaClone = handshakeHash.getSHAClone();
	boolean tls = protocolVersion.v >= ProtocolVersion.TLS10.v;
	if (algorithm.equals("RSA")) {
	    if (tls) {
		// nothing to do
	    } else { // SSLv3
		updateDigest(md5Clone, MAC.MD5_pad1, MAC.MD5_pad2, masterSecret);
		updateDigest(shaClone, MAC.SHA_pad1, MAC.SHA_pad2, masterSecret);
	    }
	    // need to use these hashes directly
	    ((RSASignature)sig).setHashes(md5Clone, shaClone);
	} else { // DSA
	    if (tls) {
		sig.update(shaClone.digest());
	    } else { //SSLv3
		shaClone.update(masterSecret);
		shaClone.update(MAC.SHA_pad1);
		byte[] sha1Hash = shaClone.digest();
    
		sig.update(masterSecret);
		sig.update(MAC.SHA_pad2);
		sig.update(sha1Hash);
	    }
	}
    }
    
    /*
     * Update the MessageDigest for SSLv3 certificate verify or finished 
     * message calculation. The digest must already have been updated with 
     * all preceding handshake messages.
     * Used by the Finished class as well.
     */
    static void updateDigest(MessageDigest md, byte[] pad1, byte[] pad2,
	    byte[] masterSecret) {
	md.update(masterSecret);
	md.update(pad1);
	byte[] temp = md.digest();

	md.update(masterSecret);
	md.update(pad2);
	md.update(temp);
    }
    
    int messageLength() {
	return 2 + signature.length;
    }

    void send(HandshakeOutStream s) throws IOException {
	s.putBytes16(signature);
    }

    void print(PrintStream s) throws IOException {
	s.println("*** CertificateVerify");
    }
}


/*
 * FINISHED ... sent by both CLIENT and SERVER
 *
 * This is the FINISHED message as defined in the SSL and TLS protocols.
 * Both protocols define this handshake message slightly differently.
 * This class supports both formats.
 *
 * When handshaking is finished, each side sends a "change_cipher_spec"
 * record, then immediately sends a "finished" handshake message prepared
 * according to the newly adopted cipher spec.
 *
 * NOTE that until this is sent, no application data may be passed, unless
 * some non-default cipher suite has already been set up on this connection
 * connection (e.g. a previous handshake arranged one).
 */
static final class Finished extends HandshakeMessage {

    int messageType() { return ht_finished; }
    
    // constant for a Finished message sent by the client
    final static int CLIENT = 1;

    // constant for a Finished message sent by the server
    final static int SERVER = 2;

    // enum Sender:  "CLNT" and "SRVR"
    private static final byte[] SSL_CLIENT = { 0x43, 0x4C, 0x4E, 0x54 };
    private static final byte[] SSL_SERVER = { 0x53, 0x52, 0x56, 0x52 };
    
    /*
     * Contents of the finished message ("checksum"). For TLS, it
     * is 12 bytes long, for SSLv3 36 bytes.
     */
    private byte[] verifyData;

    /*
     * Create a finished message to send to the remote peer.
     */
    Finished(ProtocolVersion protocolVersion, HandshakeHash handshakeHash, 
	    int sender, byte[] master) {
	verifyData = getFinished(protocolVersion, handshakeHash, sender, 
				 master);
    }
    
    /*
     * Constructor that reads FINISHED message from stream.
     */
    Finished(ProtocolVersion protocolVersion, HandshakeInStream input)
	    throws IOException {
	int msgLen = (protocolVersion.v >= ProtocolVersion.TLS10.v) ? 12 : 36;
	verifyData = new byte[msgLen];
	input.read(verifyData);
    }

    /*
     * Verify that the hashes here are what would have been produced
     * according to a given set of inputs.  This is used to ensure that
     * both client and server are fully in sync, and that the handshake
     * computations have been successful.
     */
     boolean verify(ProtocolVersion protocolVersion, 
	     HandshakeHash handshakeHash, int sender, byte[] master) {
	byte[] myFinished = getFinished(protocolVersion, handshakeHash, 
					sender, master);
	return Arrays.equals(myFinished, verifyData);
    }
    
    /*
     * Perform the actual finished message calculation.
     */
    private static byte[] getFinished(ProtocolVersion protocolVersion, 
	    HandshakeHash handshakeHash, int sender, byte[] masterSecret) {
	byte[] tlsLabel, sslLabel;
	if (sender == CLIENT) {
	    sslLabel = SSL_CLIENT;
	    tlsLabel = PRF.LABEL_CLIENT_FINISHED;
	} else if (sender == SERVER) {
	    sslLabel = SSL_SERVER;
	    tlsLabel = PRF.LABEL_SERVER_FINISHED;
	} else {
	    throw new RuntimeException("Invalid sender: " + sender);
	}
	MessageDigest md5Clone = handshakeHash.getMD5Clone();
	MessageDigest shaClone = handshakeHash.getSHAClone();
	if (protocolVersion.v >= ProtocolVersion.TLS10.v) {
	    // TLS
	    byte[] finished = new byte[12];
	    byte[] md5Digest = md5Clone.digest();
	    byte[] shaDigest = shaClone.digest();
	    PRF.compute(md5Clone, shaClone, masterSecret, tlsLabel, md5Digest,
	    		shaDigest, finished);
	    return finished;
	} else {
	    // SSLv3
	    updateDigest(md5Clone, sslLabel, MAC.MD5_pad1, MAC.MD5_pad2, 
	    		 masterSecret);
	    updateDigest(shaClone, sslLabel, MAC.SHA_pad1, MAC.SHA_pad2, 
	    		 masterSecret);
	    byte[] finished = new byte[36];
	    try {
		md5Clone.digest(finished, 0, 16);
		shaClone.digest(finished, 16, 20);
	    } catch (DigestException e) {
		// cannot occur
		throw new RuntimeException("Digest failed", e);
	    }
	    return finished;
	}
    }
    
    /*
     * Update the MessageDigest for SSLv3 finished message calculation.
     * The digest must already have been updated with all preceding handshake
     * messages. This operation is almost identical to the certificate verify
     * hash, reuse that code.
     */
    private static void updateDigest(MessageDigest md, byte[] sender, 
	    byte[] pad1, byte[] pad2, byte[] masterSecret) {
	md.update(sender);
	CertificateVerify.updateDigest(md, pad1, pad2, masterSecret);
    }
    
    int messageLength() {
	return verifyData.length;
    }

    void send(HandshakeOutStream out) throws IOException {
	out.write(verifyData);
    }

    void print(PrintStream s) throws IOException {
	s.println("*** Finished");
	if (debug != null && Debug.isOn("verbose")) {
	    Debug.println(s, "verify_data", verifyData);
	    s.println("***");
	}
    }
}

//
// END of nested classes
//

}
