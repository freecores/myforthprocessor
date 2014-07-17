/*
 * @(#)DigestUtils.java	1.26 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.digest;

import java.util.Map;
import java.util.Arrays;
import java.math.BigInteger;

import java.io.UnsupportedEncodingException;
import java.io.IOException;
import java.security.MessageDigest;
import java.security.AccessController;
import java.security.PrivilegedAction;
import java.security.NoSuchAlgorithmException;
import java.security.InvalidKeyException;
import java.security.spec.KeySpec;
import java.security.spec.InvalidKeySpecException;
import java.security.InvalidAlgorithmParameterException;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.Mac;
import javax.crypto.SecretKeyFactory;
import javax.crypto.BadPaddingException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.spec.DESKeySpec;
import javax.crypto.spec.DESedeKeySpec;

import com.sun.security.sasl.preview.*;
import com.sun.security.sasl.util.*;



/**
 * Utility class for DIGEST-MD5 mechanism. Provides utility methods 
 * and contains two inner classes which implement the DigestSecurityCtx
 * interface. The inner classes provide the funtionality to allow
 * for quality-of-protection (QOP) with integrity checking and
 * privacy.
 *
 * @author Jonathan Bruce
 */
class DigestUtils extends SaslImpl {
    // Inherit debug option from SaslImpl
    protected static final boolean debug = SaslImpl.debug;
    
    // Property for specifying cipher explicitly
    private static final String CIPHER_PROPERTY =
        "com.sun.security.sasl.digest.cipher";

    // Property for turning on/off chaining
    private static final String NOCHAIN_PROPERTY =
        "com.sun.security.sasl.digest.nochain";
    private static final boolean nochain = noChaining();

    private static final boolean noChaining() {
	String prop = (String)
	    AccessController.doPrivileged(
		new PrivilegedAction() {
		public Object run() {
		    try {
			return System.getProperty(NOCHAIN_PROPERTY);
		    } catch (SecurityException e) {
		    }
		    return null;
		}
	    }
	    );
	return "true".equalsIgnoreCase(prop);
    }

    private final byte[] EMPTY_BYTE_ARRAY = new byte[0];
    protected DigestSecurityCtx secCtx;
    protected byte[] H_A1; // component of response-value
    protected String encoding = "8859_1";  // default unless server specifies utf-8

    protected String cipherSuite;
    protected String specifiedCipher;  // cipher explicitly requested by user

    /* Supported ciphers for 'auth-conf' */
    static protected final int DES3 = 0;
    static protected final int RC4 = 1;
    static protected final int DES = 2;
    static protected final int RC4_56 = 3;
    static protected final int RC4_40 = 4;
    static protected final String[] CIPHER_TOKENS = { "3des",
						      "rc4",
						      "des",	
						      "rc4-56",
						      "rc4-40" };
    /**
     * Constuctor for DigestUtils class. Calls super constructor
     * to parse properties for mechanism.
     *
     * @params props A map of property/value pairs
     * @throws SaslException if a mis-match between the user specified
     * cipher strength and the cipher strength that JCE can support.
     */
    DigestUtils(Map props) throws SaslException {
        super(props); // sets QOP, STENGTH and BUFFER_SIZE
	if (props != null) {
	    specifiedCipher = (String)props.get(CIPHER_PROPERTY);
	}

	if (debug) {
	    if (specifiedCipher != null) {
		System.err.println("explicitly specified cipher: " + 
		    specifiedCipher);
	    }
	}
    }

    /**
     * Retrieves the SASL mechanism IANA name.
     * 
     * @return The String "DIGEST-MD5"
     */
    public String getMechanismName() {
        return "DIGEST-MD5";
    }

    /**
     * Unwrap the incoming message using the wrap method of the secCtx object
     * instance.
     *
     * @param incoming The byte array containing the incoming bytes.
     * @param start The offset from which to read the byte array.
     * @param len The number of bytes to read from the offset.
     * @return The unwrapped message according to either the integrity or 
     * privacy quality-of-protection specifications.
     * @throws SaslException if an error occurs when unwrapping the incoming 
     * message
     */
    public byte[] unwrap(byte[] incoming, int start, int len) throws SaslException {
	if (!completed) {
	    throw new SaslException("Not completed");
	}
	
	return (secCtx.unwrap(incoming, start, len));
    }

    /**
     * Wrap outgoing bytes using the wrap method of the secCtx object
     * instance.
     *
     * @param outgoing The byte array containing the outgoing bytes.
     * @param start The offset from which to read the byte array.
     * @param len The number of bytes to read from the offset.
     * @return The wrapped message according to either the integrity or
     * privacy quality-of-protection specifications.
     * @throws SaslException if an error occurs when wrapping the outgoing
     * message
     */
    public byte[] wrap(byte[] outgoing, int start, int len) throws SaslException {
	if (!completed) {
	    throw new SaslException("Not completed");
	}
	
	return (secCtx.wrap(outgoing, start, len));	
    }

