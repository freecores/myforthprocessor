/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import java.util.*;

import java.security.NoSuchAlgorithmException;

import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * An SSL/TLS CipherSuite. Constants for the standard key exchange, cipher,
 * and mac algorithms are defined in CipherSuiteConstants.
 *
 * The CipherSuite class and the inner classes defined in this file roughly
 * follow the type safe enum pattern described in Effective Java. This means:
 *
 *  . instances are immutable, classes are final
 *
 *  . there is a unique instance of every value, i.e. there are never two 
 *    instances representing the same CipherSuite, etc. This means equality 
 *    tests can be performed using == instead of equals() (although that works 
 *    as well). [A minor exception are *unsupported* CipherSuites read from a 
 *    handshake message, but this is irrelevant]
 *
 *  . instances are obtained using the static valueOf() factory methods or
 *    by referencing the objects defined in CipherSuiteConstants.
 *
 *  . properties are defined as final variables and made available as
 *    package private variables without method accessors
 *
 *  . if the member variable allowed is false, the given algorithm is either
 *    unavailable or disabled at compile time
 *
 * @version 1.4, 06/24/03
 */
final class CipherSuite implements Comparable, CipherSuiteConstants {
    
    // minimum priority for supported CipherSuites
    final static int SUPPORTED_SUITES_PRIORITY = 1;
    
    // minimum priority for default enabled CipherSuites
    final static int DEFAULT_SUITES_PRIORITY = 300;

    // flag indicating if CipherSuite availability can change dynamically.
    // this is the case when we rely on a JCE cipher implementation that
    // may not be available in the installed JCE providers. we have internal
    // crypto and assume that the SunJCE provider is always installed, so
    // this flag is false.
    final static boolean DYNAMIC_AVAILABILITY = false;

    // Map Integer(id) -> CipherSuite
    // contains all known CipherSuites
    private final static Map idMap;
    
    // Map String(name) -> CipherSuite
    // contains only supported CipherSuites (i.e. allowed == true)
    private final static Map nameMap;

    // Protocol defined CipherSuite name, e.g. SSL_RSA_WITH_RC4_128_MD5
    // we use TLS_* only for new CipherSuites, still SSL_* for old ones
    final String name;
    
    // id in 16 bit MSB format, i.e. 0x0004 for SSL_RSA_WITH_RC4_128_MD5
    final int id;
    
    // priority for the internal default preference order. the higher the 
    // better. Each supported CipherSuite *must* have a unique priority.
    // Ciphersuites with priority >= DEFAULT_SUITES_PRIORITY are enabled
    // by default
    final int priority;
    
    // key exchange, bulk cipher, and mac algorithms. See those classes below.
    final KeyExchange keyExchange;
    final BulkCipher cipher;
    final MacAlg macAlg;
    
    // whether a CipherSuite qualifies as exportable under 512/40 bit rules.
    final boolean exportable;
    
    // true iff implemented and enabled at compile time
    final boolean allowed;

    private CipherSuite(String name, int id, int priority, 
	    KeyExchange keyExchange, BulkCipher cipher, boolean allowed) {
	this.name = name;
	this.id = id;
	this.priority = priority;
	this.keyExchange = keyExchange;
	this.cipher = cipher;
	this.exportable = cipher.exportable;
	if (name.endsWith("_MD5")) {
	    macAlg = M_MD5;
	} else if (name.endsWith("_SHA")) {
	    macAlg = M_SHA;
	} else if (name.endsWith("_NULL")) {
	    macAlg = M_NULL;
	} else {
	    throw new IllegalArgumentException
		    ("Unknown MAC algorithm for ciphersuite " + name);
	}

	allowed &= keyExchange.allowed;
	allowed &= cipher.allowed;
	if ((ExportControl.hasStrongCrypto == false) && (exportable == false)) {
	    allowed = false;
	}
	this.allowed = allowed;
    }

    private CipherSuite(String name, int id) {
	this.name = name;
	this.id = id;
	this.allowed = false;

	this.priority = 0;
	this.keyExchange = null;
	this.cipher = null;
	this.macAlg = null;
	this.exportable = false;
    }

