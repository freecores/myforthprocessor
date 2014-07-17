/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


package com.sun.net.ssl.internal.ssl;

import com.sun.net.ssl.internal.ssl.CipherSuite.*;

/**
 * Interface defining standard key exchange, bulk cipher, and mac algorithms.
 * Can be implemented by a class to avoid typing CipherSuiteConstants.xxx.
 * Once import of static constants is available in Java, these
 * definitions should be moved to the CipherSuite class.
 *
 * @version 1.3, 06/24/03
 */
interface CipherSuiteConstants {

    // key exchange algorithms
    final static KeyExchange K_NULL       = new KeyExchange("NULL",       false);
    final static KeyExchange K_RSA        = new KeyExchange("RSA",        true);
    final static KeyExchange K_RSA_EXPORT = new KeyExchange("RSA_EXPORT", true);
    final static KeyExchange K_DH_RSA     = new KeyExchange("DH_RSA",     false);
    final static KeyExchange K_DH_DSS     = new KeyExchange("DH_DSS",     false);
    final static KeyExchange K_DHE_DSS    = new KeyExchange("DHE_DSS",    true);
    final static KeyExchange K_DHE_RSA    = new KeyExchange("DHE_RSA",    true);
    final static KeyExchange K_DH_ANON    = new KeyExchange("DH_anon",    true);

    // export strength ciphers
    final static BulkCipher B_NULL    = new BulkCipher("NULL",   0,  0, 0, true);
    final static BulkCipher B_RC4_40  = new BulkCipher("RC4/40", 5, 16, 0, true);
    final static BulkCipher B_RC2_40  = new BulkCipher("RC2/40", 5, 16, 8, false);
    final static BulkCipher B_DES_40  = new BulkCipher("DES/40", 5,  8, 8, true);
    
    // domestic strength ciphers
    final static BulkCipher B_RC4_128 = new BulkCipher("RC4/128", 16,  0, true);
    final static BulkCipher B_DES     = new BulkCipher("DES",      8,  8, true);
    final static BulkCipher B_3DES    = new BulkCipher("3DES",    24,  8, true);
    final static BulkCipher B_IDEA    = new BulkCipher("IDEA",    16,  8, false);
    final static BulkCipher B_AES_128 = new BulkCipher("AES/128", 16, 16, true);
    final static BulkCipher B_AES_256 = new BulkCipher("AES/256", 32, 16, true);

    // MACs
    final static MacAlg M_NULL = new MacAlg("NULL", 0);
    final static MacAlg M_MD5  = new MacAlg("MD5", 16);
    final static MacAlg M_SHA  = new MacAlg("SHA", 20);

    // dummy ciphersuite SSL_NULL_WITH_NULL_NULL
    final static CipherSuite C_NULL = CipherSuite.valueOf(0, 0);
    
}
