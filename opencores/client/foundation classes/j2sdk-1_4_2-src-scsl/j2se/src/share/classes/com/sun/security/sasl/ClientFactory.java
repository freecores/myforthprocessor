/*
 * @(#)ClientFactory.java	1.12 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl; 

import com.sun.security.sasl.preview.*;
import com.sun.security.sasl.util.Policy;

import java.util.Map;
import javax.security.auth.callback.*;

/**
  * Client factory for EXTERNAL.
  *
  * @author Rosanna Lee
  */
public class ClientFactory implements SaslClientFactory {
    private static final String myMechs[] = {
	"EXTERNAL", //
    };

    // %%% RL: Policies should actually depend on the external channel
    private static final int mechPolicies[] = {
	Policy.NOPLAINTEXT|Policy.NOACTIVE|Policy.NODICTIONARY	    //EXTERNAL
    };

    private static final int EXTERNAL = 0;

    public ClientFactory() {
    }

    public SaslClient createSaslClient(String[] mechs,
	String authorizationId,
	String protocol,
	String serverName,
	Map props,
	CallbackHandler cbh) throws SaslException {

	    for (int i = 0; i < mechs.length; i++) {
		if (mechs[i].equals(myMechs[EXTERNAL]) 
		    && Policy.checkPolicy(mechPolicies[EXTERNAL], props)) {
		    return new External(authorizationId);
		} 
	    }
	    return null;
    };

    public String[] getMechanismNames(Map props) {
	return Policy.filterMechs(myMechs, mechPolicies, props);
    }
}
