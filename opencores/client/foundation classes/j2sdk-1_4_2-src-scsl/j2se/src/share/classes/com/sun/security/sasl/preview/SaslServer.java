/*
 * @(#)SaslServer.java	1.10 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

/**
 * Performs SASL authentication as a server.
 *<p>
 * A server such an LDAP server gets an instance of this
 * class in order to perform authentication defined by a specific SASL
 * mechanism. Invoking methods on the <tt>SaslServer</tt> instance
 * generates challenges according to the SASL
 * mechanism implemented by the <tt>SaslServer</tt>.
 * As the authentication proceeds, the instance
 * encapsulates the state of a SASL server's authentication exchange. 
 *<p>
 * Here's an example of how an LDAP server might use a <tt>SaslServer</tt>.
 * It first gets an instance of a <tt>SaslServer</tt> for the SASL mechanism
 * requested by the client:
 *<blockquote><pre>
 * SaslServer ss = Sasl.createSaslServer(mechanism,
 *     "ldap", myFQDN, props, callbackHandler);
 *</pre></blockquote>
 * It can then proceed to use the server for authentication.
 * For example, suppose the LDAP server received an LDAP BIND request
 * containing the name of the SASL mechanism and an (optional) initial
 * response. It then might use the server as follows:
 *<blockquote><pre>
 * while (!ss.isComplete()) {
 *     try {
 *         byte[] challenge = ss.evaluateResponse(response);
 *         if (ss.isComplete()) {
 *             ldap.sendBindResponse(mechanism, challenge, SUCCESS);
 *         } else {
 *             ldap.sendBindResponse(mechanism, challenge, SASL_BIND_IN_PROGRESS);
 *             response = ldap.readBindRequest();
 *         }
 *     } catch (SaslException e) {
 * 	    ldap.sendErrorResponse(e);
 * 	    break;
 *     }
 * }
 * if (ss.isComplete() && res.status == SUCCESS) {
 *    String qop = sc.getNegotiatedProperty(Sasl.QOP);
 *    if (qop != null 
 *        && (qop.equalsIgnoreCase("auth-int") 
 *            || qop.equalsIgnoreCase("auth-conf"))) {
 *
 *      // Use SaslServer.wrap() and SaslServer.unwrap() for future
 *      // communication with client
 *	ldap.in = new SecureInputStream(ss, ldap.in);
 *	ldap.out = new SecureOutputStream(ss, ldap.out);
 *    }
 * }
 *</pre></blockquote>
 *
 * @see Sasl
 * @see SaslServerFactory
 *
 * @author Rosanna Lee
 * @author Rob Weltman
 */
public abstract interface SaslServer {

    /**
     * Returns the IANA-registered mechanism name of this SASL server.
     * (e.g. "CRAM-MD5", "GSSAPI").
     * @return A non-null string representing the IANA-registered mechanism name.
     */
    public abstract String getMechanismName();

    /**
     * Evaluates the response data and generates a challenge.
     *
     * If a response is received from the client during the authentication 
     * process, this method is called to prepare an appropriate next 
     * challenge to submit to the client. The challenge is null if the 
     * authentication has succeeded and no more challenge data is to be sent 
     * to the client. It is non-null if the authentication must be continued 
     * by sending a challenge to the client, or if the authentication has 
     * succeeded but challenge data needs to be processed by the client. 
     * <tt>isComplete()</tt> should be called 
     * after each call to <tt>evaluateResponse()</tt>,to determine if any further 
     * response is needed from the client. 
     *
     * @param response The non-null (but possibly empty) response sent
     * by the client.
     *
     * @return The possibly null challenge to send to the client.
     * It is null if the authentication has succeeded and there is
     * no more challenge data to be sent to the client.
     * @exception SaslException If an error occurred while processing
     * the response or generating a challenge.
     */
    public abstract byte[] evaluateResponse(byte[] response) 
	throws SaslException;

    /**
      * Determines whether the authentication exchange has completed.
      * This method is typically called after each invocation of 
      * <tt>evaluateResponse()</tt> to determine whether the 
      * authentication has completed successfully or should be continued. 
      * @return true if the authentication exchange has completed; false otherwise.
      */
    public abstract boolean isComplete();

    /**
     * Reports the authorization ID in effect for the client of this
     * session. 
     * This method can only be called if isComplete() returns true. 
     * @return The authorization ID of the client.
     * @exception SaslException if this authentication session has not completed
     */
    public String getAuthorizationID() throws SaslException;
	
    /**
     * Unwraps a byte array received from the client.
     * This method can be called only after the authentication exchange has
     * completed (i.e., when <tt>isComplete()</tt> returns true) and only if
     * the authentication exchange has negotiated integrity and/or privacy 
     * as the quality of protection; otherwise, a <tt>SaslException</tt> is thrown.
     *<p>
     * <tt>incoming</tt> is the contents of the SASL buffer as defined in RFC 2222
     * without the leading four octet field that represents the length.
     * <tt>offset</tt> and <tt>len</tt> specify the portion of <tt>incoming</tt>
     * to use.
     *
     * @param incoming A non-null byte array containing the encoded bytes
     * 		      from the client.
     * @param offset The starting position at <tt>incoming</tt> of the bytes to use.
     * @param len The number of bytes from <tt>incoming</tt> to use.
     * @return A non-null byte array containing the decoded bytes.
     * @exception SaslException if the authentication exchange has not completed,
     * if the negotiated quality of protection has neither integrity nor 
     * privacy, or if <tt>incoming</tt> cannot be successfully unwrapped.
     */
    public abstract byte[] unwrap(byte[] incoming, int offset, int len)
	throws SaslException;

    /**
     * Wraps a byte array to be sent to the client.
     * This method can be called only after the authentication exchange has
     * completed (i.e., when <tt>isComplete()</tt> returns true) and only if
     * the authentication exchange has negotiated integrity and/or privacy 
     * as the quality of protection; otherwise, a <tt>SaslException</tt> is thrown.
     *<p>
     * The result of this method
     * will make up the contents of the SASL buffer as defined in RFC 2222
     * without the leading four octet field that represents the length.
     * <tt>offset</tt> and <tt>len</tt> specify the portion of <tt>outgoing</tt>
     * to use.
     *
     * @param outgoing A non-null byte array containing the bytes to encode.
     * @param offset The starting position at <tt>outgoing</tt> of the bytes to use.
     * @param len The number of bytes from <tt>outgoing</tt> to use.
     * @return A non-null byte array containing the encoded bytes.
     * @exception SaslException if the authentication exchange has not completed,
     * if the negotiated quality of protection has neither integrity nor privacy,
     * or if <tt>outgoing</tt> cannot be successfully wrapped.
     */
    public abstract byte[] wrap(byte[] outgoing, int offset, int len) 
	throws SaslException;

    /**
     * Retrieves the negotiated property.
     * This method can be called only after the authentication exchange has
     * completed (i.e., when <tt>isComplete()</tt> returns true); otherwise, a
     * <tt>SaslException</tt> is thrown.
     * 
     * @return The value of the negotiated property. If null, the property was
     * not negotiated or is not applicable to this mechanism.
     * @exception SaslException if this authentication exchange has not completed
     */

    public abstract String getNegotiatedProperty(String propName) 
	throws SaslException;

     /**
      * Disposes of any system resources or security-sensitive information
      * the SaslServer might be using. Invoking this method invalidates
      * the SaslServer instance. This method is idempotent.
      * @throws SaslException If a problem was encountered while disposing
      * the resources.
      */
    public abstract void dispose() throws SaslException;
}
