/*
 * Copyright 2003 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

package javax.crypto;

import java.security.GeneralSecurityException;

/**
 * This is the generic ExemptionMechanism exception.
 *
 * @version 1.4, 06/24/03
 * @since 1.4
 */

public class ExemptionMechanismException extends GeneralSecurityException {

    /**
     * Constructs a ExemptionMechanismException with no detailed message.
     * (A detailed message is a String that describes this particular
     * exception.)
     */
    public ExemptionMechanismException() {
	super();
    }

    /**
     * Constructs a ExemptionMechanismException with the specified
     * detailed message. (A detailed message is a String that describes
     * this particular exception.)
     *
     * @param msg the detailed message.
     */
   public ExemptionMechanismException(String msg) {
       super(msg);
    }
}