    public void dispose() throws SaslException {
	if (secCtx != null) {
	    secCtx = null; 
	}
    }

        	      
    /**
     * Implementation of the DigestSecurityCtx interface allowing for messages
     * between the client and server to be integrity checked. After a
     * successful DIGEST-MD5 authentication, integtrity checking is invoked
     * if the SASL QOP (quality-of-protection) is set to 'auth-int'.
     * <p>
     * Further details on the integrity-protection mechanism can be found
     * at section 2.3 - Integrity protection in the
     * <a href="http://www.ietf.org/rfc/rfc2831.txt">RFC2831</a> definition.
     *
     * @author Jonathan Bruce
     */
    class DigestIntegrity implements DigestSecurityCtx {

	/* Used for generating integrity keys - specified in RFC 2831*/
	static final private String CLIENT_INT_MAGIC = "Digest session key to " +
	    "client-to-server signing key magic constant";
	static final private String SVR_INT_MAGIC = "Digest session key to " +
	    "server-to-client signing key magic constant";

	/* Key pairs for integrity checking */
	protected byte[] Kic; 
	protected byte[] Kis;
	protected int cltSvrSeqNum = 0;
	protected int svrCltSeqNum = 0;

	// outgoing messageType and sequenceNum
	protected final byte[] messageType = new byte[2];
	protected byte[] sequenceNum = new byte[4];

	/**
	 * Initializes DigestIntegrity implementation of DigestSecurityCtx to
	 * enable DIGEST-MD5 integrity checking.
         *
         * @throws SaslException if an error is encountered generating the
         * key-pairs for integrity checking.
	 */
	DigestIntegrity() throws SaslException {
	    /* Initialize magic strings */

	    try {
		generateIntegrityKeyPair(); 

	    } catch (UnsupportedEncodingException e) {
		throw new SaslException(
		    "DIGEST-MD5: Error encoding strings into UTF-8", e);

	    } catch (IOException e) {
		throw new SaslException("DIGEST-MD5: Error accessing buffers " +
		    "required to create integrity key pairs", e);

	    } catch (NoSuchAlgorithmException e) {
		throw new SaslException("DIGEST-MD5: Unsupported digest " +
		    "algorithm used to create integrity key pairs", e);
	    }

	    /* Message type is a fixed value */
	    intToNetworkByteOrder(1, messageType, 0, 2);
	}

	/**
         * Generate client-server, server-client key pairs for DIGEST-MD5
         * integrity checking.
	 * 
         * @throws UnsupportedEncodingException if the UTF-8 encoding is not
         * supported on the platform.
	 * @throws IOException if an error occurs when writing to or from the
         * byte array output buffers.
	 * @throws NoSuchAlgorithmException if the MD5 message digest algorithm
         * cannot loaded.
	 */
	private void generateIntegrityKeyPair()
	    throws UnsupportedEncodingException, IOException,
		NoSuchAlgorithmException {

	    byte[] cimagic = CLIENT_INT_MAGIC.getBytes(encoding);
	    byte[] simagic = SVR_INT_MAGIC.getBytes(encoding);
	    
	    MessageDigest md5 = MessageDigest.getInstance("MD5");

	    // Both client-magic-keys and server-magic-keys are the same length
	    byte[] keyBuffer = new byte[H_A1.length + cimagic.length];
	    
	    // Kic: Key for protecting msgs from client to server.
	    System.arraycopy(H_A1, 0, keyBuffer, 0, H_A1.length);
	    System.arraycopy(cimagic, 0, keyBuffer, H_A1.length, cimagic.length);
	    md5.update(keyBuffer);
	    Kic = md5.digest();

	    // Kis: Key for protecting msgs from server to client
	    // No need to recopy H_A1
	    System.arraycopy(simagic, 0, keyBuffer, H_A1.length, simagic.length);

	    md5.update(keyBuffer);
	    Kis = md5.digest();

	    if (debug) {
		SaslImpl.traceOutput("Kic:", Kic);
		SaslImpl.traceOutput("Kis:", Kis);
	    }
	}

