/*
 * @(#)DigestMD5.java	1.29 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.digest;

import java.security.AccessController;
import java.security.Provider;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.io.ByteArrayOutputStream;
import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Arrays;

import com.sun.security.sasl.preview.*;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.UnsupportedCallbackException;

/**     
  * An implementation of the DIGEST-MD5 
  * (<a href="http://www.ietf.org/rfc/rfc2831.txt">RFC 2831</a>) SASL 
  * (<a href="http://www.ietf.org/rfc/rfc2222.txt">RFC 2222</a>) mechanism. 
  * The DIGEST-MD5 mechanism provides a means by which a SASL enabled client 
  * can authenticate and securely communicate with a server. It offers a
  * more secure framework than the CRAM-MD5 SASL mechanism as it  
  * prevents chosen plaintext attacks and permits mutual authentication.
  * <p>
  * The DIGEST-MD5 SASL mechanism specifies two modes of authentication.
  * <ul><li>Initial Authentication
  * <li>Subsequent Authentication - optional, (currently unsupported) 
  * </ul> 
  * A DIGEST-MD5 authentication is initiated when a digest-challenge is
  * received by the client - the DIGEST-MD5 mechanism does not have a defined 
  * initial client response. A suitable digest-response is then generated
  * by the client and returned to the server. If the server deems the 
  * digest-response to be correct, the client receives an 
  * authentication-success message at which point the SASL exchange 
  * is complete.
  * <p>
  * Optionally, the user may specify a quality of protection (QOP). There
  * are a range of three protections that can be specified using the
  * Java SASL properties as detailed below.
  * <p>
  * <code><b>'javax.security.sasl.qop'</b></code> - Quality of Protection 
  * <ul>
  * <li><code><b>'auth'</b></code> - authentication
  * <br>
  * This indicates that the user wishes to authenticate only. This is the
  * <i>default</i> QOP setting.
  * 
  * <li><code><b>'auth-int'</b></code> - authentication with integrity
  * protection.
  * <br> 
  * After a successful authentication, all subsequent communications 
  * between the client and server (up to the next authentication) undergo
  * integrity checking to ensure the validity of each message.
  * 
  * <li><code><b>'auth-conf'</b></code> - authentication with integrity
  * protection and encryption.
  * <br>
  * After a successful authentication, all subsequent communications 
  * between the client and server (up to the next authentication) are 
  * encrypted and integrity-checked.  
  * </ul>
  * <p>
  * The strength of the encryption is specified using the 
  * <code><b>'javax.security.sasl.strength'</b></code> property. The strength
  * can be set to one of <code><b>'high'</b></code>, 
  * <code><b>'medium'</b></code> or <code><b>'low'</b></code>, or an
  * ordered, comma-separated list of these strength values.
  * Assuming there is mutual client and server support for chosen strength,
  * the strength settings break down as follows:
  * <p>
  * <code><b>'javax.security.sasl.strength'</b></code> - Cipher strength
  * <p>
  * <ul>
  * <li><code><b>'high'</b></code>
  * - Exchanges between the client and server are encrypted using the
  * Triple DES cipher or RC4 (128-bit key). If strength is unspecified,
  * this is the <i>default</i> cipher strength if it is available.
  * <li><code><b>'medium'</b></code>
  * - Exchanges between the client and server are encrypted using the DES or
  * RC4 (56-bit key), depending on their availablility. 
  * <li><code><b>'low'</b></code>
  * - Exchanges between the client and server are encrypted using RC4
  *  (40-bit key).
  * To specify a specific cipher, use the
  *<tt>"com.sun.security.sasl.digest.cipher"</tt> property to name the cipher.
  * This setting must be compatible with the setting of the
  * "javax.security.sasl.strength" property.
  * </ul>
  * <p>
  * If the mechanism is unable to support a chosen cipher strength, the 
  * mechanism aborts and the DIGEST-MD5 authentication aborts.
  *
  * @see <a href="http://www.ietf.org/rfc/rfc2222.txt">RFC 2222</a>
  * - Simple Authentication and Security Layer (SASL)
  * @see <a href="http://www.ietf.org/rfc/rfc2831.txt">RFC 2831</a>
  * - Using Digest Authentication as a SASL Mechanism
  * @see <a href="http://java.sun.com/products/jce">Java(TM)
  * Cryptography Extension 1.2.1 (JCE)</a>
  * @see <a href="http://java.sun.com/products/jaas">Java(TM) 
  * Authentication and Authorization Service (JAAS)</a>
  *
  * @author Jonathan Bruce
  */  

final class DigestMD5 extends DigestUtils implements SaslClient {
    /* Used to track progress of authentication */
    private boolean isStepTwoCompleted = false;

    /* Constants - defined in RFC2831 */
    private static final int MAX_CHALLENGE_LENGTH = 2048;
    private static final int MAX_RESPONSE_LENGTH = 4096;
    
    /* Allows ability to offer a a choice of realms */
    private CallbackHandler cbh;

    
    /*
     * Variables to be set by component values of 
     * digest-challenge. 
     */
    private String userName;
    private String authzIdStr;
    private byte[] authzId;
    private String selectedQopStr;
    private byte selectedQop;
    private String authRealm;
    private List realmChoices;
    private String digestUri;
    private char[] passwd;
    private byte[] nonce;
    private String negotiatedStrength;
    private boolean useUTF8 = false;

