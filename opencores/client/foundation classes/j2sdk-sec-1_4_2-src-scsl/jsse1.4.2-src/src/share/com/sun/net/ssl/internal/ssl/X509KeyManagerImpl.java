/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.security.*;
import java.security.cert.*;
import java.security.cert.Certificate;
import java.util.*;
import java.net.Socket;

import javax.security.auth.x500.X500Principal;

import javax.net.ssl.X509KeyManager;

/**
 * An implemention of X509KeyManager backed by a KeyStore.
 *
 * The backing KeyStore is inspected when this object is constructed.
 * All key entries containing a PrivateKey and a non-empty chain of
 * X509Certificate are then copied into an internal store. This means
 * that subsequent modifications of the KeyStore have no effect on the
 * X509KeyManagerImpl object.
 * 
 * Note that this class assumes that all keys are protected by the same
 * password.
 *
 * The JSSE handshake code currently calls into this class via
 * chooseClientAlias() and chooseServerAlias() to find the certificates to
 * use. As implemented here, both always return the first alias returned by 
 * getClientAliases() and getServerAliases(). In turn, these methods are 
 * implemented by calling getAliases(), which performs the actual lookup.
 *
 * Note that this class currently implements no checking of the local
 * certificates. In particular, it is *not* guaranteed that:
 *  . the certificates are within their validity period and not revoked
 *  . the signatures verify
 *  . they form a PKIX compliant chain.
 *  . the certificate extensions allow the certificate to be used for
 *    the desired purpose.
 *
 * Chains that fail any of these criteria will probably be rejected by
 * the remote peer.
 *
 * @version 1.21 06/24/03
 */
final class X509KeyManagerImpl implements X509KeyManager {

    private static final Debug debug = Debug.getInstance("ssl");
    
    private static final String[] STRING0 = new String[0];

    /*
     * The credentials from the KeyStore as
     * Map: String(alias) -> X509Credentials(credentials)
     */
    private Map credentialsMap;
    
    /*
     * Cached server aliases for the case issuers == null.
     * (in the current JSSE implementation, issuers are always null for
     * server certs). See chooseServerAlias() for details.
     *
     * Map: X500Principal(issuer) -> String(alias)
     */
    private Map serverAliasCache;
    
    /*
     * Basic container for credentials implemented as an inner class.
     */
    private static class X509Credentials {
        PrivateKey privateKey;
	X509Certificate[] certificates;
	private Set issuerX500Principals;
	
	X509Credentials(PrivateKey privateKey, X509Certificate[] certificates) {
	    // assert privateKey and certificates != null
	    this.privateKey = privateKey;
	    this.certificates = certificates;
	}
	
	synchronized Set getIssuerX500Principals() {
	    // lazy initialization
	    if (issuerX500Principals == null) {
	        issuerX500Principals = new HashSet();
		for (int i = 0; i < certificates.length; i++) {
		    issuerX500Principals.add(
		    		certificates[i].getIssuerX500Principal());
		}
	    }
	    return issuerX500Principals;
	}
    }

    X509KeyManagerImpl(KeyStore ks, char[] password) throws KeyStoreException, 
	    NoSuchAlgorithmException, UnrecoverableKeyException {

        credentialsMap = new HashMap();
	serverAliasCache = new HashMap();
	if (ks == null) {
	    return;
	}
	
	for (Enumeration aliases = ks.aliases(); aliases.hasMoreElements(); ) {
	    String alias = (String)aliases.nextElement();
	    if (!ks.isKeyEntry(alias)) {
	        continue;
	    }
	    Key key = ks.getKey(alias, password);
	    if (key instanceof PrivateKey == false) {
	        continue;
	    }
	    Certificate[] certs = ks.getCertificateChain(alias);
	    if ((certs == null) || (certs.length == 0) || 
	    	    !(certs[0] instanceof X509Certificate)) {
		continue;
	    }
	    if (!(certs instanceof X509Certificate[])) {
		Certificate[] tmp = new X509Certificate[certs.length];
		System.arraycopy(certs, 0, tmp, 0, certs.length);
		certs = tmp;
	    }

	    X509Credentials cred = new X509Credentials((PrivateKey)key, 
		(X509Certificate[])certs);
	    credentialsMap.put(alias, cred);
	    if (debug != null && Debug.isOn("keymanager")) {
		System.out.println("***");
		System.out.println("found key for : " + alias);
		for (int i = 0; i < certs.length; i++) {
		    System.out.println("chain [" + i + "] = "
		    + certs[i]);
		}
		System.out.println("***");
	    }
	}
    }

    /*
     * Returns the certificate chain associated with the given alias.
     *
     * @return the certificate chain (ordered with the user's certificate first
     * and the root certificate authority last)
     */
    public X509Certificate[] getCertificateChain(String alias) {
	if (alias == null) {
	    return null;
	}
	X509Credentials cred = (X509Credentials)credentialsMap.get(alias);
	if (cred == null) {
	    return null;
	} else {
	    return (X509Certificate[])cred.certificates.clone();
	}
    }