	/**
	 * Append MAC onto outgoing message.
         *
         * @param outgoing A non-null byte array containing the outgoing message.
         * @param start The offset from which to read the byte array.
	 * @param len The non-zero number of bytes for be read from the offset.
	 * @return The message including the integrity MAC	
	 * @throws SaslException if an error is encountered converting a string
         * into a UTF-8 byte encoding, or if the MD5 message digest algorithm
         * cannot be found or if there is an error writing to the byte array
         * output buffers.
  	 */
	public byte[] wrap(byte[] outgoing, int start, int len) 
	    throws SaslException {

	    if (len == 0) {
		return EMPTY_BYTE_ARRAY;
	    }	    	    

	    /* wrapped = message, MAC, message type, sequence number */
	    byte[] wrapped = new byte[len+10+2+4];

	    /* Start with message itself */
	    System.arraycopy(outgoing, start, wrapped, 0, len);

	    incrementSeqNum(sequenceNum, 0, 4);

	    /* Calculate MAC */
	    byte[] KicMAC = getHMAC(Kic, sequenceNum, outgoing, start, len);

	    if (debug) {
		SaslImpl.traceOutput("DigestIntegrity.wrap():",
		    outgoing, start, len);
		SaslImpl.traceOutput("seqNum:", sequenceNum);
		SaslImpl.traceOutput("KicMAC:", KicMAC);
	    }

	    /* Add MAC[0..9] to message */
	    System.arraycopy(KicMAC, 0, wrapped, len, 10);
		
	    /* Add message type [0..1] */
	    System.arraycopy(messageType, 0, wrapped, len+10, 2);
		
	    /* Add sequence number [0..3] */
	    System.arraycopy(sequenceNum, 0, wrapped, len+12, 4);
	    if (debug) {
		SaslImpl.traceOutput("wrapped:", wrapped);
	    }
	    return wrapped;
	}

	/**
	 * Return verified message without MAC - only if the received MAC
         * and re-generated MAC are the same.
	 * 
         * @param incoming A non-null byte array containing the incoming
         * message.
         * @param start The offset from which to read the byte array.
	 * @param len The non-zero number of bytes to read from the offset
	 * position.
	 * @return The verified message or null if integrity checking fails.
	 * @throws SaslException if an error is encountered converting a string
         * into a UTF-8 byte encoding, or if the MD5 message digest algorithm
         * cannot be found or if there is an error writing to the byte array
         * output buffers
	 */
	public byte[] unwrap(byte[] incoming, int start, int len) 
	    throws SaslException {

	    if (len == 0) {
		return EMPTY_BYTE_ARRAY;
	    }

	    // shave off last 16 bytes of message
	    byte[] serverMAC = new byte[10];
	    byte[] msg = new byte[len - 16];
	    byte[] srvMsgType = new byte[2];
	    byte[] srvSeqNum = new byte[4];

	    /* Get Msg, MAC, msgType, sequenceNum */
	    System.arraycopy(incoming, start, msg, 0, msg.length);
	    System.arraycopy(incoming, start+msg.length, serverMAC, 0, 10);
	    System.arraycopy(incoming, start+msg.length+10, srvMsgType, 0, 2);
	    System.arraycopy(incoming, start+msg.length+12, srvSeqNum, 0, 4);

	    /* Calculate MAC to ensure integrity */
	    byte[] KisMAC = getHMAC(Kis, srvSeqNum, msg, 0, msg.length);

	    if (debug) {
		SaslImpl.traceOutput("DigestIntegrity.unwrap() [msg]:",
		    msg);
		SaslImpl.traceOutput("serverMAC:", serverMAC);
		SaslImpl.traceOutput("messageType:", srvMsgType);
		SaslImpl.traceOutput("sequenceNum:", srvSeqNum);
		SaslImpl.traceOutput("KisMAC:", KisMAC);
	    }

	    /* First, compare MAC's before updating any of our state */
	    if (!compareMACs(serverMAC, KisMAC)) {
		//  Discard message and do not increment sequence number
		if (debug) {
		    System.err.println(
			"DigestIntegrity.unwrap(): unmatched MACs");
		}
		return EMPTY_BYTE_ARRAY;
	    }

	    /* Ensure server-sequence numbers are correct */	
	    if (svrCltSeqNum != networkByteOrderToInt(srvSeqNum, 0, 4)) {
		throw new SaslException("DIGEST-MD5: Out of order " +
		    "sequencing of messages from server. Got: " + 
		    networkByteOrderToInt(srvSeqNum, 0, 4) + 
		    " Expected: " +	svrCltSeqNum);
	    }

	    if (!Arrays.equals(messageType, srvMsgType)) {
		throw new SaslException("DIGEST-MD5: invalid message type: " +
		    networkByteOrderToInt(srvMsgType, 0, 2));
	    }

	    // Increment sequence number and return message
	    svrCltSeqNum++;
	    return msg;
	}