    /* Generated on each initial authentication */
    private byte[] c_nonce;

    private static final String[] DIRECTIVE_KEY = {
	"realm",   // optional, multiple OK
	"qop",     // optional, multiple probably OK, but RFC 2831 not explicit
	"algorithm",  // must appear exactly once
	"nonce",   // must appear exactly once
	"maxbuf",  // optional, atmost once
	"charset", // optional, atmost once
	"cipher",  // exactly once if qop is "auth-conf"
	"rspauth", // not used in initial auth; atmost once for subsequent auth
	"stale", // not used in initial auth; atmost once for subsequent auth
    };

    private final byte[][] challengeVal = new byte[DIRECTIVE_KEY.length][];
    private static final int REALM = 0;
    private static final int QOP = 1; 
    private static final int ALGORITHM = 2; 
    private static final int NONCE = 3;
    private static final int MAXBUF = 4; 
    private static final int CHARSET = 5; 
    private static final int CIPHER = 6; 
    private static final int RESPONSE_AUTH = 7; 
    private static final int STALE = 8;
    private int nonceCount = 0;

    private static final String[] JCE_CIPHER_NAME = {
	"Cipher.DESede",
	"Cipher.RC4",
	"Cipher.DES",
    };

    // ---------- Extension of property names from SaslImpl ------------
    /*
     * If QOP is set to 'auth-conf', a DIGEST-MD5 mechanism must have
     * support for the DES and Triple DES cipher algorithms (optionally,
     * support for RC4 [128/56/40 bit keys] ciphers) to provide for
     * confidentiality. See RFC 2831 for details. This implementation 
     * provides support for DES, Triple DES and RC4 ciphers.
     *     
     * The value of strength effects the strength of cipher used. The mappings
     * of 'high', 'medium', and 'low' give the following behaviour.
     *
     * 	HIGH_STRENGTH 	- Triple DES
     *  		- RC4 (128bit)
     *  MEDIUM_STRENGTH	- DES
     *   		- RC4 (56bit)
     * 	LOW_SRENGTH	- RC4 (40bit)
     */
    static final private byte DES_3_STRENGTH = HIGH_STRENGTH; 
    static final private byte RC4_STRENGTH = HIGH_STRENGTH; 
    static final private byte DES_STRENGTH = MEDIUM_STRENGTH; 
    static final private byte RC4_56_STRENGTH = MEDIUM_STRENGTH; 
    static final private byte RC4_40_STRENGTH = LOW_STRENGTH;    
    static final private byte UNSET = (byte)0;
    static protected final byte[] CIPHER_MASKS = { DES_3_STRENGTH,
						   RC4_STRENGTH,
						   DES_STRENGTH,
						   RC4_56_STRENGTH,
						   RC4_40_STRENGTH }; 

    /**
      * Constructor for DIGEST-MD5 mechanism.
      *
      * @param authzId A non-null String representing the principal 
      * for which authorization is being granted..
      * @param digestURI A non-null String representing detailing the
      * combined protocol and host being used for authentication.
      * @param props The possibly null properties to be used by the SASL 
      * mechanism to configure the authentication exchange.
      * @param cbh The non-null CallbackHanlder object for callbacks
      * @throws SaslException if no authentication ID or password is supplied
      */
    DigestMD5(String authzIdStr, String digestUri, Map props, 
	CallbackHandler cbh) throws SaslException {
	
	super(props);

	// authzID can only be encoded in UTF8 - RFC 2222
	if (authzIdStr != null) {
	    this.authzIdStr = authzIdStr;
	    try {		
		this.authzId = authzIdStr.getBytes("UTF8");
	    		
	    } catch (UnsupportedEncodingException e) {
		throw new SaslException(
		    "DIGEST-MD5: Error encoding authzId value into UTF-8", e);
	    }
	}
 	
	this.digestUri = digestUri;
	this.cbh = cbh;
   }

    /**
     * DIGEST-MD5 has no initial response
     *
     * @return false
     */
    public boolean hasInitialResponse() {
	return false; 
    }

    /**
     * Process the challenge data.
     * 
     * The server sends a digest-challenge which the client must reply to 
     * in a digest-response. When the authentication is complete, the
     * completed field is set to true. 
     *       
     * @param challengeData A non-null byte array containing the challenge 
     * data from the server.
     * @return A possibly null byte array containing the response to 
     * be sent to the server.
     *
     * @throws SaslException If the platform does not have MD5 digest support
     * or if the server sends an invalid challenge.
     */	
    public byte[] evaluateChallenge(byte[] challengeData) throws SaslException {
	
	if (challengeData.length > MAX_CHALLENGE_LENGTH) {
	    throw new SaslException(
		"DIGEST-MD5: Invalid digest-challenge length. Got:  " +
		challengeData.length + " Expected < " + MAX_CHALLENGE_LENGTH);
	}	

	/* Extract and process digest-challenge */
	extractChallenge(challengeData);

	/* Initial authenticaton */
	if (!isStepTwoCompleted) {	    
	    try {
		checkDigestChallenge();
		checkQopSupport();
		isStepTwoCompleted = true;				
		return (generateDigestResponse());
	    } catch (SaslException e) {
		throw e; // rethrow
	    } catch (IOException e) {
		throw new SaslException("DIGEST-MD5: Error generating " +
		    "digest response-value", e);
	    }
	} else {
	    /* Process server response to digest-repsonse */	
	    verifyResponseValue(challengeVal[RESPONSE_AUTH]);

	    completed = true;
	    /* Initialize DigestSecurityCtx implementation */
	    if (integrity && privacy) {
		secCtx = new DigestPrivacy();
	    } else if (integrity) {
		secCtx = new DigestIntegrity();
	    } 

	    return null; // Mechanism has completed.
	}
    }

