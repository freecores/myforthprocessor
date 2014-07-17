/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.interfaces;

import java.math.BigInteger;

/**
 * The interface to a Diffie-Hellman public key.
 *
 * @author Jan Luehe
 *
 * @version 1.12, 06/24/03
 *
 * @see DHKey
 * @see DHPrivateKey
 * @since 1.4
 */
public interface DHPublicKey extends DHKey, java.security.PublicKey {
    
    /**
     * Returns the public value, <code>y</code>.
     *
     * @return the public value, <code>y</code>
     */
    BigInteger getY();
}
