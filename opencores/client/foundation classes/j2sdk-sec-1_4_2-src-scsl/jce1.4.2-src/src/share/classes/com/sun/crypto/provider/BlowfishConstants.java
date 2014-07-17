/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */
 
package com.sun.crypto.provider;

/**
 * This class defines the constants used by the Blowfish algorithm
 * implementation.
 * 
 * @author Jan Luehe
 * @version 1.6, 06/24/03
 *
 * @see BlowfishCipher
 * @see BlowfishCrypt
 */

interface BlowfishConstants {
    int BLOWFISH_BLOCK_SIZE = 8; // number of bytes
    int BLOWFISH_MAX_KEYSIZE = 56; // number of bytes
}
    
