/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.interfaces;

import java.math.BigInteger;

/**
 * The interface to a Diffie-Hellman private key.
 *
 * @author Jan Luehe
 *
 * @version 1.10, 06/24/03
 *
 * @see DHKey
 * @see DHPublicKey
 * @since 1.4
 */
public interface DHPrivateKey extends DHKey, java.security.PrivateKey {
    
    /**
     * Returns the private value, <code>x</code>.
     *
     * @return the private value, <code>x</code>
     */
    BigInteger getX();
}