	/**
         * Generates MAC to be appended onto out-going messages.
	 * 
	 * @param Ki A non-null byte array containing the key for the digest
         * @param SeqNum A non-null byte array contain the sequence number
         * @param msg  The message to be digested
	 * @param start The offset from which to read the msg byte array
	 * @param len The non-zero number of bytes to be read from the offset
	 * @return The MAC of a message.
	 *
	 * @throws SaslException if an error occurs when generating MAC.
	 */
	protected byte[] getHMAC(byte[] Ki, byte[] seqnum, byte[] msg,
	    int start, int len) throws SaslException {

	    byte[] seqAndMsg = new byte[4+len];
	    System.arraycopy(seqnum, 0, seqAndMsg, 0, 4);
	    System.arraycopy(msg, start, seqAndMsg, 4, len);

 	    try {
 		SecretKey keyKi = new SecretKeySpec(Ki, "HmacMD5");
 		Mac m = Mac.getInstance("HmacMD5");
 		m.init(keyKi);
 		m.update(seqAndMsg);
 		byte[] hMAC_MD5 = m.doFinal();

		/* First 10 bytes of HMAC_MD5 digest */
		byte macBuffer[] = new byte[10];
		System.arraycopy(hMAC_MD5, 0, macBuffer, 0, 10);

		return macBuffer;
 	    } catch (InvalidKeyException e) {
		throw new SaslException("DIGEST-MD5: Invalid bytes used for " +
		    "key of HMAC-MD5 hash.", e);
	    } catch (NoSuchAlgorithmException e) {
		throw new SaslException("DIGEST-MD5: Error creating " +
		    "instance of MD5 digest algorithm", e);
	    }
	}

	/**
	 * Compare two byte arrays. 
         * 
         * @param A non-null byte array containing a MAC block
         * @param A non-null byte array containing a MAC block
         * @return true if the byte arrays are the same, false otherwise.
         */
	protected boolean compareMACs(byte[] mac1, byte[] mac2) {
	    return (Arrays.equals(mac1, mac2));
	}

	/**
	 * Incremnet client-server sequence count and set anwer in NBO in
	 * sequenceNum array
	 *
	 * @param A non-null byte array for the resulting sequence number in NBO
	 * @param start The offset from where to read the byte array.
	 * @param count The number of bytes to read from the off
	 */
	protected void incrementSeqNum(byte[] sequenceNum, int start, int count) {
	    intToNetworkByteOrder(cltSvrSeqNum++, sequenceNum, start, count);
	}
    }
    

    /**
     * Implementation of the DigestSecurityCtx interface allowing for messages
     * between the client and server to be integrity checked and encrypted. 
     * After a successful DIGEST-MD5 authentication, privacy is invoked if the 
     * SASL QOP (quality-of-protection) is set to 'auth-conf'.
     * <p>
     * Further details on the integrity-protection mechanism can be found
     * at section 2.4 - Confidentiality protection in 
     * <a href="http://www.ietf.org/rfc/rfc2831.txt">RFC2831</a> definition.
     * 
     * @author Jonathan Bruce
     */
    final class DigestPrivacy extends DigestIntegrity implements DigestSecurityCtx {
	/* Used for generating privacy keys - specified in RFC 2831 */
	static final private String CLIENT_CONF_MAGIC =
	    "Digest H(A1) to client-to-server sealing key magic constant";
	static final private String SVR_CONF_MAGIC =
	    "Digest H(A1) to server-to-client sealing key magic constant";

	private IvParameterSpec IVcc; // client-to-server DES IV
	private IvParameterSpec IVcs; // server-to-client DES IV
	private int seqNum;

	private Cipher cipherCc;
	private Cipher cipherCs;
	private SecretKey keyCc;
	private SecretKey keyCs;

	/**
	 * Initializes the cipher object instances for client-server encryption
	 * and server-client decryption.
	 *
	 * @throws SaslException if an error occurs with the Key
	 * initialization, or a string cannot be encoded into a byte array
	 * using the UTF-8 encoding, or an error occurs when writing to a
	 * byte array output buffers or the mechanism cannot load the MD5
	 * message digest algorithm or invalid initialization parameters are
	 * passed to the cipher object instances.
	 */
	DigestPrivacy() throws SaslException {	

	    super(); // generate Kic, Kis keys for integrity-checking.

	    try {
		generatePrivacyKeyPair();	
	
		/* Initialise cipher objects */
		cipherCc.init(Cipher.ENCRYPT_MODE, keyCc, IVcc);
		cipherCs.init(Cipher.DECRYPT_MODE, keyCs, IVcs);

	    } catch (InvalidKeyException e) {
		throw new SaslException("DIGEST-MD5: Invalid byte array " +
		    "used to create cipher keys", e);

	    } catch (UnsupportedEncodingException e) {
		throw new SaslException(
		    "DIGEST-MD5: Error encoding string value into UTF-8", e);

	    } catch (IOException e) {
		throw new SaslException("DIGEST-MD5: Error accessing " +
		    "buffers required to generate cipher keys", e);

	    } catch (NoSuchAlgorithmException e) {
		throw new SaslException("DIGEST-MD5: Error creating " +
		    "instance of required cipher", e);

	    } catch (InvalidAlgorithmParameterException e) {
		throw new SaslException("DIGEST-MD5: Invalid cipher " +
		    "algorithem parameter used to create cipher instance", e);
	    }
	}