    /*
     * Returns the key associated with the given alias
     */
    public PrivateKey getPrivateKey(String alias) {
	if (alias == null) {
	    return null;
	}
	X509Credentials cred = (X509Credentials)credentialsMap.get(alias);
	if (cred == null) {
	    return null;
	} else {
	    return cred.privateKey;
	}
    }

    /*
     * Choose an alias to authenticate the client side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String chooseClientAlias(String[] keyTypes, Principal[] issuers,
	    Socket socket) {
	/*
	 * We currently don't do anything with socket, but
	 * someday we might.  It might be a useful hint for
	 * selecting one of the aliases we get back from
	 * getClientAliases().
	 */

	if (keyTypes == null) {
	    return null;
	}

	for (int i = 0; i < keyTypes.length; i++) {
	    String[] aliases = getClientAliases(keyTypes[i], issuers);
	    if ((aliases != null) && (aliases.length > 0)) {
		return aliases[0];
	    }
	}
	return null;
    }
    
    /*
     * Choose an alias to authenticate the server side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String chooseServerAlias(String keyType,
	    Principal[] issuers, Socket socket) {
	/*
	 * We currently don't do anything with socket, but
	 * someday we might.  It might be a useful hint for
	 * selecting one of the aliases we get back from
	 * getServerAliases().
	 */
	if (keyType == null) {
	    return null;
	}

	String[] aliases;

	if (issuers == null || issuers.length == 0) {
	    aliases = (String[])serverAliasCache.get(keyType);
	    if (aliases == null) {
		aliases = getServerAliases(keyType, issuers);
	        // Cache the result (positive and negative lookups)
		if (aliases == null) {
		    aliases = STRING0;
		}
		serverAliasCache.put(keyType, aliases);
	    }
	} else {
	    aliases = getServerAliases(keyType, issuers);
	}
	if ((aliases != null) && (aliases.length > 0)) {
	    return aliases[0];
	}
	return null;
    }

    /*
     * Get the matching aliases for authenticating the client side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String[] getClientAliases(String keyType, Principal[] issuers) {
        return getAliases(keyType, issuers);
    }

    /*
     * Get the matching aliases for authenticating the server side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String[] getServerAliases(String keyType, Principal[] issuers) {
        return getAliases(keyType, issuers);
    }
    
    /*
     * Get the matching aliases for authenticating the either side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     *
     * Issuers comes to us in the form of X500Principal[].
     */
    private String[] getAliases(String keyType, Principal[] issuers) {
	if (keyType == null) {
	    return null;
	}
	if (issuers == null) {
	    issuers = new X500Principal[0];
	}
	if (issuers instanceof X500Principal[] == false) {
	    // normally, this will never happen but try to recover if it does
	    issuers = convertPrincipals(issuers);
	}
	
	X500Principal[] x500Issuers = (X500Principal[])issuers;
	// the algorithm below does not produce duplicates, so avoid Set
	List aliases = new ArrayList();
	
	for (Iterator t = credentialsMap.entrySet().iterator(); t.hasNext(); ) {
	    Map.Entry entry = (Map.Entry)t.next();
	    String alias = (String)entry.getKey();
	    X509Credentials credentials = (X509Credentials)entry.getValue();
	    X509Certificate[] certs = credentials.certificates;
	    
	    if (!keyType.equals(certs[0].getPublicKey().getAlgorithm())) {
		continue;
	    }
	    
	    if (issuers.length == 0) {
	    	// no issuer specified, match all
	        aliases.add(alias);
		if (debug != null && Debug.isOn("keymanager")) {
		    System.out.println("matching alias: " + alias);
		}
	    } else {
	        Set certIssuers = credentials.getIssuerX500Principals();
	        for (int i = 0; i < x500Issuers.length; i++) {
	            if (certIssuers.contains(issuers[i])) {
		        aliases.add(alias);
			if (debug != null && Debug.isOn("keymanager")) {
			    System.out.println("matching alias: " + alias);
			}
			break;
		    }
		}
	    }
	}

	String[] aliasStrings = (String[])aliases.toArray(STRING0);
	return ((aliasStrings.length == 0) ? null : aliasStrings);
    }

    /*
     * Convert an array of Principals to an array of X500Principals, if
     * possible. Principals that cannot be converted are ignored.
     */
    private static X500Principal[] convertPrincipals(Principal[] principals) {
        List list = new ArrayList(principals.length);
	for (int i=0; i<principals.length; i++) {
	    Principal p = principals[i];
	    if (p instanceof X500Principal) {
	        list.add(p);
	    } else {
	        try {
		    list.add(new X500Principal(p.getName()));
		} catch (IllegalArgumentException e) {
		    // ignore
		}
	    }
	}
	return (X500Principal[])list.toArray(new X500Principal[list.size()]);
    }

}