    /**
     * Parses digest-challenge string, extracting each token
     * and value(s)
     *
     * @param buf A non-null digest-challenge string.
     * @throws SaslException if the buf cannot be parsed according to RFC 2831
     */
    private void extractChallenge(byte[] buf) throws SaslException {
	ByteArrayOutputStream key = new ByteArrayOutputStream(10);
	ByteArrayOutputStream value = new ByteArrayOutputStream(10);
	boolean gettingKey = true;
	boolean gettingQuotedValue = false;
	boolean expectSeparator = false;
	byte bch;
	
	int i = skipLws(buf, 0);
	while (i < buf.length) {
	    bch = buf[i];

	    if (gettingKey) {
		if (bch == ',') {
		    if (key.size() != 0) {
			throw new SaslException("Directive key contains a ',':" + 
			    key);
		    }
		    // Empty element, skip separator and lws
		    i = skipLws(buf, i+1);

		} else if (bch == '=') {
		    if (key.size() == 0) {
			throw new SaslException("Empty directive key");
		    }
		    gettingKey = false;      // Termination of key
		    i = skipLws(buf, i+1);   // Skip to next nonwhitespace

		    // Check whether value is quoted
		    if (i < buf.length) {
			if (buf[i] == '"') {
			    gettingQuotedValue = true;
			    ++i; // Skip quote
			} 
		    } else {
			throw new SaslException(
			    "Valueless directive found: " + key.toString());
		    }
		} else if (isLws(bch)) {
		    // LWS that occurs after key
		    i = skipLws(buf, i+1);

		    // Expecting '='
		    if (i < buf.length) {
			if (buf[i] != '=') {
			    throw new SaslException("'=' expected after key: " +
				key.toString());
			} 
		    } else {
			throw new SaslException(
			    "'=' expected after key: " + key.toString());
		    }
		} else {
		    key.write(bch);    // Append to key
		    ++i;               // Advance
		}
	    } else if (gettingQuotedValue) {
		// Getting a quoted value
		if (bch == '\\') {
		    // quoted-pair = "\" CHAR  ==> CHAR
		    ++i;       // Skip escape
		    if (i < buf.length) {
			value.write(buf[i]);
			++i;   // Advance
		    } else {
			// Trailing escape in a quoted value
			throw new SaslException(
			    "Unmatched quote found for directive: " 
			    + key.toString() + " with value: " + value.toString());
		    }
		} else if (bch == '"') {
		    // closing quote
		    ++i;  // Skip closing quote
		    gettingQuotedValue = false;
		    expectSeparator = true;
		} else {
		    value.write(bch);
		    ++i;  // Advance
		}

	    } else if (isLws(bch) || bch == ',') {
		//  Value terminated

		extractDirective(key.toString(), value.toByteArray());
		key.reset();
		value.reset();
		gettingKey = true;
		gettingQuotedValue = expectSeparator = false;
		i = skipLws(buf, i+1);   // Skip separator and LWS

	    } else if (expectSeparator) {
		throw new SaslException(
		    "Expecting comma or linear whitespace after quoted string: \"" 
			+ value.toString() + "\"");
	    } else {
		value.write(bch);   // Unquoted value
		++i;                // Advance
	    }
	}

	if (gettingQuotedValue) {
	    throw new SaslException(
		"Unmatched quote found for directive: " + key.toString() +
		" with value: " + value.toString());
	}

	// Get last pair
	if (key.size() > 0) {
	    extractDirective(key.toString(), value.toByteArray());
	}
    }

    // Is character a linear white space?
    // LWS            = [CRLF] 1*( SP | HT )
    // %%% Note that we're checking individual bytes instead of CRLF
    private static boolean isLws(byte b) {
	switch (b) {
	case 13:   // US-ASCII CR, carriage return
	case 10:   // US-ASCII LF, linefeed
	case 32:   // US-ASCII SP, space
	case 9:    // US-ASCII HT, horizontal-tab
	    return true;
	}
	return false;
    }

    // Skip all linear white spaces
    private static int skipLws(byte[] buf, int start) {
	int i;
	for (i = start; i < buf.length; i++) {
	    if (!isLws(buf[i])) {
		return i;
	    }
	}
	return i;
    }

