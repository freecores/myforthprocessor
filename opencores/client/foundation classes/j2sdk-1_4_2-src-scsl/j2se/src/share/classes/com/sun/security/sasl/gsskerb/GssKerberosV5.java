/*
 * @(#)GssKerberosV5.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.gsskerb; 

import com.sun.security.sasl.preview.*;
import com.sun.security.sasl.util.SaslImpl;
import java.io.IOException;
import java.util.Map;

// JAAS
import javax.security.auth.callback.CallbackHandler;

// JGSS
import org.ietf.jgss.*;

/**
  * Implements the GSSAPI SASL mechanism for Kerberos V5. 
  * (<A HREF="ftp://ftp.isi.edu/in-notes/rfc2222.txt">RFC 2222</A>,
  * <a HREF="http://www.ietf.org/internet-drafts/draft-ietf-cat-sasl-gssapi-04.txt">draft-ietf-cat-sasl-gssapi-04.txt</a>).
  * It uses the Java Bindings for GSSAPI
  * (<A HREF="ftp://ftp.isi.edu/in-notes/rfc2853.txt">RFC 2853</A>)
  * for getting GSSAPI/Kerberos V5 support.
  *
  * The client/server interactions are:
  * C0: bind (GSSAPI, initial response)
  * S0: sasl-bind-in-progress, challenge 1 (output of accept_sec_context or [])
  * C1: bind (GSSAPI, response 1 (output of init_sec_context or []))
  * S1: sasl-bind-in-progress challenge 2 (security layer, server max recv size)
  * C2: bind (GSSAPI, response 2 (security layer, client max recv size, authzid))
  * S2: bind success response
  *
  * @author Rosanna Lee
  */

final class GssKerberosV5 extends SaslImpl implements SaslClient {
    // ---------------- Constants -----------------
    static final private String KRB5_OID_STR = "1.2.840.113554.1.2.2";
    static private Oid KRB5_OID;

    static {
	try {
	    KRB5_OID = new Oid(KRB5_OID_STR);
	} catch (GSSException ignore) {}
    }

    private boolean finalHandshake = false; 
    private boolean mutual = false;       // default false
    private int gssQop = 0;
    private byte[] authzID;
    private GSSContext secCtx = null;
    private MessageProp msgProp;          // QOP and privacy for unwrap


    /**
     * Creates a SASL mechanism with client credentials that it needs 
     * to participate in GSS-API/Kerberos v5 authentication exchange 
     * with the server.
     */
    GssKerberosV5(String authzID, String protocol, String serverName,
	Map props, CallbackHandler cbh) throws SaslException {
	    super(props);

	String service = protocol + "@" + serverName;

	try {
	    GSSManager mgr = GSSManager.getInstance();

	    // Create the name for the requested service entity for Krb5 mech
	    GSSName acceptorName = mgr.createName(service,
		GSSName.NT_HOSTBASED_SERVICE, KRB5_OID);

	    // Create a context using default credentials for Krb5 mech
	    secCtx = mgr.createContext(acceptorName,
		KRB5_OID, /* mechanism */
		null,       /* default credentials */
		GSSContext.INDEFINITE_LIFETIME);

	    // Parse properties  to set desired context options
	    if (props != null) {
		// Mutual authentication
		String prop = (String)props.get(Sasl.SERVER_AUTH);
		if (prop != null) {
		    mutual = "true".equalsIgnoreCase(prop);
		}
	    }
	    secCtx.requestMutualAuth(mutual);
	    if (debug) {
		System.err.println("client allQop: " + allQop);
	    }

	    // Always specify potential need for integrity and confidentiality
	    // Decision will be made during final handshake
	    secCtx.requestConf(true);
	    secCtx.requestInteg(true);
		
	} catch (GSSException e) {
	    throw new SaslException("Failure to initialize security context", e);
	}

	if (authzID != null && authzID.length() > 0) {
	    try {
		this.authzID = authzID.getBytes("UTF8");
	    } catch (IOException e) {
		throw new SaslException("Cannot encode authorization ID", e);
	    }
	}
    }

    public boolean hasInitialResponse() {
	return true;
    }

    synchronized public void dispose() throws SaslException {
	if (secCtx != null) {
	    try {
		secCtx.dispose();
	    } catch (GSSException e) {
		throw new SaslException("Problem disposing GSS context", e);
	    }
	    secCtx = null;
	}
    }

    protected void finalize() throws Throwable {
	dispose();
    }

