/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.util.*;
import java.math.*;
import java.security.*;
import java.security.cert.*;
import java.net.Socket;
import javax.net.ssl.*;

public final class SSLContextImpl extends SSLContextSpi {

    private static SSLContextImpl defaultContext;
    private X509KeyManager keyManager;
    private X509TrustManager trustManager;
    private SecureRandom secureRandom;
    
    private EphemeralKeyManager ephemeralKeyManager = new EphemeralKeyManager();

    private SSLSessionContextImpl clientCache = new SSLSessionContextImpl();
    private SSLSessionContextImpl serverCache = new SSLSessionContextImpl();

    private static final Debug debug = Debug.getInstance("ssl");
    private boolean isInitialized = false;

    protected void engineInit(KeyManager[] km, TrustManager[] tm,
				SecureRandom sr)
	throws KeyManagementException {
	keyManager = new JsseX509KeyManager(km);

	if (tm == null) {
	    TrustManagerFactory tmf;
	    try {
		tmf = TrustManagerFactory.getInstance("SunX509");
		tmf.init((KeyStore)null);
		tm = tmf.getTrustManagers();
	    } catch (Exception e) {
		// eat
	    }
	}
	trustManager = new JsseX509TrustManager(tm);

	if (sr == null)
	    sr = new SecureRandom();
	secureRandom = sr;

	/*
	 * The initial delay of seeding the random number generator
	 * could be long enough to cause the initial handshake on our
	 * first connection to timeout and fail. Make sure it is
	 * primed and ready by getting some initial output from it.
	 */
	if (debug != null && Debug.isOn("sslctx")) {
	    System.out.println("trigger seeding of SecureRandom");
	}
	secureRandom.nextInt();
	if (debug != null && Debug.isOn("sslctx")) {
	    System.out.println("done seeding SecureRandom");
	}
	isInitialized = true;
    }

    protected SSLSocketFactory engineGetSocketFactory() {
	if (!isInitialized) {
	    throw new IllegalStateException("SSLContextImpl is not initialized");
	}
	return new SSLSocketFactoryImpl(this);
    }

    protected SSLServerSocketFactory engineGetServerSocketFactory() {
	if (!isInitialized) {
	    throw new IllegalStateException("SSLContext is not initialized");
	}
	return new SSLServerSocketFactoryImpl(this);
    }

    protected SSLSessionContext engineGetClientSessionContext() {
	return clientCache;
    }

    protected SSLSessionContext engineGetServerSessionContext() {
	return serverCache;
    }

    SecureRandom getSecureRandom() {
	return secureRandom;
    }

    X509KeyManager getX509KeyManager() {
	return keyManager;
    }

    X509TrustManager getX509TrustManager() {
	return trustManager;
    }
    
    EphemeralKeyManager getEphemeralKeyManager() {
	return ephemeralKeyManager;
    }

    synchronized static SSLContextImpl getDefaultContext() {
	if (defaultContext != null)
	    return defaultContext;
	try {
	    final String defaultKeyStore;
	    String defaultKeyStoreType;
	    String defaultKeyStorePassword = null;
	    final HashMap props = new HashMap();
	    FileInputStream fs = null;
	    KeyStore ks = null;
	    SSLContextImpl ctx = null;

	    AccessController.doPrivileged(
			new PrivilegedExceptionAction() {
		public Object run() throws Exception {
		    props.put("keyStore",  System.getProperty(
				"javax.net.ssl.keyStore", ""));
		    props.put("keyStoreType", System.getProperty(
				"javax.net.ssl.keyStoreType",
				KeyStore.getDefaultType()));
		    props.put("keyStorePasswd", System.getProperty(
				"javax.net.ssl.keyStorePassword", ""));
		    return null;
		}
	    });
	    defaultKeyStore = (String) props.get("keyStore");
	    defaultKeyStoreType = (String) props.get("keyStoreType");
	    if (debug != null && Debug.isOn("defaultctx")) {
		System.out.println("keyStore is : " + defaultKeyStore);
		System.out.println("keyStore type is : " +
					defaultKeyStoreType);
	    }

	    if (defaultKeyStore.length() != 0) {
		fs = (FileInputStream) AccessController.doPrivileged(
			new PrivilegedExceptionAction() {
		    public Object run() throws Exception {
			return new FileInputStream(defaultKeyStore);
		    }
		});
	    }

	    /**
	     * Try to initialize key store.
	     */
	    if ((defaultKeyStoreType.length()) != 0) {
		if (debug != null && Debug.isOn("defaultctx")) {
		    System.out.println("init keystore");
		}
		ks = KeyStore.getInstance(defaultKeyStoreType);
		char[] passwd = null;
		defaultKeyStorePassword = (String) props.get("keyStorePasswd");
		if ((defaultKeyStorePassword.length()) != 0)
		    passwd = defaultKeyStorePassword.toCharArray();
		ks.load(fs, passwd);
		  // Zero out the temporary password storage
		if (passwd != null) {
		    for (int i = 0; i < passwd.length; i++) {
			passwd[i] = (char) 0;
		    }
		}
	    }
	    if (fs != null) {
		fs.close();
		fs = null;
	    }

	    /*
	     * Try to initialize key manager.
	     */
	    if (debug != null && Debug.isOn("defaultctx")) {
		System.out.println("init keymanager of type " +
		    KeyManagerFactory.getDefaultAlgorithm());
	    }
	    KeyManagerFactory kmf = KeyManagerFactory.getInstance(
		KeyManagerFactory.getDefaultAlgorithm());
	    kmf.init(ks, defaultKeyStorePassword.toCharArray());
	    KeyManager[] km = kmf.getKeyManagers();

	    /*
	     * Try to initialize trust store.
	     */
	    ks = TrustManagerFactoryImpl.getCacertsKeyStore("defaultctx");

	    /*
	     * Try to initialize trust manager.
	     */
	    TrustManagerFactory tmf = TrustManagerFactory.getInstance(
		TrustManagerFactory.getDefaultAlgorithm());
	    TrustManager[] tm = null;
	    tmf.init(ks);
	    tm = tmf.getTrustManagers();

	    if (debug != null && Debug.isOn("defaultctx")) {
		System.out.println("init context");
	    }
	    ctx = new SSLContextImpl();
	    ctx.engineInit(km, tm, null);

	    defaultContext = ctx;

	} catch (RuntimeException re) {
             if (debug != null && Debug.isOn("defaultctx")) {
                 System.out.println("default context init failed: " +
                 re);
             }
             throw re;
        } catch (Exception e) {
            if (debug != null && Debug.isOn("defaultctx")) {
                System.out.println("default context init failed: " + e);
            }
            throw new RuntimeException("Default SSL context init failed: " +
                e.getMessage());
	} catch (Error err) {
	    if (debug != null && Debug.isOn("defaultctx")) {
		System.out.println("default context init failed: " + err);
	    }
	    throw err;
	}
	return defaultContext;
    }
}