	/**	
	 * Generates client-server and server-client keys to encrypt and
	 * decrypt messages. Also generates IVs for DES ciphers.	 
	 *
	 * @throws IOException if an error occurs when writing to or from the
         * byte array output buffers.
	 * @throws NoSuchAlgorithmException if the MD5 message digest algorithm
         * cannot loaded.
	 * @throws UnsupportedEncodingException if an UTF-8 encoding is not
         * supported on the platform.
         * @throw SaslException if an error occurs initializing the keys and
	 * IVs for the chosen cipher.
	 */
	private void generatePrivacyKeyPair() throws IOException,
	    NoSuchAlgorithmException, UnsupportedEncodingException,
	    SaslException {
		
	    byte[] ccmagic = CLIENT_CONF_MAGIC.getBytes(encoding);
	    byte[] scmagic = SVR_CONF_MAGIC.getBytes(encoding);
	    
	    /* Kcc = MD5{H(A1)[0..n], "Digest ... client-to-server"} */
	    MessageDigest md5 = MessageDigest.getInstance("MD5");

	    int n;
	    if (cipherSuite.equals(CIPHER_TOKENS[RC4_40])) {
		n = 5; 		/* H(A1)[0..5] */
	    } else if (cipherSuite.equals(CIPHER_TOKENS[RC4_56])) {
		n = 7;		/* H(A1)[0..7] */
	    } else { // des and 3des and rc4
		n = 16;		/* H(A1)[0..16] */
	    }

	    /* {H(A1)[0..n], "Digest ... client-to-server..."} */
	    // Both client-magic-keys and server-magic-keys are the same length
	    byte[] keyBuffer = 	new byte[n + ccmagic.length];
	    System.arraycopy(H_A1, 0, keyBuffer, 0, n);   // H(A1)[0..n]

	    /* Kcc: Key for encrypting messages from client->server */
	    System.arraycopy(ccmagic, 0, keyBuffer, n, ccmagic.length);
	    md5.update(keyBuffer);
	    byte[] Kcc = md5.digest();	    

	    /* Kcs: Key for decrypting messages from server->client */
	    // No need to copy H_A1 again since it hasn't changed
	    System.arraycopy(scmagic, 0, keyBuffer, n, scmagic.length);
	    md5.update(keyBuffer);
	    byte[] Kcs = md5.digest();

	    if (debug) {
		SaslImpl.traceOutput("Kcc: ", Kcc);
		SaslImpl.traceOutput("Kcs: ", Kcs); 
	    }
	    
	    /* Initialize cipher objects */
	    if (cipherSuite.indexOf(CIPHER_TOKENS[RC4]) > -1) {
		try {
		    cipherCc = Cipher.getInstance("RC4");
		    cipherCs = Cipher.getInstance("RC4");
		    keyCc = new SecretKeySpec(Kcc, "RC4");
		    keyCs = new SecretKeySpec(Kcs, "RC4");
		} catch (javax.crypto.NoSuchPaddingException e) {
		    throw new SaslException(
			"DIGEST-MD5: Incorrect padding used for RC4 cipher", e);
 		}

	    } else if ((cipherSuite.equals(CIPHER_TOKENS[DES])) ||
		(cipherSuite.equals(CIPHER_TOKENS[DES3]))) {

		// DES or 3DES 

		try {
		    String cipherFullname, cipherShortname;

		    // Use "NoPadding" when specifying cipher names
		    // RFC 2831 already defines padding rules for producing
		    // 8-byte aligned blocks
		    if (cipherSuite.equals(CIPHER_TOKENS[DES])) {
			cipherFullname = "DES/CBC/NoPadding";
			cipherShortname = "des";
		    } else {
			/* 3DES */
			cipherFullname = "DESede/CBC/NoPadding";
			cipherShortname = "desede";
		    }

		    cipherCc = Cipher.getInstance(cipherFullname);
		    cipherCs = Cipher.getInstance(cipherFullname);

		    /* Generate client-server key */
		    keyCc = makeDesKeys(Kcc, cipherShortname);

		    /* Generate server-client key */
		    keyCs = makeDesKeys(Kcs, cipherShortname);
		    
		    // Set up the DES IV, which is the last 8 bytes of Kcc or Kcs
		    IVcc = new IvParameterSpec(Kcc, 8, 8);
		    IVcs = new IvParameterSpec(Kcs, 8, 8);

		    if (debug) {
			SaslImpl.traceOutput(cipherSuite + " IVcc: ",
			    IVcc.getIV());
			SaslImpl.traceOutput(cipherSuite + " IVcs: ",
			    IVcs.getIV());
		    }
		    
		} catch (NoSuchPaddingException e) {
		    throw new SaslException("DIGEST-MD5: Unsupported " +
			"padding used for chosen cipher", e);

		} catch (InvalidKeyException e) {
		    throw new SaslException("DIGEST-MD5: Invalid data " +
			"used to initialize keys", e);

		} catch (InvalidKeySpecException e) {
		    throw new SaslException("DIGEST-MD5: Unsupported key " + 
			"specification used.", e);
		}
	    }

	    if (debug) {
		SaslImpl.traceOutput(cipherSuite + " keyCc: ",
		    keyCc.getEncoded()); 
		SaslImpl.traceOutput(cipherSuite + " keyCs: ",
		    keyCs.getEncoded());
	    }
	}




