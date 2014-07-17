/*
 * @(#)ClientFactory.java	1.13 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.digest;

import com.sun.security.sasl.preview.*;
import com.sun.security.sasl.util.*;
import javax.security.auth.callback.*;
import java.util.Map;

import java.io.IOException;

/**
  * Client factory for DIGEST-MD5 SASL mechanism.
  *
  * @author Jonathan Bruce
  */
  
public class ClientFactory implements SaslClientFactory {

    static private final String myMechs[] = { "DIGEST-MD5" };
    static private final int DIGEST_MD5 = 0;
    static private final int mechPolicies[] = {
	Policy.NOPLAINTEXT|Policy.NOANONYMOUS};
          
    /**
      * Empty constructor.
      */	
    public ClientFactory() {
    }
	
    /**
     * Returns a new instance of the DIGEST-MD5 SASL authentication 
     * mechanism. 
     *
     * @throws SaslException If there is an error creating the DigestMD5
     * SASL client.
     * @returns a new SaslClient ; otherwise null if unsuccessful.
     */	
    public SaslClient createSaslClient(String[] mechs, 
	 String authorizationId, String protocol, String serverName,
	 Map props, CallbackHandler cbh) 
	 throws SaslException {			
          
         for (int i=0; i<mechs.length; i++) {
            if (mechs[i].equals(myMechs[DIGEST_MD5]) &&
		   Policy.checkPolicy(mechPolicies[DIGEST_MD5], props)) {
		return new DigestMD5(authorizationId, 
		    protocol + "/" + serverName, 
		    props, 
		    cbh);
	    }
	}
	return null;		
    }
	
    /**
      * Returns the authentication mechanisms that this SaslFactory
      * implementation can produce. 
      *
      * @returns String[] DigestMD5
      */	
    public String[] getMechanismNames(Map env) {
	return Policy.filterMechs(myMechs, mechPolicies, env);
    }    
}	
