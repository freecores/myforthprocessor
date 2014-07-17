/*
 * @(#)ClientFactory.java	1.8 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.security.sasl.gsskerb;

import com.sun.security.sasl.preview.*;
import com.sun.security.sasl.util.Policy;

import java.util.Map;
import javax.security.auth.callback.CallbackHandler;

/**
  * Client factory for GSSAPI (Kerberos V5)
  *
  * @author Rosanna Lee
  */
public class ClientFactory implements SaslClientFactory {
    private static final String myMechs[] = {
        "GSSAPI"};

    private static final int mechPolicies[] = {
	Policy.NOPLAINTEXT|Policy.NOANONYMOUS|Policy.NOACTIVE
    };

    private static final int GSS_KERB_V5 = 0;

    public ClientFactory() {
    }

    public SaslClient createSaslClient(String[] mechs,
	String authorizationId,
	String protocol,
	String serverName,
	Map props,
	CallbackHandler cbh) throws SaslException {

	    for (int i = 0; i < mechs.length; i++) {
		if (mechs[i].equals(myMechs[GSS_KERB_V5])
		    && Policy.checkPolicy(mechPolicies[GSS_KERB_V5], props)) {
		    return new GssKerberosV5(
			authorizationId, 
			protocol, 
			serverName,
			props,
			cbh);
		}
	    }
	    return null;
    };

    public String[] getMechanismNames(Map props) {
	return Policy.filterMechs(myMechs, mechPolicies, props);
    }
}