	// -------------------------------------------------------------------

	/**
	 * Encrypt out-going message.
	 *
         * @param outgoing A non-null byte array containing the outgoing message.
         * @param start The offset from which to read the byte array.
	 * @param len The non-zero number of bytes to be read from the offset.
	 * @return The encrypted message.
         *
	 * @throws SaslException if an error occurs when writing to or from the
         * byte array output buffers or if the MD5 message digest algorithm
         * cannot loaded or if an UTF-8 encoding is not supported on the
	 * platform.
	 */	     
	public byte[] wrap(byte[] outgoing, int start, int len)
	    throws SaslException {
	    
	    if (len == 0) {
		return EMPTY_BYTE_ARRAY;
	    }

	    /* HMAC(Ki, {SeqNum, msg})[0..9] */
	    incrementSeqNum(sequenceNum, 0, 4);
	    byte[] KicMAC = getHMAC(Kic, sequenceNum, outgoing, start, len);

	    if (debug) {
		SaslImpl.traceOutput("DigestPrivacy.wrap() [outgoing]:",
		    outgoing, start, len);
		SaslImpl.traceOutput("seqNum:", sequenceNum);
		SaslImpl.traceOutput("KicMAC:", KicMAC);
	    }

	    // Calculate padding
	    int bs = cipherCc.getBlockSize();
	    byte[] padding;
	    if (bs > 1 ) {
		int pad = bs - ((len + 10) % bs); // add 10 for HMAC[0..9]
		padding = new byte[pad];
		for (int i=0; i < pad; i++) {
		    padding[i] = (byte)pad;
		}
	    } else {
		padding = EMPTY_BYTE_ARRAY;
	    }

	    byte[] toBeEncrypted = new byte[len+padding.length+10];

	    /* {msg, pad, HMAC(Ki, {SeqNum, msg}[0..9])} */
	    System.arraycopy(outgoing, start, toBeEncrypted, 0, len);
	    System.arraycopy(padding, 0, toBeEncrypted, len, padding.length);
	    System.arraycopy(KicMAC, 0, toBeEncrypted, len+padding.length, 10);

	    if (debug) {
		SaslImpl.traceOutput(
		    "DigestPrivacy.wrap() [before encryption]",
		    toBeEncrypted);
	    }
		
	    /* CIPHER(Kc, {msg, pad, HMAC(Ki, {SeqNum, msg}[0..9])}) */
	    byte[] cipherBlock;
	    try {
		if (nochain) {
		    // Do not do CBC (chaining) across packets
		    cipherBlock = cipherCc.doFinal(toBeEncrypted);
		} else {
		    // Do CBC (chaining) across packets
		    cipherBlock = cipherCc.update(toBeEncrypted);
		}

		if (cipherBlock == null) {
		    // update() can return null 
		    throw new IllegalBlockSizeException(""+toBeEncrypted.length);
		}
		
	    } catch (BadPaddingException e) {
		throw new SaslException(
		    "DIGEST-MD5: Invalid padding used for block cipher", e);

	    } catch (IllegalBlockSizeException e) {
		throw new SaslException(
		    "DIGEST-MD5: Invalid block size for cipher", e);
	    }

	    byte[] wrapped = new byte[cipherBlock.length+2+4];
	    System.arraycopy(cipherBlock, 0, wrapped, 0, cipherBlock.length);
	    System.arraycopy(messageType, 0, wrapped, cipherBlock.length, 2);
	    System.arraycopy(sequenceNum, 0, wrapped, cipherBlock.length+2, 4);

	    if (debug) {
		SaslImpl.traceOutput(
		    "DigestPrivacy.wrap() [after encryption]", wrapped);
	    }

	    return wrapped;
	}

