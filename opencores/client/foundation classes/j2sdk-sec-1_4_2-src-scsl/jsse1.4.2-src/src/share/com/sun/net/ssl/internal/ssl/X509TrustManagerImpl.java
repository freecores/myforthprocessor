/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.util.*;
import java.security.*;
import java.security.cert.*;

import javax.net.ssl.*;

import sun.security.validator.*;

/**
 * This class implements the SunJSSE X.509 trust manager using the internal
 * validator API in J2SE core. The logic in this class is minimal.<p>
 *
 * This class supports both the Simple validation algorithm from previous
 * JSSE versions and PKIX validation. Currently, it is not possible for the
 * application to specify PKIX parameters other than trust anchors. This will
 * be fixed in a future release using new APIs. When that happens, it may also
 * make sense to separate the Simple and PKIX trust managers into separate
 * classes.
 *
 * @version 1.47, 06/24/03
 * @author Andreas Sterbenz
 */
final class X509TrustManagerImpl implements X509TrustManager {
    
    /**
     * Flag indicating whether to enable revocation check for the PKIX trust
     * manager. Typically, this will only work if the PKIX implementation
     * supports CRL distribution points as we do not manually setup CertStores.
     */
    private final static boolean checkRevocation = 
    	Debug.getBooleanProperty("com.sun.net.ssl.checkRevocation", false);
    
    private final String validatorType;
    
    /**
     * The Set of trusted X509Certificates. 
     */
    private final Set trustedCerts;
    
    // note that we need separate validator for client and server due to
    // the different extension checks. They are initialized lazily on demand.
    private volatile Validator clientValidator, serverValidator;

    private static final Debug debug = Debug.getInstance("ssl");

    X509TrustManagerImpl(String validatorType, KeyStore ks) 
	    throws KeyStoreException {
	this.validatorType = validatorType;
	if (ks == null) {
	    trustedCerts = Collections.EMPTY_SET;
	} else {
	    trustedCerts = KeyStores.getTrustedCerts(ks);
	}
	if (debug != null && Debug.isOn("trustmanager")) {
	    for (Iterator t = trustedCerts.iterator(); t.hasNext(); ) {
		System.out.println("adding as trusted cert:");
		X509Certificate cert = (X509Certificate)t.next();
		System.out.println("  Subject: "
					+ cert.getSubjectX500Principal());
		System.out.println("  Issuer:  " 
					+ cert.getIssuerX500Principal());
		System.out.println("  Algorithm: "
					+ cert.getPublicKey().getAlgorithm()
					+ "; Serial number: 0x"
					+ cert.getSerialNumber().toString(16));
		System.out.println("  Valid from "
					+ cert.getNotBefore() + " until "
					+ cert.getNotAfter());
		System.out.println();
	    }
	}
    }
    
    /**
     * Set the default parameters for the given validator. Currently only
     * used to enabled revocation checking for PKIX validators.
     */
    private void setParameters(Validator v) {
	if (v instanceof PKIXValidator) {
	    PKIXValidator pkixValidator = (PKIXValidator)v;
	    pkixValidator.getParameters().setRevocationEnabled(checkRevocation);
	}
    }
    
    /**
     * Returns true if the client certificate can be trusted.
     *
     * @param chain certificates which establish an identity for the client.
     *	    Chains of arbitrary length are supported, and certificates
     *	    marked internally as trusted will short-circuit signature checks.
     * @throws CertificateException if the certificate chain is not trusted
     *	    by this TrustManager.
     */  
    public void checkClientTrusted(X509Certificate chain[], String authType)
	    throws CertificateException {
	// assume double checked locking with a volatile flag works
	// (guaranteed under the new Tiger memory model)
	Validator v = clientValidator;
	if (v == null) {
	    synchronized (this) {
		if (v == null) {
		    v = Validator.getInstance(validatorType, 
					      Validator.VAR_TLS_CLIENT,
					      trustedCerts);
		    setParameters(v);
		    clientValidator = v;
		}
	    }
	}
	X509Certificate[] trustedChain = v.validate(chain);
	if (debug != null && Debug.isOn("trustmanager")) {
	    System.out.println("Found trusted certificate:");
	    System.out.println(trustedChain[trustedChain.length - 1]);
	}
    }

    /**
     * Returns true if the server certifcate can be trusted.
     *
     * @param chain certificates which establish an identity for the server.
     *	    Chains of arbitrary length are supported, and certificates
     *      marked internally as trusted will short-circuit signature checks.
     * @throws CertificateException if the certificate chain is not trusted
     *      by this TrustManager.
     */
    public void checkServerTrusted(X509Certificate chain[], String authType)
	    throws CertificateException {
	// assume double checked locking with a volatile flag works
	// (guaranteed under the new Tiger memory model)
	Validator v = serverValidator;
	if (v == null) {
	    synchronized (this) {
		if (v == null) {
		    v = Validator.getInstance(validatorType, 
					      Validator.VAR_TLS_SERVER,
					      trustedCerts);
		    setParameters(v);
		    serverValidator = v;
		}
	    }
	}
	X509Certificate[] trustedChain = v.validate(chain, null, authType);
	if (debug != null && Debug.isOn("trustmanager")) {
	    System.out.println("Found trusted certificate:");
	    System.out.println(trustedChain[trustedChain.length - 1]);
	}
    }
    
    /**
     * Returns a list of CAs accepted to authenticate entities for the
     * specified purpose.
     *
     * @param purpose activity for which CAs should be trusted
     * @return list of CAs accepted for authenticating such tasks
     */
    public X509Certificate[] getAcceptedIssuers() {
	X509Certificate[] certsArray = new X509Certificate[trustedCerts.size()];
	trustedCerts.toArray(certsArray);
	return certsArray;
    }

}