class JsseX509TrustManager implements X509TrustManager {
    X509TrustManager trustManager;

    /*
     * We only use the first instance of X509TrustManager passed to us.
     */
    JsseX509TrustManager(TrustManager[] tm) {

tm_found:
	for (int i = 0; tm != null && i < tm.length; i++) {
	    if (tm[i] instanceof X509TrustManager) {
		trustManager = (X509TrustManager)tm[i];
		break tm_found;
	    }
	}
    }

    /*
     * Given the partial or complete certificate chain
     * provided by the peer, build a certificate path
     * to a trusted root and return if it can be
     * validated and is trusted for client SSL authentication.
     * If not, it throws an exception.
     */
    public void checkClientTrusted(X509Certificate[] chain, String authType)
	throws CertificateException {
	if (trustManager != null) {
	    trustManager.checkClientTrusted(chain, authType);
	} else {
	    throw new CertificateException(
		"No X509TrustManager implementation avaiable");
	}
    }

    /*
     * Given the partial or complete certificate chain
     * provided by the peer, build a certificate path
     * to a trusted root and return if it can be
     * validated and is trusted for server SSL authentication.
     * If not, it throws an exception.
     */
    public void checkServerTrusted(X509Certificate[] chain, String authType)
	throws CertificateException {
	if (trustManager != null) {
	    trustManager.checkServerTrusted(chain, authType);
	} else {
	    throw new CertificateException(
		"No X509TrustManager implementation available");
	}
    }

    /*
     * Return an array of issuer certificates which are trusted
     * for authenticating peers.
     */
    public X509Certificate[] getAcceptedIssuers() {
	if (trustManager != null)
	    return trustManager.getAcceptedIssuers();
	else
	    return new X509Certificate[0];
    }
}

class JsseX509KeyManager implements X509KeyManager {
    X509KeyManager keyManager;

    /*
     * We only use the first instance of X509KeyManager passed to us.
     */
    JsseX509KeyManager(KeyManager[] km) {

km_found:
	for (int i = 0; km != null && i < km.length; i++) {
	    if (km[i] instanceof X509KeyManager) {
		keyManager = (X509KeyManager)km[i];
		break km_found;
	    }
	}
    }

    /*
     * Get the matching aliases for authenticating the client side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String[] getClientAliases(String keyType, Principal[] issuers) {
	if (keyManager != null)
	    return keyManager.getClientAliases(keyType, issuers);
	else
	    return null;
    }

    /*
     * Choose an alias to authenticate the client side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String chooseClientAlias(String[] keyTypes, Principal[] issuers,
	    Socket socket) {
	if (keyManager != null)
	    return keyManager.chooseClientAlias(keyTypes, issuers, socket);
	else
	    return null;
    }

    /*
     * Get the matching aliases for authenticating the server side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String[] getServerAliases(String keyType, Principal[] issuers) {
	if (keyManager != null)
	    return keyManager.getServerAliases(keyType, issuers);
	else
	    return null;
    }

    /*
     * Choose an alias to authenticate the server side of a secure
     * socket given the public key type and the list of
     * certificate issuer authorities recognized by the peer (if any).
     */
    public String chooseServerAlias(String keyType, Principal[] issuers,
	    Socket socket) {
	if (keyManager != null)
	    return keyManager.chooseServerAlias(keyType, issuers, socket);
	else
	    return null;
    }

    /**
     * Returns the certificate chain associated with the given alias.
     *
     * @param alias the alias name
     *
     * @return the certificate chain (ordered with the user's certificate first
     * and the root certificate authority last)
     */
    public X509Certificate[] getCertificateChain(String alias) {
	if (keyManager != null)
	    return keyManager.getCertificateChain(alias);
	else
	    return null;
    }

    /*
     * Returns the key associated with the given alias, using the given
     * password to recover it.
     *
     * @param alias the alias name
     *
     * @return the requested key
     */
    public PrivateKey getPrivateKey(String alias) {
	if (keyManager != null)
	    return keyManager.getPrivateKey(alias);
	else
	    return null;
    }
}