    /**
     * Processes directive/value pairs from the digest-challenge and
     * fill out the challengeVal array.
     * 
     * @param key A non-null String challenge token name.
     * @param value A non-null String token value.
     * @throws SaslException if a either the key or the value is null
     */
    private void extractDirective(String key, byte[] value) throws SaslException {

	/* Process realm, qop, maxbuf, charset, algorithm, cipher, rspauth, nonce */

	for (int i = 0; i < DIRECTIVE_KEY.length; i++) {
	    if (key.equalsIgnoreCase(DIRECTIVE_KEY[i])) {
		if (challengeVal[i] == null) {
		    challengeVal[i] = value;
		    if (debug) {
			System.err.println(DIRECTIVE_KEY[i] + "=" + 
			    new String(challengeVal[i]));
		    }
		} else if (i == REALM) {
		    // > 1 realm specified
		    if (realmChoices == null) {
			realmChoices = new ArrayList(3);
			realmChoices.add(challengeVal[i]); // add first
		    }
		    realmChoices.add(value);

		} else if (i == QOP) {
		    // Merge old and new directive values comma-separated list

		    int oldlen = challengeVal[i].length;
		    byte[] merged = new byte[oldlen + 1 + value.length];
		    System.arraycopy(challengeVal[i], 0, merged, 0, oldlen);
		    merged[oldlen] = (byte)',';
		    System.arraycopy(value, 0, merged, oldlen+1, value.length);

		    if (debug) {
			System.err.println(DIRECTIVE_KEY[i] + "=" + 
			    new String(challengeVal[i]) + "->" + 
			    new String(merged));
		    }
		    challengeVal[i] = merged;

		} else {
		    throw new SaslException(
			"DIGEST-MD5: server sent more than one " +
			key + " directive: " + new String(value));
		}

		break; // end search
	    }
	}
     }

   /**
    * Checks the challengeVal array by dealing with digest-challenge 
    * directive values that are multi-valued and ensure that mandatory
    * directives not missing from the digest-challenge.
    * 
    * @throws SaslException if a sasl is a the mechanism cannot 
    * correcly handle a ChoiceCallback or if a violation in the
    * digest-challenge format is detected.
    */   
    private void checkDigestChallenge()
	throws SaslException, UnsupportedEncodingException {
	    
	/* CHARSET: optional atmost once */
	if (challengeVal[CHARSET] != null) {
	    if (!"UTF-8".equalsIgnoreCase(
		new String(challengeVal[CHARSET], encoding))) {
		throw new SaslException("DIGEST-MD5: digest-challenge format " +
		    "violation. Unrecognised charset value: " + 
		    new String(challengeVal[CHARSET]));
	    } else {
		encoding = "UTF8";
		useUTF8 = true;
	    }
	}
	
	/* ALGORITHM: required exactly once */
	if (challengeVal[ALGORITHM] == null) {
	    throw new SaslException("DIGEST-MD5: Digest-challenge format " +
		"violation: algorithm directive missing");
	} else if (!"md5-sess".equalsIgnoreCase(
	    new String(challengeVal[ALGORITHM], encoding))) {
	    throw new SaslException("DIGEST-MD5: Digest-challenge format " +
		"violation. Invalid value for 'algorithm' directive: " +
		challengeVal[ALGORITHM]);		    
	}

	/* NONCE: required exactly once */
	if (challengeVal[NONCE] == null) {
	    throw new SaslException("DIGEST-MD5: Digest-challenge format " +
		"violation: nonce directive missing");
	} else {
	    nonce = challengeVal[NONCE];
	    ++nonceCount;
	}


	try {
	    /* REALM: optional, if multiple, stored in realmChoices */
	    String[] realmTokens = null;

	    if (challengeVal[REALM] != null) {
		if (realmChoices == null) {
		    // Only one realm specified
		    realmTokens = new String[] {
			new String(challengeVal[REALM], encoding)};
		} else {
		    realmTokens = new String[realmChoices.size()];
		    for (int i = 0; i < realmTokens.length; i++) {
			realmTokens[i] = 
			    new String((byte[])realmChoices.get(i), encoding);
		    }
		}
	    } 	    
		    	       	    
	    NameCallback ncb = authzIdStr == null ?
		new NameCallback("DIGEST-MD5 authentication ID: ") :
		new NameCallback("DIGEST-MD5 authentication ID: ", authzIdStr);
	    PasswordCallback pcb = 
		new PasswordCallback("DIGEST-MD5 password: ", false);	    

	    if (realmTokens == null) {
		// RFC 2831: the client SHOULD solicit a realm from the user.
		RealmCallback tcb = new RealmCallback("DIGEST-MD5 realm: ");
		cbh.handle(new Callback[] {tcb, ncb, pcb});

		/* Acquire realm from RealmCallback */
		authRealm = tcb.getText();
		if (authRealm == null) {
		    authRealm = "";
		}
	    } else {
		RealmChoiceCallback ccb = new RealmChoiceCallback(
		    "DIGEST-MD5 realm: ",
		    realmTokens,
		    0, false);
		cbh.handle(new Callback[] {ccb, ncb, pcb});

		/* Acquire realm from RealmChoiceCallback*/
		authRealm = realmTokens[ccb.getSelectedIndexes()[0]];
	    }

	    passwd = pcb.getPassword();
	    pcb.clearPassword();
	    userName = ncb.getName();

	} catch (UnsupportedCallbackException e) {
	    throw new SaslException("DIGEST-MD5: Cannot perform callback to " +
		"acquire realm, authentication ID or password", e);	

	} catch (IOException e) {
	    throw new SaslException(
		"DIGEST-MD5: Error acquiring realm, authentication ID or password", e);
	}

	if (userName == null || passwd == null) {
	    throw new SaslException(
		"DIGEST-MD5: authentication ID and password must be specified");
	}

	/* MAXBUF: optional atmost once */	
	int srvMaxBufSize =
	    (challengeVal[MAXBUF] == null) ? 65536 
	    : Integer.parseInt(new String(challengeVal[MAXBUF], encoding));
	sendMaxBufSize = 
	    (sendMaxBufSize == 0) ? srvMaxBufSize 
	    : Math.min(sendMaxBufSize, srvMaxBufSize);
    }