    /**
     * Return whether this CipherSuite is available for use. A
     * CipherSuite may be unavailable even if it is supported
     * (i.e. allowed == true) if the required JCE cipher is not installed.
     * In some configuration, this situation may change over time, call 
     * CipherSuiteList.clearAvailableCache() before this method to obtain
     * the most current status.
     */
    boolean isAvailable() {
	return allowed && cipher.isAvailable();
    }

    /**
     * Compares CipherSuites based on their priority. Has the effect of
     * sorting CipherSuites when put in a sorted collection, which is
     * used by CipherSuiteList. Follows standard Comparable contract.
     *
     * Note that for unsupported CipherSuites parsed from a handshake
     * message we violate the equals() contract.
     */
    public int compareTo(Object o) {
	return ((CipherSuite)o).priority - priority;
    }

    /**
     * Returns this.name.
     */
    public String toString() {
	return name;
    }
    
    /**
     * Return a CipherSuite for the given name. The returned CipherSuite
     * is supported by this implementation but may not actually be
     * currently useable. See isAvailable().
     *
     * @exception IllegalArgumentException if the CipherSuite is unknown or
     * unsupported.
     */
    static CipherSuite valueOf(String s) {
	if (s == null) {
	    throw new IllegalArgumentException("Name must not be null");
	}
	CipherSuite c = (CipherSuite)nameMap.get(s);
	if ((c == null) || (c.allowed == false)) {
	    throw new IllegalArgumentException("Unsupported ciphersuite " + s);
	}
	return c;
    }

    /**
     * Return a CipherSuite with the given ID. A temporary object is
     * constructed if the ID is unknown. Use isAvailable() to verify that
     * the CipherSuite can actually be used.
     */
    static CipherSuite valueOf(int id1, int id2) {
	id1 &= 0xff;
	id2 &= 0xff;
	int id = (id1 << 8) | id2;
	CipherSuite c = (CipherSuite)idMap.get(new Integer(id));
	if (c == null) {
	    String h1 = Integer.toString(id1, 16);
	    String h2 = Integer.toString(id2, 16);
	    c = new CipherSuite("Unknown 0x" + h1 + ":0x" + h2, id);
	}
	return c;
    }
    
    // for use by CipherSuiteList only
    static Collection allowedCipherSuites() {
	return nameMap.values();
    }

    private static void add(String name, int id, int priority, 
	    KeyExchange keyExchange, BulkCipher cipher, boolean allowed) {
	CipherSuite c = new CipherSuite(name, id, priority, keyExchange, 
					cipher, allowed);
	if (idMap.put(new Integer(id), c) != null) {
	    throw new RuntimeException("Duplicate ciphersuite definition: "
	    				+ id + ", " + name);
	}
	if (c.allowed) {
	    if (nameMap.put(name, c) != null) {
		throw new RuntimeException("Duplicate ciphersuite definition: "
					    + id + ", " + name);
	    }
	}
    }
    
    private static void add(String name, int id) {
	CipherSuite c = new CipherSuite(name, id);
	if (idMap.put(new Integer(id), c) != null) {
	    throw new RuntimeException("Duplicate ciphersuite definition: "
	    				+ id + ", " + name);
	}
    }