	/*
	 * Decrypt incoming messages and verify their integrity.
	 *
         * @param incoming A non-null byte array containing the incoming
         * encrypted message.
         * @param start The offset from which to read the byte array.
	 * @param len The non-zero number of bytes to read from the offset
	 * position.
	 * @return The decrypted, verified message or null if integrity
	 * checking 
	 * fails.
	 * @throws SaslException if there are the SASL buffer is empty or if
         * if an error occurs reading the SASL buffer.
	 */
	public byte[] unwrap(byte[] incoming, int start, int len) 
	    throws SaslException {
	    
	    if (len == 0) {
		return EMPTY_BYTE_ARRAY;
	    }
	    
	    byte[] encryptedMsg = new byte[len - 6];
	    byte[] srvMsgType = new byte[2];
	    byte[] srvSeqNum = new byte[4];

	    /* Get cipherMsg; msgType; sequenceNum */
	    System.arraycopy(incoming, start, 
		encryptedMsg, 0, encryptedMsg.length);
	    System.arraycopy(incoming, start+encryptedMsg.length,
		srvMsgType, 0, 2);
	    System.arraycopy(incoming, start+encryptedMsg.length+2,
		srvSeqNum, 0, 4);

	    if (debug) {
		System.err.println(
		    "DigestPrivacy.unwrap() expecting sequence num: " +
		    svrCltSeqNum);
		SaslImpl.traceOutput("encryptedMsg:", encryptedMsg);
	    }

	    // Decrypt message
	    /* CIPHER(Kc, {msg, pad, HMAC(Ki, {SeqNum, msg}[0..9])}) */
	    byte[] decryptedMsg;

            try {
		if (nochain) {
		    // Do not do CBC (chaining) across packets
		    decryptedMsg = cipherCs.doFinal(encryptedMsg);
		} else {
		    // Do CBC (chaining) across packets
		    decryptedMsg = cipherCs.update(encryptedMsg);
		}

		if (decryptedMsg == null) {
		    // update() can return null 
		    throw new IllegalBlockSizeException(""+encryptedMsg.length);
		}
		
	    } catch (BadPaddingException e) {
		throw new SaslException("DIGEST-MD5: Incorrect padding " + 
		    "used with chosen cipher", e);

	    } catch (IllegalBlockSizeException e) {
		throw new SaslException("DIGEST-MD5: Illegal block " +
		    "sizes used with chosen cipher", e);
	    }

	    byte[] msgWithPadding = new byte[decryptedMsg.length - 10];
	    byte[] serverMAC = new byte[10];
		    
	    System.arraycopy(decryptedMsg, 0, 
		msgWithPadding, 0, msgWithPadding.length);
	    System.arraycopy(decryptedMsg, msgWithPadding.length,
		serverMAC, 0, 10);

	    if (debug) {
		SaslImpl.traceOutput("decrypted msg:", decryptedMsg);
		SaslImpl.traceOutput("messageType:", srvMsgType);
		SaslImpl.traceOutput("sequenceNum:", srvSeqNum);
		SaslImpl.traceOutput("serverMAC:", serverMAC);
	    }

	    int msgLength = msgWithPadding.length;
	    int blockSize = cipherCs.getBlockSize();
	    if (blockSize > 1) {
		// get value of last octet of the byte array 
		msgLength -= (int)msgWithPadding[msgWithPadding.length - 1];
		if (msgLength < 0) {
		    //  Discard message and do not increment sequence number
		    if (debug) {
			System.err.println(
			    "DigestPrivacy.unwrap(): incorrect padding: " +
			    (int)msgWithPadding[msgWithPadding.length - 1]);
		    }
		    return EMPTY_BYTE_ARRAY;
		}
	    }
		
	    /* Re-calculate MAC to ensure integrity */
	    byte[] KisMAC = getHMAC(Kis, srvSeqNum, msgWithPadding, 0, msgLength);
		    
	    if (debug) {
		SaslImpl.traceOutput("KisMAC:", KisMAC);
	    }

	    // First, compare MACs before updating state
	    if (!compareMACs(serverMAC, KisMAC)) {
		//  Discard message and do not increment sequence number
		if (debug) {
		    System.err.println("DigestPrivacy.unwrap(): unmatched MACs");
		}
		return EMPTY_BYTE_ARRAY;
	    }

	    /* Ensure sequence number is correct */
	    if (svrCltSeqNum != networkByteOrderToInt(srvSeqNum, 0, 4)) {
		throw new SaslException("DIGEST-MD5: Out of order " +
		    "sequencing of messages from server. Got: " +
		    networkByteOrderToInt(srvSeqNum, 0, 4) + " Expected: " +
		    svrCltSeqNum);		
	    }

	    /* Check message type */
	    if (!Arrays.equals(messageType, srvMsgType)) {
		throw new SaslException("DIGEST-MD5: invalid message type: " +
		    networkByteOrderToInt(srvMsgType, 0, 2));
	    }

	    // Increment sequence number and return message
	    svrCltSeqNum++;

	    if (msgLength == msgWithPadding.length) {
		return msgWithPadding; // no padding
	    } else {
		// Get a copy of the message without padding
		byte[] clearMsg = new byte[msgLength];
		System.arraycopy(msgWithPadding, 0, clearMsg, 0, msgLength);
		return clearMsg;
	    }
	}
    }

    // ---------------- DES and 3 DES key manipulation routines
	
    /* Mask used to check for parity adjustment */
    private static final byte[] PARITY_BIT_MASK = {
	(byte)0x80, (byte)0x40, (byte)0x20, (byte)0x10,
	(byte)0x08, (byte)0x04, (byte)0x02
    };
    private static final BigInteger MASK = new BigInteger("7f", 16);