    /**
     * Parses the 'qop' directive. If 'auth-conf' is specified by
     * the client and offered as a QOP option by the server, then a check
     * is client-side supported ciphers is performed.
     *
     * @throws SaslException
     */
    private void checkQopSupport() throws IOException {

	/* QOP: optional; if multiple, merged earlier */
	if (challengeVal[QOP] == null) {	
	    selectedQopStr = "auth";
	    selectedQop = NO_PROTECTION;
	    
	} else { // process

	    String qopOptions = new String(challengeVal[QOP], encoding);

	    String[] serverQopTokens = new String[3];
	    byte[] serverQop = parseQop(qopOptions, serverQopTokens, 
		true /* ignore unrecognized tokens */);
	    byte serverAllQop = combineMasks(serverQop);
	    byte qopLevel = findPreferredMask(serverAllQop, qop);

	    switch (qopLevel) {
	    case 0:
		throw new SaslException("DIGEST-MD5: No common protection " +
		    "layer between client and server");
	    
	    case NO_PROTECTION:
		selectedQopStr = serverQopTokens[2];
		break;

	    case INTEGRITY_ONLY_PROTECTION:
		selectedQopStr = serverQopTokens[1];
		integrity = true;
		break;

	    case PRIVACY_PROTECTION:
		selectedQopStr = serverQopTokens[0];
		privacy = integrity = true;
		checkStrengthSupport();
		break;
	    }

	    if (integrity && privacy) {	    
		rawSendSize = sendMaxBufSize - 26;
	    } else if (integrity) {
		rawSendSize = sendMaxBufSize - 16;
	    }
	}

	if (debug) {
	    System.err.println("rawSendSize: " + rawSendSize);
	}
     }
    
    /**
     * Processes the 'cipher' digest-challenge directive. This allows the
     * mechanism to check for client-side support against the list of
     * supported ciphers send by the server. If no match is found,
     * the mechanism aborts.
     * 
     * @throws SaslException If an error is encountered in processing
     * the cipher digest-challenge directive or if no client-side
     * support is found.
     */
    private void checkStrengthSupport() throws IOException {
	
        /* CIPHER: required exactly once if qop=auth-conf */	
	if (challengeVal[CIPHER] == null) {
	    throw new SaslException("DIGEST-MD5: server did not specify " +
		"cipher to use for 'auth-conf'");
	}
	    	
	// only need to check cipher support if 'auth-conf'

	// First determine ciphers that server supports
	String cipherOptions = new String(challengeVal[CIPHER], encoding);
	StringTokenizer parser = new StringTokenizer(cipherOptions, ", \t\n");
	int tokenCount = parser.countTokens();	    
	String token = null;
	byte[] serverCiphers = { UNSET,
				 UNSET,
				 UNSET,
				 UNSET,
				 UNSET }; 
	String[] serverCipherStrs = new String[serverCiphers.length];

	// Parse the digest-challenge; mark each that server supports
	for (int i = 0; i < tokenCount; i++) {
	    token = parser.nextToken();
	    for (int j = 0; j < CIPHER_TOKENS.length; j++) {
		if (token.equalsIgnoreCase(CIPHER_TOKENS[j])) {
		    serverCiphers[j] |= CIPHER_MASKS[j];
		    serverCipherStrs[j] = token; // keep for replay to server
		    if (debug) {
			System.err.println("server supports " + token);
		    }
		}
	    }	
	}

	// Determine which ciphers are available on client
	Provider[] providers = java.security.Security.getProviders();
	Set set;

	byte[] clntCiphers = new byte[serverCiphers.length];
	for (int j = 0; j < providers.length; j++) {		
	    set = providers[j].keySet();
	    
	    if (set.contains(JCE_CIPHER_NAME[DES3])) {
		if (debug) {
		    System.err.println("client supports 3des");
		}
		clntCiphers[DES3] |= CIPHER_MASKS[DES3]; 
	    }
	    if (set.contains(JCE_CIPHER_NAME[DES])) {
		if (debug) {
		    System.err.println("client supports des");
		}
		clntCiphers[DES] |= CIPHER_MASKS[DES]; 
	    }
	    if (set.contains(JCE_CIPHER_NAME[RC4])) {
		if (debug) {
		    System.err.println("client supports rc4");
		}
		clntCiphers[RC4] |= CIPHER_MASKS[RC4]; 
		clntCiphers[RC4_56] |= CIPHER_MASKS[RC4_56]; 
		clntCiphers[RC4_40] |= CIPHER_MASKS[RC4_40];
	    }			
	}

	// Take intersection of server and client supported ciphers
	byte inter = 0;
	for (int i = 0; i < serverCiphers.length; i++) {
	    serverCiphers[i] &= clntCiphers[i];
	    inter |= serverCiphers[i];
	}

	if (inter == UNSET) {
	    throw new SaslException(
		"DIGEST-MD5: Client supports none of these cipher suites: " +
		cipherOptions);
	} 
   
	// now have a clear picture of user / client; client / server
	// cipher options. Leverage strength array against what is
	// supported to choose a cipher.   
	cipherSuite = findCipherAndStrength(serverCiphers, serverCipherStrs);

	if (debug) {
	    System.err.println("cipherSuite: " + cipherSuite);
	}

	if (cipherSuite == null) {
	    throw new SaslException("DIGEST-MD5: Unable to negotiate " +
		"a strength level for 'auth-conf'");
	}
	     	
    }