    static {
	idMap = new HashMap();
	nameMap = new HashMap();
	
	// NOTE: all CipherSuites with (allowed == true) must have unique 
	// priorities, otherwise strange things may happen.
	
	// CipherSuites w/ priority >= CipherSuiteList.DEFAULT_SUITES_PRIORITY
	// are enabled by default.
	
	add("SSL_NULL_WITH_NULL_NULL",		     0x0000,   1,
					    K_NULL,       B_NULL,    false);
	
	add("SSL_RSA_WITH_RC4_128_MD5",              0x0004, 950, 
					    K_RSA,        B_RC4_128, true);
	add("SSL_RSA_WITH_RC4_128_SHA",              0x0005, 940, 
					    K_RSA,        B_RC4_128, true);
	add("TLS_RSA_WITH_AES_128_CBC_SHA",          0x002f, 830, 
					    K_RSA,        B_AES_128, true);
	add("TLS_DHE_RSA_WITH_AES_128_CBC_SHA",      0x0033, 825, 
					    K_DHE_RSA,    B_AES_128, true);
	add("TLS_DHE_DSS_WITH_AES_128_CBC_SHA",      0x0032, 820, 
					    K_DHE_DSS,    B_AES_128, true);
	add("SSL_RSA_WITH_3DES_EDE_CBC_SHA",         0x000a, 650, 
					    K_RSA,        B_3DES,    true);
	add("SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA",     0x0016, 640, 
					    K_DHE_RSA,    B_3DES,    true);
	add("SSL_DHE_DSS_WITH_3DES_EDE_CBC_SHA",     0x0013, 630, 
					    K_DHE_DSS,    B_3DES,    true);
	add("SSL_RSA_WITH_DES_CBC_SHA",              0x0009, 550, 
					    K_RSA,        B_DES,     true);
	add("SSL_DHE_RSA_WITH_DES_CBC_SHA",          0x0015, 540, 
					    K_DHE_RSA,    B_DES,     true);
	add("SSL_DHE_DSS_WITH_DES_CBC_SHA",          0x0012, 530, 
					    K_DHE_DSS,    B_DES,     true);
	add("SSL_RSA_EXPORT_WITH_RC4_40_MD5",        0x0003, 350,
					    K_RSA_EXPORT, B_RC4_40,  true);
	add("SSL_RSA_EXPORT_WITH_DES40_CBC_SHA",     0x0008, 340,
					    K_RSA_EXPORT, B_DES_40,  true);
	add("SSL_DHE_RSA_EXPORT_WITH_DES40_CBC_SHA", 0x0014, 335,
					    K_DHE_RSA,    B_DES_40,  true);
	add("SSL_DHE_DSS_EXPORT_WITH_DES40_CBC_SHA", 0x0011, 330,
					    K_DHE_DSS,    B_DES_40,  true);
	add("TLS_RSA_WITH_AES_256_CBC_SHA",          0x0035, 290, 
					    K_RSA,        B_AES_256, true);
	add("TLS_DHE_RSA_WITH_AES_256_CBC_SHA",      0x0039, 285, 
					    K_DHE_RSA,    B_AES_256, true);
	add("TLS_DHE_DSS_WITH_AES_256_CBC_SHA",      0x0038, 280, 
					    K_DHE_DSS,    B_AES_256, true);
	add("SSL_RSA_WITH_NULL_MD5",                 0x0001, 250,
					    K_RSA,        B_NULL,    true);
	add("SSL_RSA_WITH_NULL_SHA",                 0x0002, 240,
					    K_RSA,        B_NULL,    true);
	add("SSL_DH_anon_WITH_RC4_128_MD5",          0x0018, 190,
					    K_DH_ANON,    B_RC4_128, true);
	add("TLS_DH_anon_WITH_AES_128_CBC_SHA",      0x0034, 185,
					    K_DH_ANON,    B_AES_128, true);
	add("TLS_DH_anon_WITH_AES_256_CBC_SHA",      0x003a, 182,
					    K_DH_ANON,    B_AES_256, true);
	add("SSL_DH_anon_WITH_3DES_EDE_CBC_SHA",     0x001b, 180,
					    K_DH_ANON,    B_3DES,    true);
	add("SSL_DH_anon_WITH_DES_CBC_SHA",          0x001a, 150,
					    K_DH_ANON,    B_DES,     true);
	add("SSL_DH_anon_EXPORT_WITH_RC4_40_MD5",    0x0017, 130,
					    K_DH_ANON,    B_RC4_40,  true);
	add("SSL_DH_anon_EXPORT_WITH_DES40_CBC_SHA", 0x0019, 120,
					    K_DH_ANON,    B_DES_40,  true);
					    
	// Register the names of a few additional CipherSuites.
	// Makes them show up as names instead of numbers in
	// the debug output.

	// remaining unsupported ciphersuites defined in RFC2246.
	add("SSL_RSA_EXPORT_WITH_RC2_CBC_40_MD5",      0x0006);
	add("SSL_RSA_WITH_IDEA_CBC_SHA",               0x0007);
	add("SSL_DH_DSS_EXPORT_WITH_DES40_CBC_SHA",    0x000b);
	add("SSL_DH_DSS_WITH_DES_CBC_SHA",             0x000c);
	add("SSL_DH_DSS_WITH_3DES_EDE_CBC_SHA",        0x000d);
	add("SSL_DH_RSA_EXPORT_WITH_DES40_CBC_SHA",    0x000e);
	add("SSL_DH_RSA_WITH_DES_CBC_SHA",             0x000f);
	add("SSL_DH_RSA_WITH_3DES_EDE_CBC_SHA",        0x0010);
	
	// SSL 3.0 Fortezza ciphersuites
	add("SSL_FORTEZZA_DMS_WITH_NULL_SHA",          0x001c);
	add("SSL_FORTEZZA_DMS_WITH_FORTEZZA_CBC_SHA",  0x001d);
	
	// 1024/56 bit exportable ciphersuites from expired internet draft
	add("SSL_RSA_EXPORT1024_WITH_DES_CBC_SHA",     0x0062);
	add("SSL_DHE_DSS_EXPORT1024_WITH_DES_CBC_SHA", 0x0063);
	add("SSL_RSA_EXPORT1024_WITH_RC4_56_SHA",      0x0064);
	add("SSL_DHE_DSS_EXPORT1024_WITH_RC4_56_SHA",  0x0065);
	add("SSL_DHE_DSS_WITH_RC4_128_SHA",            0x0066);
	
	// Kerberos ciphersuites from RFC2712
	add("TLS_KRB5_WITH_DES_CBC_SHA",               0x001e);
	add("TLS_KRB5_WITH_3DES_EDE_CBC_SHA",          0x001f);
	add("TLS_KRB5_WITH_RC4_128_SHA",               0x0020);
	add("TLS_KRB5_WITH_IDEA_CBC_SHA",              0x0021);
	add("TLS_KRB5_WITH_DES_CBC_MD5",               0x0022);
	add("TLS_KRB5_WITH_3DES_EDE_CBC_MD5",          0x0023);
	add("TLS_KRB5_WITH_RC4_128_MD5",               0x0024);
	add("TLS_KRB5_WITH_IDEA_CBC_MD5",              0x0025);
	add("TLS_KRB5_EXPORT_WITH_DES_CBC_40_SHA",     0x0026);
	add("TLS_KRB5_EXPORT_WITH_RC2_CBC_40_SHA",     0x0027);
	add("TLS_KRB5_EXPORT_WITH_RC4_40_SHA",         0x0028);
	add("TLS_KRB5_EXPORT_WITH_DES_CBC_40_MD5",     0x0029);
	add("TLS_KRB5_EXPORT_WITH_RC2_CBC_40_MD5",     0x002a);
	add("TLS_KRB5_EXPORT_WITH_RC4_40_MD5",         0x002b);
	
	// Netscape old and new SSL 3.0 FIPS ciphersuites
	// see http://www.mozilla.org/projects/security/pki/nss/ssl/fips-ssl-ciphersuites.html
	add("NETSCAPE_RSA_FIPS_WITH_3DES_EDE_CBC_SHA", 0xffe0);
	add("NETSCAPE_RSA_FIPS_WITH_DES_CBC_SHA",      0xffe1);
	add("SSL_RSA_FIPS_WITH_DES_CBC_SHA",           0xfefe);
	add("SSL_RSA_FIPS_WITH_3DES_EDE_CBC_SHA",      0xfeff);
    }

