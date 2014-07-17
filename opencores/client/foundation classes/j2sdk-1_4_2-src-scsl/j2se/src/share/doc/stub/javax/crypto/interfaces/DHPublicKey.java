/*
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)DHPublicKey.java	1.3 03/01/23
 */
  
/*
 * NOTE:
 * Because of various external restrictions (i.e. US export
 * regulations, etc.), the actual source code can not be provided
 * at this time. This file represents the skeleton of the source
 * file, so that javadocs of the API can be created.
 */

package javax.crypto.interfaces;

import java.math.BigInteger;

/** 
 * The interface to a Diffie-Hellman public key.
 *
 * @author Jan Luehe
 *
 * @version 1.3, 01/23/03
 *
 * @see DHKey
 * @see DHPrivateKey
 * @since 1.4
 */
public interface DHPublicKey extends DHKey, java.security.PublicKey
{

    /** 
     * Returns the public value, <code>y</code>.
     *
     * @return the public value, <code>y</code>
     */
    public BigInteger getY();
}