    /**	
     * Steps through the ordered 'strength' array, and compares it with 
     * the 'supportedCiphers' array. The cipher returned represents
     * the best possible cipher based on the strength preference and the 
     * available ciphers on both the server and client environments.
     *
     * @param tokens The array of cipher tokens sent by server
     * @return The agreed cipher.
     */
    private String findCipherAndStrength(byte[] supportedCiphers, 
	String[] tokens) {
	byte s;
	for (int i = 0; i < strength.length; i++) {
	    if ((s=strength[i]) != 0) {
		for (int j = 0; j < supportedCiphers.length; j++) {

		    // If user explicitly requested cipher, then it
		    // must be the one we choose

		    if (s == supportedCiphers[j] &&
			(specifiedCipher == null || 
			    specifiedCipher.equals(tokens[j]))) {
			switch (s) {
			case HIGH_STRENGTH:
			    negotiatedStrength = "high";
			    break;
			case MEDIUM_STRENGTH:
			    negotiatedStrength = "medium";
			    break;
			case LOW_STRENGTH:
			    negotiatedStrength = "low";
			    break;
			}
			
			return tokens[j];
		    }
		}
	    }
	}
	
	return null;  // none found
    }

    /**
     * Returns digest-response suitable for an initial authentication.
     * 
     * The following are qdstr-val (quoted string values) as per RFC 2831,
     * which means that any embedded quotes must be escaped.
     *    realm-value
     *    nonce-value
     *    username-value
     *    cnonce-value
     *    authzid-value
     * @returns <tt>digest-response</tt> in a byte array
     * @throws SaslException if there is an error generating the
     * response value or the cnonce value.
     */
    private byte[] generateDigestResponse() throws IOException {

	ByteArrayOutputStream digestResp = new ByteArrayOutputStream();

	if (useUTF8) {
	    digestResp.write("charset=".getBytes(encoding));
	    digestResp.write(challengeVal[CHARSET]);
	    digestResp.write(',');
	}

	digestResp.write(("username=\"" + 
	    quotedStringValue(userName) + "\",").getBytes(encoding));
	digestResp.write(("realm=\"" + 
	    quotedStringValue(authRealm) + "\",").getBytes(encoding));

	digestResp.write("nonce=\"".getBytes(encoding));
	writeQuotedStringValue(digestResp, nonce);
	digestResp.write('"');
	digestResp.write(',');

	digestResp.write(("nc=" + nonceCountToHex() + ",").getBytes(encoding));

	c_nonce = generateCNONCE();
	digestResp.write("cnonce=\"".getBytes(encoding));
	writeQuotedStringValue(digestResp, c_nonce);
	digestResp.write("\",".getBytes(encoding));
	digestResp.write(("digest-uri=\"" + digestUri + "\",").getBytes(encoding));

	digestResp.write("maxbuf=".getBytes(encoding));
	digestResp.write(String.valueOf(recvMaxBufSize).getBytes(encoding));
	digestResp.write(",".getBytes(encoding));
	    	    		
	try {
	    digestResp.write("response=".getBytes(encoding));
	    digestResp.write(generateResponseValue("AUTHENTICATE"));
	    digestResp.write(",".getBytes(encoding));
	} catch (Exception e) {
	    throw new SaslException(
		"DIGEST-MD5: Error generating response value", e);
	}
	
	digestResp.write(("qop=" + selectedQopStr).getBytes(encoding));

	if (cipherSuite != null) {
	    digestResp.write((",cipher=\"" + cipherSuite + "\"").getBytes(encoding));
	}
		
	if (authzId != null) {
	    digestResp.write(",authzid=\"".getBytes(encoding));
	    writeQuotedStringValue(digestResp, authzId);
	    digestResp.write("\"".getBytes(encoding));
	}	    
	
	if (digestResp.size() > MAX_RESPONSE_LENGTH) {
	    throw new SaslException ("DIGEST-MD5: digest-response size too " +
		"large. Length: "  + digestResp.size());
	}
	return digestResp.toByteArray();
     }