    /**
     * An SSL/TLS key exchange algorithm. 
     *     
     * The standard algorithms are defined in CipherSuiteConstants.
     */
    final static class KeyExchange {

	// name of the key exchange algorithm, e.g. DHE_DSS
	final String name;
	final boolean allowed;

	KeyExchange(String name, boolean allowed) {
	    this.name = name;
	    this.allowed = allowed;
	}

	public String toString() {
	    return name;
	}
    }

    /**
     * An SSL/TLS bulk cipher algorithm. One instance per combination of
     * cipher and key length.
     *
     * Also contains a factory method to obtain in initialized CipherBox
     * for this algorithm.
     *     
     * The standard algorithms are defined in CipherSuiteConstants.
     */
    final static class BulkCipher {

	// Map BulkCipher -> Boolean(available)
	private final static Map availableCache = new HashMap(8);
	
	// descriptive name, including key size for ciphers with variable
	// size keys. e.g. RC4/128
	final String name;
	
	// supported and compile time enabled. Also see isAvailable()
	final boolean allowed;

	// number of bytes of entropy in the key
	final int keySize;	

	// length of the actual cipher key in bytes.
	// for non-exportable ciphers, this is the same as keySize
	final int expandedKeySize;
	
	// size of the IV (also block size)
	final int ivSize;
	
