/*
 * @(#)DefaultCallbackHandler.java	1.11 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap.sasl;

import javax.security.auth.callback.*;
import com.sun.security.sasl.preview.RealmCallback;
import com.sun.security.sasl.preview.RealmChoiceCallback;
import java.io.IOException;

/**
 * DefaultCallbackHandler for satisfying NameCallback and
 * PasswordCallback for an LDAP client.
 * NameCallback is used for getting the authentication ID and is
 * gotten from the java.naming.security.principal property.
 * PasswordCallback is gotten from the java.naming.security.credentials
 * property and must be of type String, char[] or byte[].
 * If byte[], it is assumed to have UTF-8 encoding.
 *
 * If the caller of getPassword() will be using the password as
 * a byte array, then it should encode the char[] array returned by
 * getPassword() into a byte[] using UTF-8.
 *
 * @author Rosanna Lee
 */
final class DefaultCallbackHandler implements CallbackHandler {
    private char[] passwd;
    private String authenticationID;
    private String authRealm;

    DefaultCallbackHandler(String principal, Object cred, String realm) throws IOException {
	authenticationID = principal;
	authRealm = realm;       
	if (cred instanceof String) {
	    passwd = ((String)cred).toCharArray();
	} else if (cred instanceof char[]) {
	    passwd = (char[])((char[])cred).clone();
	} else if (cred != null) {
	    // assume UTF-8 encoding
	    String orig = new String((byte[])cred, "UTF8");
	    passwd = orig.toCharArray();
	}
    }

    public void handle(Callback[] callbacks) 
	throws java.io.IOException, UnsupportedCallbackException {
	    for (int i = 0; i < callbacks.length; i++) {
		if (callbacks[i] instanceof NameCallback) {
		    ((NameCallback)callbacks[i]).setName(authenticationID);
		    
		} else if (callbacks[i] instanceof PasswordCallback) {
		    ((PasswordCallback)callbacks[i]).setPassword(passwd);
		    
		} else if (callbacks[i] instanceof RealmChoiceCallback) {	
		    /* Deals with a choice of realms */		    
		    String[] choices = 
			((RealmChoiceCallback)callbacks[i]).getChoices();
		    int selected = 0;
		    
		    if (authRealm != null) {
			selected = -1; // no realm chosen
			for (int j = 0; j < choices.length; j++) {
 			    if (choices[j].equals(authRealm)) {
				selected = j;
 			    }
			}		    
			if (selected == -1) {
			    StringBuffer allChoices = new StringBuffer();
			    for (int j = 0; j <  choices.length; j++) {
				allChoices.append(choices[j] + ",");
			    }
			    throw new IllegalArgumentException("Cannot match " +
				"'java.naming.security.sasl.realm' property value, '" +
				authRealm + "' with choices " + allChoices +
				"in RealmChoiceCallback");
			}
		    }

		    ((RealmChoiceCallback)callbacks[i]).setSelectedIndex(selected);
		    		    		   
		} else if (callbacks[i] instanceof RealmCallback) {
		    /* No realm specified in digest-challenge */
		    if (authRealm != null) {
			((RealmCallback)callbacks[i]).setText(authRealm);
		    } else {
			((RealmCallback)callbacks[i]).setText("");
		    }
		} else {
		    throw new UnsupportedCallbackException(callbacks[i]);
		}
	    }
    }

    void clearPassword() {
	if (passwd != null) {
	    for (int i = 0; i < passwd.length; i++) {
		passwd[i] = '\0';
	    }
	    passwd = null;
	}
    }

    protected void finalize() throws Throwable {
	clearPassword();
    }
}
