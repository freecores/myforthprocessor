/*
 * @(#)External.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl; 

import java.io.IOException;
import com.sun.security.sasl.preview.*;

/**
  * Implements the EXTERNAL SASL mechanism. 
  * (<A HREF="ftp://ftp.isi.edu/in-notes/rfc2222.txt">RFC 2222</A>).
  * The EXTERNAL mechanism returns the optional authorization ID as
  * the initial response. It processes no challenges.
  *
  * @author Rosanna Lee
  */
public class External implements SaslClient {
    private byte[] username;
    private boolean completed = false;

    /**
     * Constructs an External mechanism with optional authorization ID.
     * 
     * @param authorizationID If non-null, used to specify authorization ID.
     * @throws SaslException if cannot convert authorizationID into UTF-8
     *     representation.
     */
    public External(String authorizationID) throws SaslException {
	if (authorizationID != null) {
	    try {
		username = ((String)authorizationID).getBytes("UTF8");
	    } catch (java.io.UnsupportedEncodingException e) {
		throw new SaslException("Cannot convert " + authorizationID +
		    " into UTF-8", e);
	    }
	} else {
	    username = new byte[0];
	}
    }

    /**
     * Retrieves this mechanism's name for initiating the "EXTERNAL" protocol
     * exchange.
     *
     * @return  The string "EXTERNAL".
     */
    public String getMechanismName() {
	return "EXTERNAL";
    }

    /**
     * This mechanism has an initial response.
     */
    public boolean hasInitialResponse() {
	return true;
    }

    public void dispose() throws SaslException {
    }

    /**
     * Processes the challenge data.
     * It returns the EXTERNAL mechanism's initial response, 
     * which is the authorization id encoded in UTF-8.
     * This is the optional information that is sent along with the SASL command.
     * After this method is called, isComplete() returns true.
     * 
     * @param challengeData Ignored.
     * @return The possible empty initial response.
     * @throws SaslException If authentication has already been called.
     */
    public byte[] evaluateChallenge(byte[] challengeData) 
	throws SaslException {
	if (completed) {
	    throw new SaslException("Already completed");
	}
	completed = true;
	return username;
    }

    /**
     * Returns whether this mechanism is complete.
     * @return true if initial response has been sent; false otherwise.
     */
    public boolean isComplete() {
	return completed;
    }

    /**
      * Unwraps the incoming buffer.
      *
      * @throws SaslException Not applicable to this mechanism.
      */
    public byte[] unwrap(byte[] incoming, int offset, int len)
	throws SaslException {
	if (completed) {
	    throw new SaslException("EXTERNAL has no supported QOP");
	} else {
	    throw new SaslException("Not completed");
	}
    }

    /**
      * Wraps the outgoing buffer.
      *
      * @throws SaslException Not applicable to this mechanism.
      */
    public byte[] wrap(byte[] outgoing, int offset, int len) 
	throws SaslException {
	if (completed) {
	    throw new SaslException("EXTERNAL has no supported QOP");
	} else {
	    throw new SaslException("Not completed");
	}
    }

    /**
     * Retrieves the negotiated property.
     * This method can be called only after the authentication exchange has
     * completed (i.e., when <tt>isComplete()</tt> returns true); otherwise, a
     * <tt>SaslException</tt> is thrown.
     * 
     * @return null No property is applicable to this mechanism.
     * @exception SaslException if this authentication exchange has not completed
     */
    public String getNegotiatedProperty(String propName) throws SaslException {
	if (completed) {
	    return null;
	} else {
	    throw new SaslException("Not completed");
	}
    }
}