    /**
     * Processes the challenge data.
     * 
     * The server sends a challenge data using which the client must
     * process using GSS_Init_sec_context.
     * As per RFC 2222, when GSS_S_COMPLETE is returned, we do
     * an extra handshake to determine the negotiated security protection
     * and buffer sizes.
     *
     * @param challengeData A non-null byte array containing the
     * challenge data from the server.
     * @return A non-null byte array containing the response to be
     * sent to the server.
     */
    public byte[] evaluateChallenge(byte[] challengeData) throws SaslException {
	if (completed) {
	    throw new SaslException(
		"SASL authentication already complete");
	}

	if (finalHandshake) {
	    return doFinalHandshake(challengeData);
	} else {

	    // Security context not established yet; continue with init

	    try {
		byte[] gssOutToken = secCtx.initSecContext(challengeData,
		    0, challengeData.length);

		if (debug) {
		    traceOutput("initial response: [raw]", challengeData);
		    traceOutput("initial response: [after initSecCtx]",
			gssOutToken);
		}
		    

		if (secCtx.isEstablished()) {
		    finalHandshake = true;
		    if (gssOutToken == null) {
			// RFC 2222 7.2.1:  Client responds with no data
			return new byte[0];
		    }
		}

		return gssOutToken;
	    } catch (GSSException e) {
		throw new SaslException("GSS initiate failed", e);
	    }
	}
    }

    private byte[] doFinalHandshake(byte[] challengeData) throws SaslException {
	try {
	    // Security context already established. challengeData
	    // should contain security layers and server's maximum buffer size

	    if (debug) {
		traceOutput("challenge [raw]:", challengeData);
	    }

	    if (challengeData.length == 0) {
		// Received S0, should return []
		return new byte[0];
	    }

	    // Received S1 (security layer, server max recv size)
	    
	    byte[] gssOutToken = secCtx.unwrap(challengeData, 0,
		challengeData.length, new MessageProp(0, false));

	    // First octet is a bit-mask specifying the protections
	    // supported by the server
	    if (debug) {
		traceOutput("challenge [unwrapped]:", gssOutToken);
		System.err.println("Server protections: " + gssOutToken[0]);
	    }

	    // Client selects preferred protection
	    // qop is ordered list of qop values
	    byte selectedQop = findPreferredMask(gssOutToken[0], qop);

	    if (selectedQop == 0) {
		throw new SaslException(
		    "No common protection layer between client and server");
	    }

	    if ((selectedQop&PRIVACY_PROTECTION) != 0) {
		privacy = true;
		integrity = true;
	    } else if ((selectedQop&INTEGRITY_ONLY_PROTECTION) != 0) {
		integrity = true;
	    } 

	    // 2nd-4th octets specifies maximum buffer size expected by
	    // server (in network byte order)
	    int srvMaxBufSize = networkByteOrderToInt(gssOutToken, 1, 3);

	    // Determine the max send buffer size based on what the
	    // server is able to receive and our specified max
	    sendMaxBufSize = (sendMaxBufSize == 0) ? srvMaxBufSize :
		Math.min(sendMaxBufSize, srvMaxBufSize);

	    // Update context to limit size of returned buffer
	    rawSendSize = secCtx.getWrapSizeLimit(gssQop, privacy,
		sendMaxBufSize);

	    if (debug) {
		System.err.println("client max recv size: " + recvMaxBufSize);
		System.err.println("server max recv size: " + srvMaxBufSize);
		System.err.println("rawSendSize: " + rawSendSize);
	    }

	    // Construct negotiated security layers and client's max
	    // receive buffer size and authzID
	    int len = 4;
	    if (authzID != null) {
		len += authzID.length;
	    }

	    byte[] gssInToken = new byte[len];
	    gssInToken[0] = selectedQop;

	    if (debug) {
		System.err.println("selected protection: " + selectedQop);
		System.err.println("privacy: " + privacy);
		System.err.println("integrity: " + integrity);
	    }

	    intToNetworkByteOrder(recvMaxBufSize, gssInToken, 1, 3);
	    if (authzID != null) {
		// copy authorization id
		System.arraycopy(authzID, 0, gssInToken, 4, authzID.length);
	    }

	    if (debug) {
		traceOutput("response [raw]", gssInToken);
	    }

	    gssOutToken = secCtx.wrap(gssInToken,
		0, gssInToken.length,
		new MessageProp(0 /* qop */, false /* privacy */));

	    if (debug) {
		traceOutput("response [after wrap]", gssOutToken);
	    }
	    
	    completed = true;  // server authenticated
	    msgProp = new MessageProp(gssQop, privacy);

	    return gssOutToken;
	} catch (GSSException e) {
	    throw new SaslException("Final handshake failed", e);
	}
    }


    /**
     * Retrieves this mechanism's name.
     *
     * @return  The string "GSSAPI".
     */
    public String getMechanismName() {
	return "GSSAPI";
    }

    public byte[] unwrap(byte[] incoming, int start, int len) 
	throws SaslException {
	if (!completed) {
	    throw new SaslException("Not completed");
	}

	try {
	    return secCtx.unwrap(incoming, start, len, msgProp);
	} catch (GSSException e) {
	    throw new SaslException("Problems unwrapping SASL buffer", e);
	}
    }

    public byte[] wrap(byte[] outgoing, int start, int len) throws SaslException {
	if (!completed) {
	    throw new SaslException("Not completed");
	}

	// Generate GSS token 
	try {
	    return secCtx.wrap(outgoing, start, len, msgProp);

	} catch (GSSException e) {
	    throw new SaslException("Problem performing GSS wrap", e);
	}
    }
}
