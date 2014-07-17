/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.interfaces;

import javax.crypto.spec.DHParameterSpec;

/**
 * The interface to a Diffie-Hellman key.
 *
 * @author Jan Luehe
 *
 * @version 1.7, 06/24/03
 *
 * @see javax.crypto.spec.DHParameterSpec
 * @see DHPublicKey
 * @see DHPrivateKey
 * @since 1.4
 */
public interface DHKey {

    /**
     * Returns the key parameters.
     *
     * @return the key parameters
     */
    DHParameterSpec getParams();
}
