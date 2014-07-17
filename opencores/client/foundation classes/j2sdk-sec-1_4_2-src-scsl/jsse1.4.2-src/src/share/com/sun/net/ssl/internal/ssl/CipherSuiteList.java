/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.io.*;
import java.util.*;

import javax.net.ssl.SSLException;

/**
 * A list of CipherSuites. Also maintains the lists of supported and
 * default ciphersuites and supports I/O from handshake streams.
 *
 * Instances of this class are immutable.
 *
 * @version 1.4, 06/24/03
 */
final class CipherSuiteList {

    // lists of supported and default enabled ciphersuites
    // created on demand
    private static CipherSuiteList supportedSuites, defaultSuites;

    private final Collection cipherSuites;
    private String[] suiteNames;
    
    // for use by buildAvailableCache() only
    private CipherSuiteList(Collection cipherSuites) {
	this.cipherSuites = cipherSuites;
    }
    
    /**
     * Create a CipherSuiteList with a single element.
     */
    CipherSuiteList(CipherSuite suite) {
	cipherSuites = new ArrayList(1);
	cipherSuites.add(suite);
    }
    
    /**
     * Construct a CipherSuiteList from a array of names. We don't bother
     * to eliminate duplicates.
     *
     * @exception IllegalArgumentException if the array or any of its elements
     * is null or if the ciphersuite name is unrecognized or unsupported
     * using currently installed providers.
     */
    CipherSuiteList(String[] names) {
	if (names == null) {
	    throw new IllegalArgumentException("CipherSuites may not be null");
	}
	cipherSuites = new ArrayList(names.length);
	// refresh available cache once if a CipherSuite is not available
	// (maybe new JCE providers have been installed)
	boolean refreshed = false;
	for (int i = 0; i < names.length; i++) {
	    String suiteName = names[i];
	    CipherSuite suite = CipherSuite.valueOf(suiteName);
	    if (suite.isAvailable() == false) {
		if (refreshed == false) {
		    // clear the cache so that the isAvailable() call below
		    // does a full check
		    clearAvailableCache();
		    refreshed = true;
		}
		// still missing?
		if (suite.isAvailable() == false) {
		    throw new IllegalArgumentException("Cannot support "
		    	+ suiteName + " with currently installed providers");
		}
	    }
	    cipherSuites.add(suite);
	}
    }
    
    /**
     * Read a CipherSuiteList from a HandshakeInStream in V3 ClientHello
     * format. Does not check if the listed ciphersuites are known or
     * supported.
     */
    CipherSuiteList(HandshakeInStream in) throws IOException {
	byte[] bytes = in.getBytes16();
	if ((bytes.length & 1) != 0) {
	    throw new SSLException("Invalid ClientHello message");
	}
	cipherSuites = new ArrayList(bytes.length >> 1);
	for (int i = 0; i < bytes.length; i += 2) {
	    cipherSuites.add(CipherSuite.valueOf(bytes[i], bytes[i+1]));
	}
    }
    
    /**
     * Return whether this list contains the given CipherSuite.
     */
    boolean contains(CipherSuite suite) {
	return cipherSuites.contains(suite);
    }
    
    /**
     * Return an Iterator for the CipherSuites in this list.
     */
    Iterator iterator() {
	return cipherSuites.iterator();
    }
    
    /**
     * Return the number of CipherSuites in this list.
     */
    int size() {
	return cipherSuites.size();
    }
    
    /**
     * Return an array with the names of the CipherSuites in this list.
     */
    synchronized String[] toStringArray() {
	if (suiteNames == null) {
	    suiteNames = new String[cipherSuites.size()];
	    Iterator t = cipherSuites.iterator();
	    for (int i = 0; i < suiteNames.length; i++ ) {
		CipherSuite c = (CipherSuite)t.next();
		suiteNames[i] = c.name;
	    }
	}
	return (String[])suiteNames.clone();
    }
    
    public String toString() {
	return cipherSuites.toString();
    }
    
    /** 
     * Write this list to an HandshakeOutStream in V3 ClientHello format.
     */
    void send(HandshakeOutStream s) throws IOException {
	byte[] suiteBytes = new byte[cipherSuites.size() * 2];
	int i = 0;
	for (Iterator t = cipherSuites.iterator(); t.hasNext(); i += 2) {
	    CipherSuite c = (CipherSuite)t.next();
	    suiteBytes[i] = (byte)(c.id >> 8);
	    suiteBytes[i+1] = (byte)c.id;
	}
	s.putBytes16(suiteBytes);
    }

    /**
     * Clear cache of available ciphersuites. If we support all ciphers
     * internally, there is no need to clear the cache and calling this
     * method has no effect.
     */
    static synchronized void clearAvailableCache() {
	if (CipherSuite.DYNAMIC_AVAILABILITY) {
	    supportedSuites = null;
	    defaultSuites = null;
	    CipherSuite.BulkCipher.clearAvailableCache();
	}
    }

    /**
     * Return the list of all available CipherSuites with a priority of
     * minPriority or above.
     * Should be called with the Class lock held.
     */
    private static CipherSuiteList buildAvailableCache(int minPriority) {
	// SortedSet automatically arranges ciphersuites in default
	// preference order
	Set cipherSuites = new TreeSet();
	Collection allowedCipherSuites = CipherSuite.allowedCipherSuites();
	for (Iterator t = allowedCipherSuites.iterator(); t.hasNext(); ) {
	    CipherSuite c = (CipherSuite)t.next();
	    if ((c.allowed == false) || (c.priority < minPriority)) {
		continue;
	    }
	    if (c.isAvailable()) {
		cipherSuites.add(c);
	    }
	}
	return new CipherSuiteList(cipherSuites);
    }

    /**
     * Return supported CipherSuites in preference order.
     */
    static synchronized CipherSuiteList getSupported() {
	if (supportedSuites == null) {
	    supportedSuites = 
	    	buildAvailableCache(CipherSuite.SUPPORTED_SUITES_PRIORITY);
	}
	return supportedSuites;
    }

    /**
     * Return default enabled CipherSuites in preference order.
     */
    static synchronized CipherSuiteList getDefault() {
	if (defaultSuites == null) {
	    defaultSuites = 
	    	buildAvailableCache(CipherSuite.DEFAULT_SUITES_PRIORITY);
	}
	return defaultSuites;
    }
    
}