    /**
     * Sets the parity bit (0th bit) in each byte so that each byte
     * contains an odd number of 1's.
     */
    private static void setParityBit(byte[] key) {
	for (int i = 0; i < key.length; i++) {
	    int bitCount = 0;
	    for (int maskIndex = 0;
		 maskIndex < PARITY_BIT_MASK.length; maskIndex++) {
		if ((key[i] & PARITY_BIT_MASK[maskIndex])
		    == PARITY_BIT_MASK[maskIndex]) {
		    bitCount++;
		}
	    }
	    if ((bitCount & 0x01) == 1) {
		// Odd number of 1 bits in the top 7 bits. Set parity bit to 0
		key[i] = (byte)(key[i] & (byte)0xfe);
	    } else {
		// Even number of 1 bits in the top 7 bits. Set parity bit to 1
		key[i] = (byte)(key[i] | 1);
	    }
	}
    }

    /**
     * Expands a 7-byte array into an 8-byte array that contains parity bits
     * The binary format of a cryptographic key is: 
     *     (B1,B2,...,B7,P1,B8,...B14,P2,B15,...,B49,P7,B50,...,B56,P8) 
     * where (B1,B2,...,B56) are the independent bits of a DES key and 
     * (PI,P2,...,P8) are reserved for parity bits computed on the preceding 
     * seven independent bits and set so that the parity of the octet is odd, 
     * i.e., there is an odd number of "1" bits in the octet.
     */
    private static byte[] addDesParity(byte[] input, int offset, int len) {
	if (len != 7) 
	    throw new IllegalArgumentException(
		"Invalid length of DES Key Value:" + len);

	byte[] raw = new byte[7];
	System.arraycopy(input, offset, raw, 0, len);

	byte[] result = new byte[8];
	BigInteger in = new BigInteger(raw);

	// Shift 7 bits each time into a byte
	for (int i=result.length-1; i>=0; i--) {
	    result[i] = in.and(MASK).toByteArray()[0];
	    result[i] <<= 1;         // make room for parity bit
	    in = in.shiftRight(7);
	}
	setParityBit(result);
	return result;
    }
    
    /** 
     * Create parity-adjusted keys suitable for DES / DESede encryption. 
     * 
     * @param input A non-null byte array containing key material for 
     * DES / DESede.
     * @param desStrength A string specifying eithe a DES or a DESede key.
     * @return SecretKey An instance of either DESKeySpec or DESedeKeySpec.
     *
     * @throws NoSuchAlgorithmException if the either the DES or DESede 
     * algorithms cannote be lodaed by JCE.
     * @throws InvalidKeyException if an invalid array of bytes is used
     * as a key for DES or DESede.
     * @throws InvalidKeySpecException in an invalid parameter is passed
     * to either te DESKeySpec of the DESedeKeySpec constructors.
     */
    private static SecretKey makeDesKeys(byte[] input, String desStrength)
	throws NoSuchAlgorithmException, InvalidKeyException,
	    InvalidKeySpecException {

	// Generate first subkey using first 7 bytes
	byte[] subkey1 = addDesParity(input, 0, 7);

	KeySpec spec = null;
	SecretKeyFactory desFactory =
	    SecretKeyFactory.getInstance(desStrength);

	if (desStrength.equals("des")) {
	    spec = new DESKeySpec(subkey1, 0);
	    if (debug && SaslImpl.verbose) {
		SaslImpl.traceOutput("des key input: ", input);
		SaslImpl.traceOutput("des key parity-adjusted: ", subkey1);
		SaslImpl.traceOutput("des key material: ",
		    ((DESKeySpec)spec).getKey());
		System.err.println(" is parity-adjusted? " + 
		    DESKeySpec.isParityAdjusted(subkey1, 0));
	    }
		    
	} else if (desStrength.equals("desede")) {

	    // Generate second subkey using second 7 bytes
	    byte[] subkey2 = addDesParity(input, 7, 7);

	    // Construct 24-byte encryption-decryption-encryption sequence
	    byte[] ede = new byte[subkey1.length*2+subkey2.length];
	    System.arraycopy(subkey1, 0, ede, 0, subkey1.length);
	    System.arraycopy(subkey2, 0, ede, subkey1.length, subkey2.length);
	    System.arraycopy(subkey1, 0, ede, subkey1.length+subkey2.length,
		subkey1.length);

	    spec = new DESedeKeySpec(ede, 0);
	    if (debug && SaslImpl.verbose) {
		SaslImpl.traceOutput("3des key input: ", input);
		SaslImpl.traceOutput("3des key ede: ", ede);
		SaslImpl.traceOutput("3des key material: ",
		    ((DESedeKeySpec)spec).getKey());
		System.err.println(" is parity-adjusted? " + 
		    DESedeKeySpec.isParityAdjusted(ede, 0));
	    }
	} else {
	    throw new IllegalArgumentException("Invalid DES strength:" +
		desStrength);
	}
	return desFactory.generateSecret(spec);
    }
}
