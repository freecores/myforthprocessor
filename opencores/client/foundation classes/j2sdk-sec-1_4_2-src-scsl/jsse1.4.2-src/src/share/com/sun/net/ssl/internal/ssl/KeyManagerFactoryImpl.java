/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package com.sun.net.ssl.internal.ssl;

import java.util.*;
import java.io.InputStream;
import java.math.*;
import java.security.*;
import javax.net.ssl.*;

public final class KeyManagerFactoryImpl extends KeyManagerFactorySpi {

    private X509KeyManager keyManager = null;
    private boolean isInitialized = false;

    protected void engineInit(KeyStore ks, char[] password) throws
	    KeyStoreException, NoSuchAlgorithmException,
	    UnrecoverableKeyException {
	keyManager = new X509KeyManagerImpl(ks, password);
	isInitialized = true;
    }

    protected void engineInit(ManagerFactoryParameters spec) throws
	    InvalidAlgorithmParameterException {
	throw new InvalidAlgorithmParameterException(
	    "SunJSSE does not use ManagerFactoryParameters");
    }

    /**
     * Returns one key manager for each type of key material.
     */
    protected KeyManager[] engineGetKeyManagers() {
	if (!isInitialized) {
	    throw new IllegalStateException(
			"KeyManagerFactoryImpl is not initialized");
	}
	return new KeyManager[] { keyManager };
    }
}