    /** 
     * Assembles response-value for digest-response.
     *
     * @param authMethod "AUTHENTICATE" for client-generated response;
     *        "" for server-generated response
     * @return A non-null byte array containing the repsonse-value.
     * @throws NoSuchAlgorithmException if the platform does not have MD5 
     * digest support.
     * @throws UnsupportedEncodingException if a an error occurs
     * encoding a string into either Latin-1 or UTF-8.
     * @throws IOException if an error occurs writing to the output
     * byte array buffer.
     */
    private byte[] generateResponseValue(String authMethod)
	throws NoSuchAlgorithmException,
	UnsupportedEncodingException, IOException {

	byte[] digest;
	MessageDigest md5 = MessageDigest.getInstance("MD5");
	byte[] hexA1, hexA2 = null;
	ByteArrayOutputStream A2, beginA1, A1, KD = null;

	// A2 
	// --
	// A2 = { "AUTHENTICATE:", digest-uri-value,
  	// [:00000000000000000000000000000000] }  // if auth-int or auth-conf
        //
	A2 = new ByteArrayOutputStream();
	A2.write((authMethod + ":" + digestUri).getBytes(encoding));
	if (selectedQopStr.equals("auth-conf") ||
	    selectedQopStr.equals("auth-int")) {
	    if (debug) {
		System.err.println("selectedQopStr: " + selectedQopStr);
	    }
	    A2.write(":00000000000000000000000000000000".getBytes(encoding));
	}
	md5.update(A2.toByteArray());
	digest = md5.digest();
	hexA2 = binaryToHex(digest); 

	// A1 
	// --
	// H(user-name : realm-value : passwd) 
	//
	beginA1 = new ByteArrayOutputStream();
	byte[] colon = ":".getBytes(encoding);
        beginA1.write(stringToByte_8859_1(userName));  
        beginA1.write(colon); 
	// if no realm, realm will be an empty string	
	beginA1.write(stringToByte_8859_1(authRealm));	
	beginA1.write(colon); 
        beginA1.write(stringToByte_8859_1(new String(passwd)));

	md5.update(beginA1.toByteArray());
	digest = md5.digest(); 

	if (debug) {
	    System.err.print("H(" + beginA1.toString() + ")");
	    System.err.println(" = " + new String(binaryToHex(digest)));
	}
	    
	// A1
	// --       
	// A1 = { H ( {user-name : realm-value : passwd } ), 
	// : nonce-value, : cnonce-value : authzid-value
	//
	A1 = new ByteArrayOutputStream();
	A1.write(digest);   
	A1.write(colon);
	A1.write(nonce);
	A1.write(colon);
	A1.write(c_nonce);

	if (authzId != null) {
	    A1.write(colon);
	    A1.write(authzId);
	}	    
	md5.update(A1.toByteArray());
	digest = md5.digest(); 	
	H_A1 = digest; // Record H(A1). Use for integrity & privacy.
	hexA1 = binaryToHex(digest);

	if (debug) {
	    System.err.println("H(A1) = " + new String(hexA1));
	}
	
	// 
	// H(k, : , s);
	//
	KD = new ByteArrayOutputStream();
	KD.write(hexA1);
	KD.write(colon);
        KD.write(nonce);
        KD.write(colon);		  
	KD.write(nonceCountToHex().getBytes(encoding));
	KD.write(colon);
        KD.write(c_nonce);
	KD.write(colon);
	KD.write(selectedQopStr.getBytes(encoding));
	KD.write(colon);
	KD.write(hexA2);
	md5.update(KD.toByteArray());
	digest = md5.digest(); 

	return (binaryToHex(digest));
    }   

    /** 
     * Generate random-string used for digest-response.
     * This method uses Random to get random bytes and then
     * base64 encodes the bytes. Could also use binaryToHex() but this
     * is slightly faster and a more compact representation of the same info.
     * @return A non-null byte array containing the CNONCE value for the 
     * digest-respnse.
     * Could use SecureRandom to be more secure but it is very slow.
     */

    /** This array maps the characters to their 6 bit values */
    private final static char pem_array[] = {
	//       0   1   2   3   4   5   6   7
		'A','B','C','D','E','F','G','H', // 0
		'I','J','K','L','M','N','O','P', // 1
		'Q','R','S','T','U','V','W','X', // 2
		'Y','Z','a','b','c','d','e','f', // 3
		'g','h','i','j','k','l','m','n', // 4
		'o','p','q','r','s','t','u','v', // 5
		'w','x','y','z','0','1','2','3', // 6
		'4','5','6','7','8','9','+','/'  // 7
    };

    // Make sure that this is a multiple of 3
    private static final int RAW_CNONCE_SIZE = 30;

    // Base 64 encoding turns each 3 bytes into 4
    private static final int ENCODED_CNONCE_SIZE = RAW_CNONCE_SIZE*4/3;
    
    private static final byte[] generateCNONCE() {

	// SecureRandom random = new SecureRandom();
	Random random = new Random();
	byte[] randomData = new byte[RAW_CNONCE_SIZE];
	random.nextBytes(randomData);

	byte[] cnonce = new byte[ENCODED_CNONCE_SIZE];

	// Base64-encode bytes
	byte a, b, c;
	int j = 0;
        for (int i = 0; i < randomData.length; i += 3) {
	    a = randomData[i];
	    b = randomData[i+1];
	    c = randomData[i+2];
	    cnonce[j++] = (byte)(pem_array[(a >>> 2) & 0x3F]);
	    cnonce[j++] = (byte)(pem_array[((a << 4) & 0x30) + ((b >>> 4) & 0xf)]);
	    cnonce[j++] = (byte)(pem_array[((b << 2) & 0x3c) + ((c >>> 6) & 0x3)]);
	    cnonce[j++] = (byte)(pem_array[c & 0x3F]);
        }

	return cnonce;

	// %%% For testing using RFC 2831 example, uncomment the following 2 lines
	// System.out.println("!!!Using RFC 2831's cnonce for testing!!!");
	// return "OA6MHXh6VqTrRk".getBytes();
    }

