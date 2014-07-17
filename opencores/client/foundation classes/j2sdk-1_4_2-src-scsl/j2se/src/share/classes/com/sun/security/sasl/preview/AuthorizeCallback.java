/*
 * @(#)AuthorizeCallback.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.preview;

import javax.security.auth.callback.Callback;

/**
  * This callback is used by <tt>SaslServer</tt> to determine whether
  * one entity (identified by an authenticated authentication id) 
  * can act on
  * behalf of another entity (identified by an authorization id).
  *
  * @author Rosanna Lee
  * @author Rob Weltman
  */
public class AuthorizeCallback implements Callback {
    private String authenticationID;
    private String authorizationID;
    private String authorizedID;
    private boolean authorized;

    /**
     * Constructs an instance of <tt>AuthorizeCallback</tt>.
     *
     * @param authnID	The (authenticated) authentication id.
     * @param authzID   The authorization id.
     */
    public AuthorizeCallback(String authnID, String authzID) {
	authenticationID = authnID;
	authorizationID = authzID;
    }

    /**
     * Returns the authentication id to check.
     * @return The authentication id to check.
     */
    public String getAuthenticationID() {
	return authenticationID;
    }

    /**
     * Returns the authorization id to check.
     * @return The authentication id to check.
     */
    public String getAuthorizationID() {
	return authorizationID;
    }

    /**
     * Determines whether the authentication id is allowed to
     * act on behalf of the authorization id.
     *
     * @return <tt>true</tt> if authorization is allowed; <tt>false</tt> otherwise
     * @see #setAuthorized(boolean)
     * @see #getAuthorizedID()
     */
    public boolean isAuthorized() {
	return authorized;
    }

    /**
     * Sets whether the authorization is allowed.
     * @param ok <tt>true</tt> if authorization is allowed; <tt>false</tt> otherwise
     * @see #isAuthorized
     * @see #setAuthorizedID(java.lang.String)
     */
    public void setAuthorized(boolean ok) {
	authorized = ok;
    }

    /**
     * Returns the id of the authorized user.
     * @return The id of the authorized user. <tt>null</tt> means the
     * authorization failed.
     * @see #setAuthorized(boolean)
     * @see #setAuthorizedID(java.lang.String)
     */
    public String getAuthorizedID() {
	if (!authorized) {
	    return null;
	}
	return (authorizedID == null) ? authorizationID : authorizedID;
    }

    /**
     * Sets the id of the authorized entity. Called by handler only when the id
     * is different from getAuthorizationID(). For example, the id
     * might need to be canonicalized for the environment in which it
     * will be used.
     * @param The id of the authorized user.
     * @see #setAuthorized(boolean)
     * @see #getAuthorizedID
     */
    public void setAuthorizedID(String id) {
	authorizedID = id;
    }
}
