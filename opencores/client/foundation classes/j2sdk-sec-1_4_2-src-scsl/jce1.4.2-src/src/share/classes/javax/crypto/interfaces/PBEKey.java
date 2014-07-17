/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto.interfaces;

import java.math.BigInteger;

/**
 * The interface to a PBE key.
 *
 * @author Valerie Peng 
 *
 * @version 1.5, 06/24/03
 *
 * @see javax.crypto.spec.PBEKeySpec
 * @see javax.crypto.SecretKey
 * @since 1.4
 */
public interface PBEKey extends javax.crypto.SecretKey {
    
    /**
     * Returns the password.
     *
     * <p> Note: this method should return a copy of the password. It is
     * the caller's responsibility to zero out the password information after
     * it is no longer needed.
     *
     * @return the password.
     */
    char[] getPassword();

    /**
     * Returns the salt or null if not specified.
     *
     * <p> Note: this method should return a copy of the salt. It is
     * the caller's responsibility to zero out the salt information after
     * it is no longer needed.
     *
     * @return the salt.
     */        
    byte[] getSalt();

    /**
     * Returns the iteration count or 0 if not specified.
     *
     * @return the iteration count.
     */        
    int getIterationCount();
}