    /**
     * Takes 'nonceCount' value and returns HEX value of the value.
     *  
     * @return A non-null String representing the current NONCE-COUNT
     */
    private String nonceCountToHex() {

	String str = Integer.toHexString(nonceCount);
	StringBuffer pad = new StringBuffer();

	if (str.length() < 8) {
	    for (int i = 0; i < 8-str.length(); i ++) {
		pad.append("0");
	    }
	}
	
	return new String(pad + str);
    }	    

    /**  
     * Convert a byte array to hexadecimal string.
     *
     * @param a non-null byte array
     * @return a non-null String contain the HEX value
     */
    private byte[] binaryToHex(byte[] digest) throws UnsupportedEncodingException {

	StringBuffer digestString = new StringBuffer();

	for (int i = 0; i < digest.length; i ++) {
	    if ((digest[i] & 0x000000ff) < 0x10) {
		digestString.append("0"+
		    Integer.toHexString(digest[i] & 0x000000ff));
	    } else {
		digestString.append(
		    Integer.toHexString(digest[i] & 0x000000ff));
	    }
	}
	return digestString.toString().getBytes(encoding);
    }

    /**
     * Used to convert username-value, passwd or realm to 8859_1 encoding
     * if all chars in string are within the 8859_1 (Latin 1) encoding range.
     * 
     * @param a non-null String 
     * @return a non-nuill byte array containing the correct character encoding
     * for username, paswd or realm.
     */
    private byte[] stringToByte_8859_1(String str) throws SaslException {
	
	char[] buffer = str.toCharArray();

	try {
	    if (useUTF8) {
		for( int i = 0; i< buffer.length; i++ ) {
		    if( buffer[i] > '\u00FF' ) {
			return str.getBytes("UTF8");
		    } 
		}
	    }
	    return str.getBytes("8859_1");
	} catch (UnsupportedEncodingException e) {
	    throw new SaslException(
		"cannot encode string in UTF8 or 8859-1 (Latin-1)", e);
	}	    
    }

    public String getNegotiatedProperty(String propName) throws SaslException {
	if (completed) {
	    if (propName.equals(Sasl.STRENGTH)) {
		return negotiatedStrength;
	    } else {
		return super.getNegotiatedProperty(propName);
	    }
	} else {
	    throw new SaslException("Not completed");
	}
    }

    // See Section 7.2 of RFC 2831; double-quote character is not allowed
    // unless escaped; also escape the escape character and CTL chars except LWS
    private static boolean needEscape(String str) {
	int len = str.length();
	for (int i = 0; i < len; i++) {
	    if (needEscape(str.charAt(i))) {
		return true;
	    }
	}
	return false;
    }

    // Determines whether a character needs to be escaped in a quoted string
    private static boolean needEscape(char ch) {
	return ch == '"' ||  // escape char
	    ch == '\\' ||    // quote
	    ch == 127 ||     // DEL

	    // 0 <= ch <= 31 except CR, HT and LF
	    (ch >= 0 && ch <= 31 && ch != 13 && ch != 9 && ch != 10);
    }

    private static String quotedStringValue(String str) {
	if (needEscape(str)) {
	    int len = str.length();
	    char[] buf = new char[len+len];
	    int j = 0;
	    char ch;
	    for (int i = 0; i < len; i++) {
		ch = str.charAt(i);
		if (needEscape(ch)) {
		    buf[j++] =  '\\';
		} 
		buf[j++] = ch;
	    }
	    return new String(buf, 0, j);
	} else {
	    return str;
	}
    }

    // Check if a byte[] contains characters that must be quoted
    // and write the resulting, possibly escaped, characters to out.
    private static void writeQuotedStringValue(ByteArrayOutputStream out, 
	byte[] buf) {

	int len = buf.length;
	byte ch;
	for (int i = 0; i < len; i++) {
	    ch = buf[i];
	    if (needEscape((char)ch)) {
		out.write('\\');
	    } 
	    out.write(ch);
	}
    }

    /**
     * From RFC 2831, Section 2.1.3: Step Three
     * [Server] sends a message formatted as follows:
     *     response-auth = "rspauth" "=" response-value
     * where response-value is calculated as above, using the values sent in
     * step two, except that if qop is "auth", then A2 is
     *
     *  A2 = { ":", digest-uri-value }
     *
     * And if qop is "auth-int" or "auth-conf" then A2 is
     *
     *  A2 = { ":", digest-uri-value, ":00000000000000000000000000000000" }
     */
    private void verifyResponseValue(byte[] fromServer) throws SaslException {
	if (fromServer == null) {
	    throw new SaslException("DIGEST-MD5: Authenication failed. " +
		"Expecting 'rspauth' authentication success message");
	}

	try {
	    byte[] expectedFromServer = generateResponseValue("");
	    if (!Arrays.equals(expectedFromServer, fromServer)) {
		/* Server's rspauth value does not match */
		throw new SaslException(
		    "Server's rspauth value does not match what client expects");
	    }
	} catch (NoSuchAlgorithmException e) {
	    throw new SaslException(
		"Problem generating response-value for verification", e);
	} catch (IOException e) {
	    throw new SaslException(
		"Problem generating response-value for verification", e);
	}
    }
}