	// exportable under 512/40 bit rules
	final boolean exportable;

	BulkCipher(String name, int keySize, int expandedKeySize, int ivSize, 
		boolean allowed) {
	    this.name = name;
	    this.keySize = keySize;
	    this.ivSize = ivSize;
	    this.allowed = allowed;

	    this.expandedKeySize = expandedKeySize;
	    this.exportable = true;
	}

	BulkCipher(String name, int keySize, int ivSize, boolean allowed) {
	    this.name = name;
	    this.keySize = keySize;
	    this.ivSize = ivSize;
	    this.allowed = allowed;

	    this.expandedKeySize = keySize;
	    this.exportable = false;
	}
	
	/**
	 * Return an initialized CipherBox for this BulkCipher.
	 * IV may be null for stream ciphers.
	 *
	 * @exception NoSuchAlgorithmException if anything goes wrong
	 */
	CipherBox newCipher(ProtocolVersion version, byte[] key, byte[] iv,
		boolean encrypt) throws NoSuchAlgorithmException {
	    return CipherBox.newCipherBox(version, this, key, iv, encrypt);
	}
	
	/**
	 * Test if this bulk cipher is available. For use by CipherSuite.
	 * 
	 * Currently all supported ciphers except AES are always available 
	 * via the JSSE internal implementations. We also assume AES/128
	 * is always available since it is shipped with the SunJCE provider.
	 * However, AES/256 is unavailable when the default JCE policy
	 * jurisdiction files are installed because of key length restrictions.
	 */
	boolean isAvailable() {
	    if (allowed == false) {
		return false;
	    }
	    if (this == B_AES_256) {
		return isAvailable(this);
	    }
	    // always available
	    return true;
	}

	// for use by CipherSuiteList.clearAvailableCache();
	static synchronized void clearAvailableCache() {
	    if (DYNAMIC_AVAILABILITY) {
	        availableCache.clear();
	    }
	}
	
	private static synchronized boolean isAvailable(BulkCipher cipher) {
	    Boolean b = (Boolean)availableCache.get(cipher);
	    if (b == null) {
		try {
		    byte[] key = new byte[cipher.expandedKeySize];
		    byte[] iv = new byte[cipher.ivSize];
		    cipher.newCipher(ProtocolVersion.DEFAULT, key, iv, true);
		    b = Boolean.TRUE;
		} catch (NoSuchAlgorithmException e) {
		    b = Boolean.FALSE;
		}
		availableCache.put(cipher, b);
	    }
	    return b.booleanValue();
	}

	public String toString() {
	    return name;
	}
    }

    /**
     * An SSL/TLS key MAC algorithm. 
     *     
     * Also contains a factory method to obtain in initialized MAC
     * for this algorithm.
     *     
     * The standard algorithms are defined in CipherSuiteConstants.
     */
    final static class MacAlg {

	// descriptive name, e.g. MD5
	final String name;
	
	// size of the MAC value (and MAC key) in bytes
	final int size;

	MacAlg(String name, int size) {
	    this.name = name;
	    this.size = size;
	}
	
	/**
	 * Return an initialized MAC for this MacAlg. ProtocolVersion
	 * must either be SSL30 (SSLv3 custom MAC) or TLS10 (std. HMAC).
	 *
	 * @exception NoSuchAlgorithmException if anything goes wrong
	 */
	MAC newMac(ProtocolVersion protocolVersion, byte[] secret) 
		throws NoSuchAlgorithmException {
	    return new MAC(this, protocolVersion, secret);
	}

	public String toString() {
	    return name;
	}
    }

}
