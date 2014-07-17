/*
 * @(#)ShapingException.java	1.6 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.font;


/**
 * This exception is thrown if the underlying unicode character engine
 * determines that unicode combined forms or shaping may be required
 */
public class ShapingException extends java.lang.RuntimeException {
    public ShapingException(String s) {
        super (s);
    }
}
