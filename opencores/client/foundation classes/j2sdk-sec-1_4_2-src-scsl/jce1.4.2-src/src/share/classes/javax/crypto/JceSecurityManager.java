/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.*;
import java.net.*;
import java.util.*;
import java.util.jar.*;

/**
 * The JCE security manager.
 *
 * <p>The JCE security manager is responsible for determining the maximum
 * allowable cryptographic strength for a given applet/application, for a given
 * algorithm, by consulting the configured jurisdiction policy files and
 * the cryptographic permissions bundled with the applet/application.
 *
 * <p>Note that this security manager is never installed, only instantiated.
 *
 * @author Jan Luehe
 * @version 1.14, 06/24/03
 *
 * @since 1.4
 */

final class JceSecurityManager extends SecurityManager {

    private static final String permsFile = "cryptoPerms";
    private static final CryptoPermissions defaultPolicy;
    private static final CryptoPermissions exemptPolicy;
    private static final CryptoAllPermission allPerm;
    private static final Vector TrustedCallersCache = new Vector(2);
    private static final Map exemptCache = new HashMap();

    static {
	defaultPolicy = JceSecurity.getDefaultPolicy();
	exemptPolicy = JceSecurity.getExemptPolicy();
	allPerm = CryptoAllPermission.INSTANCE;
    }

    /**
     * Returns the maximum allowable crypto strength for the given
     * applet/application, for the given algorithm.
     */
    CryptoPermission getCryptoPermission(String alg) {

	// If CryptoAllPermission is granted by default, we return that.
	// Otherwise, this will be the permission we return if anything goes
	// wrong.
	CryptoPermission defaultPerm = getDefaultPermission(alg);
	if (defaultPerm == CryptoAllPermission.INSTANCE) {
	    return defaultPerm;
	}

	// Determine the codebase of the caller of the JCE API.
	// This is the codebase of the first protection domain that is
	// different from our own.
	Class[] context = getClassContext();
	URL callerCodeBase = null;
	int i;
	for (i=0; i<context.length; i++) {
	    callerCodeBase = JceSecurity.getCodeBase(context[i]);
	    if (callerCodeBase != null) {
		break;
	    }
	}

	if (i == context.length) {
	    return defaultPerm;
	}
	
	CryptoPermissions appPerms;
	synchronized (this.getClass()) {
	    if (exemptCache.containsKey(callerCodeBase)) {
		appPerms = (CryptoPermissions)exemptCache.get(callerCodeBase);
	    } else {
		appPerms = getAppPermissions(callerCodeBase);
		exemptCache.put(callerCodeBase, appPerms);
	    }
	}
	
	if (appPerms == null) {
	    return defaultPerm;
	}
	
	// If the app was granted the special CryptoAllPermission, return that.
	if (appPerms.implies(allPerm)) {
	    return allPerm;
	}

	// Check if the crypto permissions granted to the app contain a
	// crypto permission for the requested algorithm that does not require
	// any exemption mechanism to be enforced.
	// Return that permission, if present.
	PermissionCollection appPc = appPerms.getPermissionCollection(alg);
	if (appPc == null) {
	    return defaultPerm;
	}	    
	Enumeration enum = appPc.elements();
	while (enum.hasMoreElements()) {
	    CryptoPermission cp = (CryptoPermission)enum.nextElement();
	    if (cp.getExemptionMechanism() == null) {
		return cp;
	    }
	}

	// Check if the jurisdiction file for exempt applications contains
	// any entries for the requested algorithm. 
        // If not, return the default permission.
	PermissionCollection exemptPc =
	    exemptPolicy.getPermissionCollection(alg);
	if (exemptPc == null) {
	    return defaultPerm;
	}	    	

	// In the jurisdiction file for exempt applications, go through the
	// list of CryptoPermission entries for the requested algorithm, and
	// stop at the first entry:
	//  - that is implied by the collection of crypto permissions granted
	//    to the app, and 
	//  - whose exemption mechanism is available from one of the
	//    registered CSPs
	enum = exemptPc.elements();
	while (enum.hasMoreElements()) {
	    CryptoPermission cp = (CryptoPermission)enum.nextElement();
	    try {
		ExemptionMechanism.getInstance(cp.getExemptionMechanism());
		if (cp.getAlgorithm().equals(
				      CryptoPermission.ALG_NAME_WILDCARD)) {
		    CryptoPermission newCp = 
	                new CryptoPermission(
		            alg,
			    cp.getMaxKeySize(),
			    cp.getAlgorithmParameterSpec(),
                            cp.getExemptionMechanism());
		    if (appPerms.implies(newCp)) {
			return newCp;
		    }
		}

		if (appPerms.implies(cp)) {
		    return cp;
		}
	    } catch (Exception e) {
		continue;
	    }
	}
	return defaultPerm;
    }
	
    private static CryptoPermissions getAppPermissions(URL callerCodeBase) {
	// Check if app is exempt, and retrieve the permissions bundled with it
	JarFile jf;
	try {
	    jf = JceSecurity.verifyExemptJar(callerCodeBase);
	} catch (Exception e) {
	    return null;
	}
	JarEntry je = jf.getJarEntry(permsFile);
	if (je == null) {
	    // app is not exempt
	    return null;
	}

	//
	// Applet/application is exempt
	//

	CryptoPermissions appPerms = new CryptoPermissions();
	try {
	    appPerms.load(jf.getInputStream(je));
	} catch (Exception e) {
	    return null;
	}

	jf = null;

	return appPerms;
    }

    /**
     * Returns the default permission for the given algorithm.
     */
    private CryptoPermission getDefaultPermission(String alg) {
	Enumeration enum =
	    defaultPolicy.getPermissionCollection(alg).elements();
	return (CryptoPermission)enum.nextElement();
    }

    // See  bug 4341369 & 4334690 for more info.
    boolean isCallerTrusted() {
	// Get the caller and its codebase.
	Class[] context = getClassContext();
	URL callerCodeBase = null;
	int i;
	for (i=0; i<context.length; i++) {
	    callerCodeBase = JceSecurity.getCodeBase(context[i]);
	    if (callerCodeBase != null) {
		break;
	    }
	}
	// The caller is in the JCE framework.
	if (i == context.length) {
	    return true;
	}
	//The caller has been verified.
	if (TrustedCallersCache.contains(context[i])) {
	    return true;
	}
	// Check whether the caller is a trusted provider.
	try {
	    JceSecurity.verifyProviderJar(callerCodeBase);
	} catch (Exception e2) {
	    return false;
	}
	TrustedCallersCache.addElement(context[i]);
	return true;	    
    }
}
